// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_ACCESSIBILITY_TESTING_ACCESSIBILITY_SELECTION_TEST_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_ACCESSIBILITY_TESTING_ACCESSIBILITY_SELECTION_TEST_H_

#include <string>

#include "third_party/blink/renderer/modules/accessibility/testing/accessibility_test.h"
#include "third_party/blink/renderer/platform/wtf/allocator.h"

namespace blink {

class AXObject;
class AXSelection;
class LocalFrameClient;

// Makes writing and debugging selection tests easier.
class AccessibilitySelectionTest : public AccessibilityTest {
  USING_FAST_MALLOC(AccessibilitySelectionTest);

 public:
  AccessibilitySelectionTest(LocalFrameClient* local_frame_client = nullptr);

 protected:
  // Gets the inner HTML of the accessibility tree and annotates it with markers
  // indicating the anchor and focus of |selection|.
  std::string GetSelectionText(const AXSelection& selection) const;

  // Gets the inner HTML of the accessibility subtree rooted at |subtree| and
  // annotates it with markers indicating the anchor and focus of |selection|.
  std::string GetSelectionText(const AXSelection& selection,
                               const AXObject& subtree) const;

  // Sets |selection_text| as inner HTML of the document body and returns the
  // root of the accessibility tree at body.
  AXObject* SetSelectionText(const std::string& selection_text) const;

  // Sets |selection_text| as inner HTML of |element| and returns the root of
  // the accessibility subtree at |element|.
  AXObject* SetSelectionText(const std::string& selection_text,
                             HTMLElement& element) const;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_ACCESSIBILITY_TESTING_ACCESSIBILITY_SELECTION_TEST_H_
