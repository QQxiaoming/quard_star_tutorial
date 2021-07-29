// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_WEBRTC_WEBRTC_SET_DESCRIPTION_OBSERVER_H_
#define CONTENT_RENDERER_MEDIA_WEBRTC_WEBRTC_SET_DESCRIPTION_OBSERVER_H_

#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "content/common/content_export.h"
#include "content/renderer/media/webrtc/rtc_peer_connection_handler.h"
#include "content/renderer/media/webrtc/rtc_rtp_receiver.h"
#include "content/renderer/media/webrtc/rtc_rtp_sender.h"
#include "content/renderer/media/webrtc/rtc_rtp_transceiver.h"
#include "content/renderer/media/webrtc/transceiver_state_surfacer.h"
#include "content/renderer/media/webrtc/webrtc_media_stream_track_adapter_map.h"
#include "third_party/webrtc/api/jsep.h"
#include "third_party/webrtc/api/peerconnectioninterface.h"
#include "third_party/webrtc/api/rtcerror.h"
#include "third_party/webrtc/api/rtpreceiverinterface.h"
#include "third_party/webrtc/api/setremotedescriptionobserverinterface.h"
#include "third_party/webrtc/rtc_base/refcount.h"
#include "third_party/webrtc/rtc_base/refcountedobject.h"
#include "third_party/webrtc/rtc_base/scoped_ref_ptr.h"

