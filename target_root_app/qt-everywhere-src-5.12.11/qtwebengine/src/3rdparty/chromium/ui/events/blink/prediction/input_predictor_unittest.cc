// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_BLINK_PREDICTION_INPUT_PREDICTOR_UNITTEST_H_
#define UI_EVENTS_BLINK_PREDICTION_INPUT_PREDICTOR_UNITTEST_H_

#include "ui/events/blink/prediction/input_predictor.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/events/blink/blink_event_util.h"

namespace {

constexpr double kEpsilon = 0.1;

}  // namespace

namespace ui {

// Base class for predictor unit tests
class InputPredictorTest : public testing::Test {
 public:
  InputPredictorTest() {}

  static base::TimeTicks FromMilliseconds(int64_t ms) {
    return blink::WebInputEvent::GetStaticTimeStampForTests() +
           base::TimeDelta::FromMilliseconds(ms);
  }

  void ValidatePredictor(const std::vector<double>& x,
                         const std::vector<double>& y,
                         const std::vector<double>& timestamp_ms) {
    predictor_->Reset();
    for (size_t i = 0; i < timestamp_ms.size(); i++) {
      if (predictor_->HasPrediction()) {
        ui::InputPredictor::InputData result;
        EXPECT_TRUE(predictor_->GeneratePrediction(
            FromMilliseconds(timestamp_ms[i]), &result));
        EXPECT_NEAR(result.pos.x(), x[i], kEpsilon);
        EXPECT_NEAR(result.pos.y(), y[i], kEpsilon);
      }
      InputPredictor::InputData data = {gfx::PointF(x[i], y[i]),
                                        FromMilliseconds(timestamp_ms[i])};
      predictor_->Update(data);
    }
  }

 protected:
  std::unique_ptr<InputPredictor> predictor_;

  DISALLOW_COPY_AND_ASSIGN(InputPredictorTest);
};

}  // namespace ui

#endif  // UI_EVENTS_BLINK_PREDICTION_INPUT_PREDICTOR_UNITTEST_H_
