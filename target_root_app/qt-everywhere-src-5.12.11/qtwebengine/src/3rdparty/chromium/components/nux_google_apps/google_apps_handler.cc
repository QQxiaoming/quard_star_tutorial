// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/nux_google_apps/google_apps_handler.h"

#include "base/bind.h"
#include "base/metrics/field_trial_params.h"
#include "base/metrics/histogram_macros.h"
#include "base/stl_util.h"
#include "base/strings/utf_string_conversions.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/common/bookmark_pref_names.h"
#include "components/favicon/core/favicon_service.h"
#include "components/grit/components_resources.h"
#include "components/grit/components_scaled_resources.h"
#include "components/nux/show_promo_delegate.h"
#include "components/nux_google_apps/constants.h"
#include "components/prefs/pref_service.h"
#include "components/strings/grit/components_strings.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/base/resource/resource_bundle.h"

namespace nux_google_apps {

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class GoogleApps {
  kGmail = 0,
  kYouTube = 1,
  kMaps = 2,
  kTranslate = 3,
  kNews = 4,
  kChromeWebStore = 5,
  kCount,
};

const char* kGoogleAppsInteractionHistogram =
    "FirstRun.NewUserExperience.GoogleAppsInteraction";

// Strings in costants not translated because this is an experiment.
// Translate before wide release.

constexpr const char* kGoogleAppNames[] = {
    "Gmail", "YouTube", "Maps", "Translate", "News", "Chrome Web Store",
};

constexpr const char* kGoogleAppUrls[] = {
    "https://gmail.com",       "https://youtube.com",
    "https://maps.google.com", "https://translate.google.com",
    "https://news.google.com", "https://chrome.google.com/webstore",
};

constexpr const int kGoogleAppIconSize = 48;  // Pixels.
constexpr const int kGoogleAppIcons[] = {
    IDR_NUX_GOOGLE_APPS_GMAIL_1X, IDR_NUX_GOOGLE_APPS_YOUTUBE_1X,
    IDR_NUX_GOOGLE_APPS_MAPS_1X,  IDR_NUX_GOOGLE_APPS_TRANSLATE_1X,
    IDR_NUX_GOOGLE_APPS_NEWS_1X,  IDR_NUX_GOOGLE_APPS_CHROME_STORE_1X,
};

static_assert(base::size(kGoogleAppNames) == base::size(kGoogleAppUrls),
              "names and urls must match");
static_assert(base::size(kGoogleAppNames) == (size_t)GoogleApps::kCount,
              "names and histograms must match");
static_assert(base::size(kGoogleAppNames) == base::size(kGoogleAppIcons),
              "names and icons must match");

GoogleAppsHandler::GoogleAppsHandler(PrefService* prefs,
                                     favicon::FaviconService* favicon_service,
                                     bookmarks::BookmarkModel* bookmark_model)
    : prefs_(prefs),
      favicon_service_(favicon_service),
      bookmark_model_(bookmark_model) {}

GoogleAppsHandler::~GoogleAppsHandler() {}

void GoogleAppsHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "rejectGoogleApps",
      base::BindRepeating(&GoogleAppsHandler::HandleRejectGoogleApps,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "addGoogleApps",
      base::BindRepeating(&GoogleAppsHandler::HandleAddGoogleApps,
                          base::Unretained(this)));
}

void GoogleAppsHandler::HandleRejectGoogleApps(const base::ListValue* args) {
  UMA_HISTOGRAM_ENUMERATION(kGoogleAppsInteractionHistogram,
                            GoogleAppsInteraction::kNoThanks,
                            GoogleAppsInteraction::kCount);
}

