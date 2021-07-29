// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_HOST_HOST_GPU_MEMORY_BUFFER_MANAGER_H_
#define COMPONENTS_VIZ_HOST_HOST_GPU_MEMORY_BUFFER_MANAGER_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "base/trace_event/memory_dump_provider.h"
#include "components/viz/host/viz_host_export.h"
#include "gpu/command_buffer/client/gpu_memory_buffer_manager.h"
#include "gpu/ipc/host/gpu_memory_buffer_support.h"

namespace gpu {
class GpuMemoryBufferSupport;
}

namespace viz {

namespace mojom {
class GpuService;
}

// This GpuMemoryBufferManager implementation is for [de]allocating GPU memory
// from the GPU process over the mojom.GpuService api.
class VIZ_HOST_EXPORT HostGpuMemoryBufferManager
    : public gpu::GpuMemoryBufferManager,
      public base::trace_event::MemoryDumpProvider {
 public:
  // All function of HostGpuMemoryBufferManager must be called the thread
  // associated with |task_runner|, other than the constructor and the
  // gpu::GpuMemoryBufferManager implementation (which can be called from any
  // thread).
  HostGpuMemoryBufferManager(
      int client_id,
      std::unique_ptr<gpu::GpuMemoryBufferSupport> gpu_memory_buffer_support,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  ~HostGpuMemoryBufferManager() override;

  // This is called whenever GPU service is started, or with nullptr value when
  // GPU service is shut down (e.g. GPU process crashes). It will invalidate any
  // allocated buffer. If |gpu_serivce| is non-nullptr, it will retry allocation
  // requests for pending memory buffers; otherwise, the requests will be cached
  // and retried when a GPU service is available.
  void SetGpuService(mojom::GpuService* gpu_service);

  // This is called when no new GPU service is going to be available. It would
  // drop memory buffer allocation requests and call their callbacks with null
  // handles indicating failure..
  void DropPendingAllocationRequests();

  void DestroyGpuMemoryBuffer(gfx::GpuMemoryBufferId id,
                              int client_id,
                              const gpu::SyncToken& sync_token);

  void DestroyAllGpuMemoryBufferForClient(int client_id);

  void AllocateGpuMemoryBuffer(
      gfx::GpuMemoryBufferId id,
      int client_id,
      const gfx::Size& size,
      gfx::BufferFormat format,
      gfx::BufferUsage usage,
      gpu::SurfaceHandle surface_handle,
      base::OnceCallback<void(gfx::GpuMemoryBufferHandle)> callback);

  // Overridden from gpu::GpuMemoryBufferManager:
  std::unique_ptr<gfx::GpuMemoryBuffer> CreateGpuMemoryBuffer(
      const gfx::Size& size,
      gfx::BufferFormat format,
      gfx::BufferUsage usage,
      gpu::SurfaceHandle surface_handle) override;
  void SetDestructionSyncToken(gfx::GpuMemoryBuffer* buffer,
                               const gpu::SyncToken& sync_token) override;

  // Overridden from base::trace_event::MemoryDumpProvider:
  bool OnMemoryDump(const base::trace_event::MemoryDumpArgs& args,
                    base::trace_event::ProcessMemoryDump* pmd) override;

 private:
  struct PendingBufferInfo {
    PendingBufferInfo();
    PendingBufferInfo(PendingBufferInfo&&);
    ~PendingBufferInfo();

    gfx::Size size;
    gfx::BufferFormat format;
    gfx::BufferUsage usage;
    gpu::SurfaceHandle surface_handle;
    base::OnceCallback<void(gfx::GpuMemoryBufferHandle)> callback;
  };
  using PendingBuffers =
      std::unordered_map<gfx::GpuMemoryBufferId,
                         PendingBufferInfo,
                         BASE_HASH_NAMESPACE::hash<gfx::GpuMemoryBufferId>>;

  struct AllocatedBufferInfo {
    AllocatedBufferInfo();
    ~AllocatedBufferInfo();

    gfx::GpuMemoryBufferType type = gfx::EMPTY_BUFFER;
    size_t buffer_size_in_bytes = 0;
    base::UnguessableToken shared_memory_guid;
  };
  using AllocatedBuffers =
      std::unordered_map<gfx::GpuMemoryBufferId,
                         AllocatedBufferInfo,
                         BASE_HASH_NAMESPACE::hash<gfx::GpuMemoryBufferId>>;

  uint64_t ClientIdToTracingId(int client_id) const;
  void OnGpuMemoryBufferAllocated(int gpu_service_version,
                                  int client_id,
                                  gfx::GpuMemoryBufferId id,
                                  gfx::GpuMemoryBufferHandle handle);

  mojom::GpuService* gpu_service_ = nullptr;

  // This is incremented every time |gpu_service_| is updated in order check
  // whether a buffer is allocated by the most current GPU service or not.
  int gpu_service_version_ = 0;

  const int client_id_;
  int next_gpu_memory_id_ = 1;

  std::unordered_map<int, PendingBuffers> pending_buffers_;
  std::unordered_map<int, AllocatedBuffers> allocated_buffers_;

  std::unique_ptr<gpu::GpuMemoryBufferSupport> gpu_memory_buffer_support_;

  const gpu::GpuMemoryBufferConfigurationSet native_configurations_;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
  base::WeakPtr<HostGpuMemoryBufferManager> weak_ptr_;
  base::WeakPtrFactory<HostGpuMemoryBufferManager> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(HostGpuMemoryBufferManager);
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_HOST_HOST_GPU_MEMORY_BUFFER_MANAGER_H_
