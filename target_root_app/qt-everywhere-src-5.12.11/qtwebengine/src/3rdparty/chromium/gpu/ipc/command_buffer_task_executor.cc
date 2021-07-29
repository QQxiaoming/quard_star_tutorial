// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/ipc/command_buffer_task_executor.h"

#include "gpu/command_buffer/service/gpu_tracer.h"
#include "gpu/command_buffer/service/mailbox_manager_factory.h"
#include "gpu/command_buffer/service/memory_program_cache.h"
#include "gpu/command_buffer/service/program_cache.h"
#include "ui/gl/gl_share_group.h"

namespace gpu {

CommandBufferTaskExecutor::CommandBufferTaskExecutor(
    const GpuPreferences& gpu_preferences,
    const GpuFeatureInfo& gpu_feature_info,
    MailboxManager* mailbox_manager,
    scoped_refptr<gl::GLShareGroup> share_group)
    : gpu_preferences_(gpu_preferences),
      gpu_feature_info_(gpu_feature_info),
      mailbox_manager_(mailbox_manager),
      share_group_(share_group),
      shader_translator_cache_(gpu_preferences_) {
  if (!mailbox_manager_) {
    // TODO(piman): have embedders own the mailbox manager.
    owned_mailbox_manager_ = gles2::CreateMailboxManager(gpu_preferences_);
    mailbox_manager_ = owned_mailbox_manager_.get();
  }
}

CommandBufferTaskExecutor::~CommandBufferTaskExecutor() = default;

scoped_refptr<gl::GLShareGroup> CommandBufferTaskExecutor::share_group() {
  if (!share_group_)
    share_group_ = base::MakeRefCounted<gl::GLShareGroup>();
  return share_group_;
}

gles2::Outputter* CommandBufferTaskExecutor::outputter() {
  if (!outputter_) {
    outputter_ =
        std::make_unique<gles2::TraceOutputter>("InProcessCommandBuffer Trace");
  }
  return outputter_.get();
}

gles2::ProgramCache* CommandBufferTaskExecutor::program_cache() {
  if (!program_cache_ &&
      (gl::g_current_gl_driver->ext.b_GL_ARB_get_program_binary ||
       gl::g_current_gl_driver->ext.b_GL_OES_get_program_binary) &&
      !gpu_preferences().disable_gpu_program_cache) {
    bool disable_disk_cache =
        gpu_preferences_.disable_gpu_shader_disk_cache ||
        gpu_feature_info_.IsWorkaroundEnabled(gpu::DISABLE_PROGRAM_DISK_CACHE);
    program_cache_ = std::make_unique<gles2::MemoryProgramCache>(
        gpu_preferences_.gpu_program_cache_size, disable_disk_cache,
        gpu_feature_info_.IsWorkaroundEnabled(
            gpu::DISABLE_PROGRAM_CACHING_FOR_TRANSFORM_FEEDBACK),
        &activity_flags_);
  }
  return program_cache_.get();
}

}  // namespace gpu
