// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_STARTUP_DATA_IMPL_H_
#define CONTENT_BROWSER_STARTUP_DATA_IMPL_H_

#include <memory>

#include "content/browser/browser_process_sub_thread.h"
#include "content/public/browser/startup_data.h"

namespace content {

// The browser implementation of StartupData.
struct StartupDataImpl : public StartupData {
  StartupDataImpl();
  ~StartupDataImpl() override;

  // TODO(hanxi): add ServiceManagerContext* here.
  std::unique_ptr<BrowserProcessSubThread> thread;
};

}  // namespace content

#endif  // CONTENT_BROWSER_STARTUP_DATA_IMPL_H_
