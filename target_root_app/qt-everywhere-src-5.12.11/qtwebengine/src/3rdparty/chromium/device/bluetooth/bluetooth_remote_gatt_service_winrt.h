// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_BLUETOOTH_REMOTE_GATT_SERVICE_WINRT_H_
#define DEVICE_BLUETOOTH_BLUETOOTH_REMOTE_GATT_SERVICE_WINRT_H_

#include <windows.devices.bluetooth.genericattributeprofile.h>
#include <wrl/client.h>

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "device/bluetooth/bluetooth_export.h"
#include "device/bluetooth/bluetooth_remote_gatt_service.h"
#include "device/bluetooth/bluetooth_uuid.h"

namespace device {

class BluetoothDevice;

class DEVICE_BLUETOOTH_EXPORT BluetoothRemoteGattServiceWinrt
    : public BluetoothRemoteGattService {
 public:
  static std::unique_ptr<BluetoothRemoteGattServiceWinrt> Create(
      BluetoothDevice* device,
      Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::
                                 GenericAttributeProfile::IGattDeviceService>
          gatt_service);
  ~BluetoothRemoteGattServiceWinrt() override;

  // BluetoothRemoteGattService:
  std::string GetIdentifier() const override;
  BluetoothUUID GetUUID() const override;
  bool IsPrimary() const override;
  BluetoothDevice* GetDevice() const override;
  std::vector<BluetoothRemoteGattService*> GetIncludedServices() const override;

 private:
  BluetoothRemoteGattServiceWinrt(
      BluetoothDevice* device,
      Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::
                                 GenericAttributeProfile::IGattDeviceService>
          gatt_service,
      BluetoothUUID uuid,
      uint16_t attribute_handle);

  BluetoothDevice* device_;
  Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::
                             GenericAttributeProfile::IGattDeviceService>
      gatt_service_;
  BluetoothUUID uuid_;
  uint16_t attribute_handle_;
  std::string identifier_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothRemoteGattServiceWinrt);
};

}  // namespace device

#endif  // DEVICE_BLUETOOTH_BLUETOOTH_REMOTE_GATT_SERVICE_WINRT_H_
