// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/ios/form_util/form_activity_tab_helper.h"

#import <Foundation/Foundation.h>

#include "base/values.h"
#include "components/autofill/ios/form_util/form_activity_observer.h"
#include "ios/web/public/web_state/form_activity_params.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

DEFINE_WEB_STATE_USER_DATA_KEY(autofill::FormActivityTabHelper);

namespace autofill {

namespace {
// Prefix for the form activity event commands. Must be kept in sync with
// form.js.
const char kCommandPrefix[] = "form";
}

// static
FormActivityTabHelper* FormActivityTabHelper::GetOrCreateForWebState(
    web::WebState* web_state) {
  FormActivityTabHelper* helper = FromWebState(web_state);
  if (!helper) {
    CreateForWebState(web_state);
    helper = FromWebState(web_state);
    DCHECK(helper);
  }
  return helper;
}

FormActivityTabHelper::FormActivityTabHelper(web::WebState* web_state)
    : web_state_(web_state) {
  web_state_->AddObserver(this);
  web_state_->AddScriptCommandCallback(
      base::BindRepeating(&FormActivityTabHelper::OnFormCommand,
                          base::Unretained(this)),
      kCommandPrefix);
}

FormActivityTabHelper::~FormActivityTabHelper() {
  if (web_state_) {
    web_state_->RemoveObserver(this);
    web_state_->RemoveScriptCommandCallback(kCommandPrefix);
    web_state_ = nullptr;
  }
}

void FormActivityTabHelper::AddObserver(FormActivityObserver* observer) {
  observers_.AddObserver(observer);
}

void FormActivityTabHelper::RemoveObserver(FormActivityObserver* observer) {
  observers_.RemoveObserver(observer);
}

bool FormActivityTabHelper::OnFormCommand(const base::DictionaryValue& message,
                                          const GURL& url,
                                          bool has_user_gesture,
                                          bool form_in_main_frame) {
  std::string command;
  if (!message.GetString("command", &command)) {
    DLOG(WARNING) << "JS message parameter not found: command";
    return NO;
  }
  if (command == "form.submit") {
    return FormSubmissionHandler(message, has_user_gesture, form_in_main_frame);
  }
  if (command == "form.activity") {
    return HandleFormActivity(message, has_user_gesture, form_in_main_frame);
  }
  return false;
}

bool FormActivityTabHelper::HandleFormActivity(
    const base::DictionaryValue& message,
    bool has_user_gesture,
    bool form_in_main_frame) {
  web::FormActivityParams params;
  if (!message.GetString("formName", &params.form_name) ||
      !message.GetString("fieldName", &params.field_name) ||
      !message.GetString("fieldIdentifier", &params.field_identifier) ||
      !message.GetString("fieldType", &params.field_type) ||
      !message.GetString("type", &params.type) ||
      !message.GetString("value", &params.value) ||
      !message.GetBoolean("hasUserGesture", &params.has_user_gesture)) {
    params.input_missing = true;
  }

  params.is_main_frame = form_in_main_frame;
  for (auto& observer : observers_)
    observer.OnFormActivity(web_state_, params);
  return true;
}

bool FormActivityTabHelper::FormSubmissionHandler(
    const base::DictionaryValue& message,
    bool has_user_gesture,
    bool form_in_main_frame) {
  std::string href;
  if (!message.GetString("href", &href)) {
    DLOG(WARNING) << "JS message parameter not found: href";
    return false;
  }
  std::string form_name;
  message.GetString("formName", &form_name);
  // We decide the form is user-submitted if the user has interacted with
  // the main page (using logic from the popup blocker), or if the keyboard
  // is visible.
  BOOL submitted_by_user =
      has_user_gesture || [web_state_->GetWebViewProxy() keyboardAccessory];

  for (auto& observer : observers_)
    observer.DidSubmitDocument(web_state_, form_name, submitted_by_user,
                               form_in_main_frame);
  return true;
}

void FormActivityTabHelper::WebStateDestroyed(web::WebState* web_state) {
  DCHECK_EQ(web_state_, web_state);
  web_state_->RemoveScriptCommandCallback(kCommandPrefix);
  web_state_->RemoveObserver(this);
  web_state_ = nullptr;
}

}  // namespace autofill
