// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BACKGROUND_FETCH_STORAGE_DATABASE_TASK_H_
#define CONTENT_BROWSER_BACKGROUND_FETCH_STORAGE_DATABASE_TASK_H_

#include <memory>
#include <set>
#include <string>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "content/browser/background_fetch/background_fetch_registration_id.h"
#include "content/browser/cache_storage/cache_storage_manager.h"
#include "third_party/blink/public/platform/modules/background_fetch/background_fetch.mojom.h"

namespace content {

class BackgroundFetchDataManager;
class CacheStorageManager;
class ChromeBlobStorageContext;
class ServiceWorkerContextWrapper;

// Note that this also handles non-error cases where the NONE is NONE.
using HandleBackgroundFetchErrorCallback =
    base::OnceCallback<void(blink::mojom::BackgroundFetchError)>;

namespace background_fetch {

class DatabaseTask;

// A DatabaseTaskHost is the entity responsible for scheduling and running
// DatabaseTasks, usually the BackgroundFetchDataManager. Note that a
// DatabaseTask can be a DatabaseTaskHost, in order to run subtasks.
class DatabaseTaskHost {
 public:
  virtual void OnTaskFinished(DatabaseTask* task) = 0;
  virtual BackgroundFetchDataManager* data_manager() = 0;
  virtual ~DatabaseTaskHost();

  base::WeakPtr<DatabaseTaskHost> GetWeakPtr();

 protected:
  DatabaseTaskHost();

 private:
  base::WeakPtrFactory<DatabaseTaskHost> weak_factory_;
};

// A DatabaseTask is an asynchronous "transaction" that needs to read/write the
// Service Worker Database.
//
// Only one DatabaseTask can run at once per StoragePartition, and no other code
// reads/writes Background Fetch keys, so each task effectively has an exclusive
// lock, except that core Service Worker code may delete all keys for a
// ServiceWorkerRegistration or the entire database at any time.
//
// A DatabaseTask can also optionally run other DatabaseTasks as subtasks. This
// allows us to re-use commonly used DatabaseTasks. Subtasks are started as soon
// as they are added, and cannot outlive the parent DatabaseTask.
class DatabaseTask : public DatabaseTaskHost {
 public:
  ~DatabaseTask() override;

  virtual void Start() = 0;

 protected:
  explicit DatabaseTask(DatabaseTaskHost* host);

  // Each task MUST call this once finished, even if exceptions occur, to
  // release their lock and allow the next task to execute.
  // This should be called in FinishWithError() for consistency.
  void Finished();

  void AddDatabaseTask(std::unique_ptr<DatabaseTask> task);
  void AddSubTask(std::unique_ptr<DatabaseTask> task);

  // Abandon all fetches for a given service worker.
  void AbandonFetches(int64_t service_worker_registration_id);

  ServiceWorkerContextWrapper* service_worker_context();

  CacheStorageManager* cache_manager();

  std::set<std::string>& ref_counted_unique_ids();

  ChromeBlobStorageContext* blob_storage_context();

  // DatabaseTaskHost implementation.
  void OnTaskFinished(DatabaseTask* finished_subtask) override;
  BackgroundFetchDataManager* data_manager() override;

 private:
  // Each task must override this function and perform the following steps:
  // 1) Report error (UMA) if applicable.
  // 2) Run the provided callback.
  // 3) Call Finished().
  virtual void FinishWithError(blink::mojom::BackgroundFetchError error) = 0;

  DatabaseTaskHost* host_;

  // Owns a reference to the CacheStorageManager in case Shutdown was
  // called and the DatabaseTask needs to finish.
  scoped_refptr<CacheStorageManager> cache_manager_;

  // Map the raw pointer to its unique_ptr, to make lookups easier.
  std::map<DatabaseTask*, std::unique_ptr<DatabaseTask>> active_subtasks_;

  DISALLOW_COPY_AND_ASSIGN(DatabaseTask);
};

}  // namespace background_fetch

}  // namespace content

#endif  // CONTENT_BROWSER_BACKGROUND_FETCH_STORAGE_DATABASE_TASK_H_
