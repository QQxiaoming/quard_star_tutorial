// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/accessibility/mojom/ax_event_mojom_traits.h"

#include "mojo/public/cpp/test_support/test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/accessibility/ax_event.h"
#include "ui/accessibility/mojom/ax_event.mojom.h"

using mojo::test::SerializeAndDeserialize;

TEST(AXEventMojomTraitsTest, RoundTrip) {
  ui::AXEvent input;
  input.event_type = ax::mojom::Event::kFocus;
  input.id = 111;
  input.event_from = ax::mojom::EventFrom::kUser;
  input.action_request_id = 222;
  ui::AXEvent output;
  EXPECT_TRUE(SerializeAndDeserialize<ax::mojom::AXEvent>(&input, &output));
  EXPECT_EQ(ax::mojom::Event::kFocus, output.event_type);
  EXPECT_EQ(111, output.id);
  EXPECT_EQ(ax::mojom::EventFrom::kUser, output.event_from);
  EXPECT_EQ(222, output.action_request_id);
}
