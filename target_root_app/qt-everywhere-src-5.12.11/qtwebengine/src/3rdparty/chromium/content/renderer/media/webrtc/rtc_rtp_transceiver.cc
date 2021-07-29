// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/webrtc/rtc_rtp_transceiver.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"

namespace content {

RtpTransceiverState::RtpTransceiverState(
    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
    scoped_refptr<webrtc::RtpTransceiverInterface> webrtc_transceiver,
    base::Optional<RtpSenderState> sender_state,
    base::Optional<RtpReceiverState> receiver_state,
    base::Optional<std::string> mid,
    bool stopped,
    webrtc::RtpTransceiverDirection direction,
    base::Optional<webrtc::RtpTransceiverDirection> current_direction,
    base::Optional<webrtc::RtpTransceiverDirection> fired_direction)
    : main_task_runner_(std::move(main_task_runner)),
      signaling_task_runner_(std::move(signaling_task_runner)),
      webrtc_transceiver_(std::move(webrtc_transceiver)),
      is_initialized_(false),
      sender_state_(std::move(sender_state)),
      receiver_state_(std::move(receiver_state)),
      mid_(std::move(mid)),
      stopped_(std::move(stopped)),
      direction_(std::move(direction)),
      current_direction_(std::move(current_direction)),
      fired_direction_(std::move(fired_direction)) {
  DCHECK(main_task_runner_);
  DCHECK(signaling_task_runner_);
  DCHECK(webrtc_transceiver_);
}

RtpTransceiverState::RtpTransceiverState(RtpTransceiverState&& other)
    : main_task_runner_(other.main_task_runner_),
      signaling_task_runner_(other.signaling_task_runner_),
      webrtc_transceiver_(std::move(other.webrtc_transceiver_)),
      is_initialized_(other.is_initialized_),
      sender_state_(std::move(other.sender_state_)),
      receiver_state_(std::move(other.receiver_state_)),
      mid_(std::move(other.mid_)),
      stopped_(std::move(other.stopped_)),
      direction_(std::move(other.direction_)),
      current_direction_(std::move(other.current_direction_)),
      fired_direction_(std::move(other.fired_direction_)) {
  // Explicitly null |other|'s task runners for use in destructor.
  other.main_task_runner_ = nullptr;
  other.signaling_task_runner_ = nullptr;
}

RtpTransceiverState::~RtpTransceiverState() {
  // It's OK to not be on the main thread if this state has been moved, in which
  // case |main_task_runner_| is null.
  DCHECK(!main_task_runner_ || main_task_runner_->BelongsToCurrentThread());
}

RtpTransceiverState& RtpTransceiverState::operator=(
    RtpTransceiverState&& other) {
  DCHECK_EQ(main_task_runner_, other.main_task_runner_);
  DCHECK_EQ(signaling_task_runner_, other.signaling_task_runner_);
  // Need to be on main thread for sender/receiver state's destructor that can
  // be triggered by replacing .
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  // Explicitly null |other|'s task runners for use in destructor.
  other.main_task_runner_ = nullptr;
  other.signaling_task_runner_ = nullptr;
  webrtc_transceiver_ = std::move(other.webrtc_transceiver_);
  is_initialized_ = other.is_initialized_;
  sender_state_ = std::move(other.sender_state_);
  receiver_state_ = std::move(other.receiver_state_);
  mid_ = std::move(other.mid_);
  stopped_ = std::move(other.stopped_);
  direction_ = std::move(other.direction_);
  current_direction_ = std::move(other.current_direction_);
  fired_direction_ = std::move(other.fired_direction_);
  return *this;
}

bool RtpTransceiverState::is_initialized() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return is_initialized_;
}

void RtpTransceiverState::Initialize() {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  if (sender_state_)
    sender_state_->Initialize();
  if (receiver_state_)
    receiver_state_->Initialize();
  is_initialized_ = true;
}

scoped_refptr<base::SingleThreadTaskRunner>
RtpTransceiverState::main_task_runner() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return main_task_runner_;
}

scoped_refptr<base::SingleThreadTaskRunner>
RtpTransceiverState::signaling_task_runner() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return signaling_task_runner_;
}

