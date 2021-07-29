// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/webrtc/rtc_rtp_receiver.h"

#include "base/logging.h"
#include "content/renderer/media/webrtc/rtc_rtp_contributing_source.h"
#include "content/renderer/media/webrtc/rtc_stats.h"
#include "third_party/webrtc/rtc_base/scoped_ref_ptr.h"

namespace content {

RtpReceiverState::RtpReceiverState(
    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
    scoped_refptr<webrtc::RtpReceiverInterface> webrtc_receiver,
    std::unique_ptr<WebRtcMediaStreamTrackAdapterMap::AdapterRef> track_ref,
    std::vector<std::string> stream_id)
    : main_task_runner_(std::move(main_task_runner)),
      signaling_task_runner_(std::move(signaling_task_runner)),
      webrtc_receiver_(std::move(webrtc_receiver)),
      is_initialized_(false),
      track_ref_(std::move(track_ref)),
      stream_ids_(std::move(stream_id)) {
  DCHECK(main_task_runner_);
  DCHECK(signaling_task_runner_);
  DCHECK(webrtc_receiver_);
  DCHECK(track_ref_);
}

RtpReceiverState::RtpReceiverState(RtpReceiverState&& other)
    : main_task_runner_(other.main_task_runner_),
      signaling_task_runner_(other.signaling_task_runner_),
      webrtc_receiver_(std::move(other.webrtc_receiver_)),
      is_initialized_(other.is_initialized_),
      track_ref_(std::move(other.track_ref_)),
      stream_ids_(std::move(other.stream_ids_)) {
  // Explicitly null |other|'s task runners for use in destructor.
  other.main_task_runner_ = nullptr;
  other.signaling_task_runner_ = nullptr;
}

RtpReceiverState::~RtpReceiverState() {
  // It's OK to not be on the main thread if this state has been moved, in which
  // case |main_task_runner_| is null.
  DCHECK(!main_task_runner_ || main_task_runner_->BelongsToCurrentThread());
}

RtpReceiverState& RtpReceiverState::operator=(RtpReceiverState&& other) {
  DCHECK_EQ(main_task_runner_, other.main_task_runner_);
  DCHECK_EQ(signaling_task_runner_, other.signaling_task_runner_);
  // Explicitly null |other|'s task runners for use in destructor.
  other.main_task_runner_ = nullptr;
  other.signaling_task_runner_ = nullptr;
  webrtc_receiver_ = std::move(other.webrtc_receiver_);
  track_ref_ = std::move(other.track_ref_);
  stream_ids_ = std::move(other.stream_ids_);
  return *this;
}

bool RtpReceiverState::is_initialized() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return is_initialized_;
}

void RtpReceiverState::Initialize() {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  if (is_initialized_)
    return;
  track_ref_->InitializeOnMainThread();
  is_initialized_ = true;
}

scoped_refptr<base::SingleThreadTaskRunner> RtpReceiverState::main_task_runner()
    const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return main_task_runner_;
}

scoped_refptr<base::SingleThreadTaskRunner>
RtpReceiverState::signaling_task_runner() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return signaling_task_runner_;
}

scoped_refptr<webrtc::RtpReceiverInterface> RtpReceiverState::webrtc_receiver()
    const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return webrtc_receiver_;
}

const std::unique_ptr<WebRtcMediaStreamTrackAdapterMap::AdapterRef>&
RtpReceiverState::track_ref() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return track_ref_;
}

const std::vector<std::string>& RtpReceiverState::stream_ids() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return stream_ids_;
}

