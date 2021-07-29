// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/dom/synchronous_mutation_notifier.h"

#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/synchronous_mutation_observer.h"

namespace blink {

SynchronousMutationNotifier::SynchronousMutationNotifier() = default;

void SynchronousMutationNotifier::NotifyChangeChildren(
    const ContainerNode& container) {
  ForEachObserver([&](SynchronousMutationObserver* observer) {
    observer->DidChangeChildren(container);
  });
}

void SynchronousMutationNotifier::NotifyMergeTextNodes(
    const Text& node,
    const NodeWithIndex& node_to_be_removed_with_index,
    unsigned old_length) {
  ForEachObserver([&](SynchronousMutationObserver* observer) {
    observer->DidMergeTextNodes(node, node_to_be_removed_with_index,
                                old_length);
  });
}

void SynchronousMutationNotifier::NotifyMoveTreeToNewDocument(
    const Node& root) {
  ForEachObserver([&](SynchronousMutationObserver* observer) {
    observer->DidMoveTreeToNewDocument(root);
  });
}

void SynchronousMutationNotifier::NotifySplitTextNode(const Text& node) {
  ForEachObserver([&](SynchronousMutationObserver* observer) {
    observer->DidSplitTextNode(node);
  });
}

void SynchronousMutationNotifier::NotifyUpdateCharacterData(
    CharacterData* character_data,
    unsigned offset,
    unsigned old_length,
    unsigned new_length) {
  // Using ForEachObserverWithoutChecks() instead of ForEachObserver() is
  // necessary because DocumentMarkerController::DidUpdateCharacterData ends up
  // calling SynchronousMutationNotifier::RemoveObserver, which is unsafe and
  // can result in memory corruption.
  //
  // TODO(crbug.com/862900): Fix DocumentMarkerController and switch to
  //                         ForEachObsever() here.
  ForEachObserverWithoutChecks([&](SynchronousMutationObserver* observer) {
    observer->DidUpdateCharacterData(character_data, offset, old_length,
                                     new_length);
  });
}

void SynchronousMutationNotifier::NotifyNodeChildrenWillBeRemoved(
    ContainerNode& container) {
  ForEachObserver([&](SynchronousMutationObserver* observer) {
    observer->NodeChildrenWillBeRemoved(container);
  });
}

void SynchronousMutationNotifier::NotifyNodeWillBeRemoved(Node& node) {
  ForEachObserver([&](SynchronousMutationObserver* observer) {
    observer->NodeWillBeRemoved(node);
  });
}

}  // namespace blink
