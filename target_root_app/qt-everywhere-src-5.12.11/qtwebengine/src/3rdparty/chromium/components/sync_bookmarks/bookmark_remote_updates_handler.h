// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_BOOKMARKS_BOOKMARK_REMOTE_UPDATES_HANDLER_H_
#define COMPONENTS_SYNC_BOOKMARKS_BOOKMARK_REMOTE_UPDATES_HANDLER_H_

#include <vector>

#include "components/sync/engine/non_blocking_sync_common.h"
#include "components/sync_bookmarks/synced_bookmark_tracker.h"

namespace bookmarks {
class BookmarkModel;
class BookmarkNode;
}  // namespace bookmarks

namespace sync_bookmarks {

// Responsible for processing remote updates received from the sync server.
class BookmarkRemoteUpdatesHandler {
 public:
  // |bookmark_model| and |bookmark_tracker| must not be null and most outlive
  // this object.
  BookmarkRemoteUpdatesHandler(bookmarks::BookmarkModel* bookmark_model,
                               SyncedBookmarkTracker* bookmark_tracker);
  // Processes the updates received from the sync server in |updates| and
  // updates the |bookmark_model_| and |bookmark_tracker_| accordingly.
  void Process(const syncer::UpdateResponseDataList& updates);

  // Public for testing.
  static std::vector<const syncer::UpdateResponseData*> ReorderUpdatesForTest(
      const syncer::UpdateResponseDataList& updates);

 private:
  // Reorders incoming updates such that parent creation is before child
  // creation and child deletion is before parent deletion, and deletions should
  // come last. The returned pointers point to the elements in the original
  // |updates|.
  static std::vector<const syncer::UpdateResponseData*> ReorderUpdates(
      const syncer::UpdateResponseDataList& updates);

  // Given a remote update entity, it returns the parent bookmark node of the
  // corresponding node. It returns null if the parent node cannot be found.
  const bookmarks::BookmarkNode* GetParentNode(
      const syncer::EntityData& update_entity) const;

  // Processes a remote creation of a bookmark node.
  // 1. For permanent folders, they are only registered in |bookmark_tracker_|.
  // 2. If the nodes parent cannot be found, the remote creation update is
  //    ignored.
  // 3. Otherwise, a new node is created in the local bookmark model and
  //    registered in |bookmark_tracker_|.
  void ProcessRemoteCreate(const syncer::UpdateResponseData& update);

  // Processes a remote update of a bookmark node. |update| must not be a
  // deletion, and the server_id must be already tracked, otherwise, it is a
  // creation that gets handeled in ProcessRemoteCreate(). |tracked_entity| is
  // the tracked entity for that server_id. It is passed as a dependency instead
  // of performing a lookup inside ProcessRemoteUpdate() to avoid wasting CPU
  // cycles for doing another lookup (this code runs on the UI thread).
  void ProcessRemoteUpdate(const syncer::UpdateResponseData& update,
                           const SyncedBookmarkTracker::Entity* tracked_entity);

  // Process a remote delete of a bookmark node. |update_entity| must not be a
  // deletion. |tracked_entity| is the tracked entity for that server_id. It is
  // passed as a dependency instead of performing a lookup inside
  // ProcessRemoteDelete() to avoid wasting CPU cycles for doing another lookup
  // (this code runs on the UI thread).
  void ProcessRemoteDelete(const syncer::EntityData& update_entity,
                           const SyncedBookmarkTracker::Entity* tracked_entity);

  // Associates the permanent bookmark folders with the corresponding server
  // side ids and registers the association in |bookmark_tracker_|.
  // |update_entity| must contain server_defined_unique_tag that is used to
  // determine the corresponding permanent node. All permanent nodes are assumed
  // to be directly children nodes of |kBookmarksRootId|. This method is used in
  // the initial sync cycle only.
  void AssociatePermanentFolder(const syncer::UpdateResponseData& update);

  bookmarks::BookmarkModel* const bookmark_model_;
  SyncedBookmarkTracker* const bookmark_tracker_;

  DISALLOW_COPY_AND_ASSIGN(BookmarkRemoteUpdatesHandler);
};

}  // namespace sync_bookmarks

#endif  // COMPONENTS_SYNC_BOOKMARKS_BOOKMARK_REMOTE_UPDATES_HANDLER_H_