void GoogleAppsHandler::HandleAddGoogleApps(const base::ListValue* args) {
  // Add bookmarks for all selected apps.
  int bookmarkIndex = 0;
  for (size_t i = 0; i < (size_t)GoogleApps::kCount; ++i) {
    bool selected = false;
    CHECK(args->GetBoolean(i, &selected));
    if (selected) {
      UMA_HISTOGRAM_ENUMERATION(
          "FirstRun.NewUserExperience.GoogleAppsSelection", (GoogleApps)i,
          GoogleApps::kCount);
      GURL app_url = GURL(kGoogleAppUrls[i]);
      bookmark_model_->AddURL(bookmark_model_->bookmark_bar_node(),
                              bookmarkIndex++,
                              base::ASCIIToUTF16(kGoogleAppNames[i]), app_url);

      // Preload the favicon cache with Chrome-bundled images. Otherwise, the
      // pre-populated bookmarks don't have favicons and look bad. Favicons are
      // updated automatically when a user visits a site.
      favicon_service_->MergeFavicon(
          app_url, app_url, favicon_base::IconType::kFavicon,
          ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(
              kGoogleAppIcons[i]),
          gfx::Size(kGoogleAppIconSize, kGoogleAppIconSize));
    }
  }

  // Enable bookmark bar.
  prefs_->SetBoolean(bookmarks::prefs::kShowBookmarkBar, true);

  // Wait to show bookmark bar.
  // TODO(hcarmona): Any advice here would be helpful.

  // Show bookmark bubble.
  ShowPromoDelegate::CreatePromoDelegate(
      IDS_NUX_GOOGLE_APPS_DESCRIPTION_PROMO_BUBBLE)
      ->ShowForNode(bookmark_model_->bookmark_bar_node()->GetChild(0));

  UMA_HISTOGRAM_ENUMERATION(kGoogleAppsInteractionHistogram,
                            GoogleAppsInteraction::kGetStarted,
                            GoogleAppsInteraction::kCount);
}

void GoogleAppsHandler::AddSources(content::WebUIDataSource* html_source) {
  // Localized strings.
  html_source->AddLocalizedString("noThanks", IDS_NO_THANKS);
  html_source->AddLocalizedString("getStarted",
                                  IDS_NUX_GOOGLE_APPS_GET_STARTED);
  html_source->AddLocalizedString("nuxDescription",
                                  IDS_NUX_GOOGLE_APPS_DESCRIPTION);

  // Add required resources.
  html_source->AddResourcePath("apps", IDR_NUX_GOOGLE_APPS_HTML);
  html_source->AddResourcePath("apps/nux_google_apps.js",
                               IDR_NUX_GOOGLE_APPS_JS);

  html_source->AddResourcePath("apps/nux_google_apps_proxy.html",
                               IDR_NUX_GOOGLE_APPS_PROXY_HTML);
  html_source->AddResourcePath("apps/nux_google_apps_proxy.js",
                               IDR_NUX_GOOGLE_APPS_PROXY_JS);

  html_source->AddResourcePath("apps/apps_chooser.html",
                               IDR_NUX_GOOGLE_APPS_CHOOSER_HTML);
  html_source->AddResourcePath("apps/apps_chooser.js",
                               IDR_NUX_GOOGLE_APPS_CHOOSER_JS);

  // Add icons
  html_source->AddResourcePath("apps/chrome_store_1x.png",
                               IDR_NUX_GOOGLE_APPS_CHROME_STORE_1X);
  html_source->AddResourcePath("apps/chrome_store_2x.png",
                               IDR_NUX_GOOGLE_APPS_CHROME_STORE_2X);
  html_source->AddResourcePath("apps/gmail_1x.png",
                               IDR_NUX_GOOGLE_APPS_GMAIL_1X);
  html_source->AddResourcePath("apps/gmail_2x.png",
                               IDR_NUX_GOOGLE_APPS_GMAIL_2X);
  html_source->AddResourcePath("apps/maps_1x.png", IDR_NUX_GOOGLE_APPS_MAPS_1X);
  html_source->AddResourcePath("apps/maps_2x.png", IDR_NUX_GOOGLE_APPS_MAPS_2X);
  html_source->AddResourcePath("apps/news_1x.png", IDR_NUX_GOOGLE_APPS_NEWS_1X);
  html_source->AddResourcePath("apps/news_2x.png", IDR_NUX_GOOGLE_APPS_NEWS_2X);
  html_source->AddResourcePath("apps/translate_1x.png",
                               IDR_NUX_GOOGLE_APPS_TRANSLATE_1X);
  html_source->AddResourcePath("apps/translate_2x.png",
                               IDR_NUX_GOOGLE_APPS_TRANSLATE_2X);
  html_source->AddResourcePath("apps/youtube_1x.png",
                               IDR_NUX_GOOGLE_APPS_YOUTUBE_1X);
  html_source->AddResourcePath("apps/youtube_2x.png",
                               IDR_NUX_GOOGLE_APPS_YOUTUBE_2X);
}

}  // namespace nux_google_apps