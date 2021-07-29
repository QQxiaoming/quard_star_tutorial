// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/signin/login_ui_service.h"

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"

#if !defined(OS_CHROMEOS)
#include "chrome/browser/browser_process.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/test_extension_system.h"
#include "chrome/browser/profiles/profile_attributes_entry.h"
#include "chrome/browser/profiles/profile_attributes_storage.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/browser_with_test_window_test.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/extension_builder.h"
#endif

class LoginUIServiceTest : public testing::Test {
 public:
  LoginUIServiceTest()
      : profile_manager_(TestingBrowserProcess::GetGlobal()),
        profile_(nullptr) {}
  ~LoginUIServiceTest() override {}

  void SetUp() override {
    ASSERT_TRUE(profile_manager_.SetUp());
    profile_ = profile_manager_.CreateTestingProfile("Person 1");
  }

 protected:
  content::TestBrowserThreadBundle thread_bundle_;

  TestingProfileManager profile_manager_;
  // Test profile used by all tests - this is owned by profile_manager_.
  TestingProfile* profile_;

 private:
  DISALLOW_COPY_AND_ASSIGN(LoginUIServiceTest);
};

class TestLoginUI : public LoginUIService::LoginUI {
 public:
  TestLoginUI() { }
  ~TestLoginUI() override {}
  void FocusUI() override {}

 private:
  DISALLOW_COPY_AND_ASSIGN(TestLoginUI);
};

TEST_F(LoginUIServiceTest, CanSetMultipleLoginUIs) {
  LoginUIService service(profile_);

  EXPECT_EQ(nullptr, service.current_login_ui());

  TestLoginUI ui;
  service.SetLoginUI(&ui);
  EXPECT_EQ(&ui, service.current_login_ui());

  // Test that we can replace the active login UI.
  TestLoginUI other_ui;
  service.SetLoginUI(&other_ui);
  EXPECT_EQ(&other_ui, service.current_login_ui());

  // Test that closing the non-active login UI has no effect.
  service.LoginUIClosed(&ui);
  EXPECT_EQ(&other_ui, service.current_login_ui());

  // Test that closing the foreground UI yields the background UI.
  service.SetLoginUI(&ui);
  EXPECT_EQ(&ui, service.current_login_ui());
  service.LoginUIClosed(&ui);
  EXPECT_EQ(&other_ui, service.current_login_ui());

  // Test that closing the last login UI makes the current login UI nullptr.
  service.LoginUIClosed(&other_ui);
  EXPECT_EQ(nullptr, service.current_login_ui());
}

TEST_F(LoginUIServiceTest, SetProfileBlockingErrorMessage) {
  LoginUIService service(profile_);

  service.SetProfileBlockingErrorMessage();

  EXPECT_EQ(base::string16(), service.GetLastLoginResult());
  EXPECT_EQ(base::string16(), service.GetLastLoginErrorEmail());
  EXPECT_TRUE(service.IsDisplayingProfileBlockedErrorMessage());
}

#if !defined(OS_CHROMEOS)
class LoginUIServiceLoginPopupTest : public BrowserWithTestWindowTest {
 public:
  void SetUp() override {
    BrowserWithTestWindowTest::SetUp();

    service_ = std::make_unique<LoginUIService>(profile());
    model_ = browser()->tab_strip_model();
    ASSERT_EQ(0, model_->count());
  }

  std::unique_ptr<LoginUIService> service_;
  TabStripModel* model_;
};

TEST_F(LoginUIServiceLoginPopupTest, ShowLoginPop) {
  extensions::TestExtensionSystem* extension_system =
      static_cast<extensions::TestExtensionSystem*>(
          extensions::ExtensionSystem::Get(profile()));
  extension_system->CreateExtensionService(
      base::CommandLine::ForCurrentProcess(), base::FilePath(), false);
  service_->ShowLoginPopup();
  EXPECT_EQ(1, model_->count());
}

TEST_F(LoginUIServiceLoginPopupTest, NotShowLoginPopAsLockedProfile) {
  ProfileAttributesEntry* entry;
  ASSERT_TRUE(g_browser_process->profile_manager()
                  ->GetProfileAttributesStorage()
                  .GetProfileAttributesWithPath(profile()->GetPath(), &entry));
  entry->SetIsSigninRequired(true);
  service_->ShowLoginPopup();
  EXPECT_EQ(0, model_->count());
}
#endif
