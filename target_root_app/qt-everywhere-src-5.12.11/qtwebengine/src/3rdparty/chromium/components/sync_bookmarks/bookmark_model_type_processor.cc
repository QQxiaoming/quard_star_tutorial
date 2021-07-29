// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync_bookmarks/bookmark_model_type_processor.h"

#include <utility>

#include "base/callback.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_node.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/sync/base/model_type.h"
#include "components/sync/engine/commit_queue.h"
#include "components/sync/engine/model_type_processor_proxy.h"
#include "components/sync/model/data_type_activation_request.h"
#include "components/sync/protocol/bookmark_model_metadata.pb.h"
#include "components/sync_bookmarks/bookmark_local_changes_builder.h"
#include "components/sync_bookmarks/bookmark_model_observer_impl.h"
#include "components/sync_bookmarks/bookmark_remote_updates_handler.h"
#include "components/undo/bookmark_undo_utils.h"

namespace sync_bookmarks {

namespace {

class ScopedRemoteUpdateBookmarks {
 public:
  // |bookmark_model|, |bookmark_undo_service| and |observer| must not be null
  // and must outlive this object.
  ScopedRemoteUpdateBookmarks(bookmarks::BookmarkModel* bookmark_model,
                              BookmarkUndoService* bookmark_undo_service,
                              bookmarks::BookmarkModelObserver* observer)
      : bookmark_model_(bookmark_model),
        suspend_undo_(bookmark_undo_service),
        observer_(observer) {
    // Notify UI intensive observers of BookmarkModel that we are about to make
    // potentially significant changes to it, so the updates may be batched. For
    // example, on Mac, the bookmarks bar displays animations when bookmark
    // items are added or deleted.
    DCHECK(bookmark_model_);
    bookmark_model_->BeginExtensiveChanges();
    // Shouldn't be notified upon changes due to sync.
    bookmark_model_->RemoveObserver(observer_);
  }

  ~ScopedRemoteUpdateBookmarks() {
    // Notify UI intensive observers of BookmarkModel that all updates have been
    // applied, and that they may now be consumed. This prevents issues like the
    // one described in https://crbug.com/281562, where old and new items on the
    // bookmarks bar would overlap.
    bookmark_model_->EndExtensiveChanges();
    bookmark_model_->AddObserver(observer_);
  }

 private:
  bookmarks::BookmarkModel* const bookmark_model_;

  // Changes made to the bookmark model due to sync should not be undoable.
  ScopedSuspendBookmarkUndo suspend_undo_;

  bookmarks::BookmarkModelObserver* const observer_;

  DISALLOW_COPY_AND_ASSIGN(ScopedRemoteUpdateBookmarks);
};

}  // namespace

BookmarkModelTypeProcessor::BookmarkModelTypeProcessor(
    BookmarkUndoService* bookmark_undo_service)
    : bookmark_undo_service_(bookmark_undo_service), weak_ptr_factory_(this) {}

BookmarkModelTypeProcessor::~BookmarkModelTypeProcessor() {
  if (bookmark_model_ && bookmark_model_observer_) {
    bookmark_model_->RemoveObserver(bookmark_model_observer_.get());
  }
}

void BookmarkModelTypeProcessor::ConnectSync(
    std::unique_ptr<syncer::CommitQueue> worker) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(!worker_);
  DCHECK(bookmark_model_);

  worker_ = std::move(worker);

  // |bookmark_tracker_| is instantiated only after initial sync is done.
  if (bookmark_tracker_) {
    NudgeForCommitIfNeeded();
  }
}

void BookmarkModelTypeProcessor::DisconnectSync() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  NOTIMPLEMENTED();
}

void BookmarkModelTypeProcessor::GetLocalChanges(
    size_t max_entries,
    GetLocalChangesCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  BookmarkLocalChangesBuilder builder(bookmark_tracker_.get());
  std::vector<syncer::CommitRequestData> local_changes =
      builder.BuildCommitRequests(max_entries);
  std::move(callback).Run(std::move(local_changes));
}

