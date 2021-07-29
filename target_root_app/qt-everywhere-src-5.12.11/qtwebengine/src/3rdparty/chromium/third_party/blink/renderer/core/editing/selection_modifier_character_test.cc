// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/editing/selection_modifier.h"

#include "third_party/blink/renderer/core/editing/testing/editing_test_base.h"

namespace blink {

class SelectionModifierCharacterTest : public EditingTestBase {};

// Regression test for crbug.com/834850
TEST_F(SelectionModifierCharacterTest, MoveLeftTowardsListMarkerNoCrash) {
  const SelectionInDOMTree selection =
      SetSelectionTextToBody("<ol contenteditable><li>|<br></li></ol>");
  SelectionModifier modifier(GetFrame(), selection);
  modifier.Modify(SelectionModifyAlteration::kMove,
                  SelectionModifyDirection::kLeft, TextGranularity::kCharacter);
  // Shouldn't crash here.
  EXPECT_EQ("<ol contenteditable><li>|<br></li></ol>",
            GetSelectionTextFromBody(modifier.Selection().AsSelection()));
}

// Regression test for crbug.com/861559
TEST_F(SelectionModifierCharacterTest, MoveRightInDirAutoBidiTextNoCrash) {
  const SelectionInDOMTree selection = SetSelectionTextToBody(
      u8"<pre contenteditable dir=\"auto\">\u05D0$|A$\u05D0</pre>");
  SelectionModifier modifier(GetFrame(), selection);
  modifier.Modify(SelectionModifyAlteration::kMove,
                  SelectionModifyDirection::kRight,
                  TextGranularity::kCharacter);
  // Shouldn't crash here.
  EXPECT_EQ(u8"<pre contenteditable dir=\"auto\">\u05D0|$A$\u05D0</pre>",
            GetSelectionTextFromBody(modifier.Selection().AsSelection()));
}

}  // namespace blink
