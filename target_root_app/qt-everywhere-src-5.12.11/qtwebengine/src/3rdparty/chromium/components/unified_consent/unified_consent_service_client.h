// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_UNIFIED_CONSENT_UNIFIED_CONSENT_SERVICE_CLIENT_H_
#define COMPONENTS_UNIFIED_CONSENT_UNIFIED_CONSENT_SERVICE_CLIENT_H_

#include <map>
#include <string>

#include "base/observer_list.h"
#include "components/prefs/pref_change_registrar.h"

class PrefService;

namespace unified_consent {

class UnifiedConsentServiceClient {
 public:
  enum class Service {
    // Link Doctor error pages.
    kAlternateErrorPages,
    // Metrics reporting.
    kMetricsReporting,
    // Prediction of network actions.
    kNetworkPrediction,
    // Safe browsing.
    kSafeBrowsing,
    // Extended safe browsing.
    kSafeBrowsingExtendedReporting,
    // Search suggestions.
    kSearchSuggest,
    // Spell checking.
    kSpellCheck,

    // Last element of the enum, used for iteration.
    kLast = kSpellCheck,
  };

  enum class ServiceState {
    // The service is not supported on this platform.
    kNotSupported,
    // The service is supported, but disabled.
    kDisabled,
    // The service is enabled.
    kEnabled
  };

  class Observer {
   public:
    // Called when the service state of |service| changes.
    virtual void OnServiceStateChanged(Service service) = 0;
  };

  UnifiedConsentServiceClient();
  virtual ~UnifiedConsentServiceClient();

  // Returns the ServiceState for |service|.
  virtual ServiceState GetServiceState(Service service) = 0;
  // Sets |service| enabled if it is supported on this platform.
  virtual void SetServiceEnabled(Service service, bool enabled) = 0;

  // Methods to register or remove observers.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

 protected:
  // This method adds |pref_name| to the list of prefs that will be observed for
  // changes. Whenever there's a change in the pref, all
  // |UnifiedConsentServiceClient::Observer|s are fired.
  void ObserveServicePrefChange(Service service,
                                const std::string& pref_name,
                                PrefService* pref_service);

  // Fires |OnServiceStateChanged| on all observers.
  void FireOnServiceStateChanged(Service service);

 private:
  // Callback for the pref change registrars.
  void OnPrefChanged(const std::string& pref_name);

  base::ObserverList<Observer, true> observer_list_;

  // Matches the pref name to it's service.
  std::map<std::string, Service> service_prefs_;
  // Matches pref service to it's change registrar.
  std::map<PrefService*, PrefChangeRegistrar> pref_change_registrars_;

  DISALLOW_COPY_AND_ASSIGN(UnifiedConsentServiceClient);
};

}  // namespace unified_consent

#endif  // COMPONENTS_UNIFIED_CONSENT_UNIFIED_CONSENT_SERVICE_CLIENT_H_
