// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/editing/selection_modifier.h"

#include "third_party/blink/renderer/core/editing/testing/editing_test_base.h"

namespace blink {

class SelectionModifierWordTest : public EditingTestBase {};

// Regression test for crbug.com/856417
TEST_F(SelectionModifierWordTest, MoveIntoInlineBlockWithBidiNoHang) {
  const SelectionInDOMTree selection = SetSelectionTextToBody(
      "<div contenteditable>"
      "<span>&#x05D0;|&#x05D1;&#x05D2;</span>"
      "<span style=\"display: inline-block\">foo</span>"
      "</div>");
  SelectionModifier modifier(GetFrame(), selection);
  modifier.Modify(SelectionModifyAlteration::kMove,
                  SelectionModifyDirection::kRight, TextGranularity::kWord);
  // Shouldn't hang here.
  EXPECT_EQ(
      "<div contenteditable>"
      "<span>\xD7\x90\xD7\x91\xD7\x92</span>"
      "<span style=\"display: inline-block\">foo|</span>"
      "</div>",
      GetSelectionTextFromBody(modifier.Selection().AsSelection()));
}

}  // namespace blink
