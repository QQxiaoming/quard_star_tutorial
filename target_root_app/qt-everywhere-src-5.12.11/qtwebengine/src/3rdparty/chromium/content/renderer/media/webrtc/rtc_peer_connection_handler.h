// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_WEBRTC_RTC_PEER_CONNECTION_HANDLER_H_
#define CONTENT_RENDERER_MEDIA_WEBRTC_RTC_PEER_CONNECTION_HANDLER_H_

#include <stddef.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread.h"
#include "content/common/content_export.h"
#include "content/renderer/media/webrtc/media_stream_track_metrics.h"
#include "content/renderer/media/webrtc/rtc_rtp_receiver.h"
#include "content/renderer/media/webrtc/rtc_rtp_sender.h"
#include "content/renderer/media/webrtc/transceiver_state_surfacer.h"
#include "content/renderer/media/webrtc/webrtc_media_stream_track_adapter_map.h"
#include "ipc/ipc_platform_file.h"
#include "third_party/blink/public/platform/web_media_stream_source.h"
#include "third_party/blink/public/platform/web_rtc_configuration.h"
#include "third_party/blink/public/platform/web_rtc_peer_connection_handler.h"
#include "third_party/blink/public/platform/web_rtc_stats_request.h"
#include "third_party/blink/public/platform/web_rtc_stats_response.h"

namespace blink {
class WebLocalFrame;
class WebRTCAnswerOptions;
class WebRTCDataChannelHandler;
class WebRTCLegacyStats;
class WebRTCOfferOptions;
class WebRTCPeerConnectionHandlerClient;
}  // namespace blink

namespace content {

class PeerConnectionDependencyFactory;
class PeerConnectionTracker;
class RtcDataChannelHandler;
class SetLocalDescriptionRequest;

// Mockable wrapper for blink::WebRTCStatsResponse
class CONTENT_EXPORT LocalRTCStatsResponse : public rtc::RefCountInterface {
 public:
  explicit LocalRTCStatsResponse(const blink::WebRTCStatsResponse& impl)
      : impl_(impl) {}

  virtual blink::WebRTCStatsResponse webKitStatsResponse() const;
  virtual void addStats(const blink::WebRTCLegacyStats& stats);

 protected:
  ~LocalRTCStatsResponse() override {}
  // Constructor for creating mocks.
  LocalRTCStatsResponse() {}

 private:
  blink::WebRTCStatsResponse impl_;
};

// Mockable wrapper for blink::WebRTCStatsRequest
class CONTENT_EXPORT LocalRTCStatsRequest : public rtc::RefCountInterface {
 public:
  explicit LocalRTCStatsRequest(blink::WebRTCStatsRequest impl);
  // Constructor for testing.
  LocalRTCStatsRequest();

  virtual bool hasSelector() const;
  virtual blink::WebMediaStreamTrack component() const;
  virtual void requestSucceeded(const LocalRTCStatsResponse* response);
  virtual scoped_refptr<LocalRTCStatsResponse> createResponse();

 protected:
  ~LocalRTCStatsRequest() override;

