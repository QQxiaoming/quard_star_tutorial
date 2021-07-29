// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIA_EXT_FONTMGR_FUCHSIA_H_
#define SKIA_EXT_FONTMGR_FUCHSIA_H_

#include <memory>

#include <fuchsia/fonts/cpp/fidl.h>

#include "base/containers/flat_map.h"
#include "base/macros.h"
#include "third_party/skia/include/core/SkFontMgr.h"
#include "third_party/skia/include/core/SkStream.h"
#include "third_party/skia/include/core/SkTypeface.h"

namespace skia {

class SK_API FuchsiaFontManager : public SkFontMgr {
 public:
  explicit FuchsiaFontManager(
      fuchsia::fonts::FontProviderSyncPtr font_provider);

  ~FuchsiaFontManager() override;

 protected:
  // SkFontMgr overrides.
  int onCountFamilies() const override;
  void onGetFamilyName(int index, SkString* family_name) const override;
  SkFontStyleSet* onMatchFamily(const char family_name[]) const override;
  SkFontStyleSet* onCreateStyleSet(int index) const override;
  SkTypeface* onMatchFamilyStyle(const char family_name[],
                                 const SkFontStyle&) const override;
  SkTypeface* onMatchFamilyStyleCharacter(const char family_name[],
                                          const SkFontStyle&,
                                          const char* bcp47[],
                                          int bcp47_count,
                                          SkUnichar character) const override;
  SkTypeface* onMatchFaceStyle(const SkTypeface*,
                               const SkFontStyle&) const override;
  sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttc_index) const override;
  sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                          int ttc_index) const override;
  sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                         const SkFontArguments&) const override;
  sk_sp<SkTypeface> onMakeFromFile(const char path[],
                                   int ttc_index) const override;
  sk_sp<SkTypeface> onLegacyMakeTypeface(const char family_name[],
                                         SkFontStyle) const override;

 private:
  class FontCache;

  fuchsia::fonts::FontProviderSyncPtr font_provider_;

  // Map applied to font family name before sending requests to the FontService.
  base::flat_map<std::string, std::string> font_map_;

  // FontCache keeps all SkTypeface instances returned by the manager. It allows
  // to ensure that SkTypeface object is created only once for each typeface.
  std::unique_ptr<FontCache> font_cache_;

  sk_sp<SkTypeface> default_typeface_;

  DISALLOW_COPY_AND_ASSIGN(FuchsiaFontManager);
};

}  // namespace skia

#endif  // SKIA_EXT_FONTMGR_FUCHSIA_H_
