// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_MUS_MUS_CLIENT_TEST_API_H_
#define UI_VIEWS_MUS_MUS_CLIENT_TEST_API_H_

#include <memory>
#include <utility>

#include "base/macros.h"
#include "ui/views/mus/mus_client.h"

namespace views {

class AXRemoteHost;

class MusClientTestApi {
 public:
  static void SetAXRemoteHost(std::unique_ptr<AXRemoteHost> client) {
    MusClient::Get()->ax_remote_host_ = std::move(client);
  }

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(MusClientTestApi);
};

}  // namespace views

#endif  // UI_VIEWS_MUS_MUS_CLIENT_TEST_API_H_
