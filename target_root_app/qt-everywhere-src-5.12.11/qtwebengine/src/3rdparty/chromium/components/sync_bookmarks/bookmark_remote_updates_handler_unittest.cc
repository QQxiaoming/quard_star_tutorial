// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync_bookmarks/bookmark_remote_updates_handler.h"

#include <string>

#include "components/sync/base/model_type.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::Eq;

namespace sync_bookmarks {

namespace {

// The parent tag for children of the root entity. Entities with this parent are
// referred to as top level enities.
const char kRootParentTag[] = "0";
const char kBookmarkBarTag[] = "bookmark_bar";
const char kBookmarksRootId[] = "32904_google_chrome_bookmarks";

struct BookmarkInfo {
  std::string server_id;
  std::string title;
  std::string url;  // empty for folders.
  std::string parent_id;
  std::string server_tag;
};

syncer::UpdateResponseData CreateUpdateData(const BookmarkInfo& bookmark_info) {
  syncer::EntityData data;
  data.id = bookmark_info.server_id;
  data.parent_id = bookmark_info.parent_id;
  data.server_defined_unique_tag = bookmark_info.server_tag;

  sync_pb::BookmarkSpecifics* bookmark_specifics =
      data.specifics.mutable_bookmark();
  bookmark_specifics->set_title(bookmark_info.title);
  if (bookmark_info.url.empty()) {
    data.is_folder = true;
  } else {
    bookmark_specifics->set_url(bookmark_info.url);
  }

  syncer::UpdateResponseData response_data;
  response_data.entity = data.PassToPtr();
  // Similar to what's done in the loopback_server.
  response_data.response_version = 0;
  return response_data;
}

syncer::UpdateResponseData CreateBookmarkRootUpdateData() {
  return CreateUpdateData({syncer::ModelTypeToRootTag(syncer::BOOKMARKS),
                           std::string(), std::string(), kRootParentTag,
                           syncer::ModelTypeToRootTag(syncer::BOOKMARKS)});
}

TEST(BookmarkRemoteUpdatesHandlerReorderUpdatesTest, ShouldIgnoreRootNodes) {
  syncer::UpdateResponseDataList updates;
  updates.push_back(CreateBookmarkRootUpdateData());
  std::vector<const syncer::UpdateResponseData*> ordered_updates =
      BookmarkRemoteUpdatesHandler::ReorderUpdatesForTest(updates);
  // Root node update should be filtered out.
  EXPECT_THAT(ordered_updates.size(), Eq(0U));
}

// TODO(crbug.com/516866): This should change to cover the general case of
// parents before children for non-deletions, and another test should be added
// for children before parents for deletions.
TEST(BookmarkRemoteUpdatesHandlerReorderUpdatesTest,
     ShouldPlacePermanentNodesFirstForNonDeletions) {
  const std::string kNode1Id = "node1";
  const std::string kNode2Id = "node2";
  syncer::UpdateResponseDataList updates;
  updates.push_back(CreateUpdateData(
      {kNode1Id, std::string(), std::string(), kNode2Id, std::string()}));
  updates.push_back(CreateUpdateData({kNode2Id, std::string(), std::string(),
                                      kBookmarksRootId, kBookmarkBarTag}));
  std::vector<const syncer::UpdateResponseData*> ordered_updates =
      BookmarkRemoteUpdatesHandler::ReorderUpdatesForTest(updates);

  // No update should be dropped.
  ASSERT_THAT(ordered_updates.size(), Eq(2U));

  // Updates should be ordered such that parent node update comes first.
  EXPECT_THAT(ordered_updates[0]->entity.value().id, Eq(kNode2Id));
  EXPECT_THAT(ordered_updates[1]->entity.value().id, Eq(kNode1Id));
}

}  // namespace

}  // namespace sync_bookmarks
