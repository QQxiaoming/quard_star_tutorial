// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync_bookmarks/synced_bookmark_tracker.h"

#include <utility>

#include "base/base64.h"
#include "base/sha1.h"
#include "components/bookmarks/browser/bookmark_node.h"
#include "components/sync/base/time.h"
#include "components/sync/model/entity_data.h"

namespace sync_bookmarks {

namespace {

void HashSpecifics(const sync_pb::EntitySpecifics& specifics,
                   std::string* hash) {
  DCHECK_GT(specifics.ByteSize(), 0);
  base::Base64Encode(base::SHA1HashString(specifics.SerializeAsString()), hash);
}

}  // namespace

SyncedBookmarkTracker::Entity::Entity(
    const bookmarks::BookmarkNode* bookmark_node,
    std::unique_ptr<sync_pb::EntityMetadata> metadata)
    : bookmark_node_(bookmark_node), metadata_(std::move(metadata)) {
  if (bookmark_node) {
    DCHECK(!metadata_->is_deleted());
  } else {
    DCHECK(metadata_->is_deleted());
  }
}

SyncedBookmarkTracker::Entity::~Entity() = default;

bool SyncedBookmarkTracker::Entity::IsUnsynced() const {
  return metadata_->sequence_number() > metadata_->acked_sequence_number();
}

bool SyncedBookmarkTracker::Entity::MatchesData(
    const syncer::EntityData& data) const {
  // TODO(crbug.com/516866): Check parent id and unique position.
  // TODO(crbug.com/516866): Compare the actual specifics instead of the
  // specifics hash.
  if (metadata_->is_deleted() || data.is_deleted()) {
    // In case of deletion, no need to check the specifics.
    return metadata_->is_deleted() == data.is_deleted();
  }
  return MatchesSpecificsHash(data.specifics);
}

bool SyncedBookmarkTracker::Entity::MatchesSpecificsHash(
    const sync_pb::EntitySpecifics& specifics) const {
  DCHECK(!metadata_->is_deleted());
  DCHECK_GT(specifics.ByteSize(), 0);
  std::string hash;
  HashSpecifics(specifics, &hash);
  return hash == metadata_->specifics_hash();
}

SyncedBookmarkTracker::SyncedBookmarkTracker(
    std::vector<NodeMetadataPair> nodes_metadata,
    std::unique_ptr<sync_pb::ModelTypeState> model_type_state)
    : model_type_state_(std::move(model_type_state)) {
  DCHECK(model_type_state_);
  for (NodeMetadataPair& node_metadata : nodes_metadata) {
    const std::string& sync_id = node_metadata.second->server_id();
    auto entity = std::make_unique<Entity>(node_metadata.first,
                                           std::move(node_metadata.second));
    if (node_metadata.first) {
      // Non-null node means it's not a tombstone.
      bookmark_node_to_entities_map_[node_metadata.first] = entity.get();
    } else {
      // Otherwise, it must be a deletion so we must remember to deletion
      // ordering.
      DCHECK(entity->metadata()->is_deleted());
      ordered_local_tombstones_.push_back(entity.get());
    }
    sync_id_to_entities_map_[sync_id] = std::move(entity);
  }
}

SyncedBookmarkTracker::~SyncedBookmarkTracker() = default;

const SyncedBookmarkTracker::Entity* SyncedBookmarkTracker::GetEntityForSyncId(
    const std::string& sync_id) const {
  auto it = sync_id_to_entities_map_.find(sync_id);
  return it != sync_id_to_entities_map_.end() ? it->second.get() : nullptr;
}

const SyncedBookmarkTracker::Entity*
SyncedBookmarkTracker::GetEntityForBookmarkNode(
    const bookmarks::BookmarkNode* node) const {
  auto it = bookmark_node_to_entities_map_.find(node);
  return it != bookmark_node_to_entities_map_.end() ? it->second : nullptr;
}

void SyncedBookmarkTracker::Add(const std::string& sync_id,
                                const bookmarks::BookmarkNode* bookmark_node,
                                int64_t server_version,
                                base::Time creation_time,
                                const sync_pb::UniquePosition& unique_position,
                                const sync_pb::EntitySpecifics& specifics) {
  DCHECK_GT(specifics.ByteSize(), 0);
  auto metadata = std::make_unique<sync_pb::EntityMetadata>();
  metadata->set_is_deleted(false);
  metadata->set_server_id(sync_id);
  metadata->set_server_version(server_version);
  metadata->set_creation_time(syncer::TimeToProtoTime(creation_time));
  metadata->set_modification_time(syncer::TimeToProtoTime(creation_time));
  metadata->set_sequence_number(0);
  metadata->set_acked_sequence_number(0);
  metadata->mutable_unique_position()->CopyFrom(unique_position);
  HashSpecifics(specifics, metadata->mutable_specifics_hash());
  auto entity = std::make_unique<Entity>(bookmark_node, std::move(metadata));
  bookmark_node_to_entities_map_[bookmark_node] = entity.get();
  sync_id_to_entities_map_[sync_id] = std::move(entity);
}

void SyncedBookmarkTracker::Update(
    const std::string& sync_id,
    int64_t server_version,
    base::Time modification_time,
    const sync_pb::UniquePosition& unique_position,
    const sync_pb::EntitySpecifics& specifics) {
  DCHECK_GT(specifics.ByteSize(), 0);
  auto it = sync_id_to_entities_map_.find(sync_id);
  Entity* entity = it->second.get();
  DCHECK(entity);
  entity->metadata()->set_server_id(sync_id);
  entity->metadata()->set_server_version(server_version);
  entity->metadata()->set_modification_time(
      syncer::TimeToProtoTime(modification_time));
  *entity->metadata()->mutable_unique_position() = unique_position;
  HashSpecifics(specifics, entity->metadata()->mutable_specifics_hash());
  // TODO(crbug.com/516866): in case of conflict, the entity might exist in
  // |ordered_local_tombstones_| as well if it has been locally deleted.
}

void SyncedBookmarkTracker::MarkDeleted(const std::string& sync_id) {
  auto it = sync_id_to_entities_map_.find(sync_id);
  Entity* entity = it->second.get();
  DCHECK(entity);
  entity->metadata()->set_is_deleted(true);
  // Clear all references to the deleted bookmark node.
  bookmark_node_to_entities_map_.erase(entity->bookmark_node());
  entity->clear_bookmark_node();
  ordered_local_tombstones_.push_back(entity);
}

void SyncedBookmarkTracker::Remove(const std::string& sync_id) {
  const Entity* entity = GetEntityForSyncId(sync_id);
  DCHECK(entity);
  bookmark_node_to_entities_map_.erase(entity->bookmark_node());
  ordered_local_tombstones_.erase(
      std::remove(ordered_local_tombstones_.begin(),
                  ordered_local_tombstones_.end(), entity),
      ordered_local_tombstones_.end());
  sync_id_to_entities_map_.erase(sync_id);
}

void SyncedBookmarkTracker::IncrementSequenceNumber(
    const std::string& sync_id) {
  Entity* entity = sync_id_to_entities_map_.find(sync_id)->second.get();
  DCHECK(entity);
  // TODO(crbug.com/516866): Update base hash specifics here if the entity is
  // not already out of sync.
  entity->metadata()->set_sequence_number(
      entity->metadata()->sequence_number() + 1);
}

sync_pb::BookmarkModelMetadata
SyncedBookmarkTracker::BuildBookmarkModelMetadata() const {
  sync_pb::BookmarkModelMetadata model_metadata;
  for (const std::pair<const std::string, std::unique_ptr<Entity>>& pair :
       sync_id_to_entities_map_) {
    if (pair.second->metadata()->is_deleted()) {
      // Deletions will be added later because they need to maintain the same
      // order as in |ordered_local_tombstones_|.
      continue;
    }
    DCHECK(pair.second->bookmark_node());
    sync_pb::BookmarkMetadata* bookmark_metadata =
        model_metadata.add_bookmarks_metadata();
    bookmark_metadata->set_id(pair.second->bookmark_node()->id());
    *bookmark_metadata->mutable_metadata() = *pair.second->metadata();
  }
  // Add pending deletions.
  for (const Entity* tombstone_entity : ordered_local_tombstones_) {
    DCHECK(tombstone_entity->metadata()->is_deleted());
    sync_pb::BookmarkMetadata* bookmark_metadata =
        model_metadata.add_bookmarks_metadata();
    *bookmark_metadata->mutable_metadata() = *tombstone_entity->metadata();
  }
  *model_metadata.mutable_model_type_state() = *model_type_state_;
  return model_metadata;
}

bool SyncedBookmarkTracker::HasLocalChanges() const {
  for (const std::pair<const std::string, std::unique_ptr<Entity>>& pair :
       sync_id_to_entities_map_) {
    Entity* entity = pair.second.get();
    if (entity->IsUnsynced()) {
      return true;
    }
  }
  return false;
}

std::vector<const SyncedBookmarkTracker::Entity*>
SyncedBookmarkTracker::GetEntitiesWithLocalChanges(size_t max_entries) const {
  // TODO(crbug.com/516866): Reorder local changes to e.g. parent creation
  // before child creation and the otherway around for deletions.
  // TODO(crbug.com/516866): Return no more than |max_entries|.
  std::vector<const SyncedBookmarkTracker::Entity*> entities_with_local_changes;
  for (const std::pair<const std::string, std::unique_ptr<Entity>>& pair :
       sync_id_to_entities_map_) {
    Entity* entity = pair.second.get();
    if (entity->metadata()->is_deleted()) {
      // Deletion are stored sorted in |ordered_local_tombstones_| and will be
      // added later.
      continue;
    }
    if (entity->IsUnsynced()) {
      entities_with_local_changes.push_back(entity);
    }
  }
  for (const Entity* tombstone_entity : ordered_local_tombstones_) {
    entities_with_local_changes.push_back(tombstone_entity);
  }
  return entities_with_local_changes;
}

void SyncedBookmarkTracker::UpdateUponCommitResponse(
    const std::string& old_id,
    const std::string& new_id,
    int64_t acked_sequence_number,
    int64_t server_version) {
  // TODO(crbug.com/516866): Update specifics if we decide to keep it.
  auto it = sync_id_to_entities_map_.find(old_id);
  Entity* entity =
      it != sync_id_to_entities_map_.end() ? it->second.get() : nullptr;
  if (!entity) {
    DLOG(WARNING) << "Trying to update a non existing entity.";
    return;
  }

  entity->metadata()->set_acked_sequence_number(acked_sequence_number);
  entity->metadata()->set_server_version(server_version);
  // If there are no pending commits, remove tombstones.
  if (!entity->IsUnsynced() && entity->metadata()->is_deleted()) {
    Remove(old_id);
    return;
  }

  if (old_id != new_id) {
    auto it = sync_id_to_entities_map_.find(old_id);
    entity->metadata()->set_server_id(new_id);
    sync_id_to_entities_map_[new_id] = std::move(it->second);
    sync_id_to_entities_map_.erase(old_id);
  }
}

std::size_t SyncedBookmarkTracker::TrackedEntitiesCountForTest() const {
  return sync_id_to_entities_map_.size();
}

}  // namespace sync_bookmarks
