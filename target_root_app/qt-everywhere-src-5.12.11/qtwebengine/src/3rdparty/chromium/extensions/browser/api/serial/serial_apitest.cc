// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <utility>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/api/serial/serial_api.h"
#include "extensions/browser/api/serial/serial_connection.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_registry.h"
#include "extensions/common/api/serial.h"
#include "extensions/common/switches.h"
#include "extensions/test/result_catcher.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "services/device/public/mojom/constants.mojom.h"
#include "services/device/public/mojom/serial.mojom.h"
#include "services/service_manager/public/cpp/service_context.h"
#include "testing/gmock/include/gmock/gmock.h"

// Disable SIMULATE_SERIAL_PORTS only if all the following are true:
//
// 1. You have an Arduino or compatible board attached to your machine and
// properly appearing as the first virtual serial port ("first" is very loosely
// defined as whichever port shows up in serial.getPorts). We've tested only
// the Atmega32u4 Breakout Board and Arduino Leonardo; note that both these
// boards are based on the Atmel ATmega32u4, rather than the more common
// Arduino '328p with either FTDI or '8/16u2 USB interfaces. TODO: test more
// widely.
//
// 2. Your user has permission to read/write the port. For example, this might
// mean that your user is in the "tty" or "uucp" group on Ubuntu flavors of
// Linux, or else that the port's path (e.g., /dev/ttyACM0) has global
// read/write permissions.
//
// 3. You have uploaded a program to the board that does a byte-for-byte echo
// on the virtual serial port at 57600 bps. An example is at
// chrome/test/data/extensions/api_test/serial/api/serial_arduino_test.ino.
//
#define SIMULATE_SERIAL_PORTS (1)

using testing::_;
using testing::Return;

namespace extensions {
namespace {

class FakeSerialDeviceEnumerator
    : public device::mojom::SerialDeviceEnumerator {
 public:
  FakeSerialDeviceEnumerator() = default;
  ~FakeSerialDeviceEnumerator() override = default;

 private:
  // device::mojom::SerialDeviceEnumerator methods:
  void GetDevices(GetDevicesCallback callback) override {
    std::vector<device::mojom::SerialDeviceInfoPtr> devices;
    auto device0 = device::mojom::SerialDeviceInfo::New();
    device0->path = "/dev/fakeserialmojo";
    auto device1 = device::mojom::SerialDeviceInfo::New();
    device1->path = "\\\\COM800\\";
    devices.push_back(std::move(device0));
    devices.push_back(std::move(device1));
    std::move(callback).Run(std::move(devices));
  }

  DISALLOW_COPY_AND_ASSIGN(FakeSerialDeviceEnumerator);
};

class FakeSerialIoHandler : public device::mojom::SerialIoHandler {
 public:
  FakeSerialIoHandler() {
    options_.bitrate = 9600;
    options_.data_bits = device::mojom::SerialDataBits::EIGHT;
    options_.parity_bit = device::mojom::SerialParityBit::NO_PARITY;
    options_.stop_bits = device::mojom::SerialStopBits::ONE;
    options_.cts_flow_control = false;
    options_.has_cts_flow_control = true;
  }
  ~FakeSerialIoHandler() override = default;

 private:
  // device::mojom::SerialIoHandler methods:
  void Open(const std::string& port,
            device::mojom::SerialConnectionOptionsPtr options,
            OpenCallback callback) override {
    DoConfigurePort(*options);
    std::move(callback).Run(true);
  }
  void Read(uint32_t bytes, ReadCallback callback) override {
    DCHECK(!pending_read_callback_);
    pending_read_callback_ = std::move(callback);
    pending_read_bytes_ = bytes;
    if (buffer_.empty())
      return;

    DoRead();
  }
  void Write(const std::vector<uint8_t>& data,
             WriteCallback callback) override {
    buffer_.insert(buffer_.end(), data.cbegin(), data.cend());
    std::move(callback).Run(data.size(), device::mojom::SerialSendError::NONE);
    DoRead();
  }
  void CancelRead(device::mojom::SerialReceiveError reason) override {
    if (pending_read_callback_) {
      std::move(pending_read_callback_).Run(std::vector<uint8_t>(), reason);
    }
  }
  void CancelWrite(device::mojom::SerialSendError reason) override {
  }
  void Flush(FlushCallback callback) override { std::move(callback).Run(true); }
  void GetControlSignals(GetControlSignalsCallback callback) override {
    auto signals = device::mojom::SerialDeviceControlSignals::New();
    signals->dcd = true;
    signals->cts = true;
    signals->ri = true;
    signals->dsr = true;
    std::move(callback).Run(std::move(signals));
  }
  void SetControlSignals(device::mojom::SerialHostControlSignalsPtr signals,
                         SetControlSignalsCallback callback) override {
    std::move(callback).Run(true);
  }
  void ConfigurePort(device::mojom::SerialConnectionOptionsPtr options,
                     ConfigurePortCallback callback) override {
    DoConfigurePort(*options);
    std::move(callback).Run(true);
  }
  void GetPortInfo(GetPortInfoCallback callback) override {
    auto info = device::mojom::SerialConnectionInfo::New();
    info->bitrate = options_.bitrate;
    info->data_bits = options_.data_bits;
    info->parity_bit = options_.parity_bit;
    info->stop_bits = options_.stop_bits;
    info->cts_flow_control = options_.cts_flow_control;
    std::move(callback).Run(std::move(info));
  }
  void SetBreak(SetBreakCallback callback) override {
    std::move(callback).Run(true);
  }
  void ClearBreak(ClearBreakCallback callback) override {
    std::move(callback).Run(true);
  }

