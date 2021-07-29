// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_CONTEXT_H_
#define CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_CONTEXT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/browser/background_fetch/background_fetch_data_manager_observer.h"
#include "content/browser/background_fetch/background_fetch_delegate_proxy.h"
#include "content/browser/background_fetch/background_fetch_event_dispatcher.h"
#include "content/browser/background_fetch/storage/get_initialization_data_task.h"
#include "content/browser/service_worker/service_worker_context_core_observer.h"
#include "content/common/content_export.h"
#include "content/public/browser/browser_thread.h"
#include "third_party/blink/public/platform/modules/background_fetch/background_fetch.mojom.h"

namespace storage {
class BlobDataHandle;
}

namespace content {

class BackgroundFetchJobController;
class BackgroundFetchDataManager;
struct BackgroundFetchOptions;
class BackgroundFetchRegistrationId;
class BackgroundFetchRegistrationNotifier;
class BackgroundFetchScheduler;
class BrowserContext;
class CacheStorageContextImpl;
class ServiceWorkerContextWrapper;
struct ServiceWorkerFetchRequest;

// The BackgroundFetchContext is the central moderator of ongoing background
// fetch requests from the Mojo service and from other callers.
// Background Fetch requests function similarly to normal fetches except that
// they are persistent across Chromium or service worker shutdown.
class CONTENT_EXPORT BackgroundFetchContext
    : public BackgroundFetchDataManagerObserver,
      public ServiceWorkerContextCoreObserver,
      public base::RefCountedThreadSafe<BackgroundFetchContext,
                                        BrowserThread::DeleteOnIOThread> {
 public:
  // The BackgroundFetchContext will watch the ServiceWorkerContextWrapper so
  // that it can respond to service worker events such as unregister.
  BackgroundFetchContext(
      BrowserContext* browser_context,
      const scoped_refptr<ServiceWorkerContextWrapper>& service_worker_context,
      const scoped_refptr<content::CacheStorageContextImpl>&
          cache_storage_context);

  void InitializeOnIOThread();

  // Called by the StoragePartitionImpl destructor.
  void Shutdown();

  // Gets the active Background Fetch registration identified by |developer_id|
  // for the given |service_worker_id| and |origin|. The |callback| will be
  // invoked with the registration when it has been retrieved.
  void GetRegistration(
      int64_t service_worker_registration_id,
      const url::Origin& origin,
      const std::string& developer_id,
      blink::mojom::BackgroundFetchService::GetRegistrationCallback callback);

  // Gets all the Background Fetch registration |developer_id|s for a Service
  // Worker and invokes |callback| with that list.
  void GetDeveloperIdsForServiceWorker(
      int64_t service_worker_registration_id,
      const url::Origin& origin,
      blink::mojom::BackgroundFetchService::GetDeveloperIdsCallback callback);

  // Starts a Background Fetch for the |registration_id|. The |requests| will be
  // asynchronously fetched. The |callback| will be invoked when the fetch has
  // been registered, or an error occurred that prevents it from doing so.
  void StartFetch(const BackgroundFetchRegistrationId& registration_id,
                  const std::vector<ServiceWorkerFetchRequest>& requests,
                  const BackgroundFetchOptions& options,
                  const SkBitmap& icon,
                  blink::mojom::BackgroundFetchService::FetchCallback callback);

  // Gets display size for the icon for Background Fetch UI.
  void GetIconDisplaySize(
      blink::mojom::BackgroundFetchService::GetIconDisplaySizeCallback
          callback);

  // Aborts the Background Fetch for the |registration_id|. The callback will be
  // invoked with INVALID_ID if the registration has already completed or
  // aborted, STORAGE_ERROR if an I/O error occurs, or NONE for success.
  void Abort(const BackgroundFetchRegistrationId& registration_id,
             blink::mojom::BackgroundFetchService::AbortCallback callback);

  // Registers the |observer| to be notified of progress events for the
  // registration identified by |unique_id| whenever they happen. The observer
  // will unregister itself when the Mojo endpoint goes away.
  void AddRegistrationObserver(
      const std::string& unique_id,
      blink::mojom::BackgroundFetchRegistrationObserverPtr observer);

  // Updates the title of the Background Fetch identified by |registration_id|.
  // The |callback| will be invoked when the title has been updated, or an error
  // occurred that prevents it from doing so.
  void UpdateUI(
      const BackgroundFetchRegistrationId& registration_id,
      const std::string& title,
      blink::mojom::BackgroundFetchService::UpdateUICallback callback);

  // BackgroundFetchDataManagerObserver implementation.
  void OnUpdatedUI(const BackgroundFetchRegistrationId& registration_id,
                   const std::string& title) override;
  void OnServiceWorkerDatabaseCorrupted(
      int64_t service_worker_registration_id) override;

  // ServiceWorkerContextCoreObserver implementation.
  void OnRegistrationDeleted(int64_t registration_id,
                             const GURL& pattern) override;
  void OnStorageWiped() override;

 private:
  friend class BackgroundFetchServiceTest;
  friend class BackgroundFetchJobControllerTest;
  friend class base::DeleteHelper<BackgroundFetchContext>;
  friend class base::RefCountedThreadSafe<BackgroundFetchContext,
                                          BrowserThread::DeleteOnIOThread>;
  friend struct BrowserThread::DeleteOnThread<BrowserThread::IO>;

  ~BackgroundFetchContext() override;

  void ShutdownOnIO();

  // Creates a new Job Controller for the given |registration_id| and |options|,
  // which will start fetching the files that are part of the registration.
  void CreateController(
      const BackgroundFetchRegistrationId& registration_id,
      const BackgroundFetchOptions& options,
      const SkBitmap& icon,
      const std::string& ui_title,
      size_t num_completed_requests,
      size_t num_requests,
      const std::vector<std::string>& outstanding_guids,
      std::unique_ptr<BackgroundFetchRegistration> registration);

  // Called when an existing registration has been retrieved from the data
  // manager. If the registration does not exist then |registration| is nullptr.
  void DidGetRegistration(
      blink::mojom::BackgroundFetchService::GetRegistrationCallback callback,
      blink::mojom::BackgroundFetchError error,
      std::unique_ptr<BackgroundFetchRegistration> registration);

  // Called when a new registration has been created by the data manager.
  void DidCreateRegistration(
      const BackgroundFetchRegistrationId& registration_id,
      const BackgroundFetchOptions& options,
      const SkBitmap& icon,
      size_t num_requests,
      blink::mojom::BackgroundFetchError error,
      std::unique_ptr<BackgroundFetchRegistration> registration);

  // Called by a JobController when it finishes processing. Also used to
  // implement |Abort|.
  void DidFinishJob(
      base::OnceCallback<void(blink::mojom::BackgroundFetchError)> callback,
      const BackgroundFetchRegistrationId& registration_id,
      BackgroundFetchReasonToAbort reason_to_abort);

  // Called when the data manager finishes marking a registration as deleted.
  void DidMarkForDeletion(
      const BackgroundFetchRegistrationId& registration_id,
      BackgroundFetchReasonToAbort reason_to_abort,
      base::OnceCallback<void(blink::mojom::BackgroundFetchError)> callback,
      blink::mojom::BackgroundFetchError error);

  // Called when the sequence of settled fetches for |registration_id| have been
  // retrieved from storage, and the Service Worker event can be invoked.
  void DidGetSettledFetches(
      const BackgroundFetchRegistrationId& registration_id,
      blink::mojom::BackgroundFetchError error,
      bool background_fetch_succeeded,
      std::vector<BackgroundFetchSettledFetch> settled_fetches,
      std::vector<std::unique_ptr<storage::BlobDataHandle>> blob_data_handles);

  // Called when the notification UI for the background fetch job associated
  // with |unique_id| is activated.
  void DispatchClickEvent(const std::string& unique_id);

  // Called when the data manager finishes getting the initialization data.
  void DidGetInitializationData(
      blink::mojom::BackgroundFetchError error,
      std::vector<background_fetch::BackgroundFetchInitializationData>
          initialization_data);

  // Called when all processing for the |registration_id| has been finished and
  // the job is ready to be deleted. |blob_handles| are unused, but some callers
  // use it to keep blobs alive for the right duration.
  // |partial cleanup|, when set, preserves  the registration ID, and the state
  // of Fetch when it completed, in |completed_fetches_|. This is not done when
  // fetch is aborted or cancelled. We use this information to propagate
  // BackgroundFetchClicked event to the developer, when the user taps the UI.
  void CleanupRegistration(
      const BackgroundFetchRegistrationId& registration_id,
      const std::vector<std::unique_ptr<storage::BlobDataHandle>>&
          blob_data_handles,
      mojom::BackgroundFetchState background_fetch_state,
      bool preserve_info_to_dispatch_click_event = false);

  // Called when the last JavaScript BackgroundFetchRegistration object has been
  // garbage collected for a registration marked for deletion, and so it is now
  // safe to delete the underlying registration data.
  void LastObserverGarbageCollected(
      const BackgroundFetchRegistrationId& registration_id);

  // Switches out |data_manager_| with a DataManager configured for testing
  // environments. Must be called directly after the constructor.
  void SetDataManagerForTesting(
      std::unique_ptr<BackgroundFetchDataManager> data_manager);

  // Helper method to abandon ongoing fetches for a given service worker.
  // Abandons all of them if |service_worker_registration_id| is set to
  // blink::mojom::kInvalidServiceWorkerRegistrationId.
  void AbandonFetches(int64_t service_worker_registration_id);

  // |this| is owned, indirectly, by the BrowserContext.
  BrowserContext* browser_context_;

  std::unique_ptr<BackgroundFetchDataManager> data_manager_;
  scoped_refptr<ServiceWorkerContextWrapper> service_worker_context_;
  BackgroundFetchEventDispatcher event_dispatcher_;
  std::unique_ptr<BackgroundFetchRegistrationNotifier> registration_notifier_;
  BackgroundFetchDelegateProxy delegate_proxy_;
  std::unique_ptr<BackgroundFetchScheduler> scheduler_;

  // Map from background fetch registration |unique_id|s to active job
  // controllers. Must be destroyed before |data_manager_| and
  // |registration_notifier_|.
  std::map<std::string, std::unique_ptr<BackgroundFetchJobController>>
      job_controllers_;

  // Map from background fetch registration |unique_ids|s to {background fetch
  // registration id, fetch state}. An entry in here means the fetch has
  // completed. This information is needed after the fetch has completed.
  // TODO(crbug.com/857122): Clean this up when the UI is no longer showing.
  std::map<
      std::string,
      std::pair<BackgroundFetchRegistrationId, mojom::BackgroundFetchState>>
      completed_fetches_;

  // Map from BackgroundFetchRegistrationIds to FetchCallbacks for active
  // fetches. Must be destroyed before |data_manager_| and
  // |registration_notifier_|. Since FetchCallback is a OnceCallback, please
  // erase the map entry once the calback has been invoked.
  std::map<BackgroundFetchRegistrationId,
           blink::mojom::BackgroundFetchService::FetchCallback>
      fetch_callbacks_;

  // This is used to hang the fetch logic for testing. For instance, this helps
  // us test the behavior when a service worker gets unregistered before the
  // controller has been created.
  bool hang_registration_creation_for_testing_ = false;

  base::WeakPtrFactory<BackgroundFetchContext> weak_factory_;  // Must be last.

  DISALLOW_COPY_AND_ASSIGN(BackgroundFetchContext);
};

}  // namespace content

#endif  // CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_CONTEXT_H_
