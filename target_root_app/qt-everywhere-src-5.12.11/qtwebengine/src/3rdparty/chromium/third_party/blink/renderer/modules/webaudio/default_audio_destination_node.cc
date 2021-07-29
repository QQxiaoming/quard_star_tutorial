/*
 * Copyright (C) 2011, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "third_party/blink/renderer/modules/webaudio/default_audio_destination_node.h"

#include "third_party/blink/renderer/modules/webaudio/audio_node_input.h"
#include "third_party/blink/renderer/modules/webaudio/audio_node_output.h"
#include "third_party/blink/renderer/modules/webaudio/audio_worklet.h"
#include "third_party/blink/renderer/modules/webaudio/audio_worklet_messaging_proxy.h"
#include "third_party/blink/renderer/modules/webaudio/base_audio_context.h"
#include "third_party/blink/renderer/platform/bindings/exception_messages.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/audio/audio_utilities.h"
#include "third_party/blink/renderer/platform/audio/denormal_disabler.h"
#include "third_party/blink/renderer/platform/instrumentation/tracing/trace_event.h"
#include "third_party/blink/renderer/platform/wtf/atomics.h"

namespace blink {

scoped_refptr<DefaultAudioDestinationHandler>
DefaultAudioDestinationHandler::Create(
    AudioNode& node,
    const WebAudioLatencyHint& latency_hint) {
  return base::AdoptRef(new DefaultAudioDestinationHandler(node, latency_hint));
}

DefaultAudioDestinationHandler::DefaultAudioDestinationHandler(
    AudioNode& node,
    const WebAudioLatencyHint& latency_hint)
    : AudioDestinationHandler(node),
      latency_hint_(latency_hint),
      allow_pulling_audio_graph_(false) {
  // Node-specific default channel count and mixing rules.
  channel_count_ = 2;
  SetInternalChannelCountMode(kExplicit);
  SetInternalChannelInterpretation(AudioBus::kSpeakers);
}

DefaultAudioDestinationHandler::~DefaultAudioDestinationHandler() {
  DCHECK(!IsInitialized());
}

void DefaultAudioDestinationHandler::Dispose() {
  Uninitialize();
  AudioDestinationHandler::Dispose();
}

void DefaultAudioDestinationHandler::Initialize() {
  DCHECK(IsMainThread());
  if (IsInitialized())
    return;

  CreateDestination();
  AudioHandler::Initialize();
}

void DefaultAudioDestinationHandler::Uninitialize() {
  DCHECK(IsMainThread());
  if (!IsInitialized())
    return;

  if (destination_->IsPlaying())
    StopDestination();

  AudioHandler::Uninitialize();
}

void DefaultAudioDestinationHandler::SetChannelCount(
    unsigned long channel_count,
    ExceptionState& exception_state) {
  DCHECK(IsMainThread());

  // The channelCount for the input to this node controls the actual number of
  // channels we send to the audio hardware. It can only be set if the number
  // is less than the number of hardware channels.
  if (!MaxChannelCount() || channel_count > MaxChannelCount()) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kIndexSizeError,
        ExceptionMessages::IndexOutsideRange<unsigned>(
            "channel count", channel_count, 1,
            ExceptionMessages::kInclusiveBound, MaxChannelCount(),
            ExceptionMessages::kInclusiveBound));
    return;
  }

  unsigned long old_channel_count = this->ChannelCount();
  AudioHandler::SetChannelCount(channel_count, exception_state);

  // Stop, re-create and start the destination to apply the new channel count.
  if (!exception_state.HadException() &&
      this->ChannelCount() != old_channel_count && IsInitialized()) {
    StopDestination();
    CreateDestination();
    StartDestination();
  }
}

void DefaultAudioDestinationHandler::StartRendering() {
  DCHECK(IsInitialized());
  // Context might try to start rendering again while the destination is
  // running. Ignore it when that happens.
  if (IsInitialized() && !destination_->IsPlaying()) {
    StartDestination();
  }
}

void DefaultAudioDestinationHandler::StopRendering() {
  DCHECK(IsInitialized());
  // Context might try to stop rendering again while the destination is stopped.
  // Ignore it when that happens.
  if (IsInitialized() && destination_->IsPlaying()) {
    StopDestination();
  }
}

void DefaultAudioDestinationHandler::RestartRendering() {
  StopRendering();
  StartRendering();
}

unsigned long DefaultAudioDestinationHandler::MaxChannelCount() const {
  return AudioDestination::MaxChannelCount();
}

double DefaultAudioDestinationHandler::SampleRate() const {
  return destination_ ? destination_->SampleRate() : 0;
}

void DefaultAudioDestinationHandler::Render(
    AudioBus* destination_bus,
    size_t number_of_frames,
    const AudioIOPosition& output_position) {
  TRACE_EVENT0("webaudio", "DefaultAudioDestinationHandler::Render");

  // Denormals can seriously hurt performance of audio processing. This will
  // take care of all AudioNode processes within this scope.
  DenormalDisabler denormal_disabler;

  // A sanity check for the associated context, but this does not guarantee the
  // safe execution of the subsequence operations because the hanlder holds
  // the context as |UntracedMember| and it can go away anytime.
  DCHECK(Context());
  if (!Context())
    return;

  Context()->GetDeferredTaskHandler().SetAudioThreadToCurrentThread();

  // If this node is not initialized yet, pass silence to the platform audio
  // destination. It is for the case where this node is in the middle of
  // tear-down process.
  if (!IsInitialized()) {
    destination_bus->Zero();
    return;
  }

  Context()->HandlePreRenderTasks(output_position);

  DCHECK_GE(NumberOfInputs(), 1u);
  if (NumberOfInputs() < 1) {
    destination_bus->Zero();
    return;
  }

  // Only pull on the audio graph if we have not stopped the destination.  It
  // takes time for the destination to stop, but we want to stop pulling before
  // the destination has actually stopped.
  {
    MutexTryLocker try_locker(Context()->GetTearDownMutex());
    if (try_locker.Locked() && IsPullingAudioGraphAllowed()) {
      // Renders the graph by pulling all the input(s) to this node. This will
      // in turn pull on their input(s), all the way backwards through the graph.
      AudioBus* rendered_bus = Input(0).Pull(destination_bus, number_of_frames);

      DCHECK(rendered_bus);
      if (!rendered_bus) {
        // AudioNodeInput might be in the middle of destruction. Then the
        // internal summing bus will return as nullptr. Then zero out the
        // output.
        destination_bus->Zero();
      } else if (rendered_bus != destination_bus) {
        // In-place processing was not possible. Copy the rendererd result to
        // the given |destination_bus| buffer.
        destination_bus->CopyFrom(*rendered_bus);
      }
    } else {
       destination_bus->Zero();
    }
    // Processes "automatic" nodes that are not connected to anything. This can
    // be done after copying because it does not affect the rendered result.
    Context()->GetDeferredTaskHandler().ProcessAutomaticPullNodes(
        number_of_frames);
  }

  Context()->HandlePostRenderTasks();

  // Advances the current sample-frame.
  size_t new_sample_frame = current_sample_frame_ + number_of_frames;
  ReleaseStore(&current_sample_frame_, new_sample_frame);

  Context()->UpdateWorkletGlobalScopeOnRenderingThread();
}

size_t DefaultAudioDestinationHandler::GetCallbackBufferSize() const {
  return destination_->CallbackBufferSize();
}

int DefaultAudioDestinationHandler::GetFramesPerBuffer() const {
  return destination_ ? destination_->FramesPerBuffer() : 0;
}

void DefaultAudioDestinationHandler::CreateDestination() {
  destination_ = AudioDestination::Create(*this, ChannelCount(), latency_hint_);
}

void DefaultAudioDestinationHandler::StartDestination() {
  DCHECK(IsMainThread());
  DCHECK(!destination_->IsPlaying());

  AudioWorklet* audio_worklet = Context()->audioWorklet();
  if (audio_worklet && audio_worklet->IsReady()) {
    // This task runner is only used to fire the audio render callback, so it
    // MUST not be throttled to avoid potential audio glitch.
    destination_->StartWithWorkletTaskRunner(
        audio_worklet->GetMessagingProxy()
            ->GetBackingWorkerThread()
            ->GetTaskRunner(TaskType::kInternalMedia));
  } else {
    destination_->Start();
  }

  // Allow the graph to be pulled once the destination actually starts
  // requesting data.
  EnablePullingAudioGraph();
}

void DefaultAudioDestinationHandler::StopDestination() {
  DCHECK(IsMainThread());
  // Stop pulling on the graph, even if the destination is still requesting data
  // for a while. (It may take a bit of time for the destination to stop.)
  DisablePullingAudioGraph();
  DCHECK(destination_->IsPlaying());

  destination_->Stop();
}

// -----------------------------------------------------------------------------

DefaultAudioDestinationNode::DefaultAudioDestinationNode(
    BaseAudioContext& context,
    const WebAudioLatencyHint& latency_hint)
    : AudioDestinationNode(context) {
  SetHandler(DefaultAudioDestinationHandler::Create(*this, latency_hint));
}

DefaultAudioDestinationNode* DefaultAudioDestinationNode::Create(
    BaseAudioContext* context,
    const WebAudioLatencyHint& latency_hint) {
  return new DefaultAudioDestinationNode(*context, latency_hint);
}

}  // namespace blink
