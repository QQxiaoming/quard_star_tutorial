// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/vr/openvr/openvr_render_loop.h"

#include "base/metrics/histogram_functions.h"
#include "device/vr/openvr/openvr_api_wrapper.h"
#include "device/vr/openvr/openvr_gamepad_data_fetcher.h"
#include "device/vr/openvr/openvr_type_converters.h"
#include "ui/gfx/geometry/angle_conversions.h"
#include "ui/gfx/transform.h"

#if defined(OS_WIN)
#include "device/vr/windows/d3d11_texture_helper.h"
#endif

namespace device {

namespace {

// OpenVR reports the controllers pose of the controller's tip, while WebXR
// needs to report the pose of the controller's grip (centered on the user's
// palm.) This experimentally determined value is how far back along the Z axis
// in meters OpenVR's pose needs to be translated to align with WebXR's
// coordinate system.
const float kGripOffsetZMeters = 0.08f;

// WebXR reports a pointer pose separate from the grip pose, which represents a
// pointer ray emerging from the tip of the controller. OpenVR does not report
// anything like that, and most pointers are assumed to come straight from the
// controller's tip. For consistency with other WebXR backends we'll synthesize
// a pointer ray that's angled down slightly from the controller's handle,
// defined by this angle. Experimentally determined, should roughly point in the
// same direction as a user's outstretched index finger while holding a
// controller.
const float kPointerErgoAngleDegrees = -40.0f;

gfx::Transform HmdMatrix34ToTransform(const vr::HmdMatrix34_t& mat) {
  return gfx::Transform(mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
                        mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
                        mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3], 0,
                        0, 0, 1);
}

}  // namespace

OpenVRRenderLoop::OpenVRRenderLoop(
    base::RepeatingCallback<void(OpenVRGamepadState)> update_gamepad)
    : base::Thread("OpenVRRenderLoop"),
      main_thread_task_runner_(base::ThreadTaskRunnerHandle::Get()),
      update_gamepad_(std::move(update_gamepad)),
      binding_(this),
      weak_ptr_factory_(this) {
  DCHECK(main_thread_task_runner_);
}

OpenVRRenderLoop::~OpenVRRenderLoop() {
  Stop();
}

void OpenVRRenderLoop::ClearPendingFrame() {
  has_outstanding_frame_ = false;
  if (delayed_get_frame_data_callback_) {
    base::ResetAndReturn(&delayed_get_frame_data_callback_).Run();
  }
}

void OpenVRRenderLoop::SubmitFrameMissing(int16_t frame_index,
                                          const gpu::SyncToken& sync_token) {
  // Nothing to do. It's OK to start the next frame even if the current
  // one didn't get sent to OpenVR.
  ClearPendingFrame();
}

void OpenVRRenderLoop::SubmitFrame(int16_t frame_index,
                                   const gpu::MailboxHolder& mailbox,
                                   base::TimeDelta time_waited) {
  NOTREACHED();
}

void OpenVRRenderLoop::SubmitFrameDrawnIntoTexture(
    int16_t frame_index,
    const gpu::SyncToken& sync_token,
    base::TimeDelta time_waited) {
  // Not currently implemented for Windows.
  NOTREACHED();
}

void OpenVRRenderLoop::SubmitFrameWithTextureHandle(
    int16_t frame_index,
    mojo::ScopedHandle texture_handle) {
  DCHECK(openvr_);
  vr::IVRCompositor* vr_compositor = openvr_->GetCompositor();
  DCHECK(vr_compositor);

  TRACE_EVENT1("gpu", "SubmitFrameWithTextureHandle", "frameIndex",
               frame_index);
#if defined(OS_WIN)
  MojoPlatformHandle platform_handle;
  platform_handle.struct_size = sizeof(platform_handle);
  MojoResult result = MojoUnwrapPlatformHandle(texture_handle.release().value(),
                                               nullptr, &platform_handle);
  if (result != MOJO_RESULT_OK) {
    ClearPendingFrame();
    return;
  }

  texture_helper_.SetSourceTexture(
      base::win::ScopedHandle(reinterpret_cast<HANDLE>(platform_handle.value)));
  texture_helper_.AllocateBackBuffer();
  bool copy_successful = texture_helper_.CopyTextureToBackBuffer(true);
  if (copy_successful) {
    vr::Texture_t texture;
    texture.handle = texture_helper_.GetBackbuffer().Get();
    texture.eType = vr::TextureType_DirectX;
    texture.eColorSpace = vr::ColorSpace_Auto;

    vr::VRTextureBounds_t bounds[2];
    bounds[0] = {left_bounds_.x(), left_bounds_.y(),
                 left_bounds_.width() + left_bounds_.x(),
                 left_bounds_.height() + left_bounds_.y()};
    bounds[1] = {right_bounds_.x(), right_bounds_.y(),
                 right_bounds_.width() + right_bounds_.x(),
                 right_bounds_.height() + right_bounds_.y()};

    vr::EVRCompositorError error =
        vr_compositor->Submit(vr::EVREye::Eye_Left, &texture, &bounds[0]);
    if (error != vr::VRCompositorError_None) {
      ExitPresent();
      return;
    }
    error = vr_compositor->Submit(vr::EVREye::Eye_Right, &texture, &bounds[1]);
    if (error != vr::VRCompositorError_None) {
      ExitPresent();
      return;
    }
    vr_compositor->PostPresentHandoff();
  }

  // Tell WebVR that we are done with the texture.
  submit_client_->OnSubmitFrameTransferred(copy_successful);
  submit_client_->OnSubmitFrameRendered();
#endif

  ClearPendingFrame();
}

