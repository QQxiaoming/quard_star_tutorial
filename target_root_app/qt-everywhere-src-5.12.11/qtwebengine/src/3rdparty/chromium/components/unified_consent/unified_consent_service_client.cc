// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/unified_consent/unified_consent_service_client.h"

#include "base/bind.h"

namespace unified_consent {

UnifiedConsentServiceClient::UnifiedConsentServiceClient() {}
UnifiedConsentServiceClient::~UnifiedConsentServiceClient() {}

void UnifiedConsentServiceClient::AddObserver(Observer* observer) {
  observer_list_.AddObserver(observer);
}

void UnifiedConsentServiceClient::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);
}

void UnifiedConsentServiceClient::ObserveServicePrefChange(
    Service service,
    const std::string& pref_name,
    PrefService* pref_service) {
  service_prefs_[pref_name] = service;

  // First access to the pref registrar of |pref_service| in the map
  // automatically creates an entry for it.
  PrefChangeRegistrar* pref_change_registrar =
      &(pref_change_registrars_[pref_service]);
  if (!pref_change_registrar->prefs())
    pref_change_registrar->Init(pref_service);
  pref_change_registrar->Add(
      pref_name,
      base::BindRepeating(&UnifiedConsentServiceClient::OnPrefChanged,
                          base::Unretained(this)));
}

void UnifiedConsentServiceClient::FireOnServiceStateChanged(Service service) {
  for (auto& observer : observer_list_)
    observer.OnServiceStateChanged(service);
}

void UnifiedConsentServiceClient::OnPrefChanged(const std::string& pref_name) {
  DCHECK(service_prefs_.count(pref_name));
  FireOnServiceStateChanged(service_prefs_[pref_name]);
}

}  // namespace unified_consent
