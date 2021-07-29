// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_UNALIGNED_SHARED_MEMORY_H_
#define MEDIA_BASE_UNALIGNED_SHARED_MEMORY_H_

#include <stdint.h>

#include "base/macros.h"
#include "base/memory/shared_memory.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/memory/unsafe_shared_memory_region.h"
#include "media/base/media_export.h"

namespace media {

// Wrapper over base::SharedMemory that can be mapped at unaligned offsets.
// DEPRECATED! See https://crbug.com/795291.
class MEDIA_EXPORT UnalignedSharedMemory {
 public:
  // Creates an |UnalignedSharedMemory| instance from a
  // |SharedMemoryHandle|. |size| sets the maximum size that may be mapped. This
  // instance will own the handle.
  UnalignedSharedMemory(const base::SharedMemoryHandle& handle,
                        size_t size,
                        bool read_only);
  ~UnalignedSharedMemory();

  // Map the shared memory region. Note that the passed |size| parameter should
  // be less than or equal to |size()|.
  bool MapAt(off_t offset, size_t size);
  size_t size() const { return size_; }
  void* memory() const;

 private:
  base::SharedMemory shm_;

  // The size of the region associated with |shm_|.
  size_t size_;

  // Offset withing |shm_| memory that data has been mapped; strictly less than
  // base::SysInfo::VMAllocationGranularity().
  size_t misalignment_;

  DISALLOW_COPY_AND_ASSIGN(UnalignedSharedMemory);
};

// Wrapper over base::WritableSharedMemoryMapping that is mapped at unaligned
// offsets.
class MEDIA_EXPORT WritableUnalignedMapping {
 public:
  // Creates an |UnalignedSharedMemory| instance from a
  // |UnsafeSharedMemoryRegion|. |size| sets the maximum size that may be mapped
  // within |region| and |offset| is the offset that will be mapped. |region| is
  // not retained and is used only in the constructor.
  WritableUnalignedMapping(const base::UnsafeSharedMemoryRegion& region,
                           size_t size,
                           off_t offset);

  // As above, but creates from a handle. This region will not own the handle.
  // DEPRECATED: this should be used only for the legacy shared memory
  // conversion project, see https://crbug.com/795291.
  WritableUnalignedMapping(const base::SharedMemoryHandle& handle,
                           size_t size,
                           off_t offset);

  ~WritableUnalignedMapping();

  size_t size() const { return size_; }
  void* memory() const;

  // True if the mapping backing the memory is valid.
  bool IsValid() const { return mapping_.IsValid(); }

 private:
  base::WritableSharedMemoryMapping mapping_;

  // The size of the region associated with |mapping_|.
  size_t size_;

  // Difference between actual offset within |mapping_| where data has been
  // mapped and requested offset; strictly less than
  // base::SysInfo::VMAllocationGranularity().
  size_t misalignment_;

  DISALLOW_COPY_AND_ASSIGN(WritableUnalignedMapping);
};

}  // namespace media

#endif  // MEDIA_BASE_UNALIGNED_SHARED_MEMORY_H_