void BookmarkModelTypeProcessor::OnCommitCompleted(
    const sync_pb::ModelTypeState& type_state,
    const syncer::CommitResponseDataList& response_list) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  for (const syncer::CommitResponseData& response : response_list) {
    // In order to save space, |response.id_in_request| is written when it's
    // different from |response.id|. If it's empty, then there was no id change
    // during the commit, and |response.id| carries both the old and new ids.
    const std::string& old_sync_id =
        response.id_in_request.empty() ? response.id : response.id_in_request;
    bookmark_tracker_->UpdateUponCommitResponse(old_sync_id, response.id,
                                                response.sequence_number,
                                                response.response_version);
  }
  bookmark_tracker_->set_model_type_state(
      std::make_unique<sync_pb::ModelTypeState>(type_state));
  schedule_save_closure_.Run();
}

void BookmarkModelTypeProcessor::OnUpdateReceived(
    const sync_pb::ModelTypeState& model_type_state,
    const syncer::UpdateResponseDataList& updates) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(!model_type_state.cache_guid().empty());
  DCHECK_EQ(model_type_state.cache_guid(), cache_guid_);
  DCHECK(model_type_state.initial_sync_done());

  if (!bookmark_tracker_) {
    // TODO(crbug.com/516866): Implement the merge logic.
    StartTrackingMetadata(
        std::vector<NodeMetadataPair>(),
        std::make_unique<sync_pb::ModelTypeState>(model_type_state));
  }
  // TODO(crbug.com/516866): Set the model type state.

  ScopedRemoteUpdateBookmarks update_bookmarks(
      bookmark_model_, bookmark_undo_service_, bookmark_model_observer_.get());
  BookmarkRemoteUpdatesHandler updates_handler(bookmark_model_,
                                               bookmark_tracker_.get());
  updates_handler.Process(updates);
  // Schedule save just in case one is needed.
  schedule_save_closure_.Run();
}

const SyncedBookmarkTracker* BookmarkModelTypeProcessor::GetTrackerForTest()
    const {
  return bookmark_tracker_.get();
}

std::string BookmarkModelTypeProcessor::EncodeSyncMetadata() const {
  std::string metadata_str;
  if (bookmark_tracker_) {
    sync_pb::BookmarkModelMetadata model_metadata =
        bookmark_tracker_->BuildBookmarkModelMetadata();
    model_metadata.SerializeToString(&metadata_str);
  }
  return metadata_str;
}

void BookmarkModelTypeProcessor::ModelReadyToSync(
    const std::string& metadata_str,
    const base::RepeatingClosure& schedule_save_closure,
    bookmarks::BookmarkModel* model) {
  DCHECK(model);
  DCHECK(!bookmark_model_);
  DCHECK(!bookmark_tracker_);
  DCHECK(!bookmark_model_observer_);

  bookmark_model_ = model;
  schedule_save_closure_ = schedule_save_closure;

  sync_pb::BookmarkModelMetadata model_metadata;
  model_metadata.ParseFromString(metadata_str);

  auto model_type_state = std::make_unique<sync_pb::ModelTypeState>();
  model_type_state->Swap(model_metadata.mutable_model_type_state());

  if (model_type_state->initial_sync_done()) {
    std::vector<NodeMetadataPair> nodes_metadata;
    for (sync_pb::BookmarkMetadata& bookmark_metadata :
         *model_metadata.mutable_bookmarks_metadata()) {
      // TODO(crbug.com/516866): Replace with a more efficient way to retrieve
      // all nodes and store in a map keyed by id instead of doing a lookup for
      // every id.
      const bookmarks::BookmarkNode* node = nullptr;
      if (bookmark_metadata.metadata().is_deleted()) {
        if (bookmark_metadata.has_id()) {
          DLOG(ERROR) << "Error when decoding sync metadata: Tombstones "
                         "shouldn't have a bookmark id.";
          continue;
        }
      } else {
        if (!bookmark_metadata.has_id()) {
          DLOG(ERROR)
              << "Error when decoding sync metadata: Bookmark id is missing.";
          continue;
        }
        node = GetBookmarkNodeByID(bookmark_model_, bookmark_metadata.id());
        if (node == nullptr) {
          DLOG(ERROR) << "Error when decoding sync metadata: Cannot find the "
                         "bookmark node.";
          continue;
        }
      }
      auto metadata = std::make_unique<sync_pb::EntityMetadata>();
      metadata->Swap(bookmark_metadata.mutable_metadata());
      nodes_metadata.emplace_back(node, std::move(metadata));
    }
    // TODO(crbug.com/516866): Handle local nodes that don't have a
    // corresponding
    // metadata.
    StartTrackingMetadata(std::move(nodes_metadata),
                          std::move(model_type_state));
  } else if (!model_metadata.bookmarks_metadata().empty()) {
    DLOG(ERROR)
        << "Persisted Metadata not empty while initial sync is not done.";
  }
  ConnectIfReady();
}

