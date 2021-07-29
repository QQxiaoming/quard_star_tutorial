// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/graphics/compositor_mutator_impl.h"

#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/web_thread.h"
#include "third_party/blink/renderer/platform/cross_thread_functional.h"
#include "third_party/blink/renderer/platform/graphics/compositor_animator.h"
#include "third_party/blink/renderer/platform/graphics/compositor_mutator_client.h"
#include "third_party/blink/renderer/platform/instrumentation/tracing/trace_event.h"
#include "third_party/blink/renderer/platform/waitable_event.h"
#include "third_party/blink/renderer/platform/web_task_runner.h"

namespace blink {

CompositorMutatorImpl::CompositorMutatorImpl()
    : client_(nullptr), weak_factory_(this) {
  // By default layout tests run without threaded compositing. See
  // https://crbug.com/770028 For these situations we run on the Main thread.
  if (Platform::Current()->CompositorThread())
    mutator_queue_ = Platform::Current()->CompositorThread()->GetTaskRunner();
  else
    mutator_queue_ = Platform::Current()->MainThread()->GetTaskRunner();
}

CompositorMutatorImpl::~CompositorMutatorImpl() {}

// static
std::unique_ptr<CompositorMutatorClient> CompositorMutatorImpl::CreateClient(
    base::WeakPtr<CompositorMutatorImpl>* weak_interface,
    scoped_refptr<base::SingleThreadTaskRunner>* queue) {
  DCHECK(IsMainThread());
  auto mutator = std::make_unique<CompositorMutatorImpl>();
  // This is allowed since we own the class for the duration of creation.
  *weak_interface = mutator->weak_factory_.GetWeakPtr();
  *queue = mutator->GetTaskRunner();
  return std::make_unique<CompositorMutatorClient>(std::move(mutator));
}

void CompositorMutatorImpl::Mutate(
    std::unique_ptr<CompositorMutatorInputState> mutator_input) {
  TRACE_EVENT0("cc", "CompositorMutatorImpl::mutate");
  DCHECK(mutator_queue_->BelongsToCurrentThread());
  if (animator_map_.IsEmpty())
    return;
  DCHECK(client_);

  Vector<std::unique_ptr<CompositorMutatorOutputState>> outputs(
      animator_map_.size());
  Vector<WaitableEvent> done_events(animator_map_.size());

  int index = 0;
  for (auto& pair : animator_map_) {
    CompositorAnimator* animator = pair.key;
    scoped_refptr<base::SingleThreadTaskRunner> animator_queue = pair.value;

    std::unique_ptr<AnimationWorkletInput> animator_input =
        mutator_input->TakeWorkletState(animator->GetScopeId());

    DCHECK(!animator_queue->BelongsToCurrentThread());
    std::unique_ptr<AutoSignal> done =
        std::make_unique<AutoSignal>(&done_events[index]);
    std::unique_ptr<CompositorMutatorOutputState>& output = outputs[index];

    if (animator_input) {
      PostCrossThreadTask(
          *animator_queue, FROM_HERE,
          CrossThreadBind(
              [](CompositorAnimator* animator,
                 std::unique_ptr<AnimationWorkletInput> input,
                 std::unique_ptr<AutoSignal> completion,
                 std::unique_ptr<CompositorMutatorOutputState>* output) {
                *output = animator->Mutate(std::move(input));
              },
              WrapCrossThreadWeakPersistent(animator),
              WTF::Passed(std::move(animator_input)),
              WTF::Passed(std::move(done)), CrossThreadUnretained(&output)));
    }
    index++;
  }

  for (WaitableEvent& event : done_events) {
    event.Wait();
  }

  for (auto& output : outputs) {
    // Animator that has no input does not produce any output.
    if (!output)
      continue;
    client_->SetMutationUpdate(std::move(output));
  }
}

void CompositorMutatorImpl::RegisterCompositorAnimator(
    CrossThreadPersistent<CompositorAnimator> animator,
    scoped_refptr<base::SingleThreadTaskRunner> animator_runner) {
  TRACE_EVENT0("cc", "CompositorMutatorImpl::RegisterCompositorAnimator");

  DCHECK(animator);
  DCHECK(mutator_queue_->BelongsToCurrentThread());

  animator_map_.insert(animator, animator_runner);
}

void CompositorMutatorImpl::UnregisterCompositorAnimator(
    CrossThreadPersistent<CompositorAnimator> animator) {
  TRACE_EVENT0("cc", "CompositorMutatorImpl::UnregisterCompositorAnimator");
  DCHECK(animator);
  DCHECK(mutator_queue_->BelongsToCurrentThread());

  animator_map_.erase(animator);
}

bool CompositorMutatorImpl::HasAnimators() {
  return !animator_map_.IsEmpty();
}

CompositorMutatorImpl::AutoSignal::AutoSignal(WaitableEvent* event)
    : event_(event) {
  DCHECK(event);
}

CompositorMutatorImpl::AutoSignal::~AutoSignal() {
  event_->Signal();
}

}  // namespace blink