namespace content {

// The content layer correspondent of the setLocalDescription() observer
// (webrtc::SetSessionDescriptionObserver) and setRemoteDescription() observer
// (webrtc::SetRemoteDescriptionObserverInterface). The implementation should
// process the state changes of the Set[Local/Remote]Description() by inspecting
// the updated States.
class CONTENT_EXPORT WebRtcSetDescriptionObserver
    : public base::RefCountedThreadSafe<WebRtcSetDescriptionObserver> {
 public:
  // The states as they were when the operation finished on the webrtc signaling
  // thread. Note that other operations may have occurred while jumping back to
  // the main thread, but these must be handled separately.
  struct CONTENT_EXPORT States {
    States();
    States(States&& other);
    ~States();

    States& operator=(States&& other);

    webrtc::PeerConnectionInterface::SignalingState signaling_state;
    std::vector<RtpTransceiverState> transceiver_states;

    DISALLOW_COPY_AND_ASSIGN(States);
  };

  WebRtcSetDescriptionObserver();

  // Invoked in a PostTask() on the main thread after the SetLocalDescription()
  // or SetRemoteDescription() operation completed on the webrtc signaling
  // thread.
  virtual void OnSetDescriptionComplete(webrtc::RTCError error,
                                        States states) = 0;

 protected:
  friend class base::RefCountedThreadSafe<WebRtcSetDescriptionObserver>;
  virtual ~WebRtcSetDescriptionObserver();

  DISALLOW_COPY_AND_ASSIGN(WebRtcSetDescriptionObserver);
};

// Takes care of surfacing WebRtcSetDescriptionObserver::State information from
// the webrtc signaling thread to the main thread. With the state information
// obtained, invokes |observer_|'s
// WebRtcSetDescriptionObserver::OnSetDescriptionComplete() on the main thread.
//
// This implements the behavior
// of both WebRtcSetLocalDescriptionObserverHandler and
// WebRtcSetRemoteDescriptionObserverHandler, but these are put in different
// classes because local and remote description observers have different
// interfaces in webrtc.
class CONTENT_EXPORT WebRtcSetDescriptionObserverHandlerImpl
    : public base::RefCountedThreadSafe<
          WebRtcSetDescriptionObserverHandlerImpl> {
 public:
  WebRtcSetDescriptionObserverHandlerImpl(
      scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
      scoped_refptr<webrtc::PeerConnectionInterface> pc,
      scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
      scoped_refptr<WebRtcSetDescriptionObserver> observer,
      bool surface_receivers_only);

  // Must be called on the webrtc signaling thread internally by the handler
  // when the Set[Local/Remote]Description() operation finishes.
  void OnSetDescriptionComplete(webrtc::RTCError error);

 private:
  friend class base::RefCountedThreadSafe<
      WebRtcSetDescriptionObserverHandlerImpl>;
  virtual ~WebRtcSetDescriptionObserverHandlerImpl();

  void OnSetDescriptionCompleteOnMainThread(
      webrtc::RTCError error,
      webrtc::PeerConnectionInterface::SignalingState signaling_state,
      TransceiverStateSurfacer transceiver_state_surfacer);

  scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner_;
  scoped_refptr<webrtc::PeerConnectionInterface> pc_;
  scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map_;
  scoped_refptr<WebRtcSetDescriptionObserver> observer_;
  bool surface_receivers_only_;

  DISALLOW_COPY_AND_ASSIGN(WebRtcSetDescriptionObserverHandlerImpl);
};

// An implementation of webrtc::SetSessionDescriptionObserver for performing the
// operations of WebRtcSetDescriptionObserverHandlerImpl.
class CONTENT_EXPORT WebRtcSetLocalDescriptionObserverHandler
    : public webrtc::SetSessionDescriptionObserver {
 public:
  static scoped_refptr<WebRtcSetLocalDescriptionObserverHandler> Create(
      scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
      scoped_refptr<webrtc::PeerConnectionInterface> pc,
      scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
      scoped_refptr<WebRtcSetDescriptionObserver> observer,
      bool surface_receivers_only);

  // webrtc::SetSessionDescriptionObserver implementation. Implementation calls
  // WebRtcSetDescriptionObserverHandlerImpl::OnSetDescriptionComplete().
  void OnSuccess() override;
  void OnFailure(webrtc::RTCError error) override;

 protected:
  WebRtcSetLocalDescriptionObserverHandler(
      scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
      scoped_refptr<webrtc::PeerConnectionInterface> pc,
      scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
      scoped_refptr<WebRtcSetDescriptionObserver> observer,
      bool surface_receivers_only);
  ~WebRtcSetLocalDescriptionObserverHandler() override;

  scoped_refptr<WebRtcSetDescriptionObserverHandlerImpl> handler_impl_;

  DISALLOW_COPY_AND_ASSIGN(WebRtcSetLocalDescriptionObserverHandler);
};

// An implementation of webrtc::SetRemoteDescriptionObserverInterface for
// performing the operations of WebRtcSetDescriptionObserverHandlerImpl.
class CONTENT_EXPORT WebRtcSetRemoteDescriptionObserverHandler
    : public webrtc::SetRemoteDescriptionObserverInterface {
 public:
  static scoped_refptr<WebRtcSetRemoteDescriptionObserverHandler> Create(
      scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
      scoped_refptr<webrtc::PeerConnectionInterface> pc,
      scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
      scoped_refptr<WebRtcSetDescriptionObserver> observer,
      bool surface_receivers_only);

  // webrtc::SetRemoteDescriptionObserverInterface implementation.
  // Implementation calls
  // WebRtcSetDescriptionObserverHandlerImpl::OnSetDescriptionComplete().
  void OnSetRemoteDescriptionComplete(webrtc::RTCError error) override;

 protected:
  WebRtcSetRemoteDescriptionObserverHandler(
      scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner,
      scoped_refptr<webrtc::PeerConnectionInterface> pc,
      scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map,
      scoped_refptr<WebRtcSetDescriptionObserver> observer,
      bool surface_receivers_only);
  ~WebRtcSetRemoteDescriptionObserverHandler() override;

  scoped_refptr<WebRtcSetDescriptionObserverHandlerImpl> handler_impl_;

  DISALLOW_COPY_AND_ASSIGN(WebRtcSetRemoteDescriptionObserverHandler);
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_WEBRTC_WEBRTC_SET_DESCRIPTION_OBSERVER_H_