scoped_refptr<webrtc::RtpTransceiverInterface>
RtpTransceiverState::webrtc_transceiver() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return webrtc_transceiver_;
}

const base::Optional<RtpSenderState>& RtpTransceiverState::sender_state()
    const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return sender_state_;
}

RtpSenderState RtpTransceiverState::MoveSenderState() {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  base::Optional<RtpSenderState> temp(base::nullopt);
  sender_state_.swap(temp);
  return *std::move(temp);
}

const base::Optional<RtpReceiverState>& RtpTransceiverState::receiver_state()
    const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return receiver_state_;
}

RtpReceiverState RtpTransceiverState::MoveReceiverState() {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  base::Optional<RtpReceiverState> temp(base::nullopt);
  receiver_state_.swap(temp);
  return *std::move(temp);
}

base::Optional<std::string> RtpTransceiverState::mid() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return mid_;
}

bool RtpTransceiverState::stopped() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return stopped_;
}

webrtc::RtpTransceiverDirection RtpTransceiverState::direction() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return direction_;
}

void RtpTransceiverState::set_direction(
    webrtc::RtpTransceiverDirection direction) {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  direction_ = direction;
}

base::Optional<webrtc::RtpTransceiverDirection>
RtpTransceiverState::current_direction() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return current_direction_;
}

base::Optional<webrtc::RtpTransceiverDirection>
RtpTransceiverState::fired_direction() const {
  DCHECK(main_task_runner_->BelongsToCurrentThread());
  return fired_direction_;
}

class RTCRtpTransceiver::RTCRtpTransceiverInternal
    : public base::RefCountedThreadSafe<
          RTCRtpTransceiver::RTCRtpTransceiverInternal,
          RTCRtpTransceiver::RTCRtpTransceiverInternalTraits> {
 public:
  RTCRtpTransceiverInternal(
      scoped_refptr<webrtc::PeerConnectionInterface> native_peer_connection,
      scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_map,
      RtpTransceiverState state)
      : main_task_runner_(state.main_task_runner()),
        signaling_task_runner_(state.signaling_task_runner()),
        webrtc_transceiver_(state.webrtc_transceiver()),
        state_(std::move(state)) {
    sender_ = std::make_unique<RTCRtpSender>(native_peer_connection, track_map,
                                             state_.MoveSenderState());
    receiver_ = std::make_unique<RTCRtpReceiver>(native_peer_connection,
                                                 state_.MoveReceiverState());
  }

  const RtpTransceiverState& state() const {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
    return state_;
  }

  void set_state(RtpTransceiverState state) {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
    DCHECK_EQ(state.main_task_runner(), main_task_runner_);
    DCHECK_EQ(state.signaling_task_runner(), signaling_task_runner_);
    DCHECK(state.webrtc_transceiver() == webrtc_transceiver_);
    DCHECK(state.is_initialized());
    state_ = std::move(state);
    sender_->set_state(state_.MoveSenderState());
    receiver_->set_state(state_.MoveReceiverState());
  }

  RTCRtpSender* content_sender() {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
    return sender_.get();
  }

  RTCRtpReceiver* content_receiver() {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
    return receiver_.get();
  }

  void SetDirection(webrtc::RtpTransceiverDirection direction) {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
    // This implicitly performs a blocking invoke on the webrtc signaling thread
    // due to use of PROXY references for |webrtc_transceiver_|.
    webrtc_transceiver_->SetDirection(direction);
    state_.set_direction(webrtc_transceiver_->direction());
  }

 private:
  friend struct RTCRtpTransceiver::RTCRtpTransceiverInternalTraits;

  ~RTCRtpTransceiverInternal() {
    // Ensured by destructor traits.
    DCHECK(main_task_runner_->BelongsToCurrentThread());
  }

  // Task runners and webrtc transceiver: Same information as stored in |state_|
  // but const and safe to touch on the signaling thread to avoid race with
  // set_state().
  const scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;
  const scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner_;
  const scoped_refptr<webrtc::RtpTransceiverInterface> webrtc_transceiver_;
  RtpTransceiverState state_;
  std::unique_ptr<RTCRtpSender> sender_;
  std::unique_ptr<RTCRtpReceiver> receiver_;
};