void OpenVRRenderLoop::CleanUp() {
  submit_client_ = nullptr;
  binding_.Close();
}

void OpenVRRenderLoop::UpdateLayerBounds(int16_t frame_id,
                                         const gfx::RectF& left_bounds,
                                         const gfx::RectF& right_bounds,
                                         const gfx::Size& source_size) {
  // Bounds are updated instantly, rather than waiting for frame_id.  This works
  // since blink always passes the current frame_id when updating the bounds.
  // Ignoring the frame_id keeps the logic simpler, so this can more easily
  // merge with vr_shell_gl eventually.
  left_bounds_ = left_bounds;
  right_bounds_ = right_bounds;
  source_size_ = source_size;
};

void OpenVRRenderLoop::RequestSession(
    base::OnceCallback<void()> on_presentation_ended,
    mojom::XRDeviceRuntimeSessionOptionsPtr options,
    RequestSessionCallback callback) {
  DCHECK(options->immersive);
  binding_.Close();

  if (!openvr_) {
    openvr_ = std::make_unique<OpenVRWrapper>(true);
    if (!openvr_->IsInitialized()) {
      openvr_ = nullptr;
      main_thread_task_runner_->PostTask(
          FROM_HERE, base::BindOnce(std::move(callback), false, nullptr,
                                    nullptr, nullptr));
      return;
    }

    openvr_->GetCompositor()->SuspendRendering(true);
    openvr_->GetCompositor()->SetTrackingSpace(
        vr::ETrackingUniverseOrigin::TrackingUniverseSeated);
  }

#if defined(OS_WIN)
  int32_t adapter_index;
  openvr_->GetSystem()->GetDXGIOutputInfo(&adapter_index);
  if (!texture_helper_.SetAdapterIndex(adapter_index) ||
      !texture_helper_.EnsureInitialized()) {
    openvr_ = nullptr;
    main_thread_task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(std::move(callback), false, nullptr, nullptr, nullptr));
    return;
  }
#endif
  DCHECK(!on_presentation_ended_);
  on_presentation_ended_ = std::move(on_presentation_ended);
  device::mojom::VRPresentationProviderPtr provider;
  binding_.Bind(mojo::MakeRequest(&provider));

  device::mojom::VRDisplayFrameTransportOptionsPtr transport_options =
      device::mojom::VRDisplayFrameTransportOptions::New();
  transport_options->transport_method =
      device::mojom::VRDisplayFrameTransportMethod::SUBMIT_AS_TEXTURE_HANDLE;
  // Only set boolean options that we need. Default is false, and we should be
  // able to safely ignore ones that our implementation doesn't care about.
  transport_options->wait_for_transfer_notification = true;

  // Reset the active states for all the controllers.
  for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
    InputActiveState& input_active_state = input_active_states_[i];
    input_active_state.active = false;
    input_active_state.primary_input_pressed = false;
    input_active_state.device_class = vr::TrackedDeviceClass_Invalid;
    input_active_state.controller_role = vr::TrackedControllerRole_Invalid;
  }

  main_thread_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(std::move(callback), true,
                     mojo::MakeRequest(&submit_client_),
                     provider.PassInterface(), std::move(transport_options)));
  is_presenting_ = true;
  openvr_->GetCompositor()->SuspendRendering(false);

  // Measure the VrViewerType we are presenting with.
  using ViewerMap = std::map<std::string, VrViewerType>;
  CR_DEFINE_STATIC_LOCAL(ViewerMap, viewer_types,
                         ({
                             {"Oculus Rift CV1", VrViewerType::OPENVR_RIFT_CV1},
                             {"Vive MV", VrViewerType::OPENVR_VIVE},
                         }));
  VrViewerType type = VrViewerType::OPENVR_UNKNOWN;
  std::string model =
      GetOpenVRString(openvr_->GetSystem(), vr::Prop_ModelNumber_String);
  auto it = viewer_types.find(model);
  if (it != viewer_types.end())
    type = it->second;

  base::UmaHistogramSparse("VRViewerType", static_cast<int>(type));
}

