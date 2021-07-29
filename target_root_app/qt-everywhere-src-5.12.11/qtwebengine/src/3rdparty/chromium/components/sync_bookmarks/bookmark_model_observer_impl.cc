// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync_bookmarks/bookmark_model_observer_impl.h"

#include <utility>

#include "base/guid.h"
#include "base/strings/utf_string_conversions.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_node.h"
#include "components/sync/base/hash_util.h"
#include "components/sync/base/unique_position.h"
#include "components/sync/engine/non_blocking_sync_common.h"
#include "components/sync_bookmarks/synced_bookmark_tracker.h"

namespace sync_bookmarks {

namespace {

void UpdateBookmarkSpecificsMetaInfo(
    const bookmarks::BookmarkNode::MetaInfoMap* metainfo_map,
    sync_pb::BookmarkSpecifics* bm_specifics) {
  // TODO(crbug.com/516866): update the implementation to be similar to the
  // directory implementation
  // https://cs.chromium.org/chromium/src/components/sync_bookmarks/bookmark_change_processor.cc?l=882&rcl=f38001d936d8b2abb5743e85cbc88c72746ae3d2
  for (const std::pair<std::string, std::string>& pair : *metainfo_map) {
    sync_pb::MetaInfo* meta_info = bm_specifics->add_meta_info();
    meta_info->set_key(pair.first);
    meta_info->set_value(pair.second);
  }
}

sync_pb::EntitySpecifics CreateSpecificsFromBookmarkNode(
    const bookmarks::BookmarkNode* node) {
  sync_pb::EntitySpecifics specifics;
  sync_pb::BookmarkSpecifics* bm_specifics = specifics.mutable_bookmark();
  bm_specifics->set_url(node->url().spec());
  // TODO(crbug.com/516866): Set the favicon.
  bm_specifics->set_title(base::UTF16ToUTF8(node->GetTitle()));
  bm_specifics->set_creation_time_us(
      node->date_added().ToDeltaSinceWindowsEpoch().InMicroseconds());

  bm_specifics->set_icon_url(node->icon_url() ? node->icon_url()->spec()
                                              : std::string());
  if (node->GetMetaInfoMap()) {
    UpdateBookmarkSpecificsMetaInfo(node->GetMetaInfoMap(), bm_specifics);
  }
  return specifics;
}

}  // namespace

BookmarkModelObserverImpl::BookmarkModelObserverImpl(
    const base::RepeatingClosure& nudge_for_commit_closure,
    SyncedBookmarkTracker* bookmark_tracker)
    : bookmark_tracker_(bookmark_tracker),
      nudge_for_commit_closure_(nudge_for_commit_closure) {
  DCHECK(bookmark_tracker_);
}

BookmarkModelObserverImpl::~BookmarkModelObserverImpl() = default;

void BookmarkModelObserverImpl::BookmarkModelLoaded(
    bookmarks::BookmarkModel* model,
    bool ids_reassigned) {
  NOTIMPLEMENTED();
}

void BookmarkModelObserverImpl::BookmarkModelBeingDeleted(
    bookmarks::BookmarkModel* model) {
  NOTIMPLEMENTED();
}

void BookmarkModelObserverImpl::BookmarkNodeMoved(
    bookmarks::BookmarkModel* model,
    const bookmarks::BookmarkNode* old_parent,
    int old_index,
    const bookmarks::BookmarkNode* new_parent,
    int new_index) {
  const bookmarks::BookmarkNode* node = new_parent->GetChild(new_index);

  // We shouldn't see changes to the top-level nodes.
  DCHECK(!model->is_permanent_node(node));
  // TODO(crbug.com/516866): continue only if
  // model->client()->CanSyncNode(node).
  const SyncedBookmarkTracker::Entity* entity =
      bookmark_tracker_->GetEntityForBookmarkNode(node);
  DCHECK(entity);

  const std::string& sync_id = entity->metadata()->server_id();
  const base::Time modification_time = base::Time::Now();

  const sync_pb::UniquePosition unique_position =
      ComputePosition(*new_parent, new_index, sync_id).ToProto();

  sync_pb::EntitySpecifics specifics = CreateSpecificsFromBookmarkNode(node);

  bookmark_tracker_->Update(sync_id, entity->metadata()->server_version(),
                            modification_time, unique_position, specifics);
  // Mark the entity that it needs to be committed.
  bookmark_tracker_->IncrementSequenceNumber(sync_id);
  nudge_for_commit_closure_.Run();
}

void BookmarkModelObserverImpl::BookmarkNodeAdded(
    bookmarks::BookmarkModel* model,
    const bookmarks::BookmarkNode* parent,
    int index) {
  const bookmarks::BookmarkNode* node = parent->GetChild(index);
  // TODO(crbug.com/516866): continue only if
  // model->client()->CanSyncNode(node).

  const SyncedBookmarkTracker::Entity* parent_entity =
      bookmark_tracker_->GetEntityForBookmarkNode(parent);
  if (!parent_entity) {
    DLOG(WARNING) << "Bookmark parent lookup failed";
    return;
  }
  // Similar to the diectory implementation here:
  // https://cs.chromium.org/chromium/src/components/sync/syncable/mutable_entry.cc?l=237&gsn=CreateEntryKernel
  // Assign a temp server id for the entity. Will be overriden by the actual
  // server id upon receiving commit response.
  const std::string sync_id = base::GenerateGUID();
  const int64_t server_version = syncer::kUncommittedVersion;
  const base::Time creation_time = base::Time::Now();
  const sync_pb::UniquePosition unique_position =
      ComputePosition(*parent, index, sync_id).ToProto();

  sync_pb::EntitySpecifics specifics = CreateSpecificsFromBookmarkNode(node);

  bookmark_tracker_->Add(sync_id, node, server_version, creation_time,
                         unique_position, specifics);
  // Mark the entity that it needs to be committed.
  bookmark_tracker_->IncrementSequenceNumber(sync_id);
  nudge_for_commit_closure_.Run();
}

void BookmarkModelObserverImpl::OnWillRemoveBookmarks(
    bookmarks::BookmarkModel* model,
    const bookmarks::BookmarkNode* parent,
    int old_index,
    const bookmarks::BookmarkNode* node) {
  // TODO(crbug.com/516866): continue only if
  // model->client()->CanSyncNode(node).
  ProcessDelete(parent, node);
  nudge_for_commit_closure_.Run();
}

void BookmarkModelObserverImpl::BookmarkNodeRemoved(
    bookmarks::BookmarkModel* model,
    const bookmarks::BookmarkNode* parent,
    int old_index,
    const bookmarks::BookmarkNode* node,
    const std::set<GURL>& removed_urls) {
  // All the work should have already been done in OnWillRemoveBookmarks.
  DCHECK(bookmark_tracker_->GetEntityForBookmarkNode(node) == nullptr);
}

void BookmarkModelObserverImpl::BookmarkAllUserNodesRemoved(
    bookmarks::BookmarkModel* model,
    const std::set<GURL>& removed_urls) {
  NOTIMPLEMENTED();
}

void BookmarkModelObserverImpl::BookmarkNodeChanged(
    bookmarks::BookmarkModel* model,
    const bookmarks::BookmarkNode* node) {
  // TODO(crbug.com/516866): continue only if
  // model->client()->CanSyncNode(node).

  // We shouldn't see changes to the top-level nodes.
  DCHECK(!model->is_permanent_node(node));

  const SyncedBookmarkTracker::Entity* entity =
      bookmark_tracker_->GetEntityForBookmarkNode(node);
  DCHECK(entity);
  const std::string& sync_id = entity->metadata()->server_id();
  const base::Time modification_time = base::Time::Now();
  sync_pb::EntitySpecifics specifics = CreateSpecificsFromBookmarkNode(node);

  bookmark_tracker_->Update(sync_id, entity->metadata()->server_version(),
                            modification_time,
                            entity->metadata()->unique_position(), specifics);
  // Mark the entity that it needs to be committed.
  bookmark_tracker_->IncrementSequenceNumber(sync_id);
  nudge_for_commit_closure_.Run();
}

void BookmarkModelObserverImpl::BookmarkMetaInfoChanged(
    bookmarks::BookmarkModel* model,
    const bookmarks::BookmarkNode* node) {
  BookmarkNodeChanged(model, node);
}

void BookmarkModelObserverImpl::BookmarkNodeFaviconChanged(
    bookmarks::BookmarkModel* model,
    const bookmarks::BookmarkNode* node) {
  NOTIMPLEMENTED();
}

void BookmarkModelObserverImpl::BookmarkNodeChildrenReordered(
    bookmarks::BookmarkModel* model,
    const bookmarks::BookmarkNode* node) {
  // TODO(crbug.com/516866): continue only if
  // model->client()->CanSyncNode(node).

  // The given node's children got reordered. We need to reorder all the
  // corresponding sync node.

  // TODO(crbug/com/516866): Make sure that single-move case doesn't produce
  // unnecessary updates. One approach would be something like:
  // 1. Find a subsequence of elements in the beginning of the vector that is
  //    already sorted.
  // 2. The same for the end.
  // 3. If the two overlap, adjust so they don't.
  // 4. Sort the middle, using Between (e.g. recursive implementation).

  syncer::UniquePosition position;
  syncer::UniquePosition previous_position;
  for (int i = 0; i < node->child_count(); ++i) {
    const bookmarks::BookmarkNode* child = node->GetChild(i);

    const SyncedBookmarkTracker::Entity* entity =
        bookmark_tracker_->GetEntityForBookmarkNode(child);
    DCHECK(entity);

    const std::string& sync_id = entity->metadata()->server_id();
    const std::string suffix = syncer::GenerateSyncableBookmarkHash(
        bookmark_tracker_->model_type_state().cache_guid(), sync_id);
    const base::Time modification_time = base::Time::Now();

    if (i == 0) {
      position = syncer::UniquePosition::InitialPosition(suffix);
    } else {
      position = syncer::UniquePosition::After(previous_position, suffix);
    }

    previous_position = position;

    const sync_pb::EntitySpecifics specifics =
        CreateSpecificsFromBookmarkNode(node);

    bookmark_tracker_->Update(sync_id, entity->metadata()->server_version(),
                              modification_time, position.ToProto(), specifics);
    // Mark the entity that it needs to be committed.
    bookmark_tracker_->IncrementSequenceNumber(sync_id);
  }
  nudge_for_commit_closure_.Run();
}

syncer::UniquePosition BookmarkModelObserverImpl::ComputePosition(
    const bookmarks::BookmarkNode& parent,
    int index,
    const std::string& sync_id) {
  const std::string& suffix = syncer::GenerateSyncableBookmarkHash(
      bookmark_tracker_->model_type_state().cache_guid(), sync_id);
  DCHECK_NE(0, parent.child_count());

  if (parent.child_count() == 1) {
    // No siblings, the parent has no other children.
    return syncer::UniquePosition::InitialPosition(suffix);
  }
  if (index == 0) {
    const bookmarks::BookmarkNode* successor_node = parent.GetChild(1);
    const SyncedBookmarkTracker::Entity* successor_entity =
        bookmark_tracker_->GetEntityForBookmarkNode(successor_node);
    DCHECK(successor_entity);
    // Insert at the beginning.
    return syncer::UniquePosition::Before(
        syncer::UniquePosition::FromProto(
            successor_entity->metadata()->unique_position()),
        suffix);
  }
  if (index == parent.child_count() - 1) {
    // Insert at the end.
    const bookmarks::BookmarkNode* predecessor_node =
        parent.GetChild(index - 1);
    const SyncedBookmarkTracker::Entity* predecessor_entity =
        bookmark_tracker_->GetEntityForBookmarkNode(predecessor_node);
    DCHECK(predecessor_entity);
    return syncer::UniquePosition::After(
        syncer::UniquePosition::FromProto(
            predecessor_entity->metadata()->unique_position()),
        suffix);
  }
  // Insert in the middle.
  const bookmarks::BookmarkNode* successor_node = parent.GetChild(index + 1);
  const SyncedBookmarkTracker::Entity* successor_entity =
      bookmark_tracker_->GetEntityForBookmarkNode(successor_node);
  DCHECK(successor_entity);
  const bookmarks::BookmarkNode* predecessor_node = parent.GetChild(index - 1);
  const SyncedBookmarkTracker::Entity* predecessor_entity =
      bookmark_tracker_->GetEntityForBookmarkNode(predecessor_node);
  DCHECK(predecessor_entity);
  return syncer::UniquePosition::Between(
      syncer::UniquePosition::FromProto(
          predecessor_entity->metadata()->unique_position()),
      syncer::UniquePosition::FromProto(
          successor_entity->metadata()->unique_position()),
      suffix);
}

void BookmarkModelObserverImpl::ProcessDelete(
    const bookmarks::BookmarkNode* parent,
    const bookmarks::BookmarkNode* node) {
  // If not a leaf node, process all children first.
  for (int i = 0; i < node->child_count(); ++i) {
    const bookmarks::BookmarkNode* child = node->GetChild(i);
    ProcessDelete(node, child);
  }
  // Process the current node.
  const SyncedBookmarkTracker::Entity* entity =
      bookmark_tracker_->GetEntityForBookmarkNode(node);
  // Shouldn't try to delete untracked entities.
  DCHECK(entity);
  const std::string& sync_id = entity->metadata()->server_id();
  bookmark_tracker_->MarkDeleted(sync_id);
  // Mark the entity that it needs to be committed.
  bookmark_tracker_->IncrementSequenceNumber(sync_id);
}

}  // namespace sync_bookmarks
