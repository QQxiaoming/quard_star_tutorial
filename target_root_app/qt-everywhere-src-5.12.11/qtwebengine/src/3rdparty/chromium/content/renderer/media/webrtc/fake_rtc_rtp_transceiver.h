// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_WEBRTC_FAKE_RTC_RTP_TRANSCEIVER_H_
#define CONTENT_RENDERER_MEDIA_WEBRTC_FAKE_RTC_RTP_TRANSCEIVER_H_

#include <memory>

#include "content/renderer/media/stream/media_stream_audio_source.h"
#include "third_party/blink/public/platform/web_media_constraints.h"
#include "third_party/blink/public/platform/web_media_stream_source.h"
#include "third_party/blink/public/platform/web_media_stream_track.h"
#include "third_party/blink/public/platform/web_rtc_dtmf_sender_handler.h"
#include "third_party/blink/public/platform/web_rtc_rtp_contributing_source.h"
#include "third_party/blink/public/platform/web_rtc_rtp_receiver.h"
#include "third_party/blink/public/platform/web_rtc_rtp_sender.h"
#include "third_party/blink/public/platform/web_rtc_rtp_transceiver.h"

namespace content {

// TODO(https://crbug.com/868868): Similar methods to this exist in many content
// unittests. Move to a separate file and reuse it in all of them.
blink::WebMediaStreamTrack CreateWebMediaStreamTrack(const std::string& id);

class CONTENT_EXPORT FakeRTCRtpSender : public blink::WebRTCRtpSender {
 public:
  FakeRTCRtpSender(base::Optional<std::string> track_id,
                   std::vector<std::string> stream_ids);
  FakeRTCRtpSender(const FakeRTCRtpSender&);
  ~FakeRTCRtpSender() override;
  FakeRTCRtpSender& operator=(const FakeRTCRtpSender&);

  std::unique_ptr<blink::WebRTCRtpSender> ShallowCopy() const override;
  uintptr_t Id() const override;
  blink::WebMediaStreamTrack Track() const override;
  blink::WebVector<blink::WebString> StreamIds() const override;
  void ReplaceTrack(blink::WebMediaStreamTrack with_track,
                    blink::WebRTCVoidRequest request) override;
  std::unique_ptr<blink::WebRTCDTMFSenderHandler> GetDtmfSender()
      const override;
  std::unique_ptr<webrtc::RtpParameters> GetParameters() const override;
  void SetParameters(blink::WebVector<webrtc::RtpEncodingParameters>,
                     webrtc::DegradationPreference,
                     blink::WebRTCVoidRequest) override;
  void GetStats(std::unique_ptr<blink::WebRTCStatsReportCallback>) override;

 private:
  base::Optional<std::string> track_id_;
  std::vector<std::string> stream_ids_;
};

class CONTENT_EXPORT FakeRTCRtpReceiver : public blink::WebRTCRtpReceiver {
 public:
  FakeRTCRtpReceiver(const std::string& track_id,
                     std::vector<std::string> stream_ids);
  FakeRTCRtpReceiver(const FakeRTCRtpReceiver&);
  ~FakeRTCRtpReceiver() override;
  FakeRTCRtpReceiver& operator=(const FakeRTCRtpReceiver&);

  std::unique_ptr<blink::WebRTCRtpReceiver> ShallowCopy() const override;
  uintptr_t Id() const override;
  const blink::WebMediaStreamTrack& Track() const override;
  blink::WebVector<blink::WebString> StreamIds() const override;
  blink::WebVector<std::unique_ptr<blink::WebRTCRtpContributingSource>>
  GetSources() override;
  void GetStats(std::unique_ptr<blink::WebRTCStatsReportCallback>) override;

 private:
  blink::WebMediaStreamTrack track_;
  std::vector<std::string> stream_ids_;
};

class CONTENT_EXPORT FakeRTCRtpTransceiver
    : public blink::WebRTCRtpTransceiver {
 public:
  FakeRTCRtpTransceiver(
      base::Optional<std::string> mid,
      FakeRTCRtpSender sender,
      FakeRTCRtpReceiver receiver,
      bool stopped,
      webrtc::RtpTransceiverDirection direction,
      base::Optional<webrtc::RtpTransceiverDirection> current_direction);
  ~FakeRTCRtpTransceiver() override;

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
  base::Optional<std::string> mid_;
  FakeRTCRtpSender sender_;
  FakeRTCRtpReceiver receiver_;
  bool stopped_;
  webrtc::RtpTransceiverDirection direction_;
  base::Optional<webrtc::RtpTransceiverDirection> current_direction_;
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_WEBRTC_FAKE_RTC_RTP_TRANSCEIVER_H_
