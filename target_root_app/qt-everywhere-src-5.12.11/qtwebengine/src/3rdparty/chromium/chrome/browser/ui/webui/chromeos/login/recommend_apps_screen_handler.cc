// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/recommend_apps_screen_handler.h"

#include "chrome/browser/chromeos/arc/arc_session_manager.h"
#include "chrome/browser/chromeos/login/screens/recommend_apps_screen.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/app_list/arc/arc_fast_app_reinstall_starter.h"
#include "chrome/grit/component_extension_resources.h"
#include "chrome/grit/generated_resources.h"
#include "components/arc/arc_prefs.h"
#include "components/login/localized_values_builder.h"
#include "ui/base/resource/resource_bundle.h"

namespace {

const char kJsScreenPath[] = "login.RecommendAppsScreen";

constexpr const char kUserActionSkip[] = "recommendAppsSkip";
constexpr const char kUserActionRetry[] = "recommendAppsRetry";
constexpr const char kUserActionInstall[] = "recommendAppsInstall";

}  // namespace

namespace chromeos {

RecommendAppsScreenHandler::RecommendAppsScreenHandler()
    : BaseScreenHandler(kScreenId) {
  set_call_js_prefix(kJsScreenPath);
}

RecommendAppsScreenHandler::~RecommendAppsScreenHandler() {
  for (auto& observer : observer_list_)
    observer.OnViewDestroyed(this);
}

void RecommendAppsScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("recommendAppsScreenTitle",
               IDS_LOGIN_RECOMMEND_APPS_SCREEN_TITLE);
  builder->Add("recommendAppsScreenDescription",
               IDS_LOGIN_RECOMMEND_APPS_SCREEN_DESCRIPTION);
  builder->Add("recommendAppsSkip", IDS_LOGIN_RECOMMEND_APPS_SKIP);
  builder->Add("recommendAppsInstall", IDS_LOGIN_RECOMMEND_APPS_INSTALL);
  builder->Add("recommendAppsRetry", IDS_LOGIN_RECOMMEND_APPS_RETRY);
  builder->Add("recommendAppsLoading", IDS_LOGIN_RECOMMEND_APPS_SCREEN_LOADING);
  builder->Add("recommendAppsError", IDS_LOGIN_RECOMMEND_APPS_SCREEN_ERROR);
}

void RecommendAppsScreenHandler::RegisterMessages() {
  BaseScreenHandler::RegisterMessages();
  AddCallback(kUserActionSkip, &RecommendAppsScreenHandler::HandleSkip);
  AddCallback(kUserActionRetry, &RecommendAppsScreenHandler::HandleRetry);
  AddRawCallback(kUserActionInstall,
                 &RecommendAppsScreenHandler::HandleInstall);
}

void RecommendAppsScreenHandler::AddObserver(
    RecommendAppsScreenViewObserver* observer) {
  observer_list_.AddObserver(observer);
}

void RecommendAppsScreenHandler::RemoveObserver(
    RecommendAppsScreenViewObserver* observer) {
  observer_list_.RemoveObserver(observer);
}

void RecommendAppsScreenHandler::Bind(RecommendAppsScreen* screen) {
  screen_ = screen;
  BaseScreenHandler::SetBaseScreen(screen);
}

void RecommendAppsScreenHandler::Show() {
  ShowScreen(kScreenId);

  Profile* profile = ProfileManager::GetActiveUserProfile();
  pref_service_ = profile->GetPrefs();
}

void RecommendAppsScreenHandler::Hide() {}

void RecommendAppsScreenHandler::Initialize() {}

void RecommendAppsScreenHandler::LoadAppListInUI(const base::Value& app_list) {
  if (!page_is_ready()) {
    CallJS("showError");
    return;
  }
  const ui::ResourceBundle& resource_bundle =
      ui::ResourceBundle::GetSharedInstance();
  base::StringPiece app_list_webview = resource_bundle.GetRawDataResource(
      IDR_ARC_SUPPORT_RECOMMEND_APP_LIST_VIEW_HTML);
  CallJS("setWebview", app_list_webview.as_string());
  CallJS("loadAppList", app_list);
}

void RecommendAppsScreenHandler::OnLoadError() {
  CallJS("showError");
}

void RecommendAppsScreenHandler::OnLoadSuccess(const base::Value& app_list) {
  LoadAppListInUI(app_list);
}

void RecommendAppsScreenHandler::HandleSkip() {
  for (auto& observer : observer_list_)
    observer.OnSkip();
}

void RecommendAppsScreenHandler::HandleRetry() {
  for (auto& observer : observer_list_)
    observer.OnRetry();
}

void RecommendAppsScreenHandler::HandleInstall(const base::ListValue* args) {
  pref_service_->Set(arc::prefs::kArcFastAppReinstallPackages, *args);

  arc::ArcFastAppReinstallStarter* fast_app_reinstall_starter =
      arc::ArcSessionManager::Get()->fast_app_resintall_starter();
  if (fast_app_reinstall_starter) {
    fast_app_reinstall_starter->OnAppsSelectionFinished();
  } else {
    LOG(ERROR)
        << "Cannot complete Fast App Reinstall flow. Starter is not available.";
  }

  for (auto& observer : observer_list_)
    observer.OnInstall();
}

}  // namespace chromeos
