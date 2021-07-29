// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/ios/form_util/form_activity_observer_bridge.h"

#include "base/logging.h"
#include "components/autofill/ios/form_util/form_activity_tab_helper.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace autofill {
FormActivityObserverBridge::FormActivityObserverBridge(
    web::WebState* web_state,
    id<FormActivityObserver> owner)
    : web_state_(web_state), owner_(owner) {
  FormActivityTabHelper::GetOrCreateForWebState(web_state)->AddObserver(this);
}

FormActivityObserverBridge::~FormActivityObserverBridge() {
  FormActivityTabHelper::GetOrCreateForWebState(web_state_)
      ->RemoveObserver(this);
}

void FormActivityObserverBridge::OnFormActivity(
    web::WebState* web_state,
    const web::FormActivityParams& params) {
  DCHECK_EQ(web_state, web_state_);
  if ([owner_ respondsToSelector:@selector(webState:registeredFormActivity:)]) {
    [owner_ webState:web_state registeredFormActivity:params];
  }
}

void FormActivityObserverBridge::DidSubmitDocument(web::WebState* web_state,
                                                   const std::string& form_name,
                                                   bool has_user_gesture,
                                                   bool form_in_main_frame) {
  DCHECK_EQ(web_state, web_state_);
  if ([owner_ respondsToSelector:@selector
              (webState:submittedDocumentWithFormNamed:hasUserGesture
                          :formInMainFrame:)]) {
    [owner_ webState:web_state
        submittedDocumentWithFormNamed:form_name
                        hasUserGesture:has_user_gesture
                       formInMainFrame:form_in_main_frame];
  }
}
}  // namespace autofill
