// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_remote_gatt_service_winrt.h"

#include <windows.foundation.collections.h>

#include <utility>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/strings/stringprintf.h"
#include "base/win/scoped_hstring.h"
#include "device/bluetooth/bluetooth_device.h"

namespace device {

namespace {

using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    GattDeviceService;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    IGattDeviceService;
using ABI::Windows::Foundation::Collections::IVectorView;
using Microsoft::WRL::ComPtr;

}  // namespace

// static
std::unique_ptr<BluetoothRemoteGattServiceWinrt>
BluetoothRemoteGattServiceWinrt::Create(
    BluetoothDevice* device,
    ComPtr<IGattDeviceService> gatt_service) {
  DCHECK(gatt_service);
  GUID guid;
  HRESULT hr = gatt_service->get_Uuid(&guid);
  if (FAILED(hr)) {
    VLOG(2) << "Getting UUID failed: " << logging::SystemErrorCodeToString(hr);
    return nullptr;
  }

  uint16_t attribute_handle;
  hr = gatt_service->get_AttributeHandle(&attribute_handle);
  if (FAILED(hr)) {
    VLOG(2) << "Getting AttributeHandle failed: "
            << logging::SystemErrorCodeToString(hr);
    return nullptr;
  }

  return base::WrapUnique(new BluetoothRemoteGattServiceWinrt(
      device, std::move(gatt_service), BluetoothUUID(guid), attribute_handle));
}

BluetoothRemoteGattServiceWinrt::~BluetoothRemoteGattServiceWinrt() = default;

std::string BluetoothRemoteGattServiceWinrt::GetIdentifier() const {
  return identifier_;
}

BluetoothUUID BluetoothRemoteGattServiceWinrt::GetUUID() const {
  return uuid_;
}

bool BluetoothRemoteGattServiceWinrt::IsPrimary() const {
  return true;
}

BluetoothDevice* BluetoothRemoteGattServiceWinrt::GetDevice() const {
  return device_;
}

std::vector<BluetoothRemoteGattService*>
BluetoothRemoteGattServiceWinrt::GetIncludedServices() const {
  NOTIMPLEMENTED();
  return {};
}

BluetoothRemoteGattServiceWinrt::BluetoothRemoteGattServiceWinrt(
    BluetoothDevice* device,
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::
                               GenericAttributeProfile::IGattDeviceService>
        gatt_service,
    BluetoothUUID uuid,
    uint16_t attribute_handle)
    : device_(device),
      gatt_service_(std::move(gatt_service)),
      uuid_(std::move(uuid)),
      attribute_handle_(attribute_handle),
      identifier_(base::StringPrintf("%s/%s_%04x",
                                     device_->GetIdentifier().c_str(),
                                     uuid_.value().c_str(),
                                     attribute_handle_)) {}

}  // namespace device
