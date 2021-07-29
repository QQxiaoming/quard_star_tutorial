// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/reset_password/reset_password_ui.h"

#include "base/metrics/histogram_functions.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/browser_resources.h"
#include "components/safe_browsing/common/safe_browsing_prefs.h"
#include "components/safe_browsing/password_protection/password_protection_service.h"
#include "components/strings/grit/components_strings.h"
#include "components/url_formatter/url_formatter.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_data_source.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "ui/base/l10n/l10n_util.h"

namespace safe_browsing {
using PasswordReuseEvent =
    safe_browsing::LoginReputationClientRequest::PasswordReuseEvent;
}

namespace {

constexpr char kStringTypeUMAName[] = "PasswordProtection.InterstitialString";

// Used for UMA metric logging. Please don't reorder.
// Indicates which type of strings are shown on this page.
enum StringType {
  GENERIC_NO_ORG_NAME = 0,
  GENERIC_WITH_ORG_NAME = 1,
  WARNING_NO_ORG_NAME = 2,
  WARNING_WITH_ORG_NAME = 3,
  STRING_TYPE_COUNT,
};

// Implementation of mojom::ResetPasswordHander.
class ResetPasswordHandlerImpl : public mojom::ResetPasswordHandler {
 public:
  ResetPasswordHandlerImpl(
      content::WebContents* web_contents,
      safe_browsing::ReusedPasswordType password_type,
      mojo::InterfaceRequest<mojom::ResetPasswordHandler> request)
      : web_contents_(web_contents),
        password_type_(password_type),
        binding_(this, std::move(request)) {
    DCHECK(web_contents);
  }

  ~ResetPasswordHandlerImpl() override {}

  // mojom::ResetPasswordHandler overrides:
  void HandlePasswordReset() override {
    Profile* profile =
        Profile::FromBrowserContext(web_contents_->GetBrowserContext());
    safe_browsing::ChromePasswordProtectionService* service = safe_browsing::
        ChromePasswordProtectionService::GetPasswordProtectionService(profile);
    if (service) {
      service->OnUserAction(
          web_contents_, password_type_,
          safe_browsing::PasswordProtectionService::INTERSTITIAL,
          safe_browsing::PasswordProtectionService::CHANGE_PASSWORD);
    }
  }

 private:
  content::WebContents* web_contents_;
  safe_browsing::ReusedPasswordType password_type_;
  mojo::Binding<mojom::ResetPasswordHandler> binding_;

  DISALLOW_COPY_AND_ASSIGN(ResetPasswordHandlerImpl);
};

// Gets the reused password type from post data, or returns
// REUSED_PASSWORD_TYPE_UNKNOWN if post data is not available.
safe_browsing::ReusedPasswordType GetPasswordType(
    content::WebContents* web_contents) {
  content::NavigationEntry* nav_entry =
      web_contents->GetController().GetPendingEntry();
  if (!nav_entry || !nav_entry->GetHasPostData())
    return safe_browsing::PasswordReuseEvent::REUSED_PASSWORD_TYPE_UNKNOWN;
  auto& post_data = nav_entry->GetPostData()->elements()->at(0);
  int post_data_int = -1;
  if (base::StringToInt(std::string(post_data.bytes(), post_data.length()),
                        &post_data_int)) {
    return static_cast<safe_browsing::ReusedPasswordType>(post_data_int);
  }

  return safe_browsing::PasswordReuseEvent::REUSED_PASSWORD_TYPE_UNKNOWN;
}

// Properly format host name based on text direction.
base::string16 GetFormattedHostName(const std::string host_name) {
  base::string16 host = url_formatter::IDNToUnicode(host_name);
  if (base::i18n::IsRTL())
    base::i18n::WrapStringWithLTRFormatting(&host);
  return host;
}

}  // namespace

