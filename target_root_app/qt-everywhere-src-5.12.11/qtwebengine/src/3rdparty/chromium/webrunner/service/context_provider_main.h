// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBRUNNER_SERVICE_CONTEXT_PROVIDER_MAIN_H_
#define WEBRUNNER_SERVICE_CONTEXT_PROVIDER_MAIN_H_

#include "webrunner/common/webrunner_export.h"

namespace webrunner {

// Main function for the process that implements web::ContextProvider interface.
// Called by WebRunnerMainDelegate when the process is started without --type
// argument.
WEBRUNNER_EXPORT int ContextProviderMain();

}  // namespace webrunner

#endif  // WEBRUNNER_SERVICE_CONTEXT_PROVIDER_MAIN_H_