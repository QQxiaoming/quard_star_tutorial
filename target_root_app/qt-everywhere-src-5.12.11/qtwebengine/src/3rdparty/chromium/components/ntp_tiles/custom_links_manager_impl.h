// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NTP_TILES_CUSTOM_LINKS_MANAGER_IMPL_H_
#define COMPONENTS_NTP_TILES_CUSTOM_LINKS_MANAGER_IMPL_H_

#include <utility>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "components/ntp_tiles/custom_links_manager.h"
#include "components/ntp_tiles/custom_links_store.h"
#include "components/ntp_tiles/ntp_tile.h"

class PrefService;

namespace user_prefs {
class PrefRegistrySyncable;
}  // namespace user_prefs

namespace ntp_tiles {

// Non-test implementation of the CustomLinksManager interface.
class CustomLinksManagerImpl : public CustomLinksManager {
 public:
  // Restores the previous state of |current_links_| from prefs.
  explicit CustomLinksManagerImpl(PrefService* prefs);

  ~CustomLinksManagerImpl() override;

  // CustomLinksManager implementation.
  bool Initialize(const NTPTilesVector& tiles) override;
  void Uninitialize() override;
  bool IsInitialized() const override;

  const std::vector<Link>& GetLinks() const override;

  bool AddLink(const GURL& url, const base::string16& title) override;
  bool UpdateLink(const GURL& url,
                  const GURL& new_url,
                  const base::string16& new_title) override;
  bool DeleteLink(const GURL& url) override;
  bool UndoAction() override;

  // Register preferences used by this class.
  static void RegisterProfilePrefs(
      user_prefs::PrefRegistrySyncable* user_prefs);

 private:
  void ClearLinks();
  // Returns an iterator into |custom_links_|.
  std::vector<Link>::iterator FindLinkWithUrl(const GURL& url);

  PrefService* const prefs_;
  CustomLinksStore store_;
  std::vector<Link> current_links_;
  // The state of the current list of links before the last action was
  // performed.
  base::Optional<std::vector<Link>> previous_links_;

  base::WeakPtrFactory<CustomLinksManagerImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(CustomLinksManagerImpl);
};

}  // namespace ntp_tiles

#endif  // COMPONENTS_NTP_TILES_CUSTOM_LINKS_MANAGER_IMPL_H_
