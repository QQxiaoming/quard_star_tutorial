// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/authenticator_get_info_response.h"

#include <utility>

#include "components/cbor/cbor_values.h"
#include "components/cbor/cbor_writer.h"
#include "device/fido/fido_parsing_utils.h"

namespace device {

namespace {

template <typename Container>
cbor::CBORValue::ArrayValue ToArrayValue(const Container& container) {
  cbor::CBORValue::ArrayValue value;
  value.reserve(container.size());
  for (const auto& item : container)
    value.emplace_back(item);
  return value;
}

}  // namespace

AuthenticatorGetInfoResponse::AuthenticatorGetInfoResponse(
    base::flat_set<ProtocolVersion> versions,
    base::span<const uint8_t, kAaguidLength> aaguid)
    : versions_(std::move(versions)),
      aaguid_(fido_parsing_utils::Materialize(aaguid)) {}

AuthenticatorGetInfoResponse::AuthenticatorGetInfoResponse(
    AuthenticatorGetInfoResponse&& that) = default;

AuthenticatorGetInfoResponse& AuthenticatorGetInfoResponse::operator=(
    AuthenticatorGetInfoResponse&& other) = default;

AuthenticatorGetInfoResponse::~AuthenticatorGetInfoResponse() = default;

AuthenticatorGetInfoResponse& AuthenticatorGetInfoResponse::SetMaxMsgSize(
    uint32_t max_msg_size) {
  max_msg_size_ = max_msg_size;
  return *this;
}

AuthenticatorGetInfoResponse& AuthenticatorGetInfoResponse::SetPinProtocols(
    std::vector<uint8_t> pin_protocols) {
  pin_protocols_ = std::move(pin_protocols);
  return *this;
}

AuthenticatorGetInfoResponse& AuthenticatorGetInfoResponse::SetExtensions(
    std::vector<std::string> extensions) {
  extensions_ = std::move(extensions);
  return *this;
}

AuthenticatorGetInfoResponse& AuthenticatorGetInfoResponse::SetOptions(
    AuthenticatorSupportedOptions options) {
  options_ = std::move(options);
  return *this;
}

std::vector<uint8_t> EncodeToCBOR(
    const AuthenticatorGetInfoResponse& response) {
  cbor::CBORValue::ArrayValue version_array;
  for (const auto& version : response.versions()) {
    version_array.emplace_back(version == ProtocolVersion::kCtap ? kCtap2Version
                                                                 : kU2fVersion);
  }
  cbor::CBORValue::MapValue device_info_map;
  device_info_map.emplace(cbor::CBORValue(1), cbor::CBORValue(std::move(version_array)));

  if (response.extensions())
    device_info_map.emplace(cbor::CBORValue(2), cbor::CBORValue(ToArrayValue(*response.extensions())));

  device_info_map.emplace(cbor::CBORValue(3), cbor::CBORValue(response.aaguid()));
  device_info_map.emplace(cbor::CBORValue(4), cbor::CBORValue(ConvertToCBOR(response.options())));

  if (response.max_msg_size()) {
    device_info_map.emplace(cbor::CBORValue(5), cbor::CBORValue(base::strict_cast<int64_t>(*response.max_msg_size())));
  }

  if (response.pin_protocol()) {
    device_info_map.emplace(cbor::CBORValue(6), cbor::CBORValue(ToArrayValue(*response.pin_protocol())));
  }

  auto encoded_bytes =
      cbor::CBORWriter::Write(cbor::CBORValue(std::move(device_info_map)));
  DCHECK(encoded_bytes);
  return *encoded_bytes;
}

}  // namespace device
