// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_CORE_MACH_PORT_RELAY_H_
#define MOJO_CORE_MACH_PORT_RELAY_H_

#include <set>
#include <vector>

#include "base/macros.h"
#include "base/process/port_provider_mac.h"
#include "base/synchronization/lock.h"
#include "mojo/core/channel.h"

namespace mojo {
namespace core {

// The MachPortRelay is used by a privileged process, usually the root process,
// to manipulate Mach ports in a child process. Ports can be added to and
// extracted from a child process that has registered itself with the
// |base::PortProvider| used by this class.
class MachPortRelay : public base::PortProvider::Observer {
 public:
  class Observer {
   public:
    // Called by the MachPortRelay to notify observers that a new process is
    // ready for Mach ports to be sent/received. There are no guarantees about
    // the thread this is called on, including the presence of a MessageLoop.
    // Implementations must not call AddObserver() or RemoveObserver() during
    // this function, as doing so will deadlock.
    virtual void OnProcessReady(base::ProcessHandle process) = 0;
  };

  // Used by a child process to receive Mach ports from a sender (privileged)
  // process. The Mach port in |port| is interpreted as an intermediate Mach
  // port. It replaces each Mach port with the final Mach port received from the
  // intermediate port. This method takes ownership of the intermediate Mach
  // port and gives ownership of the final Mach port to the caller.
  //
  // On failure, returns a null send right.
  //
  // See SendPortsToProcess() for the definition of intermediate and final Mach
  // ports.
  static base::mac::ScopedMachSendRight ReceiveSendRight(
      base::mac::ScopedMachReceiveRight port);

  explicit MachPortRelay(base::PortProvider* port_provider);
  ~MachPortRelay() override;

  // Sends the Mach ports attached to |message| to |process|.
  // For each Mach port attached to |message|, a new Mach port, the intermediate
  // port, is created in |process|. The message's Mach port is then sent over
  // this intermediate port and the message is modified to refer to the name of
  // the intermediate port. The Mach port received over the intermediate port in
  // the child is referred to as the final Mach port.
  //
  // All ports in |message|'s set of handles are reset by this call, and all
  // port names in the message's header are replaced with the new receive right
  // ports.
  void SendPortsToProcess(Channel::Message* message,
                          base::ProcessHandle process);

  // Given the name of a Mach send right within |process|, extracts an owned
  // send right ref and returns it. May return a null port on failure.
  base::mac::ScopedMachSendRight ExtractPort(mach_port_t port_name,
                                             base::ProcessHandle process);

  // Observer interface.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  base::PortProvider* port_provider() const { return port_provider_; }

 private:
  // base::PortProvider::Observer implementation.
  void OnReceivedTaskPort(base::ProcessHandle process) override;

  base::PortProvider* const port_provider_;

  base::Lock observers_lock_;
  std::set<Observer*> observers_;

  DISALLOW_COPY_AND_ASSIGN(MachPortRelay);
};

}  // namespace core
}  // namespace mojo

#endif  // MOJO_CORE_MACH_PORT_RELAY_H_
