// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_gatt_discoverer_winrt.h"

#include <windows.foundation.collections.h>

#include <utility>

#include "base/logging.h"
#include "device/bluetooth/bluetooth_remote_gatt_service_winrt.h"
#include "device/bluetooth/event_utils_winrt.h"

namespace device {

namespace {

using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    GattCommunicationStatus;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    GattCommunicationStatus_Success;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    GattDeviceService;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    GattDeviceServicesResult;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    IGattDeviceService;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    IGattDeviceServicesResult;
using ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice;
using ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice3;
using ABI::Windows::Foundation::IAsyncOperation;
using ABI::Windows::Foundation::Collections::IVectorView;
using Microsoft::WRL::ComPtr;

}  // namespace

BluetoothGattDiscovererWinrt::BluetoothGattDiscovererWinrt(
    ComPtr<IBluetoothLEDevice> ble_device)
    : ble_device_(std::move(ble_device)), weak_ptr_factory_(this) {}

BluetoothGattDiscovererWinrt::~BluetoothGattDiscovererWinrt() = default;

void BluetoothGattDiscovererWinrt::StartGattDiscovery(
    GattDiscoveryCallback callback) {
  callback_ = std::move(callback);
  ComPtr<IBluetoothLEDevice3> ble_device_3;
  HRESULT hr = ble_device_.As(&ble_device_3);
  if (FAILED(hr)) {
    VLOG(2) << "Obtaining IBluetoothLEDevice3 failed: "
            << logging::SystemErrorCodeToString(hr);
    std::move(callback_).Run(false);
    return;
  }

  ComPtr<IAsyncOperation<GattDeviceServicesResult*>> get_gatt_services_op;
  hr = ble_device_3->GetGattServicesAsync(&get_gatt_services_op);
  if (FAILED(hr)) {
    VLOG(2) << "BluetoothLEDevice::GetGattServicesAsync failed: "
            << logging::SystemErrorCodeToString(hr);
    std::move(callback_).Run(false);
    return;
  }

  hr = PostAsyncResults(
      std::move(get_gatt_services_op),
      base::BindOnce(&BluetoothGattDiscovererWinrt::OnGetGattServices,
                     weak_ptr_factory_.GetWeakPtr()));

  if (FAILED(hr)) {
    VLOG(2) << "PostAsyncResults failed: "
            << logging::SystemErrorCodeToString(hr);
    std::move(callback_).Run(false);
  }
}

const BluetoothGattDiscovererWinrt::GattServiceList&
BluetoothGattDiscovererWinrt::GetGattServices() const {
  return gatt_services_;
}

void BluetoothGattDiscovererWinrt::OnGetGattServices(
    ComPtr<IGattDeviceServicesResult> services_result) {
  if (!services_result) {
    VLOG(2) << "Getting GATT Services failed.";
    std::move(callback_).Run(false);
    return;
  }

  GattCommunicationStatus status;
  HRESULT hr = services_result->get_Status(&status);
  if (FAILED(hr)) {
    VLOG(2) << "Getting GATT Communication Status failed: "
            << logging::SystemErrorCodeToString(hr);
    std::move(callback_).Run(false);
    return;
  }

  if (status != GattCommunicationStatus_Success) {
    VLOG(2) << "Unexpected GattCommunicationStatus: " << status;
    // TODO(https://crbug.com/821766): Obtain and log the protocol error if
    // appropriate. Mention BT spec Version 5.0 Vol 3, Part F, 3.4.1.1 "Error
    // Response".
    std::move(callback_).Run(false);
    return;
  }

  ComPtr<IVectorView<GattDeviceService*>> services;
  hr = services_result->get_Services(&services);
  if (FAILED(hr)) {
    VLOG(2) << "Getting GATT Services failed: "
            << logging::SystemErrorCodeToString(hr);
    std::move(callback_).Run(false);
    return;
  }

  unsigned size;
  hr = services->get_Size(&size);
  if (FAILED(hr)) {
    VLOG(2) << "Getting Size of GATT Services failed: "
            << logging::SystemErrorCodeToString(hr);
    std::move(callback_).Run(false);
    return;
  }

  for (unsigned i = 0; i < size; ++i) {
    ComPtr<IGattDeviceService> service;
    hr = services->GetAt(i, &service);
    if (FAILED(hr)) {
      VLOG(2) << "GetAt(" << i
              << ") failed: " << logging::SystemErrorCodeToString(hr);
      std::move(callback_).Run(false);
      return;
    }

    gatt_services_.push_back(std::move(service));
  }

  std::move(callback_).Run(true);
}

}  // namespace device
