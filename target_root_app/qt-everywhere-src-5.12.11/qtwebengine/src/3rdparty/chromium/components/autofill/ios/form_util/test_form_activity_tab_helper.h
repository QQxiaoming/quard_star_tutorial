// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_IOS_FORM_UTIL_TEST_FORM_ACTIVITY_TAB_HELPER_H_
#define COMPONENTS_AUTOFILL_IOS_FORM_UTIL_TEST_FORM_ACTIVITY_TAB_HELPER_H_

#include <string>

#include "base/macros.h"

namespace web {
struct FormActivityParams;
class WebState;
}  // namespace web

namespace autofill {

class TestFormActivityTabHelper {
 public:
  explicit TestFormActivityTabHelper(web::WebState* web_state);
  ~TestFormActivityTabHelper();

  void OnFormActivity(const web::FormActivityParams& params);
  void OnDocumentSubmitted(const std::string& form_name,
                           bool has_user_gesture,
                           bool form_in_main_frame);

 private:
  web::WebState* web_state_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(TestFormActivityTabHelper);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_IOS_FORM_UTIL_TEST_FORM_ACTIVITY_TAB_HELPER_H_
