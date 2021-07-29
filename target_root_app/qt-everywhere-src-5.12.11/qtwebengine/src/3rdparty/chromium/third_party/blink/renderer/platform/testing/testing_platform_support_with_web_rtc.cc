// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/testing/testing_platform_support_with_web_rtc.h"

#include <memory>
#include "third_party/blink/public/platform/web_media_stream.h"
#include "third_party/blink/public/platform/web_media_stream_track.h"
#include "third_party/blink/public/platform/web_rtc_dtmf_sender_handler.h"
#include "third_party/blink/public/platform/web_rtc_rtp_transceiver.h"
#include "third_party/blink/public/platform/web_rtc_session_description.h"
#include "third_party/blink/public/platform/web_vector.h"

namespace blink {

namespace {

class DummyWebRTCRtpSender : public WebRTCRtpSender {
 private:
  static uintptr_t last_id_;

 public:
  DummyWebRTCRtpSender(WebMediaStreamTrack track)
      : id_(++last_id_), track_(std::move(track)) {}
  ~DummyWebRTCRtpSender() override {}

  std::unique_ptr<WebRTCRtpSender> ShallowCopy() const override {
    return nullptr;
  }
  uintptr_t Id() const override { return id_; }
  WebMediaStreamTrack Track() const override { return track_; }
  WebVector<WebString> StreamIds() const override {
    return std::vector<WebString>({WebString::FromUTF8("DummyStringId")});
  }
  void ReplaceTrack(WebMediaStreamTrack, WebRTCVoidRequest) override {}
  std::unique_ptr<WebRTCDTMFSenderHandler> GetDtmfSender() const override {
    return nullptr;
  }
  std::unique_ptr<webrtc::RtpParameters> GetParameters() const override {
    return std::unique_ptr<webrtc::RtpParameters>();
  }
  void SetParameters(blink::WebVector<webrtc::RtpEncodingParameters>,
                     webrtc::DegradationPreference,
                     WebRTCVoidRequest) override {}
  void GetStats(std::unique_ptr<blink::WebRTCStatsReportCallback>) override {}

  void set_track(WebMediaStreamTrack track) { track_ = std::move(track); }

 private:
  const uintptr_t id_;
  WebMediaStreamTrack track_;
};

uintptr_t DummyWebRTCRtpSender::last_id_ = 0;

class DummyRTCRtpTransceiver : public WebRTCRtpTransceiver {
 public:
  DummyRTCRtpTransceiver(WebMediaStreamTrack track)
      : track_(std::move(track)) {}
  ~DummyRTCRtpTransceiver() override {}

  WebRTCRtpTransceiverImplementationType ImplementationType() const override {
    return WebRTCRtpTransceiverImplementationType::kPlanBSenderOnly;
  }
  uintptr_t Id() const override { return 0u; }
  WebString Mid() const override { return WebString(); }
  std::unique_ptr<WebRTCRtpSender> Sender() const override {
    return std::make_unique<DummyWebRTCRtpSender>(track_);
  }
  std::unique_ptr<WebRTCRtpReceiver> Receiver() const override {
    return nullptr;
  }
  bool Stopped() const override { return true; }
  webrtc::RtpTransceiverDirection Direction() const override {
    return webrtc::RtpTransceiverDirection::kInactive;
  }
  void SetDirection(webrtc::RtpTransceiverDirection) override {}
  base::Optional<webrtc::RtpTransceiverDirection> CurrentDirection()
      const override {
    return base::nullopt;
  }
  base::Optional<webrtc::RtpTransceiverDirection> FiredDirection()
      const override {
    return base::nullopt;
  }

