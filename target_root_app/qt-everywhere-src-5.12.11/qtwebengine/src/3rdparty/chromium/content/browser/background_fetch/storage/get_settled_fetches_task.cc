// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/background_fetch/storage/get_settled_fetches_task.h"

#include "base/barrier_closure.h"
#include "content/browser/background_fetch/background_fetch_data_manager.h"
#include "content/browser/background_fetch/storage/database_helpers.h"
#include "content/browser/cache_storage/cache_storage_manager.h"
#include "content/browser/service_worker/service_worker_context_wrapper.h"

namespace content {

namespace background_fetch {

GetSettledFetchesTask::GetSettledFetchesTask(
    DatabaseTaskHost* host,
    BackgroundFetchRegistrationId registration_id,
    SettledFetchesCallback callback)
    : DatabaseTask(host),
      registration_id_(registration_id),
      settled_fetches_callback_(std::move(callback)),
      weak_factory_(this) {}

GetSettledFetchesTask::~GetSettledFetchesTask() = default;

void GetSettledFetchesTask::Start() {
  base::RepeatingClosure barrier_closure = base::BarrierClosure(
      2u, base::BindOnce(&GetSettledFetchesTask::GetResponses,
                         weak_factory_.GetWeakPtr()));

  cache_manager()->OpenCache(
      registration_id_.origin(), CacheStorageOwner::kBackgroundFetch,
      registration_id_.unique_id() /* cache_name */,
      base::BindOnce(&GetSettledFetchesTask::DidOpenCache,
                     weak_factory_.GetWeakPtr(), barrier_closure));

  service_worker_context()->GetRegistrationUserDataByKeyPrefix(
      registration_id_.service_worker_registration_id(),
      {CompletedRequestKeyPrefix(registration_id_.unique_id())},
      base::BindOnce(&GetSettledFetchesTask::DidGetCompletedRequests,
                     weak_factory_.GetWeakPtr(), barrier_closure));
}

void GetSettledFetchesTask::DidOpenCache(
    base::OnceClosure done_closure,
    CacheStorageCacheHandle handle,
    blink::mojom::CacheStorageError error) {
  if (error != blink::mojom::CacheStorageError::kSuccess) {
    // TODO(crbug.com/780025): Log failures to UMA.
    error_ = blink::mojom::BackgroundFetchError::STORAGE_ERROR;
  } else {
    DCHECK(handle.value());
    handle_ = std::move(handle);
  }
  std::move(done_closure).Run();
}

void GetSettledFetchesTask::DidGetCompletedRequests(
    base::OnceClosure done_closure,
    const std::vector<std::string>& data,
    blink::ServiceWorkerStatusCode status) {
  switch (ToDatabaseStatus(status)) {
    case DatabaseStatus::kOk:
      break;
    // TODO(crbug.com/780025): Log failures to UMA.
    case DatabaseStatus::kFailed:
      error_ = blink::mojom::BackgroundFetchError::STORAGE_ERROR;
      break;
    case DatabaseStatus::kNotFound:
      background_fetch_succeeded_ = false;
      error_ = blink::mojom::BackgroundFetchError::INVALID_ID;
      break;
  }

  completed_requests_.reserve(data.size());
  for (const std::string& serialized_completed_request : data) {
    completed_requests_.emplace_back();
    if (!completed_requests_.back().ParseFromString(
            serialized_completed_request)) {
      // Service worker database has been corrupted. Abandon fetches.
      error_ = blink::mojom::BackgroundFetchError::STORAGE_ERROR;
      background_fetch_succeeded_ = false;
      AbandonFetches(registration_id_.service_worker_registration_id());
      break;
    }
    if (!completed_requests_.back().succeeded())
      background_fetch_succeeded_ = false;
  }
  std::move(done_closure).Run();
}

void GetSettledFetchesTask::GetResponses() {
  if (error_ != blink::mojom::BackgroundFetchError::NONE) {
    FinishWithError(error_);
    return;
  }
  if (completed_requests_.empty()) {
    FinishWithError(blink::mojom::BackgroundFetchError::NONE);
    return;
  }

  base::RepeatingClosure barrier_closure = base::BarrierClosure(
      completed_requests_.size(),
      base::BindOnce(&GetSettledFetchesTask::FinishWithError,
                     weak_factory_.GetWeakPtr(),
                     blink::mojom::BackgroundFetchError::NONE));

  settled_fetches_.reserve(completed_requests_.size());
  for (const auto& completed_request : completed_requests_) {
    settled_fetches_.emplace_back();
    settled_fetches_.back().request =
        std::move(ServiceWorkerFetchRequest::ParseFromString(
            completed_request.serialized_request()));
    FillResponse(&settled_fetches_.back(), barrier_closure);
  }
}

void GetSettledFetchesTask::FillResponse(
    BackgroundFetchSettledFetch* settled_fetch,
    base::OnceClosure callback) {
  DCHECK(settled_fetch);
  DCHECK(handle_.value());

  auto request =
      std::make_unique<ServiceWorkerFetchRequest>(settled_fetch->request);

  handle_.value()->Match(std::move(request), nullptr /* match_params */,
                         base::BindOnce(&GetSettledFetchesTask::DidMatchRequest,
                                        weak_factory_.GetWeakPtr(),
                                        settled_fetch, std::move(callback)));
}

void GetSettledFetchesTask::DidMatchRequest(
    BackgroundFetchSettledFetch* settled_fetch,
    base::OnceClosure callback,
    blink::mojom::CacheStorageError error,
    std::unique_ptr<ServiceWorkerResponse> cache_response) {
  if (error != blink::mojom::CacheStorageError::kSuccess) {
    DCHECK(settled_fetch);
    FillUncachedResponse(settled_fetch, std::move(callback));
    return;
  }
  settled_fetch->response = std::move(*cache_response);
  std::move(callback).Run();
}

void GetSettledFetchesTask::FillUncachedResponse(
    BackgroundFetchSettledFetch* settled_fetch,
    base::OnceClosure callback) {
  background_fetch_succeeded_ = false;

  // TODO(rayankans): Fill unmatched response with error reports.
  settled_fetch->response.response_type =
      network::mojom::FetchResponseType::kError;
  settled_fetch->response.url_list.push_back(settled_fetch->request.url);

  std::move(callback).Run();
}

void GetSettledFetchesTask::FinishWithError(
    blink::mojom::BackgroundFetchError error) {
  std::move(settled_fetches_callback_)
      .Run(error, background_fetch_succeeded_, std::move(settled_fetches_),
           {} /* blob_data_handles */);
  Finished();  // Destroys |this|.
}

}  // namespace background_fetch

}  // namespace content
