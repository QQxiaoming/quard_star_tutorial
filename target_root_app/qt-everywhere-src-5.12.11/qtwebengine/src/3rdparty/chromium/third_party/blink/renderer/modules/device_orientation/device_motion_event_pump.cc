// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include "services/device/public/mojom/sensor.mojom-blink.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/modules/device_orientation/device_motion_data.h"
#include "third_party/blink/renderer/modules/device_orientation/device_motion_event_pump.h"
#include "ui/gfx/geometry/angle_conversions.h"

namespace {

constexpr double kDefaultPumpDelayMilliseconds =
    blink::DeviceMotionEventPump::kDefaultPumpDelayMicroseconds / 1000;

}  // namespace

namespace blink {

template class DeviceSensorEventPump<blink::WebDeviceMotionListener>;

DeviceMotionEventPump::DeviceMotionEventPump(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner)
    : DeviceSensorEventPump<blink::WebDeviceMotionListener>(task_runner),
      accelerometer_(this, device::mojom::blink::SensorType::ACCELEROMETER),
      linear_acceleration_sensor_(
          this,
          device::mojom::blink::SensorType::LINEAR_ACCELERATION),
      gyroscope_(this, device::mojom::blink::SensorType::GYROSCOPE) {}

DeviceMotionEventPump::~DeviceMotionEventPump() {
  StopIfObserving();
}

void DeviceMotionEventPump::SendStartMessage(LocalFrame* frame) {
  if (!sensor_provider_) {
    DCHECK(frame);

    frame->GetInterfaceProvider().GetInterface(
        mojo::MakeRequest(&sensor_provider_));
    sensor_provider_.set_connection_error_handler(
        WTF::Bind(&DeviceSensorEventPump::HandleSensorProviderError,
                  WTF::Unretained(this)));
  }

  accelerometer_.Start(sensor_provider_.get());
  linear_acceleration_sensor_.Start(sensor_provider_.get());
  gyroscope_.Start(sensor_provider_.get());
}

void DeviceMotionEventPump::SendStopMessage() {
  // SendStopMessage() gets called both when the page visibility changes and if
  // all device motion event listeners are unregistered. Since removing the
  // event listener is more rare than the page visibility changing,
  // Sensor::Suspend() is used to optimize this case for not doing extra work.

  accelerometer_.Stop();
  linear_acceleration_sensor_.Stop();
  gyroscope_.Stop();
}

void DeviceMotionEventPump::FireEvent(TimerBase*) {
  DCHECK(listener());

  DeviceMotionData* data = GetDataFromSharedMemory();

  // data is null if not all sensors are active
  if (data)
    listener()->DidChangeDeviceMotion(data);
}

bool DeviceMotionEventPump::SensorsReadyOrErrored() const {
  return accelerometer_.ReadyOrErrored() &&
         linear_acceleration_sensor_.ReadyOrErrored() &&
         gyroscope_.ReadyOrErrored();
}

DeviceMotionData* DeviceMotionEventPump::GetDataFromSharedMemory() {
  DeviceMotionData::Acceleration* acceleration = nullptr;
  DeviceMotionData::Acceleration* acceleration_including_gravity = nullptr;
  DeviceMotionData::RotationRate* rotation_rate = nullptr;

  if (accelerometer_.SensorReadingCouldBeRead()) {
    if (accelerometer_.reading.timestamp() == 0.0)
      return nullptr;

    acceleration_including_gravity = DeviceMotionData::Acceleration::Create(
        !std::isnan(accelerometer_.reading.accel.x.value()),
        accelerometer_.reading.accel.x,
        !std::isnan(accelerometer_.reading.accel.y.value()),
        accelerometer_.reading.accel.y,
        !std::isnan(accelerometer_.reading.accel.z.value()),
        accelerometer_.reading.accel.z);
  } else {
    acceleration_including_gravity = DeviceMotionData::Acceleration::Create(
        false, 0.0, false, 0.0, false, 0.0);
  }

  if (linear_acceleration_sensor_.SensorReadingCouldBeRead()) {
    if (linear_acceleration_sensor_.reading.timestamp() == 0.0)
      return nullptr;

    acceleration = DeviceMotionData::Acceleration::Create(
        !std::isnan(linear_acceleration_sensor_.reading.accel.x.value()),
        linear_acceleration_sensor_.reading.accel.x,
        !std::isnan(linear_acceleration_sensor_.reading.accel.y.value()),
        linear_acceleration_sensor_.reading.accel.y,
        !std::isnan(linear_acceleration_sensor_.reading.accel.z.value()),
        linear_acceleration_sensor_.reading.accel.z);
  } else {
    acceleration = DeviceMotionData::Acceleration::Create(false, 0.0, false,
                                                          0.0, false, 0.0);
  }

  if (gyroscope_.SensorReadingCouldBeRead()) {
    if (gyroscope_.reading.timestamp() == 0.0)
      return nullptr;

    rotation_rate = DeviceMotionData::RotationRate::Create(
        !std::isnan(gyroscope_.reading.gyro.x.value()),
        gfx::RadToDeg(gyroscope_.reading.gyro.x),
        !std::isnan(gyroscope_.reading.gyro.y.value()),
        gfx::RadToDeg(gyroscope_.reading.gyro.y),
        !std::isnan(gyroscope_.reading.gyro.z.value()),
        gfx::RadToDeg(gyroscope_.reading.gyro.z));
  } else {
    rotation_rate = DeviceMotionData::RotationRate::Create(false, 0.0, false,
                                                           0.0, false, 0.0);
  }

  // The device orientation spec states that interval should be in
  // milliseconds.
  // https://w3c.github.io/deviceorientation/spec-source-orientation.html#devicemotion
  return DeviceMotionData::Create(acceleration, acceleration_including_gravity,
                                  rotation_rate, kDefaultPumpDelayMilliseconds);
}
}  // namespace blink
