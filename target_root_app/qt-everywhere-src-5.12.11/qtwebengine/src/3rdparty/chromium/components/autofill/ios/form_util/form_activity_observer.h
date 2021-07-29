// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_OBSERVER_H_
#define COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_OBSERVER_H_

#include <string>

#include "base/macros.h"

namespace web {
class WebState;
struct FormActivityParams;
}  // namespace web

namespace autofill {

// Interface for observing form activity.
// It is the responsibility of the observer to unregister if the web_state
// becomes invalid.
class FormActivityObserver {
 public:
  FormActivityObserver() {}
  virtual ~FormActivityObserver() {}

  // Called when the user is typing on a form field in the main frame or in a
  // same-origin iframe. |params.input_missing| is indicating if there is any
  // error when parsing the form field information.
  // TODO(crbug.com/823285): during the transition from WebStateObserver
  // to FormActivityObserver, some class will inherit from both interface
  // so the method need to use a different name. Once the transition is
  // complete and the methods removed from WebStateObserver, this method
  // will be renamed to FormActivityRegistered.
  virtual void OnFormActivity(web::WebState* web_state,
                              const web::FormActivityParams& params) {}

  // Called on form submission in the main frame or in a same-origin iframe.
  // |has_user_gesture| is true if the user interacted with the page.
  // |form_in_main_frame| is true if the submitted form is hosted in the main
  // frame.
  // TODO(crbug.com/823285): during the transition from WebStateObserver
  // to FormActivityObserver, some class will inherit from both interface
  // so the method need to use a different name. Once the transition is
  // complete and the methods removed from WebStateObserver, this method
  // will be renamed to DocumentSubmitted.
  virtual void DidSubmitDocument(web::WebState* web_state,
                                 const std::string& form_name,
                                 bool has_user_gesture,
                                 bool form_in_main_frame) {}

 private:
  DISALLOW_COPY_AND_ASSIGN(FormActivityObserver);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_OBSERVER_H_