void OpenVRRenderLoop::ExitPresent() {
  is_presenting_ = false;
  binding_.Close();
  submit_client_ = nullptr;
  if (openvr_)
    openvr_->GetCompositor()->SuspendRendering(true);
  openvr_ = nullptr;

  has_outstanding_frame_ = false;
  delayed_get_frame_data_callback_.Reset();

  // Push out one more controller update so we don't have stale controllers.
  UpdateControllerState();

  if (on_presentation_ended_) {
    main_thread_task_runner_->PostTask(FROM_HERE,
                                       std::move(on_presentation_ended_));
  }
}

void OpenVRRenderLoop::UpdateControllerState() {
  OpenVRGamepadState state = {};

  if (openvr_) {
    vr::IVRSystem* vr_system = openvr_->GetSystem();
    vr_system->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseSeated, 0.0f,
                                               state.tracked_devices_poses,
                                               vr::k_unMaxTrackedDeviceCount);

    for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
      state.device_class[i] = vr_system->GetTrackedDeviceClass(i);
      if (state.device_class[i] == vr::TrackedDeviceClass_Controller) {
        state.have_controller_state[i] = vr_system->GetControllerState(
            i, &(state.controller_state[i]), sizeof(state.controller_state[i]));
        if (state.have_controller_state[i]) {
          state.hand[i] = vr_system->GetControllerRoleForTrackedDeviceIndex(i);
          state.supported_buttons[i] =
              vr_system->GetUint64TrackedDeviceProperty(
                  i, vr::Prop_SupportedButtons_Uint64);

          for (int j = 0; j < vr::k_unControllerStateAxisCount; ++j) {
            state.axis_type[i][j] = vr_system->GetInt32TrackedDeviceProperty(
                i, static_cast<vr::TrackedDeviceProperty>(
                       vr::Prop_Axis0Type_Int32 + j));
          }
        }
      }
    }
  }

  main_thread_task_runner_->PostTask(FROM_HERE,
                                     base::BindOnce(update_gamepad_, state));
}