base::WeakPtr<syncer::ModelTypeControllerDelegate>
BookmarkModelTypeProcessor::GetWeakPtr() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return weak_ptr_factory_.GetWeakPtr();
}

void BookmarkModelTypeProcessor::OnSyncStarting(
    const syncer::DataTypeActivationRequest& request,
    StartCallback start_callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(start_callback);
  DVLOG(1) << "Sync is starting for Bookmarks";

  cache_guid_ = request.cache_guid;
  start_callback_ = std::move(start_callback);
  DCHECK(!cache_guid_.empty());
  ConnectIfReady();
}

void BookmarkModelTypeProcessor::ConnectIfReady() {
  // Return if the model isn't ready.
  if (!bookmark_model_) {
    return;
  }
  // Return if Sync didn't start yet.
  if (!start_callback_) {
    return;
  }

  DCHECK(!cache_guid_.empty());

  if (bookmark_tracker_ &&
      bookmark_tracker_->model_type_state().cache_guid() != cache_guid_) {
    // TODO(crbug.com/820049): Properly handle a mismatch between the loaded
    // cache guid stored in |bookmark_tracker_.model_type_state_| at
    // DecodeSyncMetadata() and the one received from sync at OnSyncStarting()
    // stored in |cache_guid_|.
    return;
  }

  auto activation_context =
      std::make_unique<syncer::DataTypeActivationResponse>();
  if (bookmark_tracker_) {
    activation_context->model_type_state =
        bookmark_tracker_->model_type_state();
  } else {
    sync_pb::ModelTypeState model_type_state;
    model_type_state.mutable_progress_marker()->set_data_type_id(
        GetSpecificsFieldNumberFromModelType(syncer::BOOKMARKS));
    model_type_state.set_cache_guid(cache_guid_);
    activation_context->model_type_state = model_type_state;
  }
  activation_context->type_processor =
      std::make_unique<syncer::ModelTypeProcessorProxy>(
          weak_ptr_factory_.GetWeakPtr(),
          base::SequencedTaskRunnerHandle::Get());
  std::move(start_callback_).Run(std::move(activation_context));
}

void BookmarkModelTypeProcessor::OnSyncStopping(
    syncer::SyncStopMetadataFate metadata_fate) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  cache_guid_.clear();
  NOTIMPLEMENTED();
}

void BookmarkModelTypeProcessor::NudgeForCommitIfNeeded() {
  DCHECK(bookmark_tracker_);
  // Don't bother sending anything if there's no one to send to.
  if (!worker_) {
    return;
  }

  // Nudge worker if there are any entities with local changes.
  if (bookmark_tracker_->HasLocalChanges()) {
    worker_->NudgeForCommit();
  }
}

void BookmarkModelTypeProcessor::StartTrackingMetadata(
    std::vector<NodeMetadataPair> nodes_metadata,
    std::unique_ptr<sync_pb::ModelTypeState> model_type_state) {
  bookmark_tracker_ = std::make_unique<SyncedBookmarkTracker>(
      std::move(nodes_metadata), std::move(model_type_state));

  bookmark_model_observer_ = std::make_unique<BookmarkModelObserverImpl>(
      base::BindRepeating(&BookmarkModelTypeProcessor::NudgeForCommitIfNeeded,
                          base::Unretained(this)),
      bookmark_tracker_.get());
  bookmark_model_->AddObserver(bookmark_model_observer_.get());
}

void BookmarkModelTypeProcessor::GetAllNodesForDebugging(
    AllNodesCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  NOTIMPLEMENTED();
}

void BookmarkModelTypeProcessor::GetStatusCountersForDebugging(
    StatusCountersCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  NOTIMPLEMENTED();
}

void BookmarkModelTypeProcessor::RecordMemoryUsageHistogram() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  NOTIMPLEMENTED();
}

}  // namespace sync_bookmarks
