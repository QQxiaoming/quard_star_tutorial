// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_device_winrt.h"

#include <windows.foundation.h>

#include <utility>

#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/strings/stringprintf.h"
#include "base/win/core_winrt_util.h"
#include "base/win/scoped_hstring.h"
#include "device/bluetooth/bluetooth_adapter_winrt.h"
#include "device/bluetooth/bluetooth_gatt_discoverer_winrt.h"
#include "device/bluetooth/bluetooth_remote_gatt_service_winrt.h"
#include "device/bluetooth/event_utils_winrt.h"

namespace device {

namespace {

using ABI::Windows::Devices::Bluetooth::BluetoothConnectionStatus;
using ABI::Windows::Devices::Bluetooth::BluetoothConnectionStatus_Connected;
using ABI::Windows::Devices::Bluetooth::BluetoothLEDevice;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    GattCommunicationStatus;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    GattCommunicationStatus_Success;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    GattDeviceServicesResult;
using ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::
    IGattDeviceServicesResult;
using ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice;
using ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice3;
using ABI::Windows::Devices::Bluetooth::IBluetoothLEDeviceStatics;
using ABI::Windows::Foundation::IAsyncOperation;
using ABI::Windows::Foundation::IClosable;
using Microsoft::WRL::ComPtr;

void CloseDevice(ComPtr<IBluetoothLEDevice> ble_device) {
  if (!ble_device)
    return;

  ComPtr<IClosable> closable;
  HRESULT hr = ble_device.As(&closable);
  if (FAILED(hr)) {
    VLOG(2) << "As IClosable failed: " << logging::SystemErrorCodeToString(hr);
    return;
  }

  hr = closable->Close();
  if (FAILED(hr)) {
    VLOG(2) << "IClosable::close() failed: "
            << logging::SystemErrorCodeToString(hr);
  }
}

void RemoveConnectionStatusHandler(IBluetoothLEDevice* ble_device,
                                   EventRegistrationToken token) {
  HRESULT hr = ble_device->remove_ConnectionStatusChanged(token);
  if (FAILED(hr)) {
    VLOG(2) << "Removing ConnectionStatus Handler failed: "
            << logging::SystemErrorCodeToString(hr);
  }
}

void RemoveGattServicesChangedHandler(IBluetoothLEDevice* ble_device,
                                      EventRegistrationToken token) {
  HRESULT hr = ble_device->remove_GattServicesChanged(token);
  if (FAILED(hr)) {
    VLOG(2) << "Removing Gatt Services Changed Handler failed: "
            << logging::SystemErrorCodeToString(hr);
  }
}

}  // namespace

BluetoothDeviceWinrt::BluetoothDeviceWinrt(
    BluetoothAdapterWinrt* adapter,
    uint64_t raw_address,
    base::Optional<std::string> local_name)
    : BluetoothDevice(adapter),
      raw_address_(raw_address),
      address_(CanonicalizeAddress(raw_address)),
      local_name_(std::move(local_name)),
      weak_ptr_factory_(this) {}

BluetoothDeviceWinrt::~BluetoothDeviceWinrt() {
  CloseDevice(ble_device_);
  if (!connection_changed_token_)
    return;

  RemoveConnectionStatusHandler(ble_device_.Get(), *connection_changed_token_);
}

uint32_t BluetoothDeviceWinrt::GetBluetoothClass() const {
  NOTIMPLEMENTED();
  return 0;
}

std::string BluetoothDeviceWinrt::GetAddress() const {
  return address_;
}

BluetoothDevice::VendorIDSource BluetoothDeviceWinrt::GetVendorIDSource()
    const {
  NOTIMPLEMENTED();
  return VendorIDSource();
}

uint16_t BluetoothDeviceWinrt::GetVendorID() const {
  NOTIMPLEMENTED();
  return 0;
}

uint16_t BluetoothDeviceWinrt::GetProductID() const {
  NOTIMPLEMENTED();
  return 0;
}

uint16_t BluetoothDeviceWinrt::GetDeviceID() const {
  NOTIMPLEMENTED();
  return 0;
}

uint16_t BluetoothDeviceWinrt::GetAppearance() const {
  NOTIMPLEMENTED();
  return 0;
}

base::Optional<std::string> BluetoothDeviceWinrt::GetName() const {
  if (!ble_device_)
    return local_name_;

  HSTRING name;
  HRESULT hr = ble_device_->get_Name(&name);
  if (FAILED(hr)) {
    VLOG(2) << "Getting Name failed: " << logging::SystemErrorCodeToString(hr);
    return local_name_;
  }

  return base::win::ScopedHString(name).GetAsUTF8();
}

bool BluetoothDeviceWinrt::IsPaired() const {
  NOTIMPLEMENTED();
  return false;
}

bool BluetoothDeviceWinrt::IsConnected() const {
  return IsGattConnected();
}

bool BluetoothDeviceWinrt::IsGattConnected() const {
  if (!ble_device_)
    return false;

  BluetoothConnectionStatus status;
  HRESULT hr = ble_device_->get_ConnectionStatus(&status);
  if (FAILED(hr)) {
    VLOG(2) << "Getting ConnectionStatus failed: "
            << logging::SystemErrorCodeToString(hr);
    return false;
  }

  return status == BluetoothConnectionStatus_Connected;
}

bool BluetoothDeviceWinrt::IsConnectable() const {
  NOTIMPLEMENTED();
  return false;
}

bool BluetoothDeviceWinrt::IsConnecting() const {
  NOTIMPLEMENTED();
  return false;
}

bool BluetoothDeviceWinrt::ExpectingPinCode() const {
  NOTIMPLEMENTED();
  return false;
}

bool BluetoothDeviceWinrt::ExpectingPasskey() const {
  NOTIMPLEMENTED();
  return false;
}

bool BluetoothDeviceWinrt::ExpectingConfirmation() const {
  NOTIMPLEMENTED();
  return false;
}

void BluetoothDeviceWinrt::GetConnectionInfo(
    const ConnectionInfoCallback& callback) {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::SetConnectionLatency(
    ConnectionLatency connection_latency,
    const base::Closure& callback,
    const ErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::Connect(PairingDelegate* pairing_delegate,
                                   const base::Closure& callback,
                                   const ConnectErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::SetPinCode(const std::string& pincode) {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::SetPasskey(uint32_t passkey) {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::ConfirmPairing() {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::RejectPairing() {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::CancelPairing() {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::Disconnect(const base::Closure& callback,
                                      const ErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::Forget(const base::Closure& callback,
                                  const ErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::ConnectToService(
    const BluetoothUUID& uuid,
    const ConnectToServiceCallback& callback,
    const ConnectToServiceErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

void BluetoothDeviceWinrt::ConnectToServiceInsecurely(
    const device::BluetoothUUID& uuid,
    const ConnectToServiceCallback& callback,
    const ConnectToServiceErrorCallback& error_callback) {
  NOTIMPLEMENTED();
}

// static
std::string BluetoothDeviceWinrt::CanonicalizeAddress(uint64_t address) {
  std::string bluetooth_address = BluetoothDevice::CanonicalizeAddress(
      base::StringPrintf("%012llX", address));
  DCHECK(!bluetooth_address.empty());
  return bluetooth_address;
}

void BluetoothDeviceWinrt::CreateGattConnectionImpl() {
  ComPtr<IBluetoothLEDeviceStatics> device_statics;
  HRESULT hr = GetBluetoothLEDeviceStaticsActivationFactory(&device_statics);
  if (FAILED(hr)) {
    VLOG(2) << "GetBluetoothLEDeviceStaticsActivationFactory failed: "
            << logging::SystemErrorCodeToString(hr);
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::BindOnce(&BluetoothDeviceWinrt::DidFailToConnectGatt,
                                  weak_ptr_factory_.GetWeakPtr(),
                                  ConnectErrorCode::ERROR_FAILED));
    return;
  }

  // Note: Even though we might have obtained a BluetoothLEDevice instance in
  // the past, we need to request a new instance as the old device might have
  // been closed. See also
  // https://docs.microsoft.com/en-us/windows/uwp/devices-sensors/gatt-client#connecting-to-the-device
  ComPtr<IAsyncOperation<BluetoothLEDevice*>> from_bluetooth_address_op;
  hr = device_statics->FromBluetoothAddressAsync(raw_address_,
                                                 &from_bluetooth_address_op);
  if (FAILED(hr)) {
    VLOG(2) << "BluetoothLEDevice::FromBluetoothAddressAsync failed: "
            << logging::SystemErrorCodeToString(hr);
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::BindOnce(&BluetoothDeviceWinrt::DidFailToConnectGatt,
                                  weak_ptr_factory_.GetWeakPtr(),
                                  ConnectErrorCode::ERROR_FAILED));
    return;
  }

  hr = PostAsyncResults(
      std::move(from_bluetooth_address_op),
      base::BindOnce(&BluetoothDeviceWinrt::OnFromBluetoothAddress,
                     weak_ptr_factory_.GetWeakPtr()));

  if (FAILED(hr)) {
    VLOG(2) << "PostAsyncResults failed: "
            << logging::SystemErrorCodeToString(hr);
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::BindOnce(&BluetoothDeviceWinrt::DidFailToConnectGatt,
                                  weak_ptr_factory_.GetWeakPtr(),
                                  ConnectErrorCode::ERROR_FAILED));
  }
}

void BluetoothDeviceWinrt::DisconnectGatt() {
  CloseDevice(ble_device_);
}

HRESULT BluetoothDeviceWinrt::GetBluetoothLEDeviceStaticsActivationFactory(
    IBluetoothLEDeviceStatics** statics) const {
  return base::win::GetActivationFactory<
      IBluetoothLEDeviceStatics,
      RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice>(statics);
}

void BluetoothDeviceWinrt::OnFromBluetoothAddress(
    ComPtr<IBluetoothLEDevice> ble_device) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  if (!ble_device) {
    VLOG(2) << "Getting Device From Bluetooth Address failed.";
    DidFailToConnectGatt(ConnectErrorCode::ERROR_FAILED);
    return;
  }

  if (connection_changed_token_) {
    // As we are about to replace |ble_device_| with |ble_device| we will also
    // unregister the existing event handler and add a new one to the new
    // device.
    RemoveConnectionStatusHandler(ble_device_.Get(),
                                  *connection_changed_token_);
  }

  ble_device_ = std::move(ble_device);
  connection_changed_token_ = AddTypedEventHandler(
      ble_device_.Get(), &IBluetoothLEDevice::add_ConnectionStatusChanged,
      base::BindRepeating(&BluetoothDeviceWinrt::OnConnectionStatusChanged,
                          weak_ptr_factory_.GetWeakPtr()));

  if (gatt_services_changed_token_) {
    RemoveGattServicesChangedHandler(ble_device_.Get(),
                                     *gatt_services_changed_token_);
  }

  gatt_services_changed_token_ = AddTypedEventHandler(
      ble_device_.Get(), &IBluetoothLEDevice::add_GattServicesChanged,
      base::BindRepeating(&BluetoothDeviceWinrt::OnGattServicesChanged,
                          weak_ptr_factory_.GetWeakPtr()));

  gatt_discoverer_ =
      std::make_unique<BluetoothGattDiscovererWinrt>(ble_device_);
  // Initiating the GATT discovery will result in a GATT connection attempt as
  // well and triggers OnConnectionStatusChanged on success.
  gatt_discoverer_->StartGattDiscovery(
      base::BindOnce(&BluetoothDeviceWinrt::OnGattDiscoveryComplete,
                     weak_ptr_factory_.GetWeakPtr()));
}

void BluetoothDeviceWinrt::OnConnectionStatusChanged(
    IBluetoothLEDevice* ble_device,
    IInspectable* object) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  if (IsGattConnected()) {
    DidConnectGatt();
  } else {
    gatt_discoverer_.reset();
    gatt_services_.clear();
    device_uuids_.ClearServiceUUIDs();
    SetGattServicesDiscoveryComplete(false);
    DidDisconnectGatt();
  }
}

void BluetoothDeviceWinrt::OnGattServicesChanged(IBluetoothLEDevice* ble_device,
                                                 IInspectable* object) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  device_uuids_.ClearServiceUUIDs();
  SetGattServicesDiscoveryComplete(false);
  adapter_->NotifyDeviceChanged(this);
  if (IsGattConnected()) {
    // In order to stop a potential ongoing GATT discovery, the GattDiscoverer
    // is reset and a new discovery is initiated.
    gatt_discoverer_ =
        std::make_unique<BluetoothGattDiscovererWinrt>(ble_device);
    gatt_discoverer_->StartGattDiscovery(
        base::BindOnce(&BluetoothDeviceWinrt::OnGattDiscoveryComplete,
                       weak_ptr_factory_.GetWeakPtr()));
  }
}

void BluetoothDeviceWinrt::OnGattDiscoveryComplete(bool success) {
  if (!success) {
    if (!IsGattConnected())
      DidFailToConnectGatt(ConnectErrorCode::ERROR_FAILED);
    return;
  }

  for (const auto& gatt_service : gatt_discoverer_->GetGattServices()) {
    auto gatt_service_winrt =
        BluetoothRemoteGattServiceWinrt::Create(this, gatt_service);
    if (!gatt_service_winrt)
      continue;

    const auto& service = *gatt_service_winrt;
    gatt_services_.emplace(service.GetIdentifier(),
                           std::move(gatt_service_winrt));
  }

  device_uuids_.ReplaceServiceUUIDs(gatt_services_);
  SetGattServicesDiscoveryComplete(true);
  adapter_->NotifyGattServicesDiscovered(this);
  adapter_->NotifyDeviceChanged(this);
  gatt_discoverer_.reset();
}

}  // namespace device
