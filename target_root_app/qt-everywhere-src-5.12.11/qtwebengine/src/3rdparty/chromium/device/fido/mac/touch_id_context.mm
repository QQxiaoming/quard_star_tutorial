// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/mac/touch_id_context.h"

#import <Foundation/Foundation.h>

#include "base/bind.h"
#include "base/logging.h"
#include "base/mac/foundation_util.h"
#include "base/sequenced_task_runner.h"
#include "base/strings/sys_string_conversions.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace device {
namespace fido {
namespace mac {

namespace {
API_AVAILABLE(macosx(10.12.2))
base::ScopedCFTypeRef<SecAccessControlRef> DefaultAccessControl() {
  return base::ScopedCFTypeRef<SecAccessControlRef>(
      SecAccessControlCreateWithFlags(
          kCFAllocatorDefault, kSecAttrAccessibleWhenUnlockedThisDeviceOnly,
          kSecAccessControlPrivateKeyUsage | kSecAccessControlTouchIDAny,
          nullptr));
}
}  // namespace

TouchIdContext::TouchIdContext()
    : context_([[LAContext alloc] init]),
      access_control_(DefaultAccessControl()),
      callback_(),
      weak_ptr_factory_(this) {}

TouchIdContext::~TouchIdContext() {
  // Invalidating the LAContext will dismiss any pending UI dialog (e.g. if the
  // transaction was cancelled while we are waiting for a fingerprint). Simply
  // releasing the LAContext does not have the same effect.
  [context_ invalidate];
}

void TouchIdContext::PromptTouchId(std::string reason, Callback callback) {
  callback_ = std::move(callback);
  scoped_refptr<base::SequencedTaskRunner> runner =
      base::SequencedTaskRunnerHandle::Get();
  auto weak_self = weak_ptr_factory_.GetWeakPtr();
  // If evaluation succeeds (i.e. user provides a fingerprint), |context_| can
  // be used for one signing operation. N.B. even in |MakeCredentialOperation|,
  // we need to perform a signature for the attestation statement, so we need
  // the sign bit there.
  [context_ evaluateAccessControl:access_control_
                        operation:LAAccessControlOperationUseKeySign
                  localizedReason:base::SysUTF8ToNSString(reason)
                            reply:^(BOOL success, NSError* error) {
                              // The reply block is invoked in a separate
                              // thread. We want to invoke the callback in the
                              // original thread, so we post it onto the
                              // originating runner. The weak_self and runner
                              // variables inside the block are const-copies of
                              // the ones in the enclosing scope (c.f.
                              // http://clang.llvm.org/docs/Block-ABI-Apple.html#imported-variables).
                              if (!success) {
                                // |error| is autoreleased in this block. It
                                // is not currently passed onto the other
                                // thread running the callback; but if it
                                // were, it would have to be retained first
                                // (and probably captured in a
                                // scoped_nsobject<NSError>).
                                DCHECK(error != nil);
                                DVLOG(1) << "Touch ID prompt failed: "
                                         << base::mac::NSToCFCast(error);
                              }
                              runner->PostTask(
                                  FROM_HERE,
                                  base::BindOnce(&TouchIdContext::RunCallback,
                                                 weak_self, success));
                            }];
}

void TouchIdContext::RunCallback(bool success) {
  std::move(callback_).Run(success);
}

}  // namespace mac
}  // namespace fido
}  // namespace device
