// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_CORE_PLATFORM_HANDLE_IN_TRANSIT_H_
#define MOJO_CORE_PLATFORM_HANDLE_IN_TRANSIT_H_

#include "base/macros.h"
#include "build/build_config.h"
#include "mojo/core/scoped_process_handle.h"
#include "mojo/public/cpp/platform/platform_handle.h"

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include <mach/mach.h>
#endif

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace mojo {
namespace core {

// Owns a PlatformHandle which may actually belong to another process. On
// Windows and (sometimes) Mac, handles in a message object may take on values
// which only have meaning in the context of a remote process.
//
// This class provides a safe way of scoping the lifetime of such handles so
// that they don't leak when transmission can't be completed.
class PlatformHandleInTransit {
 public:
  PlatformHandleInTransit();
  explicit PlatformHandleInTransit(PlatformHandle handle);
  PlatformHandleInTransit(PlatformHandleInTransit&&);
  ~PlatformHandleInTransit();

  PlatformHandleInTransit& operator=(PlatformHandleInTransit&&);

  // Accessor for the owned handle. Must be owned by the calling process.
  const PlatformHandle& handle() const {
    DCHECK(!owning_process_.is_valid());
    return handle_;
  }

  // Returns the process which owns this handle. If this is invalid, the handle
  // is owned by the current process.
  const ScopedProcessHandle& owning_process() const { return owning_process_; }

  // Takes ownership of the held handle as-is. The handle must belong to the
  // current process.
  PlatformHandle TakeHandle();

  // Discards the handle owned by this object. The implication is that its
  // value has been successfully communicated to the owning process and the
  // calling process is no longer responsible for managing the handle's
  // lifetime.
  void CompleteTransit();

  // Transfers ownership of this (local) handle to |target_process|.
  bool TransferToProcess(ScopedProcessHandle target_process);

#if defined(OS_WIN)
  HANDLE remote_handle() const { return remote_handle_; }

  // Returns a new local handle, with ownership of |handle| being transferred
  // from |owning_process| to the caller.
  static PlatformHandle TakeIncomingRemoteHandle(
      HANDLE handle,
      base::ProcessHandle owning_process);
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
  // Creates a special wrapper holding an unowned Mach port name. This may refer
  // to a send or receive right in a remote task (process), and is used for
  // cases where message must retain such an object as one of its attached
  // handles. We're OK for now with leaking in any scenario where a lack of
  // strict ownership could cause leakage. See https://crbug.com/855930 for more
  // details.
  static PlatformHandleInTransit CreateForMachPortName(mach_port_t name);

  bool is_mach_port_name() const { return mach_port_name_ != MACH_PORT_NULL; }
  mach_port_t mach_port_name() const { return mach_port_name_; }
#endif

 private:
#if defined(OS_WIN)
  // We don't use a ScopedHandle (or, by extension, PlatformHandle) here because
  // the handle verifier expects all handle values to be owned by this process.
  // On Windows we use |handle_| for locally owned handles and |remote_handle_|
  // otherwise. On all other platforms we use |handle_| regardless of ownership.
  HANDLE remote_handle_ = INVALID_HANDLE_VALUE;
#endif

  PlatformHandle handle_;
  ScopedProcessHandle owning_process_;

#if defined(OS_MACOSX) && !defined(OS_IOS)
  mach_port_t mach_port_name_ = MACH_PORT_NULL;
#endif

  DISALLOW_COPY_AND_ASSIGN(PlatformHandleInTransit);
};

}  // namespace core
}  // namespace mojo

#endif  // MOJO_CORE_PLATFORM_HANDLE_IN_TRANSIT_H_
