// Copyright 2018 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYSTEM_API_DBUS_SENESCHAL_DBUS_CONSTANTS_H_
#define SYSTEM_API_DBUS_SENESCHAL_DBUS_CONSTANTS_H_

namespace vm_tools {
namespace seneschal {

const char kSeneschalInterface[] = "org.chromium.Seneschal";
const char kSeneschalServicePath[] = "/org/chromium/Seneschal";
const char kSeneschalServiceName[] = "org.chromium.Seneschal";

// Exported methods.
const char kStartServerMethod[] = "StartServer";
const char kStopServerMethod[] = "StopServer";
const char kSharePathMethod[] = "SharePath";


}  // namespace seneschal
}  // namespace vm_tools

#endif // SYSTEM_API_DBUS_SENESCHAL_DBUS_CONSTANTS_H_
