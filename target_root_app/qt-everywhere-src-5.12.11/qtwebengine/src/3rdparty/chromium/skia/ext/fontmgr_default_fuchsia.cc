// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skia/ext/fontmgr_default_fuchsia.h"

#include <fuchsia/fonts/cpp/fidl.h>

#include "base/fuchsia/component_context.h"
#include "skia/ext/fontmgr_fuchsia.h"

namespace {
// This is a purposefully leaky pointer that has ownership of the FontMgr.
SkFontMgr* g_default_fontmgr;
}  // namespace

void SetDefaultSkiaFactory(sk_sp<SkFontMgr> fontmgr) {
  SkASSERT(g_default_fontmgr == nullptr);
  g_default_fontmgr = fontmgr.release();
}

SK_API sk_sp<SkFontMgr> SkFontMgr::Factory() {
  if (g_default_fontmgr) {
    return sk_ref_sp(g_default_fontmgr);
  }
  return sk_make_sp<skia::FuchsiaFontManager>(
      base::fuchsia::ComponentContext::GetDefault()
          ->ConnectToServiceSync<fuchsia::fonts::FontProvider>());
}
