// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_OBSERVER_BRIDGE_H_
#define COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_OBSERVER_BRIDGE_H_

#import <Foundation/Foundation.h>

#include "base/macros.h"
#include "components/autofill/ios/form_util/form_activity_observer.h"

@protocol FormActivityObserver<NSObject>
@optional
// Invoked by WebStateObserverBridge::FormActivity.
// TODO(crbug.com/823285): during the transition from CRWWebStateObserver
// to FormActivityObserver, some class will implement from both protocols
// so the method need to use a different name. Once the transition is
// complete and the methods removed from CRWWebStateObserver, this method
// will be renamed to didRegisterFormActivity.
- (void)webState:(web::WebState*)webState
    registeredFormActivity:(const web::FormActivityParams&)params;

// Invoked by WebStateObserverBridge::DidSubmitDocument.
// TODO(crbug.com/823285): during the transition from CRWWebStateObserver
// to FormActivityObserver, some class will implement from both protocols
// so the method need to use a different name. Once the transition is
// complete and the methods removed from CRWWebStateObserver, this method
// will be renamed to didSubmitDocumentWithFormNamed.
- (void)webState:(web::WebState*)webState
    submittedDocumentWithFormNamed:(const std::string&)formName
                    hasUserGesture:(BOOL)hasUserGesture
                   formInMainFrame:(BOOL)formInMainFrame;

@end

namespace autofill {

// Use this class to be notified of the form activity in an Objective-C class.
// Implement the |FormActivityObserver| activity protocol and create a strong
// member FormActivityObserverBridge
// form_activity_obserber_bridge_ =
//     std::make_unique<FormActivityObserverBridge>(web_state, self);
// It is the responsibility of the owner class to delete this bridge if the
// web_state becomes invalid.
class FormActivityObserverBridge : public FormActivityObserver {
 public:
  // |owner| will not be retained.
  FormActivityObserverBridge(web::WebState* web_state,
                             id<FormActivityObserver> owner);
  ~FormActivityObserverBridge() override;

  // FormActivityObserver overrides:
  void OnFormActivity(web::WebState* web_state,
                      const web::FormActivityParams& params) override;

  void DidSubmitDocument(web::WebState* web_state,
                         const std::string& form_name,
                         bool has_user_gesture,
                         bool form_in_main_frame) override;

 private:
  web::WebState* web_state_ = nullptr;
  __weak id<FormActivityObserver> owner_ = nil;

  DISALLOW_COPY_AND_ASSIGN(FormActivityObserverBridge);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_IOS_FORM_UTIL_FORM_ACTIVITY_OBSERVER_BRIDGE_H_
