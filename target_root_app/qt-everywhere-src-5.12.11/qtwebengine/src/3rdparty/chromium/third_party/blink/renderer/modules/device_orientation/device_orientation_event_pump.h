// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_DEVICE_ORIENTATION_DEVICE_ORIENTATION_EVENT_PUMP_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_DEVICE_ORIENTATION_DEVICE_ORIENTATION_EVENT_PUMP_H_

#include "base/macros.h"
#include "services/device/public/cpp/generic_sensor/orientation_data.h"
#include "third_party/blink/public/platform/modules/device_orientation/web_device_orientation_listener.h"
#include "third_party/blink/renderer/modules/device_orientation/device_sensor_event_pump.h"
#include "third_party/blink/renderer/modules/modules_export.h"

namespace blink {

class MODULES_EXPORT DeviceOrientationEventPump
    : public DeviceSensorEventPump<blink::WebDeviceOrientationListener> {
 public:
  // Angle threshold beyond which two orientation events are considered
  // sufficiently different.
  static const double kOrientationThreshold;

  explicit DeviceOrientationEventPump(
      scoped_refptr<base::SingleThreadTaskRunner> task_runner,
      bool absolute);
  ~DeviceOrientationEventPump() override;

  // DeviceSensorEventPump:
  void SendStartMessage(LocalFrame* frame) override;
  void SendStopMessage() override;

 protected:
  // DeviceSensorEventPump:
  void FireEvent(TimerBase*) override;
  void DidStartIfPossible() override;

  SensorEntry relative_orientation_sensor_;
  SensorEntry absolute_orientation_sensor_;

 private:
  friend class DeviceOrientationEventPumpTest;
  friend class DeviceAbsoluteOrientationEventPumpTest;

  // DeviceSensorEventPump:
  bool SensorsReadyOrErrored() const override;

  void GetDataFromSharedMemory(device::OrientationData* data);

  bool ShouldFireEvent(const device::OrientationData& data) const;

  bool absolute_;
  bool fall_back_to_absolute_orientation_sensor_;
  bool should_suspend_absolute_orientation_sensor_ = false;
  device::OrientationData data_;

  DISALLOW_COPY_AND_ASSIGN(DeviceOrientationEventPump);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_DEVICE_ORIENTATION_DEVICE_ORIENTATION_EVENT_PUMP_H_
