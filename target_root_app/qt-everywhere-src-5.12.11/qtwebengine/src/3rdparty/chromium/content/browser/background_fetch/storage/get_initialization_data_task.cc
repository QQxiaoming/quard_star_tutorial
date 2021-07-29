// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/background_fetch/storage/get_initialization_data_task.h"

#include "base/barrier_closure.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task_scheduler/post_task.h"
#include "base/task_scheduler/task_traits.h"
#include "content/browser/background_fetch/background_fetch.pb.h"
#include "content/browser/background_fetch/background_fetch_data_manager.h"
#include "content/browser/background_fetch/storage/database_helpers.h"
#include "content/browser/background_fetch/storage/mark_registration_for_deletion_task.h"
#include "content/browser/service_worker/service_worker_context_wrapper.h"
#include "third_party/blink/public/common/manifest/manifest.h"
#include "ui/gfx/image/image.h"
#include "url/origin.h"

namespace content {

namespace background_fetch {

namespace {

// Base class with all the common implementation for the SubTasks
// needed in this file.
class InitializationSubTask : public DatabaseTask {
 public:
  // Holds data used by all SubTasks.
  struct SubTaskInit {
    SubTaskInit() = delete;
    ~SubTaskInit() = default;

    // Service Worker Database metadata.
    int64_t service_worker_registration_id;
    std::string unique_id;

    // The results to report.
    BackgroundFetchInitializationData* initialization_data;
  };

  InitializationSubTask(DatabaseTaskHost* host,
                        const SubTaskInit& sub_task_init,
                        base::OnceClosure done_closure)
      : DatabaseTask(host),
        sub_task_init_(sub_task_init),
        done_closure_(std::move(done_closure)) {
    DCHECK(sub_task_init_.initialization_data);
  }

  ~InitializationSubTask() override = default;

 protected:
  void FinishWithError(blink::mojom::BackgroundFetchError error) override {
    if (error != blink::mojom::BackgroundFetchError::NONE)
      sub_task_init_.initialization_data->error = error;
    std::move(done_closure_).Run();
    Finished();  // Destroys |this|.
  }

  SubTaskInit& sub_task_init() { return sub_task_init_; }

 private:
  SubTaskInit sub_task_init_;
  base::OnceClosure done_closure_;

  DISALLOW_COPY_AND_ASSIGN(InitializationSubTask);
};

// Fills the BackgroundFetchInitializationData with the most recent UI title.
class GetTitleTask : public InitializationSubTask {
 public:
  GetTitleTask(DatabaseTaskHost* host,
               const SubTaskInit& sub_task_init,
               base::OnceClosure done_closure)
      : InitializationSubTask(host, sub_task_init, std::move(done_closure)),
        weak_factory_(this) {}

  ~GetTitleTask() override = default;

  void Start() override {
    service_worker_context()->GetRegistrationUserData(
        sub_task_init().service_worker_registration_id,
        {TitleKey(sub_task_init().unique_id)},
        base::BindOnce(&GetTitleTask::DidGetTitle, weak_factory_.GetWeakPtr()));
  }

 private:
  void DidGetTitle(const std::vector<std::string>& data,
                   blink::ServiceWorkerStatusCode status) {
    switch (ToDatabaseStatus(status)) {
      case DatabaseStatus::kFailed:
        FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
        return;
      case DatabaseStatus::kNotFound:
      case DatabaseStatus::kOk:
        break;
    }

    if (!data.empty())
      sub_task_init().initialization_data->ui_title = data.front();
    FinishWithError(blink::mojom::BackgroundFetchError::NONE);
  }

  base::WeakPtrFactory<GetTitleTask> weak_factory_;  // Keep as last.
};

// Gets the number of completed fetches, the number of active fetches,
// and deletes inconsistencies in state transitions.
// 1. Get all completed requests.
// 2. Delete matching active requests.
// 3. Get active requests.
// 4. Delete matching pending requests.
class GetRequestsTask : public InitializationSubTask {
 public:
  GetRequestsTask(DatabaseTaskHost* host,
                  const SubTaskInit& sub_task_init,
                  base::OnceClosure done_closure)
      : InitializationSubTask(host, sub_task_init, std::move(done_closure)),
        weak_factory_(this) {}

