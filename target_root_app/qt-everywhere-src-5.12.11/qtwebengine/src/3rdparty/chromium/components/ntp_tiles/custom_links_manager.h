// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NTP_TILES_CUSTOM_LINKS_MANAGER_H_
#define COMPONENTS_NTP_TILES_CUSTOM_LINKS_MANAGER_H_

#include <vector>

#include "base/strings/string16.h"
#include "components/ntp_tiles/ntp_tile.h"
#include "url/gurl.h"

namespace ntp_tiles {

// Interface to manage and store custom links for the NTP. Initialized from
// MostVisitedSites.
//
// Custom links replaces the Most Visited tiles and allows users to manually
// add, edit, and delete tiles (i.e. links) up to a certain maximum. Duplicate
// URLs are not allowed, and the links are stored locally per profile.
// TODO(crbug/861831): Add Chrome sync support.
class CustomLinksManager {
 public:
  struct Link {
    GURL url;
    base::string16 title;

    bool operator==(const Link& other) const {
      return url == other.url && title == other.title;
    }
  };

  virtual ~CustomLinksManager() = default;

  // Fills the initial links with |tiles| and sets the initalized status to
  // true. Returns false and does nothing if custom links has already been
  // initialized.
  virtual bool Initialize(const NTPTilesVector& tiles) = 0;
  // Uninitializes custom links and clears the current links from storage.
  virtual void Uninitialize() = 0;
  // True if custom links is initialized and Most Visited tiles have been
  // replaced by custom links.
  virtual bool IsInitialized() const = 0;

  // Returns the current links.
  virtual const std::vector<Link>& GetLinks() const = 0;

  // Adds a link to the end of the list. Returns false and does nothing if
  // custom links is not initialized, |url| is invalid, we're at the maximum
  // number of links, or |url| already exists in the list.
  virtual bool AddLink(const GURL& url, const base::string16& title) = 0;
  // Updates the URL and/or title of the link specified by |url|. Returns
  // false and does nothing if custom links is not initialized, either URL is
  // invalid, |url| does not exist in the list, |new_url| already exists in the
  // list, or both parameters are empty.
  virtual bool UpdateLink(const GURL& url,
                          const GURL& new_url,
                          const base::string16& new_title) = 0;
  // Deletes the link with the specified |url|. Returns false and does nothing
  // if custom links is not initialized, |url| is invalid, or |url| does not
  // exist in the list.
  virtual bool DeleteLink(const GURL& url) = 0;
  // Restores the previous state of the list of links. Used to undo the previous
  // action (add, edit, delete, etc.). Returns false and does nothing if custom
  // links is not initialized or there is no previous state to restore.
  virtual bool UndoAction() = 0;
};

}  // namespace ntp_tiles

#endif  // COMPONENTS_NTP_TILES_CUSTOM_LINKS_MANAGER_H_
