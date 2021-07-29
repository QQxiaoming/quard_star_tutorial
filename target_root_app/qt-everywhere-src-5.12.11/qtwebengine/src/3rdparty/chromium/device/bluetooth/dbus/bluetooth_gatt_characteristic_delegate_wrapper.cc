// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/dbus/bluetooth_gatt_characteristic_delegate_wrapper.h"

#include "device/bluetooth/bluez/bluetooth_local_gatt_characteristic_bluez.h"

namespace bluez {

BluetoothGattCharacteristicDelegateWrapper::
    BluetoothGattCharacteristicDelegateWrapper(
        BluetoothLocalGattServiceBlueZ* service,
        BluetoothLocalGattCharacteristicBlueZ* characteristic)
    : BluetoothGattAttributeValueDelegate(service),
      characteristic_(characteristic) {}

void BluetoothGattCharacteristicDelegateWrapper::GetValue(
    const dbus::ObjectPath& device_path,
    const device::BluetoothLocalGattService::Delegate::ValueCallback& callback,
    const device::BluetoothLocalGattService::Delegate::ErrorCallback&
        error_callback) {
  service()->GetDelegate()->OnCharacteristicReadRequest(
      GetDeviceWithPath(device_path), characteristic_, 0, callback,
      error_callback);
}

void BluetoothGattCharacteristicDelegateWrapper::SetValue(
    const dbus::ObjectPath& device_path,
    const std::vector<uint8_t>& value,
    const base::Closure& callback,
    const device::BluetoothLocalGattService::Delegate::ErrorCallback&
        error_callback) {
  service()->GetDelegate()->OnCharacteristicWriteRequest(
      GetDeviceWithPath(device_path), characteristic_, value, 0, callback,
      error_callback);
}

void BluetoothGattCharacteristicDelegateWrapper::StartNotifications(
    const dbus::ObjectPath& device_path,
    device::BluetoothGattCharacteristic::NotificationType notification_type) {
  service()->GetDelegate()->OnNotificationsStart(
      GetDeviceWithPath(device_path), notification_type, characteristic_);
}

void BluetoothGattCharacteristicDelegateWrapper::StopNotifications(
    const dbus::ObjectPath& device_path) {
  service()->GetDelegate()->OnNotificationsStop(GetDeviceWithPath(device_path),
                                                characteristic_);
}

void BluetoothGattCharacteristicDelegateWrapper::PrepareSetValue(
    const dbus::ObjectPath& device_path,
    const std::vector<uint8_t>& value,
    int offset,
    bool has_subsequent_request,
    const base::Closure& callback,
    const device::BluetoothLocalGattService::Delegate::ErrorCallback&
        error_callback) {
  service()->GetDelegate()->OnCharacteristicPrepareWriteRequest(
      GetDeviceWithPath(device_path), characteristic_, value, offset,
      has_subsequent_request, callback, error_callback);
}

}  // namespace bluez