  ~GetRequestsTask() override = default;

  void Start() override {
    service_worker_context()->GetRegistrationUserDataByKeyPrefix(
        sub_task_init().service_worker_registration_id,
        CompletedRequestKeyPrefix(sub_task_init().unique_id),
        base::BindOnce(&GetRequestsTask::DidGetCompletedRequests,
                       weak_factory_.GetWeakPtr()));
  }

 private:
  void DidGetCompletedRequests(const std::vector<std::string>& data,
                               blink::ServiceWorkerStatusCode status) {
    switch (ToDatabaseStatus(status)) {
      case DatabaseStatus::kFailed:
        FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
        return;
      case DatabaseStatus::kNotFound:
      case DatabaseStatus::kOk:
        break;
    }

    sub_task_init().initialization_data->num_completed_requests = data.size();

    std::vector<std::string> active_requests_to_delete;
    active_requests_to_delete.reserve(data.size());
    for (const std::string& serialized_completed_request : data) {
      proto::BackgroundFetchCompletedRequest completed_request;
      if (!completed_request.ParseFromString(serialized_completed_request) ||
          sub_task_init().unique_id != completed_request.unique_id()) {
        FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
        return;
      }

      active_requests_to_delete.push_back(ActiveRequestKey(
          completed_request.unique_id(), completed_request.request_index()));
    }

    if (active_requests_to_delete.empty()) {
      DidClearActiveRequests(blink::ServiceWorkerStatusCode::kOk);
      return;
    }

    service_worker_context()->ClearRegistrationUserData(
        sub_task_init().service_worker_registration_id,
        std::move(active_requests_to_delete),
        base::BindOnce(&GetRequestsTask::DidClearActiveRequests,
                       weak_factory_.GetWeakPtr()));
  }

  void DidClearActiveRequests(blink::ServiceWorkerStatusCode status) {
    switch (ToDatabaseStatus(status)) {
      case DatabaseStatus::kFailed:
        FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
        return;
      case DatabaseStatus::kNotFound:
      case DatabaseStatus::kOk:
        break;
    }

    service_worker_context()->GetRegistrationUserDataByKeyPrefix(
        sub_task_init().service_worker_registration_id,
        ActiveRequestKeyPrefix(sub_task_init().unique_id),
        base::BindOnce(&GetRequestsTask::DidGetRemainingActiveRequests,
                       weak_factory_.GetWeakPtr()));
  }

  void DidGetRemainingActiveRequests(const std::vector<std::string>& data,
                                     blink::ServiceWorkerStatusCode status) {
    switch (ToDatabaseStatus(status)) {
      case DatabaseStatus::kFailed:
        FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
        return;
      case DatabaseStatus::kNotFound:
      case DatabaseStatus::kOk:
        break;
    }

    std::vector<std::string> pending_requests_to_delete;
    pending_requests_to_delete.reserve(data.size());
    for (const std::string& serialized_active_request : data) {
      proto::BackgroundFetchActiveRequest active_request;
      if (!active_request.ParseFromString(serialized_active_request)) {
        FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
        return;
      }
      DCHECK_EQ(sub_task_init().unique_id, active_request.unique_id());
      sub_task_init().initialization_data->active_fetch_guids.push_back(
          active_request.download_guid());
      pending_requests_to_delete.push_back(PendingRequestKey(
          active_request.unique_id(), active_request.request_index()));
    }

    if (pending_requests_to_delete.empty()) {
      DidClearPendingRequests(blink::ServiceWorkerStatusCode::kOk);
      return;
    }

    service_worker_context()->ClearRegistrationUserData(
        sub_task_init().service_worker_registration_id,
        std::move(pending_requests_to_delete),
        base::BindOnce(&GetRequestsTask::DidClearPendingRequests,
                       weak_factory_.GetWeakPtr()));
  }

