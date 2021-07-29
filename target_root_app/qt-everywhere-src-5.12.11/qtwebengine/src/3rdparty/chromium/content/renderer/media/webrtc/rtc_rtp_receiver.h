// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_WEBRTC_RTC_RTP_RECEIVER_H_
#define CONTENT_RENDERER_MEDIA_WEBRTC_RTC_RTP_RECEIVER_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"
#include "content/common/content_export.h"
#include "content/renderer/media/webrtc/webrtc_media_stream_track_adapter_map.h"
#include "third_party/blink/public/platform/web_media_stream.h"
#include "third_party/blink/public/platform/web_media_stream_track.h"
#include "third_party/blink/public/platform/web_rtc_rtp_receiver.h"
#include "third_party/blink/public/platform/web_rtc_rtp_transceiver.h"
#include "third_party/webrtc/api/mediastreaminterface.h"
#include "third_party/webrtc/api/peerconnectioninterface.h"
#include "third_party/webrtc/api/rtpreceiverinterface.h"

namespace content {

// This class represents the state of a receiver; a snapshot of what a
// webrtc-layer receiver looked like when it was inspected on the signaling
// thread such that this information can be moved to the main thread in a single
// PostTask. It is used to surface state changes to make the blink-layer
// receiver up-to-date.
//
// Blink objects live on the main thread and webrtc objects live on the
// signaling thread. If multiple asynchronous operations begin execution on the
// main thread they are posted and executed in order on the signaling thread.
// For example, operation A and operation B are called in JavaScript. When A is
// done on the signaling thread, webrtc object states will be updated. A
// callback is posted to the main thread so that blink objects can be updated to
// match the result of operation A. But if callback A tries to inspect the
// webrtc objects from the main thread this requires posting back to the
// signaling thread and waiting, which also includes waiting for the previously
// posted task: operation B. Inspecting the webrtc object like this does not
// guarantee you to get the state of operation A.
//
// As such, all state changes associated with an operation have to be surfaced
// in the same callback. This includes copying any states into a separate object
// so that it can be inspected on the main thread without any additional thread
// hops.
//
// The RtpReceiverState is a snapshot of what the webrtc::RtpReceiverInterface
// looked like when the RtpReceiverState was created on the signaling thread. It
// also takes care of initializing track adapters, such that we have access to a
// blink track corresponding to the webrtc track of the receiver.
//
// Except for initialization logic and operator=(), the RtpReceiverState is
// immutable and only accessible on the main thread.
//
// TODO(hbos): [Onion Soup] When the sender implementation is moved to blink
// this will be part of the blink sender instead of the content sender.
// https://crbug.com/787254
class CONTENT_EXPORT RtpReceiverState {
 public:
  RtpReceiverState(
      scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
      scoped_refptr<webrtc::RtpReceiverInterface> webrtc_receiver,
      std::unique_ptr<WebRtcMediaStreamTrackAdapterMap::AdapterRef> track_ref,
      std::vector<std::string> stream_ids);
  RtpReceiverState(RtpReceiverState&&);
  RtpReceiverState(const RtpReceiverState&) = delete;
  ~RtpReceiverState();

  // This is intended to be used for moving the object from the signaling thread
  // to the main thread and as such has no thread checks. Once moved to the main
  // this should only be invoked on the main thread.
  RtpReceiverState& operator=(RtpReceiverState&&);
  RtpReceiverState& operator=(const RtpReceiverState&) = delete;

  bool is_initialized() const;
  void Initialize();

  scoped_refptr<base::SingleThreadTaskRunner> main_task_runner() const;
  scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner() const;
  scoped_refptr<webrtc::RtpReceiverInterface> webrtc_receiver() const;
  const std::unique_ptr<WebRtcMediaStreamTrackAdapterMap::AdapterRef>&
  track_ref() const;
  const std::vector<std::string>& stream_ids() const;

 private:
  scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner_;
  scoped_refptr<webrtc::RtpReceiverInterface> webrtc_receiver_;
  bool is_initialized_;
  std::unique_ptr<WebRtcMediaStreamTrackAdapterMap::AdapterRef> track_ref_;
  std::vector<std::string> stream_ids_;
};

// Used to surface |webrtc::RtpReceiverInterface| to blink. Multiple
// |RTCRtpReceiver|s could reference the same webrtc receiver; |id| is the value
// of the pointer to the webrtc receiver.
class CONTENT_EXPORT RTCRtpReceiver : public blink::WebRTCRtpReceiver {
 public:
  static uintptr_t getId(
      const webrtc::RtpReceiverInterface* webrtc_rtp_receiver);

  RTCRtpReceiver(
      scoped_refptr<webrtc::PeerConnectionInterface> native_peer_connection,
      RtpReceiverState state);
  RTCRtpReceiver(const RTCRtpReceiver& other);
  ~RTCRtpReceiver() override;

  RTCRtpReceiver& operator=(const RTCRtpReceiver& other);

  const RtpReceiverState& state() const;
  void set_state(RtpReceiverState state);

  std::unique_ptr<blink::WebRTCRtpReceiver> ShallowCopy() const override;
  uintptr_t Id() const override;
  const blink::WebMediaStreamTrack& Track() const override;
  blink::WebVector<blink::WebString> StreamIds() const override;
  blink::WebVector<std::unique_ptr<blink::WebRTCRtpContributingSource>>
  GetSources() override;
  void GetStats(std::unique_ptr<blink::WebRTCStatsReportCallback>) override;

 private:
  class RTCRtpReceiverInternal;
  struct RTCRtpReceiverInternalTraits;

  scoped_refptr<RTCRtpReceiverInternal> internal_;
};

class CONTENT_EXPORT RTCRtpReceiverOnlyTransceiver
    : public blink::WebRTCRtpTransceiver {
 public:
  RTCRtpReceiverOnlyTransceiver(
      std::unique_ptr<blink::WebRTCRtpReceiver> receiver);
  ~RTCRtpReceiverOnlyTransceiver() override;

  blink::WebRTCRtpTransceiverImplementationType ImplementationType()
      const override;
  uintptr_t Id() const override;
  blink::WebString Mid() const override;
  std::unique_ptr<blink::WebRTCRtpSender> Sender() const override;
  std::unique_ptr<blink::WebRTCRtpReceiver> Receiver() const override;
  bool Stopped() const override;
  webrtc::RtpTransceiverDirection Direction() const override;
  void SetDirection(webrtc::RtpTransceiverDirection direction) override;
  base::Optional<webrtc::RtpTransceiverDirection> CurrentDirection()
      const override;
  base::Optional<webrtc::RtpTransceiverDirection> FiredDirection()
      const override;

 private:
  std::unique_ptr<blink::WebRTCRtpReceiver> receiver_;
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_WEBRTC_RTC_RTP_RECEIVER_H_
