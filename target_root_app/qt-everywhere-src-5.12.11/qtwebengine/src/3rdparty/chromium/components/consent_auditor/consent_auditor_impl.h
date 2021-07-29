// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CONSENT_AUDITOR_CONSENT_AUDITOR_IMPL_H_
#define COMPONENTS_CONSENT_AUDITOR_CONSENT_AUDITOR_IMPL_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "components/consent_auditor/consent_auditor.h"
#include "components/consent_auditor/consent_sync_bridge.h"
#include "components/keyed_service/core/keyed_service.h"

namespace syncer {
class ModelTypeControllerDelegate;
class UserEventService;
}  // namespace syncer

namespace sync_pb {
class UserConsentSpecifics;
class UserEventSpecifics;
}  // namespace sync_pb

class PrefService;
class PrefRegistrySimple;

namespace consent_auditor {

// TODO(vitaliii): Delete user-event-related code once USER_CONSENTS type is
// fully launched.
class ConsentAuditorImpl : public ConsentAuditor {
 public:
  ConsentAuditorImpl(
      PrefService* pref_service,
      std::unique_ptr<syncer::ConsentSyncBridge> consent_sync_bridge,
      syncer::UserEventService* user_event_service,
      const std::string& app_version,
      const std::string& app_locale);
  ~ConsentAuditorImpl() override;

  // KeyedService (through ConsentAuditor) implementation.
  void Shutdown() override;

  // Registers the preferences needed by this service.
  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  // Consent auditor implementation.

  // Records a consent for |feature| for the signed-in GAIA account with
  // the ID |account_id| (as defined in AccountInfo).
  // Consent text consisted of strings with |consent_grd_ids|, and the UI
  // element the user clicked had the ID |confirmation_grd_id|.
  // Whether the consent was GIVEN or NOT_GIVEN is passed as |status|.
  void RecordGaiaConsent(const std::string& account_id,
                         Feature feature,
                         const std::vector<int>& description_grd_ids,
                         int confirmation_grd_id,
                         ConsentStatus status) override;

  // Records that the user consented to a |feature|. The user was presented with
  // |description_text| and accepted it by interacting |confirmation_text|
  // (e.g. clicking on a button; empty if not applicable).
  // Returns true if successful.
  void RecordLocalConsent(const std::string& feature,
                          const std::string& description_text,
                          const std::string& confirmation_text) override;

  // Returns the underlying Sync integration point.
  base::WeakPtr<syncer::ModelTypeControllerDelegate>
  GetControllerDelegateOnUIThread() override;

 private:
  std::unique_ptr<sync_pb::UserEventSpecifics> ConstructUserEventSpecifics(
      const std::string& account_id,
      Feature feature,
      const std::vector<int>& description_grd_ids,
      int confirmation_grd_id,
      ConsentStatus status);

  std::unique_ptr<sync_pb::UserConsentSpecifics> ConstructUserConsentSpecifics(
      const std::string& account_id,
      Feature feature,
      const std::vector<int>& description_grd_ids,
      int confirmation_grd_id,
      ConsentStatus status);

  PrefService* pref_service_;
  std::unique_ptr<syncer::ConsentSyncBridge> consent_sync_bridge_;
  syncer::UserEventService* user_event_service_;
  std::string app_version_;
  std::string app_locale_;

  DISALLOW_COPY_AND_ASSIGN(ConsentAuditorImpl);
};

}  // namespace consent_auditor

#endif  // COMPONENTS_CONSENT_AUDITOR_CONSENT_AUDITOR_IMPL_H_
