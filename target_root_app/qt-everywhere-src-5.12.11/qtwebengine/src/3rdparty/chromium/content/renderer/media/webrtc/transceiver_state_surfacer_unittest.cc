// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/webrtc/transceiver_state_surfacer.h"

#include <memory>
#include <tuple>

#include "base/memory/ref_counted.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "base/test/scoped_task_environment.h"
#include "content/child/child_process.h"
#include "content/renderer/media/stream/media_stream_audio_source.h"
#include "content/renderer/media/webrtc/mock_peer_connection_dependency_factory.h"
#include "content/renderer/media/webrtc/mock_peer_connection_impl.h"
#include "content/renderer/media/webrtc/webrtc_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/platform/scheduler/test/renderer_scheduler_test_support.h"
#include "third_party/blink/public/platform/web_media_stream_source.h"
#include "third_party/blink/public/platform/web_media_stream_track.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_heap.h"

namespace content {

class TransceiverStateSurfacerTest : public ::testing::Test {
 public:
  void SetUp() override {
    dependency_factory_.reset(new MockPeerConnectionDependencyFactory());
    main_task_runner_ = blink::scheduler::GetSingleThreadTaskRunnerForTesting();
    track_adapter_map_ = new WebRtcMediaStreamTrackAdapterMap(
        dependency_factory_.get(), main_task_runner_);
    surfacer_.reset(new TransceiverStateSurfacer(main_task_runner_,
                                                 signaling_task_runner()));
  }

  void TearDown() override {
    // Make sure posted tasks get a chance to execute or else the stuff is
    // teared down while things are in flight.
    base::RunLoop().RunUntilIdle();
    blink::WebHeap::CollectAllGarbageForTesting();
  }

  scoped_refptr<base::SingleThreadTaskRunner> signaling_task_runner() const {
    return dependency_factory_->GetWebRtcSignalingThread();
  }

  std::unique_ptr<WebRtcMediaStreamTrackAdapterMap::AdapterRef>
  CreateLocalTrackAndAdapter(const std::string& id) {
    return track_adapter_map_->GetOrCreateLocalTrackAdapter(
        CreateBlinkLocalTrack(id));
  }

  rtc::scoped_refptr<webrtc::RtpTransceiverInterface> CreateWebRtcTransceiver(
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> local_track,
      const std::string& local_stream_id,
      const std::string& remote_track_id,
      const std::string& remote_stream_id) {
    return new rtc::RefCountedObject<FakeRtpTransceiver>(
        local_track->kind() == webrtc::MediaStreamTrackInterface::kAudioKind
            ? cricket::MEDIA_TYPE_AUDIO
            : cricket::MEDIA_TYPE_VIDEO,
        CreateWebRtcSender(local_track, local_stream_id),
        CreateWebRtcReceiver(remote_track_id, remote_stream_id), base::nullopt,
        false, webrtc::RtpTransceiverDirection::kSendRecv, base::nullopt);
  }

  rtc::scoped_refptr<webrtc::RtpSenderInterface> CreateWebRtcSender(
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track,
      const std::string& stream_id) {
    return new rtc::RefCountedObject<FakeRtpSender>(
        std::move(track), std::vector<std::string>({stream_id}));
  }

  rtc::scoped_refptr<webrtc::RtpReceiverInterface> CreateWebRtcReceiver(
      const std::string& track_id,
      const std::string& stream_id) {
    rtc::scoped_refptr<webrtc::AudioTrackInterface> remote_track =
        MockWebRtcAudioTrack::Create(track_id).get();
    rtc::scoped_refptr<webrtc::MediaStreamInterface> remote_stream(
        new rtc::RefCountedObject<MockMediaStream>(stream_id));
    return new rtc::RefCountedObject<FakeRtpReceiver>(
        remote_track.get(),
        std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>(
            {remote_stream}));
  }

  // Initializes the surfacer on the signaling thread and signals the waitable
  // event when done. The WaitableEvent's Wait() blocks the main thread until
  // initialization occurs.
  std::unique_ptr<base::WaitableEvent> AsyncInitializeSurfacerWithWaitableEvent(
      std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
          transceivers) {
    std::unique_ptr<base::WaitableEvent> waitable_event(new base::WaitableEvent(
        base::WaitableEvent::ResetPolicy::MANUAL,
        base::WaitableEvent::InitialState::NOT_SIGNALED));
    signaling_task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(
            &TransceiverStateSurfacerTest::
                AsyncInitializeSurfacerWithWaitableEventOnSignalingThread,
            base::Unretained(this), std::move(transceivers),
            waitable_event.get()));
    return waitable_event;
  }

