// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/main/viz_compositor_thread_runner.h"

#include <utility>

#include "base/command_line.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread.h"
#include "base/trace_event/memory_dump_manager.h"
#include "base/trace_event/trace_event.h"
#include "build/build_config.h"
#include "components/viz/common/switches.h"
#include "components/viz/service/display_embedder/gpu_display_provider.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "ui/gfx/switches.h"

namespace viz {
namespace {

const char kThreadName[] = "VizCompositorThread";

std::unique_ptr<base::Thread> CreateAndStartCompositorThread() {
  auto thread = std::make_unique<base::Thread>(kThreadName);

  base::Thread::Options thread_options;
#if defined(OS_WIN)
  // Windows needs a UI message loop for child HWND. Other platforms can use the
  // default message loop type.
  thread_options.message_loop_type = base::MessageLoop::TYPE_UI;
#elif defined(OS_CHROMEOS)
  thread_options.priority = base::ThreadPriority::DISPLAY;
#endif

  CHECK(thread->StartWithOptions(thread_options));
  return thread;
}

}  // namespace

VizCompositorThreadRunner::VizCompositorThreadRunner()
    : thread_(CreateAndStartCompositorThread()),
      task_runner_(thread_->task_runner()) {}

VizCompositorThreadRunner::~VizCompositorThreadRunner() {
  task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&VizCompositorThreadRunner::TearDownOnCompositorThread,
                     base::Unretained(this)));
  thread_->Stop();
}

void VizCompositorThreadRunner::CreateFrameSinkManager(
    mojom::FrameSinkManagerParamsPtr params) {
  task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          &VizCompositorThreadRunner::CreateFrameSinkManagerOnCompositorThread,
          base::Unretained(this), std::move(params)));
}

void VizCompositorThreadRunner::CreateFrameSinkManagerOnCompositorThread(
    mojom::FrameSinkManagerParamsPtr params) {
  DCHECK(task_runner_->BelongsToCurrentThread());
  DCHECK(!frame_sink_manager_);

  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  server_shared_bitmap_manager_ = std::make_unique<ServerSharedBitmapManager>();
  base::trace_event::MemoryDumpManager::GetInstance()->RegisterDumpProvider(
      server_shared_bitmap_manager_.get(), "ServerSharedBitmapManager",
      base::ThreadTaskRunnerHandle::Get());

  // Create GpuDisplayProvider usable for software compositing only.
  display_provider_ = std::make_unique<GpuDisplayProvider>(
      params->restart_id, server_shared_bitmap_manager_.get(),
      command_line->HasSwitch(switches::kHeadless),
      command_line->HasSwitch(switches::kRunAllCompositorStagesBeforeDraw));

  base::Optional<uint32_t> activation_deadline_in_frames;
  if (params->use_activation_deadline)
    activation_deadline_in_frames = params->activation_deadline_in_frames;
  frame_sink_manager_ = std::make_unique<FrameSinkManagerImpl>(
      server_shared_bitmap_manager_.get(), activation_deadline_in_frames,
      display_provider_.get());
  frame_sink_manager_->BindAndSetClient(
      std::move(params->frame_sink_manager), nullptr,
      mojom::FrameSinkManagerClientPtr(
          std::move(params->frame_sink_manager_client)));
}

void VizCompositorThreadRunner::TearDownOnCompositorThread() {
  DCHECK(task_runner_->BelongsToCurrentThread());

  if (server_shared_bitmap_manager_) {
    base::trace_event::MemoryDumpManager::GetInstance()->UnregisterDumpProvider(
        server_shared_bitmap_manager_.get());
  }

  frame_sink_manager_.reset();
  display_provider_.reset();
  server_shared_bitmap_manager_.reset();
}

}  // namespace viz
