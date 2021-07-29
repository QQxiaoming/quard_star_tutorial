// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/render_frame_metadata_observer_impl.h"

#include "base/run_loop.h"
#include "base/test/scoped_task_environment.h"
#include "build/build_config.h"
#include "cc/trees/render_frame_metadata.h"
#include "components/viz/common/quads/compositor_frame_metadata.h"
#include "content/common/render_frame_metadata.mojom.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {
namespace {

ACTION_P(InvokeClosure, closure) {
  closure.Run();
}

}  // namespace

class MockRenderFrameMetadataObserverClient
    : public mojom::RenderFrameMetadataObserverClient {
 public:
  MockRenderFrameMetadataObserverClient(
      mojom::RenderFrameMetadataObserverClientRequest client_request,
      mojom::RenderFrameMetadataObserverPtr observer)
      : render_frame_metadata_observer_client_binding_(
            this,
            std::move(client_request)),
        render_frame_metadata_observer_ptr_(std::move(observer)) {}

  MOCK_METHOD2(OnRenderFrameMetadataChanged,
               void(uint32_t frame_token,
                    const cc::RenderFrameMetadata& metadata));
  MOCK_METHOD1(OnFrameSubmissionForTesting, void(uint32_t frame_token));

 private:
  mojo::Binding<mojom::RenderFrameMetadataObserverClient>
      render_frame_metadata_observer_client_binding_;
  mojom::RenderFrameMetadataObserverPtr render_frame_metadata_observer_ptr_;

  DISALLOW_COPY_AND_ASSIGN(MockRenderFrameMetadataObserverClient);
};

class RenderFrameMetadataObserverImplTest : public testing::Test {
 public:
  RenderFrameMetadataObserverImplTest() = default;
  ~RenderFrameMetadataObserverImplTest() override = default;

  RenderFrameMetadataObserverImpl& observer_impl() { return *observer_impl_; }

  MockRenderFrameMetadataObserverClient& client() { return *client_; }

  // testing::Test:
  void SetUp() override {
    mojom::RenderFrameMetadataObserverPtr ptr;
    mojom::RenderFrameMetadataObserverRequest request = mojo::MakeRequest(&ptr);
    mojom::RenderFrameMetadataObserverClientPtrInfo client_info;
    mojom::RenderFrameMetadataObserverClientRequest client_request =
        mojo::MakeRequest(&client_info);

    client_ = std::make_unique<
        testing::NiceMock<MockRenderFrameMetadataObserverClient>>(
        std::move(client_request), std::move(ptr));
    observer_impl_ = std::make_unique<RenderFrameMetadataObserverImpl>(
        std::move(request), std::move(client_info));
    observer_impl_->BindToCurrentThread();
  }

  void TearDown() override {
    observer_impl_.reset();
    client_.reset();
    scoped_task_environment_.RunUntilIdle();
  }

 private:
  base::test::ScopedTaskEnvironment scoped_task_environment_;
  std::unique_ptr<testing::NiceMock<MockRenderFrameMetadataObserverClient>>
      client_;
  std::unique_ptr<RenderFrameMetadataObserverImpl> observer_impl_;

  DISALLOW_COPY_AND_ASSIGN(RenderFrameMetadataObserverImplTest);
};

// This test verifies that the RenderFrameMetadataObserverImpl picks up
// the frame token from CompositorFrameMetadata and passes it along to the
// client. This test also verifies that the RenderFrameMetadata object is
// passed along to the client.
TEST_F(RenderFrameMetadataObserverImplTest, ShouldSendFrameToken) {
  viz::CompositorFrameMetadata compositor_frame_metadata;
  compositor_frame_metadata.send_frame_token_to_embedder = false;
  compositor_frame_metadata.frame_token = 1337;
  cc::RenderFrameMetadata render_frame_metadata;
  render_frame_metadata.is_mobile_optimized = true;
  observer_impl().OnRenderFrameSubmission(render_frame_metadata,
                                          &compositor_frame_metadata);
  // |is_mobile_optimized| should be synchronized with frame activation so
  // RenderFrameMetadataObserverImpl should ask for the frame token from
  // Viz.
  EXPECT_TRUE(compositor_frame_metadata.send_frame_token_to_embedder);
  {
    base::RunLoop run_loop;
    EXPECT_CALL(client(),
                OnRenderFrameMetadataChanged(1337, render_frame_metadata))
        .WillOnce(InvokeClosure(run_loop.QuitClosure()));
    run_loop.Run();
  }
}

// This test verifies that a frame token is not requested from viz when
// the root scroll offset changes on Android.
#if defined(OS_ANDROID)
TEST_F(RenderFrameMetadataObserverImplTest, ShouldSendFrameTokenOnAndroid) {
  viz::CompositorFrameMetadata compositor_frame_metadata;
  compositor_frame_metadata.send_frame_token_to_embedder = false;
  compositor_frame_metadata.frame_token = 1337;
  cc::RenderFrameMetadata render_frame_metadata;
  render_frame_metadata.root_scroll_offset = gfx::Vector2dF(0.f, 1.f);
  render_frame_metadata.root_layer_size = gfx::SizeF(100.f, 100.f);
  render_frame_metadata.scrollable_viewport_size = gfx::SizeF(100.f, 50.f);
  observer_impl().OnRenderFrameSubmission(render_frame_metadata,
                                          &compositor_frame_metadata);
  // The first RenderFrameMetadata will always get a corresponding frame token
  // from Viz because this is the first frame.
  EXPECT_TRUE(compositor_frame_metadata.send_frame_token_to_embedder);
  {
    base::RunLoop run_loop;
    EXPECT_CALL(client(),
                OnRenderFrameMetadataChanged(1337, render_frame_metadata))
        .WillOnce(InvokeClosure(run_loop.QuitClosure()));
    run_loop.Run();
  }

  // Scroll back to the top.
  render_frame_metadata.root_scroll_offset = gfx::Vector2dF(0.f, 0.f);

  observer_impl().OnRenderFrameSubmission(render_frame_metadata,
                                          &compositor_frame_metadata);
  // Android does not need a corresponding frame token.
  EXPECT_FALSE(compositor_frame_metadata.send_frame_token_to_embedder);
  {
    base::RunLoop run_loop;
    // The 0u frame token indicates that the client should not expect
    // a corresponding frame token from Viz.
    EXPECT_CALL(client(),
                OnRenderFrameMetadataChanged(0u, render_frame_metadata))
        .WillOnce(InvokeClosure(run_loop.QuitClosure()));
    run_loop.Run();
  }
}
#endif

}  // namespace content
