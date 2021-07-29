// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_COCOA_ACCESSIBILITY_HOSTABLE_H_
#define UI_BASE_COCOA_ACCESSIBILITY_HOSTABLE_H_

#import <objc/objc.h>

// An object that can be hosted in another accessibility hierarchy.
// This allows for stitching together heterogenous accessibility
// hierarchies, for example the AXPlatformNodeCocoa-based views
// toolkit hierarchy and the BrowserAccessibilityCocoa-based
// web content hierarchy.
@protocol AccessibilityHostable

// Sets |accessibilityParent| as the object returned when the
// receiver is queried for its accessibility parent.
// TODO(lgrey/ellyjones): Remove this in favor of setAccessibilityParent:
// when we switch to the new accessibility API.
- (void)setAccessibilityParentElement:(id)accessibilityParent;

@end

#endif  // UI_BASE_COCOA_ACCESSIBILITY_HOSTABLE_H_
