// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webrunner/service/context_provider_main.h"

#include "base/fuchsia/scoped_service_binding.h"
#include "base/fuchsia/service_directory.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "webrunner/service/context_provider_impl.h"

namespace webrunner {

int ContextProviderMain() {
  base::MessageLoopForUI message_loop;
  base::fuchsia::ServiceDirectory* directory =
      base::fuchsia::ServiceDirectory::GetDefault();
  ContextProviderImpl context_provider;
  base::fuchsia::ScopedServiceBinding<chromium::web::ContextProvider> binding(
      directory, &context_provider);

  // TODO(crbug.com/852145): Currently the process will run until it's killed.
  base::RunLoop().Run();

  return 0;
}

}  // namespace webrunner