class RTCRtpReceiver::RTCRtpReceiverInternal
    : public base::RefCountedThreadSafe<
          RTCRtpReceiver::RTCRtpReceiverInternal,
          RTCRtpReceiver::RTCRtpReceiverInternalTraits> {
 public:
  RTCRtpReceiverInternal(
      scoped_refptr<webrtc::PeerConnectionInterface> native_peer_connection,
      RtpReceiverState state)
      : native_peer_connection_(std::move(native_peer_connection)),
        main_task_runner_(state.main_task_runner()),
        signaling_task_runner_(state.signaling_task_runner()),
        webrtc_receiver_(state.webrtc_receiver()),
        state_(std::move(state)) {
    DCHECK(native_peer_connection_);
    DCHECK(state_.is_initialized());
  }

  const RtpReceiverState& state() const {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
    return state_;
  }

  void set_state(RtpReceiverState state) {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
    DCHECK(state.main_task_runner() == main_task_runner_);
    DCHECK(state.signaling_task_runner() == signaling_task_runner_);
    DCHECK(state.webrtc_receiver() == webrtc_receiver_);
    DCHECK(state.is_initialized());
    state_ = std::move(state);
  }

  blink::WebVector<std::unique_ptr<blink::WebRTCRtpContributingSource>>
  GetSources() {
    // The webrtc_recever_ is a proxy, so this is a blocking call to the webrtc
    // signalling thread.
    auto webrtc_sources = webrtc_receiver_->GetSources();
    blink::WebVector<std::unique_ptr<blink::WebRTCRtpContributingSource>>
        sources(webrtc_sources.size());
    for (size_t i = 0; i < webrtc_sources.size(); ++i) {
      sources[i] =
          std::make_unique<RTCRtpContributingSource>(webrtc_sources[i]);
    }
    return sources;
  }

  void GetStats(std::unique_ptr<blink::WebRTCStatsReportCallback> callback) {
    signaling_task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&RTCRtpReceiverInternal::GetStatsOnSignalingThread, this,
                       std::move(callback)));
  }

 private:
  friend struct RTCRtpReceiver::RTCRtpReceiverInternalTraits;

  ~RTCRtpReceiverInternal() {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
  }

  void GetStatsOnSignalingThread(
      std::unique_ptr<blink::WebRTCStatsReportCallback> callback) {
    native_peer_connection_->GetStats(
        webrtc_receiver_.get(), RTCStatsCollectorCallbackImpl::Create(
                                    main_task_runner_, std::move(callback)));
  }

  const scoped_refptr<webrtc::PeerConnectionInterface> native_peer_connection_;
  // Task runners and webrtc receiver: Same information as stored in
  // |state_| but const and safe to touch on the signaling thread to
  // avoid race with set_state().
  const scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;
  const scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner_;
  const scoped_refptr<webrtc::RtpReceiverInterface> webrtc_receiver_;
  RtpReceiverState state_;
};

struct RTCRtpReceiver::RTCRtpReceiverInternalTraits {
 private:
  friend class base::RefCountedThreadSafe<RTCRtpReceiverInternal,
                                          RTCRtpReceiverInternalTraits>;

  static void Destruct(const RTCRtpReceiverInternal* receiver) {
    // RTCRtpReceiverInternal owns AdapterRefs which have to be destroyed on the
    // main thread, this ensures delete always happens there.
    if (!receiver->main_task_runner_->BelongsToCurrentThread()) {
      receiver->main_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(
              &RTCRtpReceiver::RTCRtpReceiverInternalTraits::Destruct,
              base::Unretained(receiver)));
      return;
    }
    delete receiver;
  }
};

uintptr_t RTCRtpReceiver::getId(
    const webrtc::RtpReceiverInterface* webrtc_rtp_receiver) {
  return reinterpret_cast<uintptr_t>(webrtc_rtp_receiver);
}

RTCRtpReceiver::RTCRtpReceiver(
    scoped_refptr<webrtc::PeerConnectionInterface> native_peer_connection,
    RtpReceiverState state)
    : internal_(new RTCRtpReceiverInternal(std::move(native_peer_connection),
                                           std::move(state))) {}

RTCRtpReceiver::RTCRtpReceiver(const RTCRtpReceiver& other)
    : internal_(other.internal_) {}

RTCRtpReceiver::~RTCRtpReceiver() {}

