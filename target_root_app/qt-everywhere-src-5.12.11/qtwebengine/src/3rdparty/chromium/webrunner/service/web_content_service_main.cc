// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <zircon/process.h>

#include "base/command_line.h"
#include "content/public/app/content_main.h"
#include "services/service_manager/embedder/switches.h"
#include "webrunner/service/common.h"
#include "webrunner/service/context_provider_main.h"
#include "webrunner/service/webrunner_main_delegate.h"

int main(int argc, const char** argv) {
  base::CommandLine::Init(argc, argv);

  std::string process_type =
      base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          service_manager::switches::kProcessType);
  zx::channel context_channel;

  if (process_type.empty()) {
    // zx_take_startup_handle() is called only when process_type is empty (i.e.
    // for Browser and ContextProvider processes). Renderer and other child
    // processes may use the same handle id for other handles.
    context_channel.reset(
        zx_take_startup_handle(webrunner::kContextRequestHandleId));

    // If |process_type| is empty then this may be a Browser process, or the
    // main ContextProvider process. Browser processes will have a
    // |context_channel| set
    if (!context_channel)
      return webrunner::ContextProviderMain();
  }

  webrunner::WebRunnerMainDelegate delegate(std::move(context_channel));
  content::ContentMainParams params(&delegate);

  // Repeated base::CommandLine::Init() is ignored, so it's safe to pass null
  // args here.
  params.argc = 0;
  params.argv = nullptr;

  return content::ContentMain(params);
}