  void DidClearPendingRequests(blink::ServiceWorkerStatusCode status) {
    switch (ToDatabaseStatus(status)) {
      case DatabaseStatus::kFailed:
        FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
        return;
      case DatabaseStatus::kNotFound:
      case DatabaseStatus::kOk:
        break;
    }

    FinishWithError(blink::mojom::BackgroundFetchError::NONE);
  }

  base::WeakPtrFactory<GetRequestsTask> weak_factory_;  // Keep as last.

  DISALLOW_COPY_AND_ASSIGN(GetRequestsTask);
};

// Deserializes the icon and creates an SkBitmap from it.
class DeserializeIconTask : public InitializationSubTask {
 public:
  DeserializeIconTask(DatabaseTaskHost* host,
                      const SubTaskInit& sub_task_init,
                      base::OnceClosure done_closure,
                      std::string* serialized_icon)
      : InitializationSubTask(host, sub_task_init, std::move(done_closure)),
        serialized_icon_(serialized_icon),
        weak_factory_(this) {}

  ~DeserializeIconTask() override = default;

  void Start() override {
    base::PostTaskWithTraitsAndReplyWithResult(
        FROM_HERE,
        {base::MayBlock(), base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN,
         base::TaskPriority::BACKGROUND},
        base::BindOnce(&DeserializeIcon, std::move(serialized_icon_)),
        base::BindOnce(&DeserializeIconTask::StoreIcon,
                       weak_factory_.GetWeakPtr()));
  }

 private:
  static SkBitmap DeserializeIcon(
      std::unique_ptr<std::string> serialized_icon) {
    return gfx::Image::CreateFrom1xPNGBytes(
               reinterpret_cast<const unsigned char*>(serialized_icon->c_str()),
               serialized_icon->size())
        .AsBitmap();
  }

  void StoreIcon(SkBitmap icon) {
    sub_task_init().initialization_data->icon = std::move(icon);
    FinishWithError(blink::mojom::BackgroundFetchError::NONE);
  }

  std::unique_ptr<std::string> serialized_icon_;

  base::WeakPtrFactory<DeserializeIconTask> weak_factory_;  // Keep as last.
};

// Fills the BackgroundFetchInitializationData with all the relevant information
// stored in the BackgroundFetchMetadata proto.
class FillFromMetadataTask : public InitializationSubTask {
 public:
  FillFromMetadataTask(DatabaseTaskHost* host,
                       const SubTaskInit& sub_task_init,
                       base::OnceClosure done_closure)
      : InitializationSubTask(host, sub_task_init, std::move(done_closure)),
        weak_factory_(this) {}

  ~FillFromMetadataTask() override = default;

  void Start() override {
    service_worker_context()->GetRegistrationUserDataByKeyPrefix(
        sub_task_init().service_worker_registration_id,
        {RegistrationKey(sub_task_init().unique_id)},
        base::BindOnce(&FillFromMetadataTask::DidGetMetadata,
                       weak_factory_.GetWeakPtr()));
  }

