// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NUX_GOOGLE_APPS_GOOGLE_APPS_HANDLER_H_
#define COMPONENTS_NUX_GOOGLE_APPS_GOOGLE_APPS_HANDLER_H_

#include "base/macros.h"
#include "base/values.h"
#include "content/public/browser/web_ui_message_handler.h"

class PrefService;

namespace bookmarks {
class BookmarkModel;
}  // namespace bookmarks

namespace content {
class WebUIDataSource;
}  // namespace content

namespace favicon {
class FaviconService;
}  // namespace favicon

namespace nux_google_apps {

extern const char* kGoogleAppsInteractionHistogram;

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class GoogleAppsInteraction {
  kPromptShown = 0,
  kNoThanks = 1,
  kGetStarted = 2,
  kCount,
};

class GoogleAppsHandler : public content::WebUIMessageHandler {
 public:
  GoogleAppsHandler(PrefService* prefs,
                    favicon::FaviconService* favicon_service,
                    bookmarks::BookmarkModel* bookmark_model);
  ~GoogleAppsHandler() override;

  // WebUIMessageHandler:
  void RegisterMessages() override;

  // Callbacks for JS APIs.
  void HandleRejectGoogleApps(const base::ListValue* args);
  void HandleAddGoogleApps(const base::ListValue* args);

  // Adds webui sources.
  static void AddSources(content::WebUIDataSource* html_source);

 private:
  // Weak reference.
  PrefService* prefs_;

  // Weak reference.
  favicon::FaviconService* favicon_service_;

  // Weak reference.
  bookmarks::BookmarkModel* bookmark_model_;

  DISALLOW_COPY_AND_ASSIGN(GoogleAppsHandler);
};

}  // namespace nux_google_apps

#endif  // COMPONENTS_NUX_GOOGLE_APPS_GOOGLE_APPS_HANDLER_H_