  // Initializes the surfacer on the signaling thread and posts back to the main
  // thread to execute the callback when done. The RunLoop quits after the
  // callback is executed. Use the RunLoop's Run() method to allow the posted
  // task (such as the callback) to be executed while waiting. The caller must
  // let the loop Run() before destroying it.
  std::unique_ptr<base::RunLoop> AsyncInitializeSurfacerWithCallback(
      std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
          transceivers,
      base::OnceCallback<void()> callback) {
    std::unique_ptr<base::RunLoop> run_loop(new base::RunLoop());
    signaling_task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&TransceiverStateSurfacerTest::
                           AsyncInitializeSurfacerWithCallbackOnSignalingThread,
                       base::Unretained(this), std::move(transceivers),
                       std::move(callback), run_loop.get()));
    return run_loop;
  }

  void ObtainStatesAndExpectInitialized(
      rtc::scoped_refptr<webrtc::RtpTransceiverInterface> webrtc_transceiver) {
    auto transceiver_states = surfacer_->ObtainStates();
    EXPECT_EQ(1u, transceiver_states.size());
    auto& transceiver_state = transceiver_states[0];
    EXPECT_EQ(transceiver_state.webrtc_transceiver().get(),
              webrtc_transceiver.get());
    // Inspect sender states.
    const auto& sender_state = transceiver_state.sender_state();
    EXPECT_TRUE(sender_state);
    EXPECT_TRUE(sender_state->is_initialized());
    const auto& webrtc_sender = webrtc_transceiver->sender();
    EXPECT_EQ(sender_state->webrtc_sender().get(), webrtc_sender.get());
    EXPECT_TRUE(sender_state->track_ref()->is_initialized());
    EXPECT_EQ(sender_state->track_ref()->webrtc_track(),
              webrtc_sender->track().get());
    EXPECT_EQ(sender_state->stream_ids(), webrtc_sender->stream_ids());
    // Inspect receiver states.
    const auto& receiver_state = transceiver_state.receiver_state();
    EXPECT_TRUE(receiver_state);
    EXPECT_TRUE(receiver_state->is_initialized());
    const auto& webrtc_receiver = webrtc_transceiver->receiver();
    EXPECT_EQ(receiver_state->webrtc_receiver().get(), webrtc_receiver.get());
    EXPECT_TRUE(receiver_state->track_ref()->is_initialized());
    EXPECT_EQ(receiver_state->track_ref()->webrtc_track(),
              webrtc_receiver->track().get());
    std::vector<std::string> receiver_stream_ids;
    for (const auto& stream : webrtc_receiver->streams()) {
      receiver_stream_ids.push_back(stream->id());
    }
    EXPECT_EQ(receiver_state->stream_ids(), receiver_stream_ids);
    // Inspect transceiver states.
    EXPECT_TRUE(
        OptionalEquals(transceiver_state.mid(), webrtc_transceiver->mid()));
    EXPECT_EQ(transceiver_state.stopped(), webrtc_transceiver->stopped());
    EXPECT_TRUE(transceiver_state.direction() ==
                webrtc_transceiver->direction());
    EXPECT_TRUE(OptionalEquals(transceiver_state.current_direction(),
                               webrtc_transceiver->current_direction()));
  }

 private:
  blink::WebMediaStreamTrack CreateBlinkLocalTrack(const std::string& id) {
    blink::WebMediaStreamSource web_source;
    web_source.Initialize(
        blink::WebString::FromUTF8(id), blink::WebMediaStreamSource::kTypeAudio,
        blink::WebString::FromUTF8("local_audio_track"), false);
    MediaStreamAudioSource* audio_source = new MediaStreamAudioSource(true);
    // Takes ownership of |audio_source|.
    web_source.SetExtraData(audio_source);

    blink::WebMediaStreamTrack web_track;
    web_track.Initialize(web_source.Id(), web_source);
    audio_source->ConnectToTrack(web_track);
    return web_track;
  }

  void AsyncInitializeSurfacerWithWaitableEventOnSignalingThread(
      std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
          transceivers,
      base::WaitableEvent* waitable_event) {
    DCHECK(signaling_task_runner()->BelongsToCurrentThread());
    surfacer_->Initialize(track_adapter_map_, std::move(transceivers));
    waitable_event->Signal();
  }

  void AsyncInitializeSurfacerWithCallbackOnSignalingThread(
      std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
          transceivers,
      base::OnceCallback<void()> callback,
      base::RunLoop* run_loop) {
    DCHECK(signaling_task_runner()->BelongsToCurrentThread());
    surfacer_->Initialize(track_adapter_map_, std::move(transceivers));
    main_task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&TransceiverStateSurfacerTest::
                           AsyncInitializeSurfacerWithCallbackOnMainThread,
                       base::Unretained(this), std::move(callback), run_loop));
  }

  void AsyncInitializeSurfacerWithCallbackOnMainThread(
      base::OnceCallback<void()> callback,
      base::RunLoop* run_loop) {
    DCHECK(main_task_runner_->BelongsToCurrentThread());
    DCHECK(surfacer_->is_initialized());
    std::move(callback).Run();
    run_loop->Quit();
  }

  base::test::ScopedTaskEnvironment scoped_task_environment_;

 protected:
  std::unique_ptr<MockPeerConnectionDependencyFactory> dependency_factory_;
  scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;
  scoped_refptr<WebRtcMediaStreamTrackAdapterMap> track_adapter_map_;
  std::unique_ptr<TransceiverStateSurfacer> surfacer_;
};