mojom::VRPosePtr OpenVRRenderLoop::GetPose() {
  vr::TrackedDevicePose_t rendering_poses[vr::k_unMaxTrackedDeviceCount];

  TRACE_EVENT0("gpu", "WaitGetPoses");
  openvr_->GetCompositor()->WaitGetPoses(
      rendering_poses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

  mojom::VRPosePtr pose = mojo::ConvertTo<mojom::VRPosePtr>(
      rendering_poses[vr::k_unTrackedDeviceIndex_Hmd]);

  // Update WebXR input sources.
  DCHECK(pose);
  pose->input_state =
      GetInputState(rendering_poses, vr::k_unMaxTrackedDeviceCount);

  return pose;
}

void OpenVRRenderLoop::Init() {
}

base::WeakPtr<OpenVRRenderLoop> OpenVRRenderLoop::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

void OpenVRRenderLoop::GetFrameData(
    mojom::VRPresentationProvider::GetFrameDataCallback callback) {
  DCHECK(is_presenting_);

  if (has_outstanding_frame_) {
    DCHECK(!delayed_get_frame_data_callback_);
    delayed_get_frame_data_callback_ =
        base::BindOnce(&OpenVRRenderLoop::GetFrameData, base::Unretained(this),
                       std::move(callback));
    return;
  }

  has_outstanding_frame_ = true;

  mojom::XRFrameDataPtr frame_data = mojom::XRFrameData::New();

  frame_data->frame_id = next_frame_id_;
  next_frame_id_ += 1;
  if (next_frame_id_ < 0) {
    next_frame_id_ = 0;
  }

  if (openvr_) {
    frame_data->pose = GetPose();
    vr::Compositor_FrameTiming timing;
    timing.m_nSize = sizeof(vr::Compositor_FrameTiming);
    bool valid_time = openvr_->GetCompositor()->GetFrameTiming(&timing);
    if (valid_time) {
      frame_data->time_delta =
          base::TimeDelta::FromSecondsD(timing.m_flSystemTimeInSeconds);
    }
  }

  // Update gamepad controllers.
  UpdateControllerState();

  std::move(callback).Run(std::move(frame_data));
}

std::vector<mojom::XRInputSourceStatePtr> OpenVRRenderLoop::GetInputState(
    vr::TrackedDevicePose_t* poses,
    uint32_t count) {
  std::vector<mojom::XRInputSourceStatePtr> input_states;

  if (!openvr_)
    return input_states;

  // Loop through every device pose and determine which are controllers
  for (uint32_t i = vr::k_unTrackedDeviceIndex_Hmd + 1; i < count; ++i) {
    const vr::TrackedDevicePose_t& pose = poses[i];
    InputActiveState& input_active_state = input_active_states_[i];

    if (!pose.bDeviceIsConnected) {
      // If this was an active controller on the last frame report it as
      // disconnected.
      if (input_active_state.active) {
        input_active_state.active = false;
        input_active_state.primary_input_pressed = false;
        input_active_state.device_class = vr::TrackedDeviceClass_Invalid;
        input_active_state.controller_role = vr::TrackedControllerRole_Invalid;
      }
      continue;
    }

    // Is this a newly connected controller?
    bool newly_active = false;
    if (!input_active_state.active) {
      input_active_state.active = true;
      input_active_state.device_class =
          openvr_->GetSystem()->GetTrackedDeviceClass(i);
      newly_active = true;
    }

    // Skip over any tracked devices that aren't controllers.
    if (input_active_state.device_class != vr::TrackedDeviceClass_Controller) {
      continue;
    }

    device::mojom::XRInputSourceStatePtr state =
        device::mojom::XRInputSourceState::New();

    vr::VRControllerState_t controller_state;
    openvr_->GetSystem()->GetControllerState(i, &controller_state,
                                             sizeof(vr::VRControllerState_t));
    bool pressed = controller_state.ulButtonPressed &
                   vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);

    state->source_id = i;
    state->primary_input_pressed = pressed;
    state->primary_input_clicked =
        (!pressed && input_active_state.primary_input_pressed);

    input_active_state.primary_input_pressed = pressed;

    if (pose.bPoseIsValid) {
      state->grip = HmdMatrix34ToTransform(pose.mDeviceToAbsoluteTracking);
      // Scoot the grip matrix back a bit so that it actually lines up with the
      // user's palm.
      state->grip->Translate3d(0, 0, kGripOffsetZMeters);
    }

    // Poll controller roll per-frame, since OpenVR controllers can swap hands.
    vr::ETrackedControllerRole controller_role =
        openvr_->GetSystem()->GetControllerRoleForTrackedDeviceIndex(i);

    // If this is a newly active controller or if the handedness has changed
    // since the last update, re-send the controller's description.
    if (newly_active || controller_role != input_active_state.controller_role) {
      device::mojom::XRInputSourceDescriptionPtr desc =
          device::mojom::XRInputSourceDescription::New();

      // It's a handheld pointing device.
      desc->target_ray_mode = device::mojom::XRTargetRayMode::POINTING;

      // Set handedness.
      switch (controller_role) {
        case vr::TrackedControllerRole_LeftHand:
          desc->handedness = device::mojom::XRHandedness::LEFT;
          break;
        case vr::TrackedControllerRole_RightHand:
          desc->handedness = device::mojom::XRHandedness::RIGHT;
          break;
        default:
          desc->handedness = device::mojom::XRHandedness::NONE;
          break;
      }
      input_active_state.controller_role = controller_role;

      // OpenVR controller are fully 6DoF.
      desc->emulated_position = false;

      // Tweak the pointer transform so that it's angled down from the
      // grip. This should be a bit more ergonomic.
      desc->pointer_offset = gfx::Transform();
      desc->pointer_offset->RotateAboutXAxis(kPointerErgoAngleDegrees);

      state->description = std::move(desc);
    }

    input_states.push_back(std::move(state));
  }

  return input_states;
}

}  // namespace device
