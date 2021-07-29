// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/command_buffer/service/service_transfer_cache.h"

#include <inttypes.h>

#include "base/bind.h"
#include "base/memory/memory_coordinator_client_registry.h"
#include "base/strings/stringprintf.h"
#include "base/sys_info.h"
#include "base/trace_event/memory_dump_manager.h"
#include "cc/paint/image_transfer_cache_entry.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "ui/gl/trace_util.h"

namespace gpu {
namespace {

size_t CacheSizeLimit() {
  size_t memory_usage = 128 * 1024 * 1024;
  if (base::SysInfo::IsLowEndDevice()) {
    // Based on the 512KB limit used for discardable images in non-OOP-R, but
    // gives an extra 256KB to allow for additional (non-image) cache items.
    memory_usage = 768 * 1024;
  }
  return memory_usage;
}

// TODO(ericrk): Move this into ServiceImageTransferCacheEntry - here for now
// due to ui/gl dependency.
void DumpMemoryForImageTransferCacheEntry(
    base::trace_event::ProcessMemoryDump* pmd,
    const std::string& dump_name,
    const cc::ServiceImageTransferCacheEntry* entry) {
  using base::trace_event::MemoryAllocatorDump;

  MemoryAllocatorDump* dump = pmd->CreateAllocatorDump(dump_name);
  dump->AddScalar(MemoryAllocatorDump::kNameSize,
                  MemoryAllocatorDump::kUnitsBytes, entry->CachedSize());

  // Alias the image entry to its skia counterpart, taking ownership of the
  // memory and preventing double counting.
  DCHECK(entry->image());
  GrBackendTexture image_backend_texture =
      entry->image()->getBackendTexture(false /* flushPendingGrContextIO */);
  GrGLTextureInfo info;
  if (image_backend_texture.getGLTextureInfo(&info)) {
    auto guid = gl::GetGLTextureRasterGUIDForTracing(info.fID);
    pmd->CreateSharedGlobalAllocatorDump(guid);
    // Importance of 3 gives this dump priority over the dump made by Skia
    // (importance 2), attributing memory here.
    const int kImportance = 3;
    pmd->AddOwnershipEdge(dump->guid(), guid, kImportance);
  }
}

}  // namespace

ServiceTransferCache::CacheEntryInternal::CacheEntryInternal(
    base::Optional<ServiceDiscardableHandle> handle,
    std::unique_ptr<cc::ServiceTransferCacheEntry> entry)
    : handle(handle), entry(std::move(entry)) {}

ServiceTransferCache::CacheEntryInternal::~CacheEntryInternal() {}

ServiceTransferCache::CacheEntryInternal::CacheEntryInternal(
    CacheEntryInternal&& other) = default;

ServiceTransferCache::CacheEntryInternal&
ServiceTransferCache::CacheEntryInternal::operator=(
    CacheEntryInternal&& other) = default;

ServiceTransferCache::ServiceTransferCache()
    : entries_(EntryCache::NO_AUTO_EVICT), cache_size_limit_(CacheSizeLimit()) {
  // In certain cases, ThreadTaskRunnerHandle isn't set (Android Webview).
  // Don't register a dump provider in these cases.
  if (base::ThreadTaskRunnerHandle::IsSet()) {
    base::trace_event::MemoryDumpManager::GetInstance()->RegisterDumpProvider(
        this, "cc::GpuImageDecodeCache", base::ThreadTaskRunnerHandle::Get());
  }
}

ServiceTransferCache::~ServiceTransferCache() {
  base::trace_event::MemoryDumpManager::GetInstance()->UnregisterDumpProvider(
      this);
}

bool ServiceTransferCache::CreateLockedEntry(const EntryKey& key,
                                             ServiceDiscardableHandle handle,
                                             GrContext* context,
                                             base::span<uint8_t> data) {
  auto found = entries_.Peek(key);
  if (found != entries_.end())
    return false;

  std::unique_ptr<cc::ServiceTransferCacheEntry> entry =
      cc::ServiceTransferCacheEntry::Create(key.entry_type);
  if (!entry)
    return false;

  if (!entry->Deserialize(context, data))
    return false;

  total_size_ += entry->CachedSize();
  entries_.Put(key, CacheEntryInternal(handle, std::move(entry)));
  EnforceLimits();
  return true;
}

void ServiceTransferCache::CreateLocalEntry(
    const EntryKey& key,
    std::unique_ptr<cc::ServiceTransferCacheEntry> entry) {
  if (!entry)
    return;

  DCHECK_EQ(entry->Type(), key.entry_type);
  DeleteEntry(key);

  total_size_ += entry->CachedSize();

  entries_.Put(key, CacheEntryInternal(base::nullopt, std::move(entry)));
  EnforceLimits();
}

bool ServiceTransferCache::UnlockEntry(const EntryKey& key) {
  auto found = entries_.Peek(key);
  if (found == entries_.end())
    return false;

  if (!found->second.handle)
    return false;
  found->second.handle->Unlock();
  return true;
}

bool ServiceTransferCache::DeleteEntry(const EntryKey& key) {
  auto found = entries_.Peek(key);
  if (found == entries_.end())
    return false;

  if (found->second.handle)
    found->second.handle->ForceDelete();
  total_size_ -= found->second.entry->CachedSize();
  entries_.Erase(found);
  return true;
}

cc::ServiceTransferCacheEntry* ServiceTransferCache::GetEntry(
    const EntryKey& key) {
  auto found = entries_.Get(key);
  if (found == entries_.end())
    return nullptr;
  return found->second.entry.get();
}

void ServiceTransferCache::EnforceLimits() {
  for (auto it = entries_.rbegin(); it != entries_.rend();) {
    if (total_size_ <= cache_size_limit_) {
      return;
    }
    if (it->second.handle && !it->second.handle->Delete()) {
      ++it;
      continue;
    }

    total_size_ -= it->second.entry->CachedSize();
    it = entries_.Erase(it);
  }
}

void ServiceTransferCache::PurgeMemory(
    base::MemoryPressureListener::MemoryPressureLevel memory_pressure_level) {
  switch (memory_pressure_level) {
    case base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE:
      // This function is only called with moderate or critical pressure.
      NOTREACHED();
      return;
    case base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_MODERATE:
      cache_size_limit_ = cache_size_limit_ / 4;
      break;
    case base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_CRITICAL:
      cache_size_limit_ = 0u;
      break;
  }

  EnforceLimits();
  cache_size_limit_ = CacheSizeLimit();
}

bool ServiceTransferCache::OnMemoryDump(
    const base::trace_event::MemoryDumpArgs& args,
    base::trace_event::ProcessMemoryDump* pmd) {
  using base::trace_event::MemoryAllocatorDump;
  using base::trace_event::MemoryDumpLevelOfDetail;

  if (args.level_of_detail == MemoryDumpLevelOfDetail::BACKGROUND) {
    std::string dump_name =
        base::StringPrintf("gpu/transfer_cache/cache_0x%" PRIXPTR,
                           reinterpret_cast<uintptr_t>(this));
    MemoryAllocatorDump* dump = pmd->CreateAllocatorDump(dump_name);
    dump->AddScalar(MemoryAllocatorDump::kNameSize,
                    MemoryAllocatorDump::kUnitsBytes, total_size_);

    // Early out, no need for more detail in a BACKGROUND dump.
    return true;
  }

  for (auto it = entries_.begin(); it != entries_.end(); it++) {
    uint32_t entry_id = it->first.entry_id;
    auto entry_type = it->first.entry_type;
    const auto* entry = it->second.entry.get();
    const cc::ServiceImageTransferCacheEntry* image_entry = nullptr;

    if (entry_type == cc::TransferCacheEntryType::kImage) {
      image_entry =
          static_cast<const cc::ServiceImageTransferCacheEntry*>(entry);
    }

    if (image_entry && image_entry->fits_on_gpu()) {
      std::string dump_name = base::StringPrintf(
          "gpu/transfer_cache/cache_0x%" PRIXPTR "/gpu/entry_%d",
          reinterpret_cast<uintptr_t>(this), entry_id);
      DumpMemoryForImageTransferCacheEntry(pmd, dump_name, image_entry);
    } else {
      std::string dump_name = base::StringPrintf(
          "gpu/transfer_cache/cache_0x%" PRIXPTR "/cpu/entry_%d",
          reinterpret_cast<uintptr_t>(this), entry_id);
      MemoryAllocatorDump* dump = pmd->CreateAllocatorDump(dump_name);
      dump->AddScalar(MemoryAllocatorDump::kNameSize,
                      MemoryAllocatorDump::kUnitsBytes, entry->CachedSize());
    }
  }

  return true;
}

ServiceTransferCache::EntryKey::EntryKey(int decoder_id,
                                         cc::TransferCacheEntryType entry_type,
                                         uint32_t entry_id)
    : decoder_id(decoder_id), entry_type(entry_type), entry_id(entry_id) {}

}  // namespace gpu
