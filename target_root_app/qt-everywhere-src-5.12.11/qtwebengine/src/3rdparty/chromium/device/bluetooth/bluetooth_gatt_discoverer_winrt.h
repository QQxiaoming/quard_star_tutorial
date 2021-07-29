// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_BLUETOOTH_GATT_DISCOVERER_WINRT_H_
#define DEVICE_BLUETOOTH_BLUETOOTH_GATT_DISCOVERER_WINRT_H_

#include <windows.devices.bluetooth.genericattributeprofile.h>
#include <windows.devices.bluetooth.h>
#include <wrl/client.h>

#include <memory>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "device/bluetooth/bluetooth_device.h"
#include "device/bluetooth/bluetooth_export.h"

namespace device {

// This class is responsible for discovering and enumerating GATT attributes on
// the provided BluetoothLEDevice. Callers are expected to instantiate the class
// and invoke StartGattDiscovery(). Once the discovery completes, the passed-in
// GattDiscoveryCallback is invoked, and discovered GATT attributes can be
// obtained by invoking the appropriate getters.
class DEVICE_BLUETOOTH_EXPORT BluetoothGattDiscovererWinrt {
 public:
  using GattDiscoveryCallback = base::OnceCallback<void(bool)>;
  using GattServiceList = std::vector<
      Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::
                                 GenericAttributeProfile::IGattDeviceService>>;

  BluetoothGattDiscovererWinrt(
      Microsoft::WRL::ComPtr<
          ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice> ble_device);
  ~BluetoothGattDiscovererWinrt();

  void StartGattDiscovery(GattDiscoveryCallback callback);
  const GattServiceList& GetGattServices() const;

 private:
  void OnGetGattServices(
      Microsoft::WRL::ComPtr<
          ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
              IGattDeviceServicesResult> services_result);

  Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice>
      ble_device_;
  GattDiscoveryCallback callback_;
  GattServiceList gatt_services_;
  THREAD_CHECKER(thread_checker_);

  // Note: This should remain the last member so it'll be destroyed and
  // invalidate its weak pointers before any other members are destroyed.
  base::WeakPtrFactory<BluetoothGattDiscovererWinrt> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothGattDiscovererWinrt);
};

}  // namespace device

#endif  // DEVICE_BLUETOOTH_BLUETOOTH_GATT_DISCOVERER_WINRT_H_