ResetPasswordUI::ResetPasswordUI(content::WebUI* web_ui)
    : ui::MojoWebUIController(web_ui) {
  base::DictionaryValue load_time_data;
  password_type_ = GetPasswordType(web_ui->GetWebContents());
  PopulateStrings(web_ui->GetWebContents(), &load_time_data);
  std::unique_ptr<content::WebUIDataSource> html_source(
      content::WebUIDataSource::Create(chrome::kChromeUIResetPasswordHost));
  html_source->AddResourcePath("reset_password.js", IDR_RESET_PASSWORD_JS);
  html_source->AddResourcePath("reset_password.mojom.js",
                               IDR_RESET_PASSWORD_MOJO_JS);
  html_source->SetDefaultResource(IDR_RESET_PASSWORD_HTML);
  html_source->AddLocalizedStrings(load_time_data);
  html_source->UseGzip();

  content::WebUIDataSource::Add(web_ui->GetWebContents()->GetBrowserContext(),
                                html_source.release());

  AddHandlerToRegistry(base::BindRepeating(
      &ResetPasswordUI::BindResetPasswordHandler, base::Unretained(this)));
}

ResetPasswordUI::~ResetPasswordUI() {}

void ResetPasswordUI::BindResetPasswordHandler(
    mojom::ResetPasswordHandlerRequest request) {
  ui_handler_ = std::make_unique<ResetPasswordHandlerImpl>(
      web_ui()->GetWebContents(), password_type_, std::move(request));
}

void ResetPasswordUI::PopulateStrings(content::WebContents* web_contents,
                                      base::DictionaryValue* load_time_data) {
  std::string org_name =
      safe_browsing::ChromePasswordProtectionService::
          GetPasswordProtectionService(
              Profile::FromBrowserContext(web_contents->GetBrowserContext()))
              ->GetOrganizationName(password_type_);
  bool known_password_type =
      password_type_ !=
      safe_browsing::PasswordReuseEvent::REUSED_PASSWORD_TYPE_UNKNOWN;
  if (!known_password_type) {
    base::UmaHistogramEnumeration(
        safe_browsing::kInterstitialActionByUserNavigationHistogram,
        safe_browsing::PasswordProtectionService::SHOWN,
        safe_browsing::PasswordProtectionService::MAX_ACTION);
  }
  int heading_string_id = known_password_type
                              ? IDS_RESET_PASSWORD_WARNING_HEADING
                              : IDS_RESET_PASSWORD_HEADING;
  base::string16 explanation_paragraph_string;
  if (org_name.empty()) {
    explanation_paragraph_string = l10n_util::GetStringUTF16(
        known_password_type ? IDS_RESET_PASSWORD_WARNING_EXPLANATION_PARAGRAPH
                            : IDS_RESET_PASSWORD_EXPLANATION_PARAGRAPH);
    base::UmaHistogramEnumeration(
        kStringTypeUMAName,
        known_password_type ? WARNING_NO_ORG_NAME : GENERIC_NO_ORG_NAME,
        STRING_TYPE_COUNT);
  } else {
    base::string16 formatted_org_name = GetFormattedHostName(org_name);
    explanation_paragraph_string = l10n_util::GetStringFUTF16(
        known_password_type
            ? IDS_RESET_PASSWORD_WARNING_EXPLANATION_PARAGRAPH_WITH_ORG_NAME
            : IDS_RESET_PASSWORD_EXPLANATION_PARAGRAPH_WITH_ORG_NAME,
        formatted_org_name);
    base::UmaHistogramEnumeration(
        kStringTypeUMAName,
        known_password_type ? WARNING_WITH_ORG_NAME : GENERIC_WITH_ORG_NAME,
        STRING_TYPE_COUNT);
  }

  load_time_data->SetString(
      "title", l10n_util::GetStringUTF16(IDS_RESET_PASSWORD_TITLE));
  load_time_data->SetString("heading",
                            l10n_util::GetStringUTF16(heading_string_id));
  load_time_data->SetString("primaryParagraph", explanation_paragraph_string);
  load_time_data->SetString(
      "primaryButtonText",
      l10n_util::GetStringUTF16(IDS_RESET_PASSWORD_BUTTON));
}
