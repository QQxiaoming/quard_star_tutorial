// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/background_fetch/storage/mark_request_complete_task.h"

#include "base/barrier_closure.h"
#include "base/guid.h"
#include "content/browser/background_fetch/background_fetch_cross_origin_filter.h"
#include "content/browser/background_fetch/background_fetch_data_manager.h"
#include "content/browser/background_fetch/storage/database_helpers.h"
#include "content/browser/background_fetch/storage/get_metadata_task.h"
#include "content/browser/blob_storage/chrome_blob_storage_context.h"
#include "content/browser/cache_storage/cache_storage_manager.h"
#include "content/browser/service_worker/service_worker_context_wrapper.h"
#include "services/network/public/cpp/cors/cors.h"
#include "storage/browser/blob/blob_data_builder.h"
#include "storage/browser/blob/blob_impl.h"
#include "storage/browser/blob/blob_storage_context.h"
#include "third_party/blink/public/mojom/blob/blob.mojom.h"

namespace content {

namespace background_fetch {

namespace {

// Returns whether the response contained in the Background Fetch |request| is
// considered OK. See https://fetch.spec.whatwg.org/#ok-status aka a successful
// 2xx status per https://tools.ietf.org/html/rfc7231#section-6.3.
bool IsOK(const BackgroundFetchRequestInfo& request) {
  int status = request.GetResponseCode();
  return network::cors::IsOkStatus(status);
}

}  // namespace

MarkRequestCompleteTask::MarkRequestCompleteTask(
    DatabaseTaskHost* host,
    BackgroundFetchRegistrationId registration_id,
    scoped_refptr<BackgroundFetchRequestInfo> request_info,
    MarkedCompleteCallback callback)
    : DatabaseTask(host),
      registration_id_(registration_id),
      request_info_(std::move(request_info)),
      callback_(std::move(callback)),
      weak_factory_(this) {}

MarkRequestCompleteTask::~MarkRequestCompleteTask() = default;

void MarkRequestCompleteTask::Start() {
  base::RepeatingClosure barrier_closure = base::BarrierClosure(
      2u, base::BindOnce(&MarkRequestCompleteTask::FinishWithError,
                         weak_factory_.GetWeakPtr(),
                         blink::mojom::BackgroundFetchError::NONE));

  StoreResponse(barrier_closure);
  UpdateMetadata(barrier_closure);
}

void MarkRequestCompleteTask::StoreResponse(base::OnceClosure done_closure) {
  auto response = std::make_unique<ServiceWorkerResponse>();
  response->url_list = request_info_->GetURLChain();
  // TODO(crbug.com/838837): fill error and cors_exposed_header_names in
  // response.
  response->response_type = network::mojom::FetchResponseType::kDefault;
  response->response_time = request_info_->GetResponseTime();

  BackgroundFetchCrossOriginFilter filter(registration_id_.origin(),
                                          *request_info_);
  if (filter.CanPopulateBody())
    PopulateResponseBody(response.get());
  else
    is_response_successful_ = false;

  if (!IsOK(*request_info_))
    is_response_successful_ = false;

  // A valid non-empty url is needed if we want to write to the cache.
  if (!request_info_->fetch_request().url.is_valid()) {
    CreateAndStoreCompletedRequest(std::move(done_closure));
    return;
  }

  cache_manager()->OpenCache(
      registration_id_.origin(), CacheStorageOwner::kBackgroundFetch,
      registration_id_.unique_id() /* cache_name */,
      base::BindOnce(&MarkRequestCompleteTask::DidOpenCache,
                     weak_factory_.GetWeakPtr(), std::move(response),
                     std::move(done_closure)));
}

void MarkRequestCompleteTask::PopulateResponseBody(
    ServiceWorkerResponse* response) {
  // Include the status code, status text and the response's body as a blob
  // when this is allowed by the CORS protocol.
  response->status_code = request_info_->GetResponseCode();
  response->status_text = request_info_->GetResponseText();
  response->headers.insert(request_info_->GetResponseHeaders().begin(),
                           request_info_->GetResponseHeaders().end());

  DCHECK(blob_storage_context());
  std::unique_ptr<storage::BlobDataHandle> blob_data_handle;

  // Prefer the blob data handle provided by the |request_info_| if one is
  // available. Otherwise create one based on the file path and size.
  if (request_info_->GetBlobDataHandle()) {
    blob_data_handle = std::make_unique<storage::BlobDataHandle>(
        request_info_->GetBlobDataHandle().value());

  } else if (request_info_->GetFileSize() > 0 &&
             !request_info_->GetFilePath().empty()) {
    // TODO(rayankans): Simplify this code by making either the download service
    // or the BackgroundFetchRequestInfo responsible for files vs. blobs.
    auto blob_builder =
        std::make_unique<storage::BlobDataBuilder>(base::GenerateGUID());
    blob_builder->AppendFile(request_info_->GetFilePath(), 0 /* offset */,
                             request_info_->GetFileSize(),
                             base::Time() /* expected_modification_time */);

    blob_data_handle = GetBlobStorageContext(blob_storage_context())
                           ->AddFinishedBlob(std::move(blob_builder));
  }

  // TODO(rayankans): Appropriately handle !blob_data_handle
  if (!blob_data_handle)
    return;

  response->blob_uuid = blob_data_handle->uuid();
  response->blob_size = blob_data_handle->size();
  blink::mojom::BlobPtr blob_ptr;
  storage::BlobImpl::Create(
      std::make_unique<storage::BlobDataHandle>(*blob_data_handle),
      MakeRequest(&blob_ptr));

  response->blob =
      base::MakeRefCounted<storage::BlobHandle>(std::move(blob_ptr));
}

void MarkRequestCompleteTask::DidOpenCache(
    std::unique_ptr<ServiceWorkerResponse> response,
    base::OnceClosure done_closure,
    CacheStorageCacheHandle handle,
    blink::mojom::CacheStorageError error) {
  if (error != blink::mojom::CacheStorageError::kSuccess) {
    // TODO(crbug.com/780025): Log failures to UMA.
    CreateAndStoreCompletedRequest(std::move(done_closure));
    return;
  }
  DCHECK(handle.value());

  auto request = std::make_unique<ServiceWorkerFetchRequest>(
      request_info_->fetch_request());

  // We need to keep the handle refcounted while the write is happening,
  // so it's passed along to the callback.
  handle.value()->Put(
      std::move(request), std::move(response),
      base::BindOnce(&MarkRequestCompleteTask::DidWriteToCache,
                     weak_factory_.GetWeakPtr(), std::move(handle),
                     std::move(done_closure)));
}

void MarkRequestCompleteTask::DidWriteToCache(
    CacheStorageCacheHandle handle,
    base::OnceClosure done_closure,
    blink::mojom::CacheStorageError error) {
  // TODO(crbug.com/780025): Log failures to UMA.
  CreateAndStoreCompletedRequest(std::move(done_closure));
}

void MarkRequestCompleteTask::CreateAndStoreCompletedRequest(
    base::OnceClosure done_closure) {
  completed_request_.set_unique_id(registration_id_.unique_id());
  completed_request_.set_request_index(request_info_->request_index());
  completed_request_.set_serialized_request(
      request_info_->fetch_request().Serialize());
  completed_request_.set_download_guid(request_info_->download_guid());
  completed_request_.set_succeeded(is_response_successful_);

  service_worker_context()->StoreRegistrationUserData(
      registration_id_.service_worker_registration_id(),
      registration_id_.origin().GetURL(),
      {{CompletedRequestKey(completed_request_.unique_id(),
                            completed_request_.request_index()),
        completed_request_.SerializeAsString()}},
      base::BindOnce(&MarkRequestCompleteTask::DidStoreCompletedRequest,
                     weak_factory_.GetWeakPtr(), std::move(done_closure)));
}

void MarkRequestCompleteTask::DidStoreCompletedRequest(
    base::OnceClosure done_closure,
    blink::ServiceWorkerStatusCode status) {
  switch (ToDatabaseStatus(status)) {
    case DatabaseStatus::kOk:
      break;
    case DatabaseStatus::kFailed:
    case DatabaseStatus::kNotFound:
      // TODO(crbug.com/780025): Log failures to UMA.
      std::move(done_closure).Run();
      return;
  }

  // Delete the active request.
  service_worker_context()->ClearRegistrationUserData(
      registration_id_.service_worker_registration_id(),
      {ActiveRequestKey(completed_request_.unique_id(),
                        completed_request_.request_index())},
      base::BindOnce(&MarkRequestCompleteTask::DidDeleteActiveRequest,
                     weak_factory_.GetWeakPtr(), std::move(done_closure)));
}

void MarkRequestCompleteTask::DidDeleteActiveRequest(
    base::OnceClosure done_closure,
    blink::ServiceWorkerStatusCode status) {
  // TODO(crbug.com/780025): Log failures to UMA.
  std::move(done_closure).Run();
}

void MarkRequestCompleteTask::UpdateMetadata(base::OnceClosure done_closure) {
  if (!request_info_->IsResultSuccess() || request_info_->GetFileSize() == 0u) {
    std::move(done_closure).Run();
    return;
  }

  AddSubTask(std::make_unique<GetMetadataTask>(
      this, registration_id_.service_worker_registration_id(),
      registration_id_.origin(), registration_id_.developer_id(),
      base::BindOnce(&MarkRequestCompleteTask::DidGetMetadata,
                     weak_factory_.GetWeakPtr(), std::move(done_closure))));
}

void MarkRequestCompleteTask::DidGetMetadata(
    base::OnceClosure done_closure,
    blink::mojom::BackgroundFetchError error,
    std::unique_ptr<proto::BackgroundFetchMetadata> metadata) {
  if (!metadata || error != blink::mojom::BackgroundFetchError::NONE) {
    // TODO(crbug.com/780025): Log failures to UMA.
    std::move(done_closure).Run();
    return;
  }

  metadata->mutable_registration()->set_download_total(
      metadata->registration().download_total() + request_info_->GetFileSize());

  service_worker_context()->StoreRegistrationUserData(
      registration_id_.service_worker_registration_id(),
      registration_id_.origin().GetURL(),
      {{RegistrationKey(registration_id_.unique_id()),
        metadata->SerializeAsString()}},
      base::BindOnce(&MarkRequestCompleteTask::DidStoreMetadata,
                     weak_factory_.GetWeakPtr(), std::move(done_closure)));
}

void MarkRequestCompleteTask::DidStoreMetadata(
    base::OnceClosure done_closure,
    blink::ServiceWorkerStatusCode status) {
  // TODO(crbug.com/780025): Log failures to UMA.
  std::move(done_closure).Run();
}

void MarkRequestCompleteTask::FinishWithError(
    blink::mojom::BackgroundFetchError error) {
  std::move(callback_).Run();
  Finished();
}

}  // namespace background_fetch

}  // namespace content
