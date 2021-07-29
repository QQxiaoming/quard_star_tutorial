// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/webrtc/webrtc_set_description_observer.h"

#include "base/logging.h"

namespace content {

WebRtcSetDescriptionObserver::States::States()
    : signaling_state(
          webrtc::PeerConnectionInterface::SignalingState::kClosed) {}

WebRtcSetDescriptionObserver::States::States(States&& other)
    : signaling_state(other.signaling_state),
      transceiver_states(std::move(other.transceiver_states)) {}

WebRtcSetDescriptionObserver::States::~States() = default;

WebRtcSetDescriptionObserver::States& WebRtcSetDescriptionObserver::States::
operator=(States&& other) {
  signaling_state = other.signaling_state;
  transceiver_states = std::move(other.transceiver_states);
  return *this;
}

WebRtcSetDescriptionObserver::WebRtcSetDescriptionObserver() = default;

WebRtcSetDescriptionObserver::~WebRtcSetDescriptionObserver() = default;

WebRtcSetDescriptionObserverHandlerImpl::
    WebRtcSetDescriptionObserverHandlerImpl(
        scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
        scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
        scoped_refptr<webrtc::PeerConnectionInterface> pc,
        scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
        scoped_refptr<WebRtcSetDescriptionObserver> observer,
        bool surface_receivers_only)
    : main_task_runner_(std::move(main_task_runner)),
      signaling_task_runner_(std::move(signaling_task_runner)),
      pc_(std::move(pc)),
      track_adapter_map_(std::move(track_adapter_map)),
      observer_(std::move(observer)),
      surface_receivers_only_(surface_receivers_only) {}

WebRtcSetDescriptionObserverHandlerImpl::
    ~WebRtcSetDescriptionObserverHandlerImpl() = default;

void WebRtcSetDescriptionObserverHandlerImpl::OnSetDescriptionComplete(
    webrtc::RTCError error) {
  CHECK(signaling_task_runner_->BelongsToCurrentThread());
  std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
      receiver_only_transceivers;
  std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers;
  if (surface_receivers_only_) {
    for (const auto& receiver : pc_->GetReceivers()) {
      transceivers.push_back(new SurfaceReceiverStateOnly(receiver));
    }
  } else {
    transceivers = pc_->GetTransceivers();
  }
  TransceiverStateSurfacer transceiver_state_surfacer(main_task_runner_,
                                                      signaling_task_runner_);
  transceiver_state_surfacer.Initialize(track_adapter_map_,
                                        std::move(transceivers));
  main_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&WebRtcSetDescriptionObserverHandlerImpl::
                                    OnSetDescriptionCompleteOnMainThread,
                                this, std::move(error), pc_->signaling_state(),
                                std::move(transceiver_state_surfacer)));
}

void WebRtcSetDescriptionObserverHandlerImpl::
    OnSetDescriptionCompleteOnMainThread(
        webrtc::RTCError error,
        webrtc::PeerConnectionInterface::SignalingState signaling_state,
        TransceiverStateSurfacer transceiver_state_surfacer) {
  CHECK(main_task_runner_->BelongsToCurrentThread());
  WebRtcSetDescriptionObserver::States states;
  states.signaling_state = signaling_state;
  states.transceiver_states = transceiver_state_surfacer.ObtainStates();
  observer_->OnSetDescriptionComplete(std::move(error), std::move(states));
}

scoped_refptr<WebRtcSetLocalDescriptionObserverHandler>
WebRtcSetLocalDescriptionObserverHandler::Create(
    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
    scoped_refptr<webrtc::PeerConnectionInterface> pc,
    scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
    scoped_refptr<WebRtcSetDescriptionObserver> observer,
    bool surface_receivers_only) {
  return new rtc::RefCountedObject<WebRtcSetLocalDescriptionObserverHandler>(
      std::move(main_task_runner), std::move(signaling_task_runner),
      std::move(pc), std::move(track_adapter_map), std::move(observer),
      surface_receivers_only);
}

WebRtcSetLocalDescriptionObserverHandler::
    WebRtcSetLocalDescriptionObserverHandler(
        scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
        scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
        scoped_refptr<webrtc::PeerConnectionInterface> pc,
        scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
        scoped_refptr<WebRtcSetDescriptionObserver> observer,
        bool surface_receivers_only)
    : handler_impl_(new WebRtcSetDescriptionObserverHandlerImpl(
          std::move(main_task_runner),
          std::move(signaling_task_runner),
          std::move(pc),
          std::move(track_adapter_map),
          std::move(observer),
          surface_receivers_only)) {}

WebRtcSetLocalDescriptionObserverHandler::
    ~WebRtcSetLocalDescriptionObserverHandler() = default;

void WebRtcSetLocalDescriptionObserverHandler::OnSuccess() {
  handler_impl_->OnSetDescriptionComplete(webrtc::RTCError::OK());
}

void WebRtcSetLocalDescriptionObserverHandler::OnFailure(
    webrtc::RTCError error) {
  handler_impl_->OnSetDescriptionComplete(std::move(error));
}

scoped_refptr<WebRtcSetRemoteDescriptionObserverHandler>
WebRtcSetRemoteDescriptionObserverHandler::Create(
    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
    scoped_refptr<webrtc::PeerConnectionInterface> pc,
    scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
    scoped_refptr<WebRtcSetDescriptionObserver> observer,
    bool surface_receivers_only) {
  return new rtc::RefCountedObject<WebRtcSetRemoteDescriptionObserverHandler>(
      std::move(main_task_runner), std::move(signaling_task_runner),
      std::move(pc), std::move(track_adapter_map), std::move(observer),
      surface_receivers_only);
}

WebRtcSetRemoteDescriptionObserverHandler::
    WebRtcSetRemoteDescriptionObserverHandler(
        scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
        scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
        scoped_refptr<webrtc::PeerConnectionInterface> pc,
        scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
        scoped_refptr<WebRtcSetDescriptionObserver> observer,
        bool surface_receivers_only)
    : handler_impl_(new WebRtcSetDescriptionObserverHandlerImpl(
          std::move(main_task_runner),
          std::move(signaling_task_runner),
          std::move(pc),
          std::move(track_adapter_map),
          std::move(observer),
          surface_receivers_only)) {}

WebRtcSetRemoteDescriptionObserverHandler::
    ~WebRtcSetRemoteDescriptionObserverHandler() = default;

void WebRtcSetRemoteDescriptionObserverHandler::OnSetRemoteDescriptionComplete(
    webrtc::RTCError error) {
  handler_impl_->OnSetDescriptionComplete(std::move(error));
}

}  // namespace content
