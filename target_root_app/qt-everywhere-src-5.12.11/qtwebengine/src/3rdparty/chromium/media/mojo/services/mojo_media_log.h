// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_MOJO_SERVICES_MOJO_MEDIA_LOG_H_
#define MEDIA_MOJO_SERVICES_MOJO_MEDIA_LOG_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequenced_task_runner.h"
#include "media/base/media_log.h"
#include "media/mojo/interfaces/media_log.mojom.h"

namespace media {

class MojoMediaLog final : public MediaLog {
 public:
  // TODO(sandersd): Template on Ptr type to support non-associated.
  explicit MojoMediaLog(mojom::MediaLogAssociatedPtrInfo remote_media_log,
                        scoped_refptr<base::SequencedTaskRunner> task_runner);
  ~MojoMediaLog() final;

  // MediaLog implementation.  May be called from any thread, but will only
  // use |remote_media_log_| on |task_runner_|.
  void AddEvent(std::unique_ptr<MediaLogEvent> event) override;

 private:
  mojom::MediaLogAssociatedPtr remote_media_log_;

  // The mojo service thread on which we'll access |remote_media_log_|.
  scoped_refptr<base::SequencedTaskRunner> task_runner_;

  base::WeakPtr<MojoMediaLog> weak_this_;

  base::WeakPtrFactory<MojoMediaLog> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(MojoMediaLog);
};

}  // namespace media

#endif  // MEDIA_MOJO_SERVICES_MOJO_MEDIA_LOG_H_
