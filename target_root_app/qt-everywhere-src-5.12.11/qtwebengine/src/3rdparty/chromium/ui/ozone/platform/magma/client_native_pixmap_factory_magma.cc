// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/platform/magma/client_native_pixmap_factory_magma.h"

#include "ui/ozone/common/stub_client_native_pixmap_factory.h"

namespace ui {

gfx::ClientNativePixmapFactory* CreateClientNativePixmapFactoryMagma() {
  return CreateStubClientNativePixmapFactory();
}

}  // namespace ui
