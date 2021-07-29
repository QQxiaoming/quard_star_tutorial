// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/core/mach_port_relay.h"

#include <mach/mach.h>

#include <utility>

#include "base/logging.h"
#include "base/mac/mach_port_util.h"
#include "base/mac/scoped_mach_port.h"
#include "base/metrics/histogram_macros.h"
#include "base/process/process.h"

namespace mojo {
namespace core {

namespace {

// Errors that can occur in the broker (privileged parent) process.
// These match tools/metrics/histograms.xml.
// This enum is append-only.
enum class BrokerUMAError : int {
  SUCCESS = 0,
  // Couldn't get a task port for the process with a given pid.
  ERROR_TASK_FOR_PID = 1,
  // Couldn't make a port with receive rights in the destination process.
  ERROR_MAKE_RECEIVE_PORT = 2,
  // Couldn't change the attributes of a Mach port.
  ERROR_SET_ATTRIBUTES = 3,
  // Couldn't extract a right from the destination.
  ERROR_EXTRACT_DEST_RIGHT = 4,
  // Couldn't send a Mach port in a call to mach_msg().
  ERROR_SEND_MACH_PORT = 5,
  // Couldn't extract a right from the source.
  ERROR_EXTRACT_SOURCE_RIGHT = 6,
  ERROR_MAX
};

// Errors that can occur in a child process.
// These match tools/metrics/histograms.xml.
// This enum is append-only.
enum class ChildUMAError : int {
  SUCCESS = 0,
  // An error occurred while trying to receive a Mach port with mach_msg().
  ERROR_RECEIVE_MACH_MESSAGE = 1,
  ERROR_MAX
};

void ReportBrokerError(BrokerUMAError error) {
  UMA_HISTOGRAM_ENUMERATION("Mojo.MachPortRelay.BrokerError",
                            static_cast<int>(error),
                            static_cast<int>(BrokerUMAError::ERROR_MAX));
}

void ReportChildError(ChildUMAError error) {
  UMA_HISTOGRAM_ENUMERATION("Mojo.MachPortRelay.ChildError",
                            static_cast<int>(error),
                            static_cast<int>(ChildUMAError::ERROR_MAX));
}

}  // namespace

// static
base::mac::ScopedMachSendRight MachPortRelay::ReceiveSendRight(
    base::mac::ScopedMachReceiveRight port) {
  // MACH_PORT_NULL doesn't need translation.
  if (!port.is_valid())
    return base::mac::ScopedMachSendRight();

  // Take ownership of the receive right. We only need it to read this single
  // send right, then it can be closed.
  base::mac::ScopedMachSendRight received_port(
      base::ReceiveMachPort(port.get()));
  if (!received_port.is_valid()) {
    ReportChildError(ChildUMAError::ERROR_RECEIVE_MACH_MESSAGE);
    DLOG(ERROR) << "Error receiving mach port";
    return base::mac::ScopedMachSendRight();
  }

  ReportChildError(ChildUMAError::SUCCESS);
  return received_port;
}

MachPortRelay::MachPortRelay(base::PortProvider* port_provider)
    : port_provider_(port_provider) {
  DCHECK(port_provider);
  port_provider_->AddObserver(this);
}

MachPortRelay::~MachPortRelay() {
  port_provider_->RemoveObserver(this);
}

void MachPortRelay::SendPortsToProcess(Channel::Message* message,
                                       base::ProcessHandle process) {
  DCHECK(message);
  mach_port_t task_port = port_provider_->TaskForPid(process);

  std::vector<PlatformHandleInTransit> handles = message->TakeHandles();
  // Message should have handles, otherwise there's no point in calling this
  // function.
  DCHECK(!handles.empty());
  for (auto& handle : handles) {
    if (!handle.handle().is_valid_mach_port())
      continue;

    if (task_port == MACH_PORT_NULL) {
      // Callers check the port provider for the task port before calling this
      // function, in order to queue pending messages. Therefore, if this fails,
      // it should be considered a genuine, bona fide, electrified, six-car
      // error.
      ReportBrokerError(BrokerUMAError::ERROR_TASK_FOR_PID);
      handle = PlatformHandleInTransit(
          PlatformHandle(base::mac::ScopedMachSendRight()));
      continue;
    }

    mach_port_name_t intermediate_port;
    base::MachCreateError error_code;
    intermediate_port = base::CreateIntermediateMachPort(
        task_port, handle.TakeHandle().TakeMachPort(), &error_code);
    if (intermediate_port == MACH_PORT_NULL) {
      BrokerUMAError uma_error;
      switch (error_code) {
        case base::MachCreateError::ERROR_MAKE_RECEIVE_PORT:
          uma_error = BrokerUMAError::ERROR_MAKE_RECEIVE_PORT;
          break;
        case base::MachCreateError::ERROR_SET_ATTRIBUTES:
          uma_error = BrokerUMAError::ERROR_SET_ATTRIBUTES;
          break;
        case base::MachCreateError::ERROR_EXTRACT_DEST_RIGHT:
          uma_error = BrokerUMAError::ERROR_EXTRACT_DEST_RIGHT;
          break;
        case base::MachCreateError::ERROR_SEND_MACH_PORT:
          uma_error = BrokerUMAError::ERROR_SEND_MACH_PORT;
          break;
      }
      ReportBrokerError(uma_error);
      handle = PlatformHandleInTransit(
          PlatformHandle(base::mac::ScopedMachSendRight()));
      continue;
    }

    handle = PlatformHandleInTransit::CreateForMachPortName(intermediate_port);
    ReportBrokerError(BrokerUMAError::SUCCESS);
  }
  message->SetHandles(std::move(handles));
}

base::mac::ScopedMachSendRight MachPortRelay::ExtractPort(
    mach_port_t port_name,
    base::ProcessHandle process) {
  // No extraction necessary for MACH_PORT_NULL.
  if (port_name == MACH_PORT_NULL)
    return base::mac::ScopedMachSendRight();

  mach_port_t task_port = port_provider_->TaskForPid(process);
  if (task_port == MACH_PORT_NULL) {
    ReportBrokerError(BrokerUMAError::ERROR_TASK_FOR_PID);
    return base::mac::ScopedMachSendRight();
  }

  mach_port_t extracted_right = MACH_PORT_NULL;
  mach_msg_type_name_t extracted_right_type;
  kern_return_t kr =
      mach_port_extract_right(task_port, port_name, MACH_MSG_TYPE_MOVE_SEND,
                              &extracted_right, &extracted_right_type);
  if (kr != KERN_SUCCESS) {
    ReportBrokerError(BrokerUMAError::ERROR_EXTRACT_SOURCE_RIGHT);
    return base::mac::ScopedMachSendRight();
  }

  ReportBrokerError(BrokerUMAError::SUCCESS);
  DCHECK_EQ(static_cast<mach_msg_type_name_t>(MACH_MSG_TYPE_PORT_SEND),
            extracted_right_type);
  return base::mac::ScopedMachSendRight(extracted_right);
}

void MachPortRelay::AddObserver(Observer* observer) {
  base::AutoLock locker(observers_lock_);
  bool inserted = observers_.insert(observer).second;
  DCHECK(inserted);
}

void MachPortRelay::RemoveObserver(Observer* observer) {
  base::AutoLock locker(observers_lock_);
  observers_.erase(observer);
}

void MachPortRelay::OnReceivedTaskPort(base::ProcessHandle process) {
  base::AutoLock locker(observers_lock_);
  for (auto* observer : observers_)
    observer->OnProcessReady(process);
}

}  // namespace core
}  // namespace mojo