struct RTCRtpTransceiver::RTCRtpTransceiverInternalTraits {
 private:
  friend class base::RefCountedThreadSafe<RTCRtpTransceiverInternal,
                                          RTCRtpTransceiverInternalTraits>;

  static void Destruct(const RTCRtpTransceiverInternal* transceiver) {
    // RTCRtpTransceiverInternal owns AdapterRefs which have to be destroyed on
    // the main thread, this ensures delete always happens there.
    if (!transceiver->main_task_runner_->BelongsToCurrentThread()) {
      transceiver->main_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(
              &RTCRtpTransceiver::RTCRtpTransceiverInternalTraits::Destruct,
              base::Unretained(transceiver)));
      return;
    }
    delete transceiver;
  }
};

uintptr_t RTCRtpTransceiver::GetId(
    const webrtc::RtpTransceiverInterface* webrtc_transceiver) {
  return reinterpret_cast<uintptr_t>(webrtc_transceiver);
}

RTCRtpTransceiver::RTCRtpTransceiver(
    scoped_refptr<webrtc::PeerConnectionInterface> native_peer_connection,
    scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_map,
    RtpTransceiverState transceiver_state)
    : internal_(new RTCRtpTransceiverInternal(std::move(native_peer_connection),
                                              std::move(track_map),
                                              std::move(transceiver_state))) {}

RTCRtpTransceiver::RTCRtpTransceiver(const RTCRtpTransceiver& other)
    : internal_(other.internal_) {}

RTCRtpTransceiver::~RTCRtpTransceiver() {}

RTCRtpTransceiver& RTCRtpTransceiver::operator=(
    const RTCRtpTransceiver& other) {
  internal_ = other.internal_;
  return *this;
}

std::unique_ptr<RTCRtpTransceiver> RTCRtpTransceiver::ShallowCopy() const {
  return std::make_unique<RTCRtpTransceiver>(*this);
}

const RtpTransceiverState& RTCRtpTransceiver::state() const {
  return internal_->state();
}

RTCRtpSender* RTCRtpTransceiver::content_sender() {
  return internal_->content_sender();
}

RTCRtpReceiver* RTCRtpTransceiver::content_receiver() {
  return internal_->content_receiver();
}

void RTCRtpTransceiver::set_state(RtpTransceiverState transceiver_state) {
  internal_->set_state(std::move(transceiver_state));
}

blink::WebRTCRtpTransceiverImplementationType
RTCRtpTransceiver::ImplementationType() const {
  return blink::WebRTCRtpTransceiverImplementationType::kFullTransceiver;
}

uintptr_t RTCRtpTransceiver::Id() const {
  return GetId(internal_->state().webrtc_transceiver().get());
}

blink::WebString RTCRtpTransceiver::Mid() const {
  const auto& mid = internal_->state().mid();
  return mid ? blink::WebString::FromUTF8(*mid)
             : blink::WebString();  // IsNull()
}

std::unique_ptr<blink::WebRTCRtpSender> RTCRtpTransceiver::Sender() const {
  return internal_->content_sender()->ShallowCopy();
}

std::unique_ptr<blink::WebRTCRtpReceiver> RTCRtpTransceiver::Receiver() const {
  return internal_->content_receiver()->ShallowCopy();
}

bool RTCRtpTransceiver::Stopped() const {
  return internal_->state().stopped();
}

webrtc::RtpTransceiverDirection RTCRtpTransceiver::Direction() const {
  return internal_->state().direction();
}

void RTCRtpTransceiver::SetDirection(
    webrtc::RtpTransceiverDirection direction) {
  internal_->SetDirection(direction);
}

base::Optional<webrtc::RtpTransceiverDirection>
RTCRtpTransceiver::CurrentDirection() const {
  return internal_->state().current_direction();
}

base::Optional<webrtc::RtpTransceiverDirection>
RTCRtpTransceiver::FiredDirection() const {
  return internal_->state().fired_direction();
}

}  // namespace content
