// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_UTIL_MAC_H_
#define UI_VIEWS_WIDGET_UTIL_MAC_H_

#import <Cocoa/Cocoa.h>

#include "base/mac/foundation_util.h"

// Weak lets Chrome launch even if a future macOS doesn't have NSThemeFrame.
WEAK_IMPORT_ATTRIBUTE
@interface NSThemeFrame : NSView
@end

#endif  // UI_VIEWS_WIDGET_UTIL_MAC_H_
