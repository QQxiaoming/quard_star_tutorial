// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/wtf/text/movable_string.h"

#include <vector>

#include "base/test/metrics/histogram_tester.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace WTF {

class MovableStringTest : public ::testing::Test {
 protected:
  void SetUp() override {
    MovableStringTable::Instance().SetRendererBackgrounded(false);
  }
};

TEST_F(MovableStringTest, Simple) {
  MovableString movable_abc(String("abc").ReleaseImpl());

  EXPECT_TRUE(MovableString().IsNull());
  EXPECT_FALSE(movable_abc.IsNull());
  EXPECT_TRUE(movable_abc.Is8Bit());
  EXPECT_EQ(3u, movable_abc.length());
  EXPECT_EQ(3u, movable_abc.CharactersSizeInBytes());

  EXPECT_EQ(String("abc"), movable_abc.ToString());
  MovableString copy = movable_abc;
  EXPECT_EQ(copy.Impl(), movable_abc.Impl());
}

TEST_F(MovableStringTest, Park) {
  base::HistogramTester histogram_tester;

  MovableString movable(String("abc").Impl());
  EXPECT_FALSE(movable.Impl()->is_parked());
  EXPECT_TRUE(movable.Impl()->Park());
  EXPECT_TRUE(movable.Impl()->is_parked());

  // Not the only one to have a reference to the string.
  String abc("abc");
  MovableString movable2(abc.Impl());
  EXPECT_FALSE(movable2.Impl()->Park());
  abc = String();
  EXPECT_TRUE(movable2.Impl()->Park());

  histogram_tester.ExpectBucketCount(
      "Memory.MovableStringParkingAction",
      MovableStringImpl::ParkingAction::kParkedInBackground, 2);
  histogram_tester.ExpectTotalCount("Memory.MovableStringParkingAction", 2);
}

TEST_F(MovableStringTest, Unpark) {
  base::HistogramTester histogram_tester;

  MovableString movable(String("abc").Impl());
  EXPECT_FALSE(movable.Impl()->is_parked());
  EXPECT_TRUE(movable.Impl()->Park());
  EXPECT_TRUE(movable.Impl()->is_parked());

  String unparked = movable.ToString();
  EXPECT_EQ(String("abc"), unparked);
  EXPECT_FALSE(movable.Impl()->is_parked());

  histogram_tester.ExpectTotalCount("Memory.MovableStringParkingAction", 2);
  histogram_tester.ExpectBucketCount(
      "Memory.MovableStringParkingAction",
      MovableStringImpl::ParkingAction::kParkedInBackground, 1);
  histogram_tester.ExpectBucketCount(
      "Memory.MovableStringParkingAction",
      MovableStringImpl::ParkingAction::kUnparkedInForeground, 1);
}

TEST_F(MovableStringTest, TableSimple) {
  base::HistogramTester histogram_tester;

  std::vector<char> data(20000, 'a');
  MovableString movable(String(data.data(), data.size()).ReleaseImpl());
  ASSERT_FALSE(movable.Impl()->is_parked());

  auto& table = MovableStringTable::Instance();
  EXPECT_EQ(1u, table.table_.size());

  // Small strings are not in the table.
  MovableString small(String("abc").ReleaseImpl());
  EXPECT_EQ(1u, table.table_.size());

  // No parking as the current state is not "backgrounded".
  table.SetRendererBackgrounded(false);
  ASSERT_FALSE(table.IsRendererBackgrounded());
  table.MaybeParkAll();
  EXPECT_FALSE(movable.Impl()->is_parked());
  histogram_tester.ExpectTotalCount("Memory.MovableStringsCount", 0);

  table.SetRendererBackgrounded(true);
  ASSERT_TRUE(table.IsRendererBackgrounded());
  table.MaybeParkAll();
  EXPECT_TRUE(movable.Impl()->is_parked());
  histogram_tester.ExpectUniqueSample("Memory.MovableStringsCount", 1, 1);

  // Park and unpark.
  movable.ToString();
  EXPECT_FALSE(movable.Impl()->is_parked());
  table.MaybeParkAll();
  EXPECT_TRUE(movable.Impl()->is_parked());
  histogram_tester.ExpectUniqueSample("Memory.MovableStringsCount", 1, 2);

  // More than one reference, no parking.
  table.SetRendererBackgrounded(false);
  String alive_unparked = movable.ToString();  // Unparked in foreground.
  table.SetRendererBackgrounded(true);
  table.MaybeParkAll();
  EXPECT_FALSE(movable.Impl()->is_parked());

  // Other reference is dropped, OK to park.
  alive_unparked = String();
  table.MaybeParkAll();
  EXPECT_TRUE(movable.Impl()->is_parked());

  histogram_tester.ExpectTotalCount("Memory.MovableStringParkingAction", 5);
  histogram_tester.ExpectBucketCount(
      "Memory.MovableStringParkingAction",
      MovableStringImpl::ParkingAction::kParkedInBackground, 3);
  histogram_tester.ExpectBucketCount(
      "Memory.MovableStringParkingAction",
      MovableStringImpl::ParkingAction::kUnparkedInBackground, 1);
  histogram_tester.ExpectBucketCount(
      "Memory.MovableStringParkingAction",
      MovableStringImpl::ParkingAction::kUnparkedInForeground, 1);
}

TEST_F(MovableStringTest, TableMultiple) {
  base::HistogramTester histogram_tester;

  size_t size_kb = 20;
  std::vector<char> data(size_kb * 1000, 'a');
  MovableString movable(String(data.data(), data.size()).ReleaseImpl());
  MovableString movable2(String(data.data(), data.size()).ReleaseImpl());

  auto& table = MovableStringTable::Instance();
  EXPECT_EQ(2u, table.table_.size());

  movable2 = MovableString();
  EXPECT_EQ(1u, table.table_.size());

  MovableString copy = movable;
  movable = MovableString();
  EXPECT_EQ(1u, table.table_.size());
  copy = MovableString();
  EXPECT_EQ(0u, table.table_.size());

  String str(data.data(), data.size());
  MovableString movable3(str.Impl());
  EXPECT_EQ(1u, table.table_.size());
  // De-duplicated.
  MovableString other_movable3(str.Impl());
  EXPECT_EQ(1u, table.table_.size());
  EXPECT_EQ(movable3.Impl(), other_movable3.Impl());

  // If all the references to a string are in the table, park it.
  str = String();
  table.SetRendererBackgrounded(true);
  ASSERT_TRUE(table.IsRendererBackgrounded());
  table.MaybeParkAll();
  EXPECT_TRUE(movable3.Impl()->is_parked());

  // Only drop it from the table when the last one is gone.
  movable3 = MovableString();
  EXPECT_EQ(1u, table.table_.size());
  other_movable3 = MovableString();
  EXPECT_EQ(0u, table.table_.size());

  histogram_tester.ExpectUniqueSample("Memory.MovableStringsCount", 1, 1);
  histogram_tester.ExpectUniqueSample("Memory.MovableStringsTotalSizeKb",
                                      size_kb, 1);
  histogram_tester.ExpectTotalCount("Memory.MovableStringParkingAction", 1);
  histogram_tester.ExpectBucketCount(
      "Memory.MovableStringParkingAction",
      MovableStringImpl::ParkingAction::kParkedInBackground, 1);
}

}  // namespace WTF
