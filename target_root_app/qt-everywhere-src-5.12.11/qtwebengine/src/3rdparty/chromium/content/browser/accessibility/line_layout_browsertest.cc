// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "content/browser/accessibility/browser_accessibility.h"
#include "content/browser/accessibility/browser_accessibility_manager.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/shell/browser/shell.h"
#include "content/test/accessibility_browser_test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

class AccessibilityLineLayoutBrowserTest : public ContentBrowserTest {
 public:
  AccessibilityLineLayoutBrowserTest() = default;
  ~AccessibilityLineLayoutBrowserTest() override = default;

 protected:
  BrowserAccessibility* FindButton(BrowserAccessibility* node) {
    if (node->GetRole() == ax::mojom::Role::kButton)
      return node;
    for (unsigned i = 0; i < node->PlatformChildCount(); i++) {
      if (BrowserAccessibility* button = FindButton(node->PlatformGetChild(i)))
        return button;
    }
    return nullptr;
  }

  int CountNextPreviousOnLineLinks(BrowserAccessibility* node) {
    int line_link_count = 0;

    int next_on_line_id =
        node->GetIntAttribute(ax::mojom::IntAttribute::kNextOnLineId);
    if (next_on_line_id) {
      BrowserAccessibility* other = node->manager()->GetFromID(next_on_line_id);
      EXPECT_TRUE(other) << "Next on line link is invalid.";
      line_link_count++;
    }
    int previous_on_line_id =
        node->GetIntAttribute(ax::mojom::IntAttribute::kPreviousOnLineId);
    if (previous_on_line_id) {
      BrowserAccessibility* other =
          node->manager()->GetFromID(previous_on_line_id);
      EXPECT_TRUE(other) << "Previous on line link is invalid.";
      line_link_count++;
    }

    for (unsigned i = 0; i < node->InternalChildCount(); i++)
      line_link_count +=
          CountNextPreviousOnLineLinks(node->InternalGetChild(i));

    return line_link_count;
  }
};

// http://crbug.com/868830 - the patch that enabled this test to pass
// caused a performance regression.
IN_PROC_BROWSER_TEST_F(AccessibilityLineLayoutBrowserTest,
                       DISABLED_WholeBlockIsUpdated) {
  ASSERT_TRUE(embedded_test_server()->Start());

  AccessibilityNotificationWaiter waiter(shell()->web_contents(),
                                         ui::kAXModeComplete,
                                         ax::mojom::Event::kLoadComplete);
  GURL url(embedded_test_server()->GetURL("/accessibility/lines/lines.html"));
  NavigateToURL(shell(), url);
  waiter.WaitForNotification();

  WebContentsImpl* web_contents =
      static_cast<WebContentsImpl*>(shell()->web_contents());
  BrowserAccessibilityManager* manager =
      web_contents->GetRootBrowserAccessibilityManager();

  // There should be at least 2 links between nodes on the same line.
  int line_link_count = CountNextPreviousOnLineLinks(manager->GetRoot());
  ASSERT_GE(line_link_count, 2);

  // Find the button and click it.
  BrowserAccessibility* button = FindButton(manager->GetRoot());
  ASSERT_NE(nullptr, button);
  manager->DoDefaultAction(*button);

  // When done the page will change the button text to "Done".
  WaitForAccessibilityTreeToContainNodeWithName(web_contents, "Done");

  // There should be at least 2 links between nodes on the same line,
  // though not necessarily the same as before.
  line_link_count = CountNextPreviousOnLineLinks(manager->GetRoot());
  ASSERT_GE(line_link_count, 2);
}

}  // namespace content
