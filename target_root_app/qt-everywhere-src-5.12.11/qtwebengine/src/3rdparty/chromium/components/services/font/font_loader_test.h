// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SERVICES_FONT_FONT_LOADER_TEST_H_
#define COMPONENTS_SERVICES_FONT_FONT_LOADER_TEST_H_

#include <utility>

#include "base/bind.h"
#include "base/macros.h"
#include "components/services/font/public/cpp/font_loader.h"
#include "components/services/font/public/interfaces/font_service.mojom.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "services/service_manager/public/cpp/service_test.h"

namespace font_service {

class FontLoaderTest : public service_manager::test::ServiceTest {
 public:
  FontLoaderTest();
  ~FontLoaderTest() override;

  // Overridden from service_manager::test::ServiceTest:
  void SetUp() override;

 protected:
  FontLoader* font_loader() { return font_loader_.get(); }

 private:
  std::unique_ptr<FontLoader> font_loader_;

  DISALLOW_COPY_AND_ASSIGN(FontLoaderTest);
};

}  // namespace font_service

#endif  // COMPONENTS_SERVICES_FONT_FONT_LOADER_TEST_H_
