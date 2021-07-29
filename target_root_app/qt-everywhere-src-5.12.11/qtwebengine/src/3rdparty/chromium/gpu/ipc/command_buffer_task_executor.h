// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_IPC_COMMAND_BUFFER_TASK_EXECUTOR_H_
#define GPU_IPC_COMMAND_BUFFER_TASK_EXECUTOR_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "gpu/command_buffer/common/activity_flags.h"
#include "gpu/command_buffer/service/framebuffer_completeness_cache.h"
#include "gpu/command_buffer/service/image_manager.h"
#include "gpu/command_buffer/service/service_discardable_manager.h"
#include "gpu/command_buffer/service/shader_translator_cache.h"
#include "gpu/config/gpu_feature_info.h"
#include "gpu/config/gpu_preferences.h"
#include "gpu/ipc/gl_in_process_context_export.h"

namespace gl {
class GLShareGroup;
}

namespace gpu {
class MailboxManager;
class SyncPointManager;

namespace gles2 {
class Outputter;
class ProgramCache;
}  // namespace gles2

// Provides accessors for GPU service objects and the serializer interface to
// the GPU thread used by InProcessCommandBuffer.
class GL_IN_PROCESS_CONTEXT_EXPORT CommandBufferTaskExecutor
    : public base::RefCountedThreadSafe<CommandBufferTaskExecutor> {
 public:
  CommandBufferTaskExecutor(const GpuPreferences& gpu_preferences,
                            const GpuFeatureInfo& gpu_feature_info,
                            MailboxManager* mailbox_manager,
                            scoped_refptr<gl::GLShareGroup> share_group);

  // Queues a task to run as soon as possible.
  virtual void ScheduleTask(base::OnceClosure task) = 0;

  // Schedules |callback| to run at an appropriate time for performing delayed
  // work.
  virtual void ScheduleDelayedWork(base::OnceClosure task) = 0;

  // Always use virtualized GL contexts if this returns true.
  virtual bool ForceVirtualizedGLContexts() = 0;

  virtual bool BlockThreadOnWaitSyncToken() const = 0;
  virtual SyncPointManager* sync_point_manager() = 0;

  const GpuPreferences& gpu_preferences() const { return gpu_preferences_; }
  const GpuFeatureInfo& gpu_feature_info() const { return gpu_feature_info_; }
  scoped_refptr<gl::GLShareGroup> share_group();
  MailboxManager* mailbox_manager() { return mailbox_manager_; }
  gles2::Outputter* outputter();
  gles2::ProgramCache* program_cache();
  gles2::ImageManager* image_manager() { return &image_manager_; }
  ServiceDiscardableManager* discardable_manager() {
    return &discardable_manager_;
  }
  gles2::ShaderTranslatorCache* shader_translator_cache() {
    return &shader_translator_cache_;
  }
  gles2::FramebufferCompletenessCache* framebuffer_completeness_cache() {
    return &framebuffer_completeness_cache_;
  }

 protected:
  friend class base::RefCountedThreadSafe<CommandBufferTaskExecutor>;

  virtual ~CommandBufferTaskExecutor();

 private:
  const GpuPreferences gpu_preferences_;
  const GpuFeatureInfo gpu_feature_info_;
  std::unique_ptr<MailboxManager> owned_mailbox_manager_;
  MailboxManager* mailbox_manager_ = nullptr;
  std::unique_ptr<gles2::Outputter> outputter_;
  scoped_refptr<gl::GLShareGroup> share_group_;
  std::unique_ptr<gles2::ProgramCache> program_cache_;
  gles2::ImageManager image_manager_;
  ServiceDiscardableManager discardable_manager_;
  gles2::ShaderTranslatorCache shader_translator_cache_;
  gles2::FramebufferCompletenessCache framebuffer_completeness_cache_;

  // No-op default initialization is used in in-process mode.
  GpuProcessActivityFlags activity_flags_;

  DISALLOW_COPY_AND_ASSIGN(CommandBufferTaskExecutor);
};

}  // namespace gpu

#endif  // GPU_IPC_COMMAND_BUFFER_TASK_EXECUTOR_H_
