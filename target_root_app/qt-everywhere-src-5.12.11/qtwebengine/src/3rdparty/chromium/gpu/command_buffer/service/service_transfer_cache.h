// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_SERVICE_SERVICE_TRANSFER_CACHE_H_
#define GPU_COMMAND_BUFFER_SERVICE_SERVICE_TRANSFER_CACHE_H_

#include <vector>

#include "base/containers/mru_cache.h"
#include "base/containers/span.h"
#include "base/memory/memory_pressure_listener.h"
#include "cc/paint/transfer_cache_entry.h"
#include "gpu/command_buffer/common/discardable_handle.h"
#include "gpu/command_buffer/service/context_group.h"
#include "gpu/gpu_gles2_export.h"

namespace gpu {

// ServiceTransferCache is a GPU process interface for retreiving cached entries
// from the transfer cache. These entries are populated by client calls to the
// ClientTransferCache.
//
// In addition to access, the ServiceTransferCache is also responsible for
// unlocking and deleting entries when no longer needed, as well as enforcing
// cache limits. If the cache exceeds its specified limits, unlocked transfer
// cache entries may be deleted.
class GPU_GLES2_EXPORT ServiceTransferCache
    : public base::trace_event::MemoryDumpProvider {
 public:
  struct GPU_GLES2_EXPORT EntryKey {
    EntryKey(int decoder_id,
             cc::TransferCacheEntryType entry_type,
             uint32_t entry_id);
    int decoder_id;
    cc::TransferCacheEntryType entry_type;
    uint32_t entry_id;
  };

  ServiceTransferCache();
  ~ServiceTransferCache() override;

  bool CreateLockedEntry(const EntryKey& key,
                         ServiceDiscardableHandle handle,
                         GrContext* context,
                         base::span<uint8_t> data);
  void CreateLocalEntry(const EntryKey& key,
                        std::unique_ptr<cc::ServiceTransferCacheEntry> entry);
  bool UnlockEntry(const EntryKey& key);
  bool DeleteEntry(const EntryKey& key);
  cc::ServiceTransferCacheEntry* GetEntry(const EntryKey& key);

  void PurgeMemory(
      base::MemoryPressureListener::MemoryPressureLevel memory_pressure_level);

  // base::trace_event::MemoryDumpProvider implementation.
  bool OnMemoryDump(const base::trace_event::MemoryDumpArgs& args,
                    base::trace_event::ProcessMemoryDump* pmd) override;

  // Test-only functions:
  void SetCacheSizeLimitForTesting(size_t cache_size_limit) {
    cache_size_limit_ = cache_size_limit;
    EnforceLimits();
  }
  size_t cache_size_for_testing() const { return total_size_; }

 private:
  void EnforceLimits();

  struct CacheEntryInternal {
    CacheEntryInternal(base::Optional<ServiceDiscardableHandle> handle,
                       std::unique_ptr<cc::ServiceTransferCacheEntry> entry);
    CacheEntryInternal(CacheEntryInternal&& other);
    CacheEntryInternal& operator=(CacheEntryInternal&& other);
    ~CacheEntryInternal();
    base::Optional<ServiceDiscardableHandle> handle;
    std::unique_ptr<cc::ServiceTransferCacheEntry> entry;
  };

  struct EntryKeyComp {
    bool operator()(const EntryKey& lhs, const EntryKey& rhs) const {
      if (lhs.decoder_id != rhs.decoder_id)
        return lhs.decoder_id < rhs.decoder_id;
      if (lhs.entry_type != rhs.entry_type)
        return lhs.entry_type < rhs.entry_type;
      return lhs.entry_id < rhs.entry_id;
    }
  };

  using EntryCache = base::MRUCache<EntryKey, CacheEntryInternal, EntryKeyComp>;
  EntryCache entries_;

  // Total size of all |entries_|. The same as summing
  // GpuDiscardableEntry::size for each entry.
  size_t total_size_ = 0;

  // The limit above which the cache will start evicting resources.
  size_t cache_size_limit_ = 0;

  DISALLOW_COPY_AND_ASSIGN(ServiceTransferCache);
};

}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_SERVICE_TRANSFER_CACHE_H_