TEST_F(TransceiverStateSurfacerTest, SurfaceTransceiverBlockingly) {
  auto local_track_adapter = CreateLocalTrackAndAdapter("local_track");
  auto webrtc_transceiver =
      CreateWebRtcTransceiver(local_track_adapter->webrtc_track(),
                              "local_stream", "remote_track", "remote_stream");
  auto waitable_event =
      AsyncInitializeSurfacerWithWaitableEvent({webrtc_transceiver});
  waitable_event->Wait();
  ObtainStatesAndExpectInitialized(webrtc_transceiver);
}

TEST_F(TransceiverStateSurfacerTest, SurfaceTransceiverInCallback) {
  auto local_track_adapter = CreateLocalTrackAndAdapter("local_track");
  auto webrtc_transceiver =
      CreateWebRtcTransceiver(local_track_adapter->webrtc_track(),
                              "local_stream", "remote_track", "remote_stream");
  auto run_loop = AsyncInitializeSurfacerWithCallback(
      {webrtc_transceiver},
      base::BindOnce(
          &TransceiverStateSurfacerTest::ObtainStatesAndExpectInitialized,
          base::Unretained(this), webrtc_transceiver));
  run_loop->Run();
}

TEST_F(TransceiverStateSurfacerTest, SurfaceSenderStateOnly) {
  auto local_track_adapter = CreateLocalTrackAndAdapter("local_track");
  auto webrtc_sender =
      CreateWebRtcSender(local_track_adapter->webrtc_track(), "local_stream");
  auto waitable_event = AsyncInitializeSurfacerWithWaitableEvent(
      {new SurfaceSenderStateOnly(webrtc_sender)});
  waitable_event->Wait();
  auto transceiver_states = surfacer_->ObtainStates();
  EXPECT_EQ(1u, transceiver_states.size());
  auto& transceiver_state = transceiver_states[0];
  EXPECT_TRUE(transceiver_state.sender_state());
  EXPECT_TRUE(transceiver_state.sender_state()->is_initialized());
  EXPECT_FALSE(transceiver_state.receiver_state());
  // Expect transceiver members be be missing for optional members and have
  // sensible values for non-optional members.
  EXPECT_FALSE(transceiver_state.mid());
  EXPECT_FALSE(transceiver_state.stopped());
  EXPECT_EQ(transceiver_state.direction(),
            webrtc::RtpTransceiverDirection::kSendOnly);
  EXPECT_FALSE(transceiver_state.current_direction());
}

TEST_F(TransceiverStateSurfacerTest, SurfaceReceiverStateOnly) {
  auto local_track_adapter = CreateLocalTrackAndAdapter("local_track");
  auto webrtc_receiver = CreateWebRtcReceiver("remote_track", "remote_stream");
  auto waitable_event = AsyncInitializeSurfacerWithWaitableEvent(
      {new SurfaceReceiverStateOnly(webrtc_receiver)});
  waitable_event->Wait();
  auto transceiver_states = surfacer_->ObtainStates();
  EXPECT_EQ(1u, transceiver_states.size());
  auto& transceiver_state = transceiver_states[0];
  EXPECT_FALSE(transceiver_state.sender_state());
  EXPECT_TRUE(transceiver_state.receiver_state());
  EXPECT_TRUE(transceiver_state.receiver_state()->is_initialized());
  // Expect transceiver members be be missing for optional members and have
  // sensible values for non-optional members.
  EXPECT_FALSE(transceiver_state.mid());
  EXPECT_FALSE(transceiver_state.stopped());
  EXPECT_EQ(transceiver_state.direction(),
            webrtc::RtpTransceiverDirection::kRecvOnly);
  EXPECT_FALSE(transceiver_state.current_direction());
}

}  // namespace content
