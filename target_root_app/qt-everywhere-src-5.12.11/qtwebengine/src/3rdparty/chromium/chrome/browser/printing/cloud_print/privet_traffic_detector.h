// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PRINTING_CLOUD_PRINT_PRIVET_TRAFFIC_DETECTOR_H_
#define CHROME_BROWSER_PRINTING_CLOUD_PRINT_PRIVET_TRAFFIC_DETECTOR_H_

#include <memory>

#include "base/callback.h"
#include "base/cancelable_callback.h"
#include "base/macros.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/network_connection_tracker.h"
#include "net/base/address_family.h"
#include "net/base/ip_endpoint.h"

namespace net {
class DatagramServerSocket;
class IOBufferWithSize;
}

namespace cloud_print {

// Detects mDns traffic that looks like "Privet" protocol.
// Can produce false positives results, but main task of the class is to avoid
// running full mDns listener if user doesn't have devices.
// When traffic is detected, class fires callback and shutdowns itself.
class PrivetTrafficDetector
    : public base::RefCountedThreadSafe<
          PrivetTrafficDetector,
          content::BrowserThread::DeleteOnIOThread>,
      private content::NetworkConnectionTracker::NetworkConnectionObserver {
 public:
  PrivetTrafficDetector(net::AddressFamily address_family,
                        const base::Closure& on_traffic_detected);

  void Start();
  void Stop();

 private:
  friend struct content::BrowserThread::DeleteOnThread<
      content::BrowserThread::IO>;
  friend class base::DeleteHelper<PrivetTrafficDetector>;
  ~PrivetTrafficDetector() override;

  // content::NetworkConnectionTracker::NetworkConnectionObserver:
  void OnConnectionChanged(network::mojom::ConnectionType type) override;

  void HandleConnectionChanged(network::mojom::ConnectionType type);
  void ScheduleRestart();
  void Restart(const net::NetworkInterfaceList& networks);
  int Bind();
  bool IsSourceAcceptable() const;
  bool IsPrivetPacket(int rv) const;
  int DoLoop(int rv);

  base::Closure on_traffic_detected_;
  scoped_refptr<base::TaskRunner> callback_runner_;
  net::NetworkInterfaceList networks_;
  net::AddressFamily address_family_;
  net::IPEndPoint recv_addr_;
  std::unique_ptr<net::DatagramServerSocket> socket_;
  scoped_refptr<net::IOBufferWithSize> io_buffer_;
  base::Time start_time_;
  int restart_attempts_;

  base::WeakPtrFactory<PrivetTrafficDetector> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(PrivetTrafficDetector);
};

}  // namespace cloud_print

#endif  // CHROME_BROWSER_PRINTING_CLOUD_PRINT_PRIVET_TRAFFIC_DETECTOR_H_