 private:
  void DidGetMetadata(const std::vector<std::string>& data,
                      blink::ServiceWorkerStatusCode status) {
    switch (ToDatabaseStatus(status)) {
      case DatabaseStatus::kFailed:
      case DatabaseStatus::kNotFound:
        FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
        return;
      case DatabaseStatus::kOk:
        break;
    }

    if (data.size() != 1u) {
      FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
      return;
    }

    proto::BackgroundFetchMetadata metadata;
    if (!metadata.ParseFromString(data[0])) {
      FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
      return;
    }

    if (sub_task_init().unique_id != metadata.registration().unique_id()) {
      FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
      return;
    }

    // Fill BackgroundFetchRegistrationId.
    sub_task_init().initialization_data->registration_id =
        BackgroundFetchRegistrationId(
            sub_task_init().service_worker_registration_id,
            url::Origin::Create(GURL(metadata.origin())),
            metadata.registration().developer_id(),
            metadata.registration().unique_id());

    // Fill BackgroundFetchRegistration.
    auto& registration = sub_task_init().initialization_data->registration;
    // TODO(crbug.com/853874): Unify conversion logic.
    registration.developer_id = metadata.registration().developer_id();
    registration.unique_id = metadata.registration().unique_id();
    registration.upload_total = metadata.registration().upload_total();
    registration.uploaded = metadata.registration().uploaded();
    registration.download_total = metadata.registration().download_total();
    registration.downloaded = metadata.registration().downloaded();

    // Total number of requests.
    sub_task_init().initialization_data->num_requests = metadata.num_fetches();

    // Fill BackgroundFetchOptions.
    auto& options = sub_task_init().initialization_data->options;
    options.title = metadata.options().title();
    options.download_total = metadata.options().download_total();
    options.icons.reserve(metadata.options().icons_size());
    for (const auto& icon : metadata.options().icons()) {
      blink::Manifest::ImageResource ir;
      ir.src = GURL(icon.src());
      ir.type = base::ASCIIToUTF16(icon.type());

      ir.sizes.reserve(icon.sizes_size());
      for (const auto& size : icon.sizes())
        ir.sizes.emplace_back(size.width(), size.height());

      ir.purpose.reserve(icon.purpose_size());
      for (auto purpose : icon.purpose()) {
        switch (purpose) {
          case proto::BackgroundFetchOptions_ImageResource_Purpose_ANY:
            ir.purpose.push_back(blink::Manifest::ImageResource::Purpose::ANY);
            break;
          case proto::BackgroundFetchOptions_ImageResource_Purpose_BADGE:
            ir.purpose.push_back(
                blink::Manifest::ImageResource::Purpose::BADGE);
            break;
        }
      }
    }

    if (!metadata.icon().empty()) {
      // Start an icon deserialization SubTask on another thread, then finish.
      AddSubTask(std::make_unique<DeserializeIconTask>(
          this, sub_task_init(),
          base::BindOnce(&FillFromMetadataTask::FinishWithError,
                         weak_factory_.GetWeakPtr(),
                         blink::mojom::BackgroundFetchError::NONE),
          metadata.release_icon()));
    } else {
      // Immediately finish.
      FinishWithError(blink::mojom::BackgroundFetchError::NONE);
    }
  }

  base::WeakPtrFactory<FillFromMetadataTask> weak_factory_;  // Keep as last.

  DISALLOW_COPY_AND_ASSIGN(FillFromMetadataTask);
};

// Asynchronously calls the SubTasks required to collect all the information for
// the BackgroundFetchInitializationData.
class FillBackgroundFetchInitializationDataTask : public InitializationSubTask {
 public:
  FillBackgroundFetchInitializationDataTask(DatabaseTaskHost* host,
                                            const SubTaskInit& sub_task_init,
                                            base::OnceClosure done_closure)
      : InitializationSubTask(host, sub_task_init, std::move(done_closure)),
        weak_factory_(this) {}

  ~FillBackgroundFetchInitializationDataTask() override = default;

  void Start() override {
    // We need 3 queries to get the initialization data. These are wrapped
    // in a BarrierClosure to avoid querying them serially.
    // 1. Metadata (+ icon deserialization)
    // 2. Request statuses and state sanitization
    // 3. UI Title
    base::RepeatingClosure barrier_closure = base::BarrierClosure(
        3u,
        base::BindOnce(
            [](base::WeakPtr<FillBackgroundFetchInitializationDataTask> task) {
              if (task)
                task->FinishWithError(
                    task->sub_task_init().initialization_data->error);
            },
            weak_factory_.GetWeakPtr()));
    AddSubTask(std::make_unique<FillFromMetadataTask>(this, sub_task_init(),
                                                      barrier_closure));
    AddSubTask(std::make_unique<GetRequestsTask>(this, sub_task_init(),
                                                 barrier_closure));
    AddSubTask(
        std::make_unique<GetTitleTask>(this, sub_task_init(), barrier_closure));
  }

