// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBRUNNER_SERVICE_COMMON_H_
#define WEBRUNNER_SERVICE_COMMON_H_

#include <zircon/processargs.h>

namespace webrunner {

constexpr uint32_t kContextRequestHandleId = PA_HND(PA_USER0, 0);

// Process type value used for the web::Context process. It is equivalent to
// the main browser process in chrome.
extern const char kProcessTypeWebContext[];

}  // namespace webrunner

#endif  // WEBRUNNER_SERVICE_COMMON_H_
