// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_TAB_HELPER_H_
#define COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_TAB_HELPER_H_

#include "base/macros.h"
#include "base/observer_list.h"
#include "base/values.h"
#import "ios/web/public/web_state/ui/crw_web_view_proxy.h"
#include "ios/web/public/web_state/web_state_observer.h"
#import "ios/web/public/web_state/web_state_user_data.h"

namespace autofill {

class FormActivityObserver;

// Observes user activity on web page forms and forwards form activity event to
// FormActivityObserver.
class FormActivityTabHelper
    : public web::WebStateObserver,
      public web::WebStateUserData<FormActivityTabHelper> {
 public:
  ~FormActivityTabHelper() override;

  static FormActivityTabHelper* GetOrCreateForWebState(
      web::WebState* web_state);

  // Observer registration methods.
  virtual void AddObserver(FormActivityObserver* observer);
  virtual void RemoveObserver(FormActivityObserver* observer);

 private:
  friend class web::WebStateUserData<FormActivityTabHelper>;
  // TestFormActivityTabHelper can be used by tests that want to simulate form
  // events without loading page and executing JavaScript.
  // To trigger events, TestFormActivityTabHelper will access |observer_|.
  friend class TestFormActivityTabHelper;

  explicit FormActivityTabHelper(web::WebState* web_state);

  // WebStateObserver implementation.
  void WebStateDestroyed(web::WebState* web_state) override;

  // Handler for "form.activity" JavaScript command.
  bool HandleFormActivity(const base::DictionaryValue& message,
                          bool has_user_gesture,
                          bool form_in_main_frame);

  // Handler for "form.submit" JavaScript command.
  bool FormSubmissionHandler(const base::DictionaryValue& message,
                             bool has_user_gesture,
                             bool form_in_main_frame);

  // Handler for "form.*" JavaScript command. Dispatch to more specific handler.
  bool OnFormCommand(const base::DictionaryValue& message,
                     const GURL& url,
                     bool has_user_gesture,
                     bool form_in_main_frame);

  // The WebState this instance is observing. Will be null after
  // WebStateDestroyed has been called.
  web::WebState* web_state_ = nullptr;

  // The observers.
  base::ObserverList<FormActivityObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(FormActivityTabHelper);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_TAB_HELPER_H_