RTCRtpReceiver& RTCRtpReceiver::operator=(const RTCRtpReceiver& other) {
  internal_ = other.internal_;
  return *this;
}

const RtpReceiverState& RTCRtpReceiver::state() const {
  return internal_->state();
}

void RTCRtpReceiver::set_state(RtpReceiverState state) {
  internal_->set_state(std::move(state));
}

std::unique_ptr<blink::WebRTCRtpReceiver> RTCRtpReceiver::ShallowCopy() const {
  return std::make_unique<RTCRtpReceiver>(*this);
}

uintptr_t RTCRtpReceiver::Id() const {
  return getId(internal_->state().webrtc_receiver().get());
}

const blink::WebMediaStreamTrack& RTCRtpReceiver::Track() const {
  return internal_->state().track_ref()->web_track();
}

blink::WebVector<blink::WebString> RTCRtpReceiver::StreamIds() const {
  const auto& stream_ids = internal_->state().stream_ids();
  blink::WebVector<blink::WebString> web_stream_ids(stream_ids.size());
  for (size_t i = 0; i < stream_ids.size(); ++i)
    web_stream_ids[i] = blink::WebString::FromUTF8(stream_ids[i]);
  return web_stream_ids;
}

blink::WebVector<std::unique_ptr<blink::WebRTCRtpContributingSource>>
RTCRtpReceiver::GetSources() {
  return internal_->GetSources();
}

void RTCRtpReceiver::GetStats(
    std::unique_ptr<blink::WebRTCStatsReportCallback> callback) {
  internal_->GetStats(std::move(callback));
}

RTCRtpReceiverOnlyTransceiver::RTCRtpReceiverOnlyTransceiver(
    std::unique_ptr<blink::WebRTCRtpReceiver> receiver)
    : receiver_(std::move(receiver)) {
  DCHECK(receiver_);
}

RTCRtpReceiverOnlyTransceiver::~RTCRtpReceiverOnlyTransceiver() {}

blink::WebRTCRtpTransceiverImplementationType
RTCRtpReceiverOnlyTransceiver::ImplementationType() const {
  return blink::WebRTCRtpTransceiverImplementationType::kPlanBReceiverOnly;
}

uintptr_t RTCRtpReceiverOnlyTransceiver::Id() const {
  NOTIMPLEMENTED();
  return 0u;
}

blink::WebString RTCRtpReceiverOnlyTransceiver::Mid() const {
  NOTIMPLEMENTED();
  return blink::WebString();
}

std::unique_ptr<blink::WebRTCRtpSender> RTCRtpReceiverOnlyTransceiver::Sender()
    const {
  NOTIMPLEMENTED();
  return nullptr;
}

std::unique_ptr<blink::WebRTCRtpReceiver>
RTCRtpReceiverOnlyTransceiver::Receiver() const {
  return receiver_->ShallowCopy();
}

bool RTCRtpReceiverOnlyTransceiver::Stopped() const {
  NOTIMPLEMENTED();
  return false;
}

webrtc::RtpTransceiverDirection RTCRtpReceiverOnlyTransceiver::Direction()
    const {
  NOTIMPLEMENTED();
  return webrtc::RtpTransceiverDirection::kSendOnly;
}

void RTCRtpReceiverOnlyTransceiver::SetDirection(
    webrtc::RtpTransceiverDirection direction) {
  NOTIMPLEMENTED();
}

base::Optional<webrtc::RtpTransceiverDirection>
RTCRtpReceiverOnlyTransceiver::CurrentDirection() const {
  NOTIMPLEMENTED();
  return webrtc::RtpTransceiverDirection::kSendOnly;
}

base::Optional<webrtc::RtpTransceiverDirection>
RTCRtpReceiverOnlyTransceiver::FiredDirection() const {
  NOTIMPLEMENTED();
  return webrtc::RtpTransceiverDirection::kSendOnly;
}

}  // namespace content
