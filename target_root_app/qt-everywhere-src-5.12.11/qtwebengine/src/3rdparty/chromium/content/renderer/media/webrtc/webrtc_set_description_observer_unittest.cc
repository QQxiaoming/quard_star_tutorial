// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/webrtc/webrtc_set_description_observer.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/optional.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/thread_task_runner_handle.h"
#include "content/child/child_process.h"
#include "content/renderer/media/webrtc/mock_peer_connection_dependency_factory.h"
#include "content/renderer/media/webrtc/mock_peer_connection_impl.h"
#include "content/renderer/media/webrtc/webrtc_media_stream_track_adapter_map.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/platform/scheduler/test/renderer_scheduler_test_support.h"
#include "third_party/blink/public/web/web_heap.h"
#include "third_party/webrtc/api/peerconnectioninterface.h"
#include "third_party/webrtc/media/base/fakemediaengine.h"
#include "third_party/webrtc/pc/test/mock_peerconnection.h"

using ::testing::Return;

namespace content {

class WebRtcSetDescriptionObserverForTest
    : public WebRtcSetDescriptionObserver {
 public:
  bool called() const { return called_; }

  const WebRtcSetDescriptionObserver::States& states() const {
    DCHECK(called_);
    return states_;
  }
  const webrtc::RTCError& error() const {
    DCHECK(called_);
    return error_;
  }

  // WebRtcSetDescriptionObserver implementation.
  void OnSetDescriptionComplete(
      webrtc::RTCError error,
      WebRtcSetDescriptionObserver::States states) override {
    called_ = true;
    error_ = std::move(error);
    states_ = std::move(states);
  }

 private:
  ~WebRtcSetDescriptionObserverForTest() override {}

  bool called_ = false;
  webrtc::RTCError error_;
  WebRtcSetDescriptionObserver::States states_;
};

// TODO(hbos): This only tests WebRtcSetRemoteDescriptionObserverHandler,
// parameterize the test to make it also test
// WebRtcSetLocalDescriptionObserverHandler and with "surface_receivers_only" as
// both true and false. https://crbug.com/865006
class WebRtcSetRemoteDescriptionObserverHandlerTest : public ::testing::Test {
 public:
  void SetUp() override {
    pc_ = new webrtc::MockPeerConnection(new webrtc::FakePeerConnectionFactory(
        std::unique_ptr<cricket::MediaEngineInterface>(
            new cricket::FakeMediaEngine())));
    dependency_factory_.reset(new MockPeerConnectionDependencyFactory());
    main_thread_ = blink::scheduler::GetSingleThreadTaskRunnerForTesting();
    scoped_refptr<WebRtcMediaStreamTrackAdapterMap> map =
        new WebRtcMediaStreamTrackAdapterMap(dependency_factory_.get(),
                                             main_thread_);
    observer_ = new WebRtcSetDescriptionObserverForTest();
    observer_handler_ = WebRtcSetRemoteDescriptionObserverHandler::Create(
        main_thread_, dependency_factory_->GetWebRtcSignalingThread(), pc_, map,
        observer_, true /* surface_receivers_only*/);
  }

  void TearDown() override { blink::WebHeap::CollectAllGarbageForTesting(); }

  void InvokeOnSetRemoteDescriptionComplete(webrtc::RTCError error) {
    base::RunLoop run_loop;
    dependency_factory_->GetWebRtcSignalingThread()->PostTask(
        FROM_HERE,
        base::BindOnce(
            &WebRtcSetRemoteDescriptionObserverHandlerTest::
                InvokeOnSetRemoteDescriptionCompleteOnSignalingThread,
            base::Unretained(this), std::move(error),
            base::Unretained(&run_loop)));
    run_loop.Run();
  }

 protected:
  void InvokeOnSetRemoteDescriptionCompleteOnSignalingThread(
      webrtc::RTCError error,
      base::RunLoop* run_loop) {
    observer_handler_->OnSetRemoteDescriptionComplete(std::move(error));
    run_loop->Quit();
  }

  // The ScopedTaskEnvironment prevents the ChildProcess from leaking a
  // TaskScheduler.
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  ChildProcess child_process_;

  scoped_refptr<webrtc::MockPeerConnection> pc_;
  std::unique_ptr<MockPeerConnectionDependencyFactory> dependency_factory_;
  scoped_refptr<base::SingleThreadTaskRunner> main_thread_;
  scoped_refptr<WebRtcSetDescriptionObserverForTest> observer_;
  scoped_refptr<WebRtcSetRemoteDescriptionObserverHandler> observer_handler_;

