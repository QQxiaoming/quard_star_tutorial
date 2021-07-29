// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_SERVICE_MAIN_VIZ_COMPOSITOR_THREAD_RUNNER_H_
#define COMPONENTS_VIZ_SERVICE_MAIN_VIZ_COMPOSITOR_THREAD_RUNNER_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "components/viz/service/viz_service_export.h"
#include "services/viz/privileged/interfaces/viz_main.mojom.h"

namespace base {
class SingleThreadTaskRunner;
class Thread;
}  // namespace base

namespace viz {
class DisplayProvider;
class FrameSinkManagerImpl;
class ServerSharedBitmapManager;

// Starts and runs the VizCompositorThread. The thread will be started when this
// object is constructed. Objects on the thread will be initialized after
// calling CreateFrameSinkManager(). Destructor will teardown objects on thread
// and then stop the thread.
// TODO(kylechar): Convert VizMainImpl to use VizCompositorThreadRunner.
class VIZ_SERVICE_EXPORT VizCompositorThreadRunner {
 public:
  VizCompositorThreadRunner();
  // Performs teardown on thread and then stops thread.
  ~VizCompositorThreadRunner();

  // Create FrameSinkManager from |params|. This can be called from the thread
  // that owns |this| to initialize state on VizCompositorThreadRunner.
  void CreateFrameSinkManager(mojom::FrameSinkManagerParamsPtr params);

 private:
  void CreateFrameSinkManagerOnCompositorThread(
      mojom::FrameSinkManagerParamsPtr params);
  void TearDownOnCompositorThread();

  // Start variables to be accessed only on |task_runner_|.
  std::unique_ptr<ServerSharedBitmapManager> server_shared_bitmap_manager_;
  std::unique_ptr<DisplayProvider> display_provider_;
  std::unique_ptr<FrameSinkManagerImpl> frame_sink_manager_;
  // End variables to be accessed only on |task_runner_|.

  std::unique_ptr<base::Thread> thread_;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  DISALLOW_COPY_AND_ASSIGN(VizCompositorThreadRunner);
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_SERVICE_MAIN_VIZ_COMPOSITOR_THREAD_RUNNER_H_
