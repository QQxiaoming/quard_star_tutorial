// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync_bookmarks/synced_bookmark_tracker.h"

#include "base/base64.h"
#include "base/sha1.h"
#include "components/bookmarks/browser/bookmark_node.h"
#include "components/sync/base/time.h"
#include "components/sync/base/unique_position.h"
#include "components/sync/model/entity_data.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::Eq;
using testing::IsNull;
using testing::NotNull;

namespace sync_bookmarks {

namespace {

sync_pb::EntitySpecifics GenerateSpecifics(const std::string& title,
                                           const std::string& url) {
  sync_pb::EntitySpecifics specifics;
  specifics.mutable_bookmark()->set_title(title);
  specifics.mutable_bookmark()->set_url(url);
  return specifics;
}

TEST(SyncedBookmarkTrackerTest, ShouldGetAssociatedNodes) {
  SyncedBookmarkTracker tracker(std::vector<NodeMetadataPair>(),
                                std::make_unique<sync_pb::ModelTypeState>());
  const std::string kSyncId = "SYNC_ID";
  const std::string kTitle = "Title";
  const GURL kUrl("http://www.foo.com");
  const int64_t kId = 1;
  const int64_t kServerVersion = 1000;
  const base::Time kCreationTime(base::Time::Now() -
                                 base::TimeDelta::FromSeconds(1));
  const syncer::UniquePosition unique_position =
      syncer::UniquePosition::InitialPosition(
          syncer::UniquePosition::RandomSuffix());
  const sync_pb::EntitySpecifics specifics =
      GenerateSpecifics(/*title=*/std::string(), /*url=*/std::string());

  bookmarks::BookmarkNode node(kId, kUrl);
  tracker.Add(kSyncId, &node, kServerVersion, kCreationTime,
              unique_position.ToProto(), specifics);
  const SyncedBookmarkTracker::Entity* entity =
      tracker.GetEntityForSyncId(kSyncId);
  ASSERT_THAT(entity, NotNull());
  EXPECT_THAT(entity->bookmark_node(), Eq(&node));
  EXPECT_THAT(entity->metadata()->server_id(), Eq(kSyncId));
  EXPECT_THAT(entity->metadata()->server_version(), Eq(kServerVersion));
  EXPECT_THAT(entity->metadata()->creation_time(),
              Eq(syncer::TimeToProtoTime(kCreationTime)));
  EXPECT_TRUE(
      syncer::UniquePosition::FromProto(entity->metadata()->unique_position())
          .Equals(unique_position));

  syncer::EntityData data;
  *data.specifics.mutable_bookmark() = specifics.bookmark();
  EXPECT_TRUE(entity->MatchesData(data));
  EXPECT_THAT(tracker.GetEntityForSyncId("unknown id"), IsNull());
}

TEST(SyncedBookmarkTrackerTest, ShouldReturnNullForDisassociatedNodes) {
  SyncedBookmarkTracker tracker(std::vector<NodeMetadataPair>(),
                                std::make_unique<sync_pb::ModelTypeState>());
  const std::string kSyncId = "SYNC_ID";
  const int64_t kId = 1;
  const int64_t kServerVersion = 1000;
  const base::Time kModificationTime(base::Time::Now() -
                                     base::TimeDelta::FromSeconds(1));
  const sync_pb::UniquePosition unique_position;
  const sync_pb::EntitySpecifics specifics =
      GenerateSpecifics(/*title=*/std::string(), /*url=*/std::string());
  bookmarks::BookmarkNode node(kId, GURL());
  tracker.Add(kSyncId, &node, kServerVersion, kModificationTime,
              unique_position, specifics);
  ASSERT_THAT(tracker.GetEntityForSyncId(kSyncId), NotNull());
  tracker.Remove(kSyncId);
  EXPECT_THAT(tracker.GetEntityForSyncId(kSyncId), IsNull());
}

TEST(SyncedBookmarkTrackerTest,
     ShouldRequireCommitRequestWhenSequenceNumberIsIncremented) {
  SyncedBookmarkTracker tracker(std::vector<NodeMetadataPair>(),
                                std::make_unique<sync_pb::ModelTypeState>());
  const std::string kSyncId = "SYNC_ID";
  const int64_t kId = 1;
  const int64_t kServerVersion = 1000;
  const base::Time kModificationTime(base::Time::Now() -
                                     base::TimeDelta::FromSeconds(1));
  const sync_pb::UniquePosition unique_position;
  const sync_pb::EntitySpecifics specifics =
      GenerateSpecifics(/*title=*/std::string(), /*url=*/std::string());
  bookmarks::BookmarkNode node(kId, GURL());
  tracker.Add(kSyncId, &node, kServerVersion, kModificationTime,
              unique_position, specifics);

  EXPECT_THAT(tracker.HasLocalChanges(), Eq(false));
  tracker.IncrementSequenceNumber(kSyncId);
  EXPECT_THAT(tracker.HasLocalChanges(), Eq(true));
  // TODO(crbug.com/516866): Test HasLocalChanges after submitting commit
  // request in a separate test probably.
}

TEST(SyncedBookmarkTrackerTest, ShouldUpdateUponCommitResponseWithNewId) {
  SyncedBookmarkTracker tracker(std::vector<NodeMetadataPair>(),
                                std::make_unique<sync_pb::ModelTypeState>());
  const std::string kSyncId = "SYNC_ID";
  const std::string kNewSyncId = "NEW_SYNC_ID";
  const int64_t kId = 1;
  const int64_t kServerVersion = 1000;
  const int64_t kNewServerVersion = 1001;
  const base::Time kModificationTime(base::Time::Now() -
                                     base::TimeDelta::FromSeconds(1));
  const sync_pb::UniquePosition unique_position;
  const sync_pb::EntitySpecifics specifics =
      GenerateSpecifics(/*title=*/std::string(), /*url=*/std::string());
  bookmarks::BookmarkNode node(kId, GURL());
  tracker.Add(kSyncId, &node, kServerVersion, kModificationTime,
              unique_position, specifics);
  ASSERT_THAT(tracker.GetEntityForSyncId(kSyncId), NotNull());
  // Receive a commit response with a changed id.
  tracker.UpdateUponCommitResponse(
      kSyncId, kNewSyncId, /*acked_sequence_number=*/1, kNewServerVersion);
  // Old id shouldn't be there.
  EXPECT_THAT(tracker.GetEntityForSyncId(kSyncId), IsNull());

  const SyncedBookmarkTracker::Entity* entity =
      tracker.GetEntityForSyncId(kNewSyncId);
  ASSERT_THAT(entity, NotNull());
  EXPECT_THAT(entity->metadata()->server_id(), Eq(kNewSyncId));
  EXPECT_THAT(entity->bookmark_node(), Eq(&node));
  EXPECT_THAT(entity->metadata()->server_version(), Eq(kNewServerVersion));
}

TEST(SyncedBookmarkTrackerTest,
     ShouldMaintainTombstoneOrderBetweenCtorAndBuildBookmarkModelMetadata) {
  // Feed a metadata batch of 5 entries to the constructor of the tracker.
  // First 2 are for node, and the last 4 are for tombstones.

  // Server ids.
  const std::string kId0 = "id0";
  const std::string kId1 = "id1";
  const std::string kId2 = "id2";
  const std::string kId3 = "id3";
  const std::string kId4 = "id4";

  const GURL kUrl("http://www.foo.com");
  bookmarks::BookmarkNode node0(/*id=*/0, kUrl);
  bookmarks::BookmarkNode node1(/*id=*/1, kUrl);

  auto metadata0 = std::make_unique<sync_pb::EntityMetadata>();
  metadata0->set_server_id(kId0);

  auto metadata1 = std::make_unique<sync_pb::EntityMetadata>();
  metadata1->set_server_id(kId1);

  auto metadata2 = std::make_unique<sync_pb::EntityMetadata>();
  metadata2->set_server_id(kId2);
  metadata2->set_is_deleted(true);

  auto metadata3 = std::make_unique<sync_pb::EntityMetadata>();
  metadata3->set_server_id(kId3);
  metadata3->set_is_deleted(true);

  auto metadata4 = std::make_unique<sync_pb::EntityMetadata>();
  metadata4->set_server_id(kId4);
  metadata4->set_is_deleted(true);

  std::vector<NodeMetadataPair> node_metadata_pairs;
  node_metadata_pairs.emplace_back(&node0, std::move(metadata0));
  node_metadata_pairs.emplace_back(&node1, std::move(metadata1));
  node_metadata_pairs.emplace_back(nullptr, std::move(metadata2));
  node_metadata_pairs.emplace_back(nullptr, std::move(metadata3));
  node_metadata_pairs.emplace_back(nullptr, std::move(metadata4));

  SyncedBookmarkTracker tracker(std::move(node_metadata_pairs),
                                std::make_unique<sync_pb::ModelTypeState>());

  sync_pb::BookmarkModelMetadata bookmark_model_metadata =
      tracker.BuildBookmarkModelMetadata();

  // Tombstones should be the last 3 entries in the metadata and in the same
  // order as given to the constructor.
  ASSERT_THAT(bookmark_model_metadata.bookmarks_metadata().size(), Eq(5));
  EXPECT_THAT(
      bookmark_model_metadata.bookmarks_metadata(2).metadata().server_id(),
      Eq(kId2));
  EXPECT_THAT(
      bookmark_model_metadata.bookmarks_metadata(3).metadata().server_id(),
      Eq(kId3));
  EXPECT_THAT(
      bookmark_model_metadata.bookmarks_metadata(4).metadata().server_id(),
      Eq(kId4));
}

TEST(SyncedBookmarkTrackerTest,
     ShouldMaintainOrderOfMarkDeletedCallsWhenBuildBookmarkModelMetadata) {
  // Server ids.
  const std::string kId0 = "id0";
  const std::string kId1 = "id1";
  const std::string kId2 = "id2";
  const std::string kId3 = "id3";
  const std::string kId4 = "id4";

  const GURL kUrl("http://www.foo.com");
  bookmarks::BookmarkNode node0(/*id=*/0, kUrl);
  bookmarks::BookmarkNode node1(/*id=*/1, kUrl);
  bookmarks::BookmarkNode node2(/*id=*/2, kUrl);
  bookmarks::BookmarkNode node3(/*id=*/3, kUrl);
  bookmarks::BookmarkNode node4(/*id=*/4, kUrl);

  auto metadata0 = std::make_unique<sync_pb::EntityMetadata>();
  metadata0->set_server_id(kId0);

  auto metadata1 = std::make_unique<sync_pb::EntityMetadata>();
  metadata1->set_server_id(kId1);

  auto metadata2 = std::make_unique<sync_pb::EntityMetadata>();
  metadata2->set_server_id(kId2);

  auto metadata3 = std::make_unique<sync_pb::EntityMetadata>();
  metadata3->set_server_id(kId3);

  auto metadata4 = std::make_unique<sync_pb::EntityMetadata>();
  metadata4->set_server_id(kId4);

  std::vector<NodeMetadataPair> node_metadata_pairs;
  node_metadata_pairs.emplace_back(&node0, std::move(metadata0));
  node_metadata_pairs.emplace_back(&node1, std::move(metadata1));
  node_metadata_pairs.emplace_back(&node2, std::move(metadata2));
  node_metadata_pairs.emplace_back(&node3, std::move(metadata3));
  node_metadata_pairs.emplace_back(&node4, std::move(metadata4));

  SyncedBookmarkTracker tracker(std::move(node_metadata_pairs),
                                std::make_unique<sync_pb::ModelTypeState>());

  // Mark entities deleted in that order kId2, kId4, kId1
  tracker.MarkDeleted(kId2);
  tracker.MarkDeleted(kId4);
  tracker.MarkDeleted(kId1);

  sync_pb::BookmarkModelMetadata bookmark_model_metadata =
      tracker.BuildBookmarkModelMetadata();

  // Tombstones should be the last 3 entries in the metadata and in the same as
  // calling MarkDeleted().
  ASSERT_THAT(bookmark_model_metadata.bookmarks_metadata().size(), Eq(5));
  EXPECT_THAT(
      bookmark_model_metadata.bookmarks_metadata(2).metadata().server_id(),
      Eq(kId2));
  EXPECT_THAT(
      bookmark_model_metadata.bookmarks_metadata(3).metadata().server_id(),
      Eq(kId4));
  EXPECT_THAT(
      bookmark_model_metadata.bookmarks_metadata(4).metadata().server_id(),
      Eq(kId1));
}

}  // namespace

}  // namespace sync_bookmarks