 private:
  blink::WebRTCStatsRequest impl_;
};

// RTCPeerConnectionHandler is a delegate for the RTC PeerConnection API
// messages going between WebKit and native PeerConnection in libjingle. It's
// owned by WebKit.
// WebKit calls all of these methods on the main render thread.
// Callbacks to the webrtc::PeerConnectionObserver implementation also occur on
// the main render thread.
class CONTENT_EXPORT RTCPeerConnectionHandler
    : public blink::WebRTCPeerConnectionHandler {
 public:
  RTCPeerConnectionHandler(
      blink::WebRTCPeerConnectionHandlerClient* client,
      PeerConnectionDependencyFactory* dependency_factory,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  ~RTCPeerConnectionHandler() override;

  void associateWithFrame(blink::WebLocalFrame* frame);

  // Initialize method only used for unit test.
  bool InitializeForTest(
      const blink::WebRTCConfiguration& server_configuration,
      const blink::WebMediaConstraints& options,
      const base::WeakPtr<PeerConnectionTracker>& peer_connection_tracker);

  // blink::WebRTCPeerConnectionHandler implementation
  bool Initialize(
      const blink::WebRTCConfiguration& server_configuration,
      const blink::WebMediaConstraints& options,
      blink::WebRTCSdpSemantics original_sdp_semantics_value) override;

  void CreateOffer(const blink::WebRTCSessionDescriptionRequest& request,
                   const blink::WebMediaConstraints& options) override;
  void CreateOffer(const blink::WebRTCSessionDescriptionRequest& request,
                   const blink::WebRTCOfferOptions& options) override;

  void CreateAnswer(const blink::WebRTCSessionDescriptionRequest& request,
                    const blink::WebMediaConstraints& options) override;
  void CreateAnswer(const blink::WebRTCSessionDescriptionRequest& request,
                    const blink::WebRTCAnswerOptions& options) override;

  void SetLocalDescription(
      const blink::WebRTCVoidRequest& request,
      const blink::WebRTCSessionDescription& description) override;
  void SetRemoteDescription(
      const blink::WebRTCVoidRequest& request,
      const blink::WebRTCSessionDescription& description) override;

  blink::WebRTCSessionDescription LocalDescription() override;
  blink::WebRTCSessionDescription RemoteDescription() override;

  webrtc::RTCErrorType SetConfiguration(
      const blink::WebRTCConfiguration& configuration) override;
  bool AddICECandidate(
      scoped_refptr<blink::WebRTCICECandidate> candidate) override;
  bool AddICECandidate(
      const blink::WebRTCVoidRequest& request,
      scoped_refptr<blink::WebRTCICECandidate> candidate) override;
  virtual void OnaddICECandidateResult(const blink::WebRTCVoidRequest& request,
                                       bool result);

  void GetStats(const blink::WebRTCStatsRequest& request) override;
  void GetStats(
      std::unique_ptr<blink::WebRTCStatsReportCallback> callback) override;
  webrtc::RTCErrorOr<std::unique_ptr<blink::WebRTCRtpTransceiver>>
  AddTransceiverWithTrack(const blink::WebMediaStreamTrack& web_track,
                          const webrtc::RtpTransceiverInit& init) override;
  webrtc::RTCErrorOr<std::unique_ptr<blink::WebRTCRtpTransceiver>>
  AddTransceiverWithKind(std::string kind,
                         const webrtc::RtpTransceiverInit& init) override;
  webrtc::RTCErrorOr<std::unique_ptr<blink::WebRTCRtpTransceiver>> AddTrack(
      const blink::WebMediaStreamTrack& web_track,
      const blink::WebVector<blink::WebMediaStream>& web_streams) override;
  webrtc::RTCErrorOr<std::unique_ptr<blink::WebRTCRtpTransceiver>> RemoveTrack(
      blink::WebRTCRtpSender* web_sender) override;

  blink::WebRTCDataChannelHandler* CreateDataChannel(
      const blink::WebString& label,
      const blink::WebRTCDataChannelInit& init) override;
  void Stop() override;
  void StopAndUnregister() override;

  blink::WebString Id() const override;

  // Delegate functions to allow for mocking of WebKit interfaces.
  // getStats takes ownership of request parameter.
  virtual void getStats(const scoped_refptr<LocalRTCStatsRequest>& request);

  // Asynchronously calls native_peer_connection_->getStats on the signaling
  // thread.
  void GetStats(webrtc::StatsObserver* observer,
                webrtc::PeerConnectionInterface::StatsOutputLevel level,
                rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> selector);

  // Tells the |client_| to close RTCPeerConnection.
  // Make it virtual for testing purpose.
  virtual void CloseClientPeerConnection();

  // Start recording an event log.
  void StartEventLog(IPC::PlatformFileForTransit file,
                     int64_t max_file_size_bytes);
  void StartEventLog();
  // Stop recording an event log.
  void StopEventLog();

  // WebRTC event log fragments sent back from PeerConnection land here.
  void OnWebRtcEventLogWrite(const std::string& output);

 protected:
  webrtc::PeerConnectionInterface* native_peer_connection() {
    return native_peer_connection_.get();
  }

  class Observer;
  friend class Observer;
  class WebRtcSetDescriptionObserverImpl;
  friend class WebRtcSetDescriptionObserverImpl;
  class SetLocalDescriptionRequest;
  friend class SetLocalDescriptionRequest;

  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state);
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state);
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state);
  void OnRenegotiationNeeded();
  void OnAddReceiverPlanB(RtpReceiverState receiver_state);
  void OnRemoveReceiverPlanB(uintptr_t receiver_id);
  void OnModifyTransceivers(std::vector<RtpTransceiverState> transceiver_states,
                            bool is_remote_description);
  void OnDataChannel(std::unique_ptr<RtcDataChannelHandler> handler);
  void OnIceCandidate(const std::string& sdp,
                      const std::string& sdp_mid,
                      int sdp_mline_index,
                      int component,
                      int address_family);

 private:
  // Record info about the first SessionDescription from the local and
  // remote side to record UMA stats once both are set.
  struct FirstSessionDescription {
    FirstSessionDescription(const webrtc::SessionDescriptionInterface* desc);

    bool audio = false;
    bool video = false;
    // If audio or video will use RTCP-mux (if there is no audio or
    // video, then false).
    bool rtcp_mux = false;
  };

  webrtc::SessionDescriptionInterface* CreateNativeSessionDescription(
      const std::string& sdp,
      const std::string& type,
      webrtc::SdpParseError* error);

  // Report to UMA whether an IceConnectionState has occurred. It only records
  // the first occurrence of a given state.
  void ReportICEState(
      webrtc::PeerConnectionInterface::IceConnectionState new_state);

  // Reset UMA related members to the initial state. This is invoked at the
  // constructor as well as after Ice Restart.
  void ResetUMAStats();

  void ReportFirstSessionDescriptions(const FirstSessionDescription& local,
                                      const FirstSessionDescription& remote);

  void AddTransceiverWithTrackOnSignalingThread(
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> webrtc_track,
      webrtc::RtpTransceiverInit init,
      TransceiverStateSurfacer* transceiver_state_surfacer,
      webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>*
          error_or_transceiver);
  void AddTransceiverWithMediaTypeOnSignalingThread(
      cricket::MediaType media_type,
      webrtc::RtpTransceiverInit init,
      TransceiverStateSurfacer* transceiver_state_surfacer,
      webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>*
          error_or_transceiver);
  void AddTrackOnSignalingThread(
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track,
      std::vector<std::string> stream_ids,
      TransceiverStateSurfacer* transceiver_state_surfacer,
      webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpSenderInterface>>*
          error_or_sender);
  bool RemoveTrackPlanB(blink::WebRTCRtpSender* web_sender);
  webrtc::RTCErrorOr<std::unique_ptr<blink::WebRTCRtpTransceiver>>
  RemoveTrackUnifiedPlan(blink::WebRTCRtpSender* web_sender);
  void RemoveTrackUnifiedPlanOnSignalingThread(
      rtc::scoped_refptr<webrtc::RtpSenderInterface> sender,
      TransceiverStateSurfacer* transceiver_state_surfacer,
      bool* result);
  std::vector<std::unique_ptr<RTCRtpSender>>::iterator FindSender(uintptr_t id);
  std::vector<std::unique_ptr<RTCRtpReceiver>>::iterator FindReceiver(
      uintptr_t id);
  std::vector<std::unique_ptr<RTCRtpTransceiver>>::iterator FindTransceiver(
      uintptr_t id);
  // For full transceiver implementations, returns the index of
  // |rtp_transceivers_| that correspond to |web_transceiver|.
  // For sender-only transceiver implementations, returns the index of
  // |rtp_senders_| that correspond to |web_transceiver.Sender()|.
  // For receiver-only transceiver implementations, returns the index of
  // |rtp_receivers_| that correspond to |web_transceiver.Receiver()|.
  // NOTREACHED()-crashes if no correspondent is found.
  size_t GetTransceiverIndex(
      const blink::WebRTCRtpTransceiver& web_transceiver);
  std::unique_ptr<RTCRtpTransceiver> CreateOrUpdateTransceiver(
      RtpTransceiverState transceiver_state);

  scoped_refptr<base::SingleThreadTaskRunner> signaling_thread() const;

  void RunSynchronousClosureOnSignalingThread(const base::Closure& closure,
                                              const char* trace_event_name);

  // Corresponds to the experimental RTCPeerConnection.id read-only attribute.
  const std::string id_;

  // Initialize() is never expected to be called more than once, even if the
  // first call fails.
  bool initialize_called_;

  // |client_| is a raw pointer to the blink object (blink::RTCPeerConnection)
  // that owns this object.
  // It is valid for the lifetime of this object, but is cleared when
  // StopAndUnregister() is called, in order to make sure it doesn't
  // interfere with garbage collection of the owner object.
  blink::WebRTCPeerConnectionHandlerClient* client_;
  // True if this PeerConnection has been closed.
  // After the PeerConnection has been closed, this object may no longer
  // forward callbacks to blink.
  bool is_closed_;
  // True if StopAndUnregister has been called.
  bool is_unregistered_;

  // |dependency_factory_| is a raw pointer, and is valid for the lifetime of
  // RenderThreadImpl.
  PeerConnectionDependencyFactory* const dependency_factory_;

  blink::WebLocalFrame* frame_ = nullptr;

  // Map and owners of track adapters. Every track that is in use by the peer
  // connection has an associated blink and webrtc layer representation of it.
  // The map keeps track of the relationship between
  // |blink::WebMediaStreamTrack|s and |webrtc::MediaStreamTrackInterface|s.
  // Track adapters are created on the fly when a component (such as a stream)
  // needs to reference it, and automatically disposed when there are no longer
  // any components referencing it.
  scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map_;
  // In Plan B, senders and receivers are added or removed independently of one
  // another. In Unified Plan, senders and receivers are created in pairs as
  // transceivers. Transceivers may become inactive, but are never removed.
  // The value of this member affects the behavior of some methods and what
  // information is surfaced from webrtc. After Initialize(), this is the actual
  // mode used, meaning "kDefault" is no longer a valid value.
  // TODO(hbos): Implement transceiver behaviors. https://crbug.com/777617
  blink::WebRTCSdpSemantics sdp_semantics_;
  // Content layer correspondents of |webrtc::RtpSenderInterface|.
  std::vector<std::unique_ptr<RTCRtpSender>> rtp_senders_;
  // Content layer correspondents of |webrtc::RtpReceiverInterface|.
  std::vector<std::unique_ptr<RTCRtpReceiver>> rtp_receivers_;
  // Content layer correspondents of |webrtc::RtpTransceiverInterface|.
  std::vector<std::unique_ptr<RTCRtpTransceiver>> rtp_transceivers_;

  base::WeakPtr<PeerConnectionTracker> peer_connection_tracker_;

  MediaStreamTrackMetrics track_metrics_;

  // Counter for a UMA stat reported at destruction time.
  int num_data_channels_created_ = 0;

  // Counter for number of IPv4 and IPv6 local candidates.
  int num_local_candidates_ipv4_ = 0;
  int num_local_candidates_ipv6_ = 0;

  // To make sure the observers are released after native_peer_connection_,
  // they have to come first.
  scoped_refptr<Observer> peer_connection_observer_;

  // |native_peer_connection_| is the libjingle native PeerConnection object.
  scoped_refptr<webrtc::PeerConnectionInterface> native_peer_connection_;

  // The last applied configuration. Used so that the constraints
  // used when constructing the PeerConnection carry over when
  // SetConfiguration is called.
  webrtc::PeerConnectionInterface::RTCConfiguration configuration_;

  // Record info about the first SessionDescription from the local and
  // remote side to record UMA stats once both are set.  We only check
  // for the first offer or answer.  "pranswer"s and "unknown"s (from
  // unit tests) are ignored.
  std::unique_ptr<FirstSessionDescription> first_local_description_;
  std::unique_ptr<FirstSessionDescription> first_remote_description_;

  base::TimeTicks ice_connection_checking_start_;

  // Track which ICE Connection state that this PeerConnection has gone through.
  bool ice_state_seen_[webrtc::PeerConnectionInterface::kIceConnectionMax] = {};

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  base::WeakPtrFactory<RTCPeerConnectionHandler> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(RTCPeerConnectionHandler);
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_WEBRTC_RTC_PEER_CONNECTION_HANDLER_H_