 private:
  WebMediaStreamTrack track_;
};

}  // namespace

MockWebRTCPeerConnectionHandler::MockWebRTCPeerConnectionHandler() = default;

MockWebRTCPeerConnectionHandler::~MockWebRTCPeerConnectionHandler() = default;

bool MockWebRTCPeerConnectionHandler::Initialize(const WebRTCConfiguration&,
                                                 const WebMediaConstraints&,
                                                 WebRTCSdpSemantics) {
  return true;
}

void MockWebRTCPeerConnectionHandler::CreateOffer(
    const WebRTCSessionDescriptionRequest&,
    const WebMediaConstraints&) {}

void MockWebRTCPeerConnectionHandler::CreateOffer(
    const WebRTCSessionDescriptionRequest&,
    const WebRTCOfferOptions&) {}

void MockWebRTCPeerConnectionHandler::CreateAnswer(
    const WebRTCSessionDescriptionRequest&,
    const WebMediaConstraints&) {}

void MockWebRTCPeerConnectionHandler::CreateAnswer(
    const WebRTCSessionDescriptionRequest&,
    const WebRTCAnswerOptions&) {}

void MockWebRTCPeerConnectionHandler::SetLocalDescription(
    const WebRTCVoidRequest&,
    const WebRTCSessionDescription&) {}

void MockWebRTCPeerConnectionHandler::SetRemoteDescription(
    const WebRTCVoidRequest&,
    const WebRTCSessionDescription&) {}

WebRTCSessionDescription MockWebRTCPeerConnectionHandler::LocalDescription() {
  return WebRTCSessionDescription();
}

WebRTCSessionDescription MockWebRTCPeerConnectionHandler::RemoteDescription() {
  return WebRTCSessionDescription();
}

webrtc::RTCErrorType MockWebRTCPeerConnectionHandler::SetConfiguration(
    const WebRTCConfiguration&) {
  return webrtc::RTCErrorType::NONE;
}

void MockWebRTCPeerConnectionHandler::GetStats(const WebRTCStatsRequest&) {}

void MockWebRTCPeerConnectionHandler::GetStats(
    std::unique_ptr<WebRTCStatsReportCallback>) {}

webrtc::RTCErrorOr<std::unique_ptr<WebRTCRtpTransceiver>>
MockWebRTCPeerConnectionHandler::AddTransceiverWithTrack(
    const WebMediaStreamTrack& track,
    const webrtc::RtpTransceiverInit&) {
  std::unique_ptr<WebRTCRtpTransceiver> transceiver =
      std::make_unique<DummyRTCRtpTransceiver>(track);
  return transceiver;
}

webrtc::RTCErrorOr<std::unique_ptr<WebRTCRtpTransceiver>>
MockWebRTCPeerConnectionHandler::AddTransceiverWithKind(
    std::string kind,
    const webrtc::RtpTransceiverInit&) {
  std::unique_ptr<WebRTCRtpTransceiver> transceiver =
      std::make_unique<DummyRTCRtpTransceiver>(WebMediaStreamTrack());
  return transceiver;
}

webrtc::RTCErrorOr<std::unique_ptr<WebRTCRtpTransceiver>>
MockWebRTCPeerConnectionHandler::AddTrack(const WebMediaStreamTrack& track,
                                          const WebVector<WebMediaStream>&) {
  std::unique_ptr<WebRTCRtpTransceiver> transceiver =
      std::make_unique<DummyRTCRtpTransceiver>(track);
  return transceiver;
}

webrtc::RTCErrorOr<std::unique_ptr<WebRTCRtpTransceiver>>
MockWebRTCPeerConnectionHandler::RemoveTrack(WebRTCRtpSender* sender) {
  static_cast<DummyWebRTCRtpSender*>(sender)->set_track(WebMediaStreamTrack());
  return std::unique_ptr<WebRTCRtpTransceiver>(nullptr);
}

WebRTCDataChannelHandler* MockWebRTCPeerConnectionHandler::CreateDataChannel(
    const WebString& label,
    const WebRTCDataChannelInit&) {
  return nullptr;
}

void MockWebRTCPeerConnectionHandler::Stop() {}

WebString MockWebRTCPeerConnectionHandler::Id() const {
  return WebString();
}

std::unique_ptr<WebRTCPeerConnectionHandler>
TestingPlatformSupportWithWebRTC::CreateRTCPeerConnectionHandler(
    WebRTCPeerConnectionHandlerClient*,
    scoped_refptr<base::SingleThreadTaskRunner>) {
  return std::make_unique<MockWebRTCPeerConnectionHandler>();
}

}  // namespace blink
