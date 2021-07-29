// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/mac/browsing_data_deletion.h"

#include <string>

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#include "base/feature_list.h"
#include "base/logging.h"
#include "base/mac/foundation_util.h"
#include "base/mac/mac_logging.h"
#include "base/mac/mac_util.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/optional.h"
#include "base/strings/sys_string_conversions.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "device/base/features.h"
#include "device/fido/mac/credential_metadata.h"
#include "device/fido/mac/keychain.h"

namespace device {
namespace fido {
namespace mac {

namespace {

bool DoDeleteWebAuthnCredentials(const std::string& keychain_access_group,
                                 const std::string& profile_metadata_secret,
                                 base::Time begin,
                                 base::Time end)
    API_AVAILABLE(macosx(10.12.2)) {
  // Query the keychain for all items tagged with the given access group, which
  // should in theory yield all WebAuthentication credentials (for all
  // profiles). Sadly, the kSecAttrAccessGroup filter doesn't quite work, and
  // so we also get results from the legacy keychain that are tagged with no
  // keychain access group.
  base::ScopedCFTypeRef<CFMutableDictionaryRef> query(
      CFDictionaryCreateMutable(kCFAllocatorDefault, 0, nullptr, nullptr));
  CFDictionarySetValue(query, kSecClass, kSecClassKey);
  CFDictionarySetValue(query, kSecAttrAccessGroup,
                       base::SysUTF8ToNSString(keychain_access_group));
  CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitAll);
  // Return the key reference and its attributes.
  CFDictionarySetValue(query, kSecReturnRef, @YES);
  CFDictionarySetValue(query, kSecReturnAttributes, @YES);

  base::ScopedCFTypeRef<CFArrayRef> keychain_items;
  {
    OSStatus status = Keychain::GetInstance().ItemCopyMatching(
        query, reinterpret_cast<CFTypeRef*>(keychain_items.InitializeInto()));
    if (status == errSecItemNotFound) {
      DVLOG(1) << "no credentials found";
      return true;
    }
    if (status != errSecSuccess) {
      OSSTATUS_DLOG(ERROR, status) << "SecItemCopyMatching failed";
      return false;
    }
  }

  bool result = true;
  // Determine which items from the result need to be deleted.
  for (CFIndex i = 0; i < CFArrayGetCount(keychain_items); ++i) {
    CFDictionaryRef attributes = base::mac::CFCast<CFDictionaryRef>(
        CFArrayGetValueAtIndex(keychain_items, i));
    // Skip items that don't belong to the correct keychain access group
    // because the kSecAttrAccessGroup filter is broken.
    CFStringRef attr_access_group =
        base::mac::GetValueFromDictionary<CFStringRef>(attributes,
                                                       kSecAttrAccessGroup);
    if (!attr_access_group || base::SysCFStringRefToUTF8(attr_access_group) !=
                                  keychain_access_group) {
      DVLOG(1) << "missing/invalid access group";
      continue;
    }
    // If the RP ID, stored encrypted in the item's label, cannot be decrypted
    // with the given metadata secret, then the credential belongs to a
    // different profile and must be ignored.
    CFStringRef sec_attr_label = base::mac::GetValueFromDictionary<CFStringRef>(
        attributes, kSecAttrLabel);
    if (!sec_attr_label) {
      DLOG(ERROR) << "missing label";
      continue;
    }
    base::Optional<std::string> opt_rp_id = CredentialMetadata::DecodeRpId(
        profile_metadata_secret, base::SysCFStringRefToUTF8(sec_attr_label));
    if (!opt_rp_id) {
      DVLOG(1) << "key doesn't belong to this profile";
      continue;
    }

    // If the creation date is out of range, the credential must be ignored.
    // If the creation date is missing for some obscure reason, we delete.
    CFDateRef creation_date_cf = base::mac::GetValueFromDictionary<CFDateRef>(
        attributes, kSecAttrCreationDate);
    if (!creation_date_cf) {
      DVLOG(1) << "missing creation date; deleting anyway";
    } else {
      base::Time creation_date = base::Time::FromCFAbsoluteTime(
          CFDateGetAbsoluteTime(creation_date_cf));
      // Browsing data deletion API uses [begin, end) intervals.
      if (creation_date < begin || creation_date >= end) {
        DVLOG(1) << "creation date out of range";
        continue;
      }
    }

    // The sane way to delete this item would be to build a query that has the
    // kSecMatchItemList field set to a list of SecKeyRef objects that need
    // deleting. Sadly, on macOS that appears to work only if you also set
    // kSecAttrNoLegacy (which is an internal symbol); otherwise it appears to
    // only search the "legacy" keychain and return errSecItemNotFound. What
    // does work however, is lookup by the (unique) kSecAttrApplicationLabel
    // (which stores the credential id). So we clumsily do this for each item
    // instead.
    CFDataRef sec_attr_app_label = base::mac::GetValueFromDictionary<CFDataRef>(
        attributes, kSecAttrApplicationLabel);
    if (!sec_attr_app_label) {
      DLOG(ERROR) << "missing application label";
      continue;
    }
    base::ScopedCFTypeRef<CFMutableDictionaryRef> delete_query(
        CFDictionaryCreateMutable(kCFAllocatorDefault, 0, nullptr, nullptr));
    CFDictionarySetValue(delete_query, kSecClass, kSecClassKey);
    CFDictionarySetValue(delete_query, kSecAttrApplicationLabel,
                         sec_attr_app_label);
    OSStatus status = Keychain::GetInstance().ItemDelete(delete_query);
    if (status != errSecSuccess) {
      OSSTATUS_DLOG(ERROR, status) << "SecItemDelete failed";
      result = false;
      continue;
    }
  }
  return result;
}

}  // namespace

bool DeleteWebAuthnCredentials(const std::string& keychain_access_group,
                               const std::string& profile_metadata_secret,
                               base::Time begin,
                               base::Time end) {
#if defined(OS_MACOSX)
  if (base::FeatureList::IsEnabled(device::kWebAuthTouchId)) {
    // Touch ID uses macOS APIs available in 10.12.2 or newer. No need to check
    // for credentials in lower OS versions.
    if (base::mac::IsAtLeastOS10_13()) {
      return DoDeleteWebAuthnCredentials(keychain_access_group,
                                         profile_metadata_secret, begin, end);
    }
  }
#endif  // defined(OS_MACOSX)
  return true;
}

}  // namespace mac
}  // namespace fido
}  // namespace device
