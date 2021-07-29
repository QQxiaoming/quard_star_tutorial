// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_remote_gatt_characteristic_winrt.h"

#include "base/logging.h"
#include "device/bluetooth/bluetooth_uuid.h"

namespace device {

BluetoothRemoteGattCharacteristicWinrt::
    BluetoothRemoteGattCharacteristicWinrt() = default;

BluetoothRemoteGattCharacteristicWinrt::
    ~BluetoothRemoteGattCharacteristicWinrt() = default;

std::string BluetoothRemoteGattCharacteristicWinrt::GetIdentifier() const {
  NOTIMPLEMENTED();
  return std::string();
}

BluetoothUUID BluetoothRemoteGattCharacteristicWinrt::GetUUID() const {
  NOTIMPLEMENTED();
  return BluetoothUUID();
}

BluetoothGattCharacteristic::Properties
BluetoothRemoteGattCharacteristicWinrt::GetProperties() const {
  NOTIMPLEMENTED();
  return Properties();
}

BluetoothGattCharacteristic::Permissions
BluetoothRemoteGattCharacteristicWinrt::GetPermissions() const {
  NOTIMPLEMENTED();
  return Permissions();
}

const std::vector<uint8_t>& BluetoothRemoteGattCharacteristicWinrt::GetValue()
    const {
  return value_;
}

BluetoothRemoteGattService* BluetoothRemoteGattCharacteristicWinrt::GetService()
    const {
  NOTIMPLEMENTED();
  return nullptr;
}

void BluetoothRemoteGattCharacteristicWinrt::ReadRemoteCharacteristic(
    const ValueCallback& callback,
    const ErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

void BluetoothRemoteGattCharacteristicWinrt::WriteRemoteCharacteristic(
    const std::vector<uint8_t>& value,
    const base::Closure& callback,
    const ErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

void BluetoothRemoteGattCharacteristicWinrt::SubscribeToNotifications(
    BluetoothRemoteGattDescriptor* ccc_descriptor,
    const base::Closure& callback,
    const ErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

void BluetoothRemoteGattCharacteristicWinrt::UnsubscribeFromNotifications(
    BluetoothRemoteGattDescriptor* ccc_descriptor,
    const base::Closure& callback,
    const ErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

}  // namespace device