  std::vector<rtc::scoped_refptr<webrtc::RtpReceiverInterface>> receivers_;
};

TEST_F(WebRtcSetRemoteDescriptionObserverHandlerTest, OnSuccess) {
  scoped_refptr<MockWebRtcAudioTrack> added_track =
      MockWebRtcAudioTrack::Create("added_track");
  scoped_refptr<webrtc::MediaStreamInterface> added_stream(
      new rtc::RefCountedObject<MockMediaStream>("added_stream"));
  scoped_refptr<webrtc::RtpReceiverInterface> added_receiver(
      new rtc::RefCountedObject<FakeRtpReceiver>(
          added_track.get(),
          std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>(
              {added_stream.get()})));

  receivers_.push_back(added_receiver.get());
  EXPECT_CALL(*pc_, signaling_state())
      .WillRepeatedly(Return(webrtc::PeerConnectionInterface::kStable));
  EXPECT_CALL(*pc_, GetReceivers()).WillRepeatedly(Return(receivers_));

  InvokeOnSetRemoteDescriptionComplete(webrtc::RTCError::OK());
  EXPECT_TRUE(observer_->called());
  EXPECT_TRUE(observer_->error().ok());

  EXPECT_EQ(webrtc::PeerConnectionInterface::kStable,
            observer_->states().signaling_state);

  EXPECT_EQ(1u, observer_->states().transceiver_states.size());
  const RtpTransceiverState& transceiver_state =
      observer_->states().transceiver_states[0];
  EXPECT_FALSE(transceiver_state.sender_state());
  EXPECT_TRUE(transceiver_state.receiver_state());
  const RtpReceiverState& receiver_state = *transceiver_state.receiver_state();
  EXPECT_EQ(added_receiver, receiver_state.webrtc_receiver());
  EXPECT_EQ(added_track, receiver_state.track_ref()->webrtc_track());
  EXPECT_EQ(1u, receiver_state.stream_ids().size());
  EXPECT_EQ(added_stream->id(), receiver_state.stream_ids()[0]);
}

TEST_F(WebRtcSetRemoteDescriptionObserverHandlerTest, OnFailure) {
  scoped_refptr<MockWebRtcAudioTrack> added_track =
      MockWebRtcAudioTrack::Create("added_track");
  scoped_refptr<webrtc::MediaStreamInterface> added_stream(
      new rtc::RefCountedObject<MockMediaStream>("added_stream"));
  scoped_refptr<webrtc::RtpReceiverInterface> added_receiver(
      new rtc::RefCountedObject<FakeRtpReceiver>(
          added_track.get(),
          std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>(
              {added_stream.get()})));

  receivers_.push_back(added_receiver.get());
  EXPECT_CALL(*pc_, signaling_state())
      .WillRepeatedly(Return(webrtc::PeerConnectionInterface::kStable));
  EXPECT_CALL(*pc_, GetReceivers()).WillRepeatedly(Return(receivers_));

  InvokeOnSetRemoteDescriptionComplete(
      webrtc::RTCError(webrtc::RTCErrorType::INVALID_PARAMETER, "Oh noes!"));
  EXPECT_TRUE(observer_->called());
  EXPECT_FALSE(observer_->error().ok());
  EXPECT_EQ(std::string("Oh noes!"), observer_->error().message());

  // Verify states were surfaced even though we got an error.
  EXPECT_EQ(webrtc::PeerConnectionInterface::kStable,
            observer_->states().signaling_state);

  EXPECT_EQ(1u, observer_->states().transceiver_states.size());
  const RtpTransceiverState& transceiver_state =
      observer_->states().transceiver_states[0];
  EXPECT_FALSE(transceiver_state.sender_state());
  EXPECT_TRUE(transceiver_state.receiver_state());
  const RtpReceiverState& receiver_state = *transceiver_state.receiver_state();
  EXPECT_EQ(added_receiver, receiver_state.webrtc_receiver());
  EXPECT_EQ(added_track, receiver_state.track_ref()->webrtc_track());
  EXPECT_EQ(1u, receiver_state.stream_ids().size());
  EXPECT_EQ(added_stream->id(), receiver_state.stream_ids()[0]);
}

}  // namespace content
