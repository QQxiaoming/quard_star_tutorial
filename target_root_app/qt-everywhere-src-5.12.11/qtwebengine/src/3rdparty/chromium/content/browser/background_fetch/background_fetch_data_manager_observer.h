// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_DATA_MANAGER_OBSERVER_H_
#define CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_DATA_MANAGER_OBSERVER_H_

namespace content {

class BackgroundFetchRegistrationId;

// Observer interface for objects that would like to be notified about changes
// committed to storage through the Background Fetch data manager. All methods
// will be invoked on the IO thread.
class BackgroundFetchDataManagerObserver {
 public:
  // Called when the |title| for the Background Fetch |registration_id| has been
  // updated in the data store.
  virtual void OnUpdatedUI(const BackgroundFetchRegistrationId& registration_id,
                           const std::string& title) = 0;

  // Called if corrupted data is found in the Service Worker database.
  virtual void OnServiceWorkerDatabaseCorrupted(
      int64_t service_worker_registration_id) = 0;

  virtual ~BackgroundFetchDataManagerObserver() {}
};

}  // namespace content

#endif  // CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_DATA_MANAGER_OBSERVER_H_