 private:
  base::WeakPtrFactory<FillBackgroundFetchInitializationDataTask>
      weak_factory_;  // Keep as last.

  DISALLOW_COPY_AND_ASSIGN(FillBackgroundFetchInitializationDataTask);
};

}  // namespace

BackgroundFetchInitializationData::BackgroundFetchInitializationData() =
    default;

BackgroundFetchInitializationData::BackgroundFetchInitializationData(
    BackgroundFetchInitializationData&&) = default;

BackgroundFetchInitializationData::~BackgroundFetchInitializationData() =
    default;

GetInitializationDataTask::GetInitializationDataTask(
    DatabaseTaskHost* host,
    GetInitializationDataCallback callback)
    : DatabaseTask(host), callback_(std::move(callback)), weak_factory_(this) {}

GetInitializationDataTask::~GetInitializationDataTask() = default;

void GetInitializationDataTask::Start() {
  service_worker_context()->GetUserDataForAllRegistrationsByKeyPrefix(
      kActiveRegistrationUniqueIdKeyPrefix,
      base::BindOnce(&GetInitializationDataTask::DidGetRegistrations,
                     weak_factory_.GetWeakPtr()));
}

void GetInitializationDataTask::DidGetRegistrations(
    const std::vector<std::pair<int64_t, std::string>>& user_data,
    blink::ServiceWorkerStatusCode status) {
  switch (ToDatabaseStatus(status)) {
    case DatabaseStatus::kFailed:
      FinishWithError(blink::mojom::BackgroundFetchError::STORAGE_ERROR);
      return;
    case DatabaseStatus::kNotFound:
    case DatabaseStatus::kOk:
      break;
  }

  if (user_data.empty()) {
    FinishWithError(blink::mojom::BackgroundFetchError::NONE);
    return;
  }

  base::RepeatingClosure barrier_closure = base::BarrierClosure(
      user_data.size(),
      base::BindOnce(&GetInitializationDataTask::FinishWithError,
                     weak_factory_.GetWeakPtr(),
                     blink::mojom::BackgroundFetchError::NONE));

  for (const auto& ud : user_data) {
    auto insertion_result = initialization_data_map_.emplace(
        ud.second, BackgroundFetchInitializationData());
    DCHECK(insertion_result.second);  // Check unique_id is in fact unique.

    AddSubTask(std::make_unique<FillBackgroundFetchInitializationDataTask>(
        this,
        InitializationSubTask::SubTaskInit{
            ud.first, ud.second,
            &insertion_result.first->second /* initialization_data */},
        barrier_closure));
  }
}
void GetInitializationDataTask::FinishWithError(
    blink::mojom::BackgroundFetchError error) {
  std::vector<BackgroundFetchInitializationData> results;
  results.reserve(initialization_data_map_.size());

  for (auto& data : initialization_data_map_) {
    if (data.second.error == blink::mojom::BackgroundFetchError::NONE) {
      // If we successfully extracted all the data, move it to the
      // initialization vector to be handed over to create a controller.
      results.emplace_back(std::move(data.second));
    } else if (!data.second.registration_id.developer_id().empty()) {
      // There was an error in getting the initialization data
      // (e.g. corrupt data, SWDB error). If the Developer ID of the fetch
      // is available, mark the registration for deletion.
      // Note that the Developer ID isn't available if the metadata extraction
      // failed.
      // TODO(crbug.com/865388): Getting the Developer ID should be possible
      // since it is part of the key for when we got the Unique ID.
      AddDatabaseTask(std::make_unique<MarkRegistrationForDeletionTask>(
          data_manager(), data.second.registration_id, base::DoNothing()));
    }
  }

  std::move(callback_).Run(error, std::move(results));
  Finished();  // Destroys |this|.
}

}  // namespace background_fetch

}  // namespace content