  void DoRead() {
    if (!pending_read_callback_) {
      return;
    }
    size_t num_bytes =
        std::min(buffer_.size(), static_cast<size_t>(pending_read_bytes_));
    std::move(pending_read_callback_)
        .Run(std::vector<uint8_t>(buffer_.data(), buffer_.data() + num_bytes),
             device::mojom::SerialReceiveError::NONE);
    buffer_.erase(buffer_.begin(), buffer_.begin() + num_bytes);
    pending_read_bytes_ = 0;
  }

  void DoConfigurePort(const device::mojom::SerialConnectionOptions& options) {
    // Merge options.
    if (options.bitrate) {
      options_.bitrate = options.bitrate;
    }
    if (options.data_bits != device::mojom::SerialDataBits::NONE) {
      options_.data_bits = options.data_bits;
    }
    if (options.parity_bit != device::mojom::SerialParityBit::NONE) {
      options_.parity_bit = options.parity_bit;
    }
    if (options.stop_bits != device::mojom::SerialStopBits::NONE) {
      options_.stop_bits = options.stop_bits;
    }
    if (options.has_cts_flow_control) {
      DCHECK(options_.has_cts_flow_control);
      options_.cts_flow_control = options.cts_flow_control;
    }
  }

  // Currently applied connection options.
  device::mojom::SerialConnectionOptions options_;
  std::vector<uint8_t> buffer_;
  FakeSerialIoHandler::ReadCallback pending_read_callback_;
  uint32_t pending_read_bytes_ = 0;

  DISALLOW_COPY_AND_ASSIGN(FakeSerialIoHandler);
};

class SerialApiTest : public ExtensionApiTest {
 public:
  SerialApiTest() {
#if SIMULATE_SERIAL_PORTS
    // Because Device Service also runs in this process(browser process), we can
    // set our binder to intercept requests for
    // SerialDeviceEnumerator/SerialIoHandler interfaces to it.
    service_manager::ServiceContext::SetGlobalBinderForTesting(
        device::mojom::kServiceName,
        device::mojom::SerialDeviceEnumerator::Name_,
        base::BindRepeating(&SerialApiTest::BindSerialDeviceEnumerator,
                            base::Unretained(this)));
    service_manager::ServiceContext::SetGlobalBinderForTesting(
        device::mojom::kServiceName, device::mojom::SerialIoHandler::Name_,
        base::BindRepeating(&SerialApiTest::BindSerialIoHandler));
#endif
  }

  ~SerialApiTest() override {
#if SIMULATE_SERIAL_PORTS
    service_manager::ServiceContext::ClearGlobalBindersForTesting(
        device::mojom::kServiceName);
#endif
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    ExtensionApiTest::SetUpCommandLine(command_line);
  }

  void SetUpOnMainThread() override { ExtensionApiTest::SetUpOnMainThread(); }

  void TearDownOnMainThread() override {
    ExtensionApiTest::TearDownOnMainThread();
  }

  void FailEnumeratorRequest() { fail_enumerator_request_ = true; }

 protected:
  void BindSerialDeviceEnumerator(
      const std::string& interface_name,
      mojo::ScopedMessagePipeHandle handle,
      const service_manager::BindSourceInfo& source_info) {
    if (fail_enumerator_request_)
      return;

    mojo::MakeStrongBinding(
        std::make_unique<FakeSerialDeviceEnumerator>(),
        device::mojom::SerialDeviceEnumeratorRequest(std::move(handle)));
  }

  static void BindSerialIoHandler(
      const std::string& interface_name,
      mojo::ScopedMessagePipeHandle handle,
      const service_manager::BindSourceInfo& source_info) {
    mojo::MakeStrongBinding(
        std::make_unique<FakeSerialIoHandler>(),
        device::mojom::SerialIoHandlerRequest(std::move(handle)));
  }

  bool fail_enumerator_request_ = false;
};

}  // namespace

IN_PROC_BROWSER_TEST_F(SerialApiTest, SerialFakeHardware) {
  ResultCatcher catcher;
  catcher.RestrictToBrowserContext(browser()->profile());

  ASSERT_TRUE(RunExtensionTest("serial/api")) << message_;
}

IN_PROC_BROWSER_TEST_F(SerialApiTest, SerialRealHardware) {
  ResultCatcher catcher;
  catcher.RestrictToBrowserContext(browser()->profile());

  ASSERT_TRUE(RunExtensionTest("serial/real_hardware")) << message_;
}

IN_PROC_BROWSER_TEST_F(SerialApiTest, SerialRealHardwareFail) {
  ResultCatcher catcher;
  catcher.RestrictToBrowserContext(browser()->profile());

  // chrome.serial.getDevices() should get an empty list when the serial
  // enumerator interface is unavailable.
  FailEnumeratorRequest();
  ASSERT_TRUE(RunExtensionTest("serial/real_hardware_fail")) << message_;
}

}  // namespace extensions
