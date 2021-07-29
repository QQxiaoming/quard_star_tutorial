// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/mojo/services/mojo_media_log.h"

#include "base/logging.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace media {

MojoMediaLog::MojoMediaLog(mojom::MediaLogAssociatedPtrInfo remote_media_log,
                           scoped_refptr<base::SequencedTaskRunner> task_runner)
    : remote_media_log_(std::move(remote_media_log)),
      task_runner_(std::move(task_runner)),
      weak_ptr_factory_(this) {
  weak_this_ = weak_ptr_factory_.GetWeakPtr();
  DVLOG(1) << __func__;
}

MojoMediaLog::~MojoMediaLog() {
  DVLOG(1) << __func__;
}

void MojoMediaLog::AddEvent(std::unique_ptr<MediaLogEvent> event) {
  DVLOG(1) << __func__;
  DCHECK(event);

  // Don't post unless we need to.  Otherwise, we can order a log entry after
  // our own destruction.  While safe, this loses the message.  This can happen,
  // for example, when we're logging why a VideoDecoder failed to initialize.
  // It will be destroyed synchronously when Initialize returns.
  //
  // Also, we post here, so this is the base case.  :)
  if (task_runner_->RunsTasksInCurrentSequence()) {
    remote_media_log_->AddEvent(*event);
    return;
  }

  // From other threads, it's okay to post without worrying about losing a
  // message.  This is because any message that's causally related to the object
  // (and thus MediaLog) being destroyed hopefully posts the result back to the
  // same sequence as |task_runner_| after we do.  Of course, async destruction
  // (e.g., the renderer destroys a MojoVideoDecoder) can still lose messages,
  // but that's really a race.
  task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&MojoMediaLog::AddEvent, weak_this_, std::move(event)));
}

}  // namespace media
