// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/services/font/font_service_app.h"

#include <utility>

#include "base/command_line.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "build/build_config.h"
#include "mojo/public/cpp/system/platform_handle.h"
#include "ppapi/buildflags/buildflags.h"
#include "services/service_manager/public/cpp/service_context.h"
#include "ui/gfx/font_fallback_linux.h"
#include "ui/gfx/font_render_params.h"

#if BUILDFLAG(ENABLE_PLUGINS)
#include "components/services/font/ppapi_fontconfig_matching.h"  // nogncheck
#endif

#if defined(OS_LINUX)
#include "base/test/fontconfig_util_linux.h"
#endif

static_assert(
    static_cast<uint32_t>(SkFontStyle::kUpright_Slant) ==
        static_cast<uint32_t>(font_service::mojom::TypefaceSlant::ROMAN),
    "Skia and font service flags must match");
static_assert(
    static_cast<uint32_t>(SkFontStyle::kItalic_Slant) ==
        static_cast<uint32_t>(font_service::mojom::TypefaceSlant::ITALIC),
    "Skia and font service flags must match");
static_assert(
    static_cast<uint32_t>(SkFontStyle::kOblique_Slant) ==
        static_cast<uint32_t>(font_service::mojom::TypefaceSlant::OBLIQUE),
    "Skia and font service flags must match");

namespace {

base::File GetFileForPath(const base::FilePath& path) {
  if (path.empty())
    return base::File();

  base::File file(path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  LOG_IF(WARNING, !file.IsValid()) << "file not valid, path=" << path.value();
  return file;
}

int ConvertHinting(gfx::FontRenderParams::Hinting hinting) {
  switch (hinting) {
    case gfx::FontRenderParams::HINTING_NONE:
      return 0;
    case gfx::FontRenderParams::HINTING_SLIGHT:
      return 1;
    case gfx::FontRenderParams::HINTING_MEDIUM:
      return 2;
    case gfx::FontRenderParams::HINTING_FULL:
      return 3;
  }
  NOTREACHED() << "Unexpected hinting value " << hinting;
  return 0;
}

font_service::mojom::RenderStyleSwitch ConvertSubpixelRendering(
    gfx::FontRenderParams::SubpixelRendering rendering) {
  switch (rendering) {
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_NONE:
      return font_service::mojom::RenderStyleSwitch::OFF;
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_RGB:
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_BGR:
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_VRGB:
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_VBGR:
      return font_service::mojom::RenderStyleSwitch::ON;
  }
  NOTREACHED() << "Unexpected subpixel rendering value " << rendering;
  return font_service::mojom::RenderStyleSwitch::NO_PREFERENCE;
}

}  // namespace

namespace font_service {

std::unique_ptr<service_manager::Service> FontServiceApp::CreateService() {
  return std::make_unique<FontServiceApp>();
}

FontServiceApp::FontServiceApp() {
  registry_.AddInterface(
      base::BindRepeating(&FontServiceApp::CreateSelf, base::Unretained(this)));
}

FontServiceApp::~FontServiceApp() {}

void FontServiceApp::OnStart() {}

void FontServiceApp::OnBindInterface(
    const service_manager::BindSourceInfo& source_info,
    const std::string& interface_name,
    mojo::ScopedMessagePipeHandle interface_pipe) {
  registry_.BindInterface(interface_name, std::move(interface_pipe));
}

void FontServiceApp::MatchFamilyName(const std::string& family_name,
                                     mojom::TypefaceStylePtr requested_style,
                                     MatchFamilyNameCallback callback) {
  SkFontConfigInterface::FontIdentity result_identity;
  SkString result_family;
  SkFontStyle result_style;
  SkFontConfigInterface* fc =
      SkFontConfigInterface::GetSingletonDirectInterface();
  const bool r = fc->matchFamilyName(
      family_name.data(),
      SkFontStyle(requested_style->weight, requested_style->width,
                  static_cast<SkFontStyle::Slant>(requested_style->slant)),
      &result_identity, &result_family, &result_style);

  if (!r) {
    mojom::TypefaceStylePtr style(mojom::TypefaceStyle::New());
    style->weight = SkFontStyle().weight();
    style->width = SkFontStyle().width();
    style->slant = static_cast<mojom::TypefaceSlant>(SkFontStyle().slant());
    std::move(callback).Run(nullptr, "", std::move(style));
    return;
  }

  // Stash away the returned path, so we can give it an ID (index)
  // which will later be given to us in a request to open the file.
  int index = FindOrAddPath(result_identity.fString);

  mojom::FontIdentityPtr identity(mojom::FontIdentity::New());
  identity->id = static_cast<uint32_t>(index);
  identity->ttc_index = result_identity.fTTCIndex;
  identity->str_representation = result_identity.fString.c_str();

  mojom::TypefaceStylePtr style(mojom::TypefaceStyle::New());
  style->weight = result_style.weight();
  style->width = result_style.width();
  style->slant = static_cast<mojom::TypefaceSlant>(result_style.slant());

  std::move(callback).Run(std::move(identity), result_family.c_str(),
                          std::move(style));
}

void FontServiceApp::OpenStream(uint32_t id_number,
                                OpenStreamCallback callback) {
  DCHECK_LT(id_number, static_cast<uint32_t>(paths_.size()));
  base::File file;
  if (id_number < static_cast<uint32_t>(paths_.size())) {
    file = GetFileForPath(base::FilePath(paths_[id_number].c_str()));
  }

  std::move(callback).Run(std::move(file));
}

void FontServiceApp::FallbackFontForCharacter(
    uint32_t character,
    const std::string& locale,
    FallbackFontForCharacterCallback callback) {
  auto fallback_font = gfx::GetFallbackFontForChar(character, locale);
  int index = FindOrAddPath(SkString(fallback_font.filename.data()));

  mojom::FontIdentityPtr identity(mojom::FontIdentity::New());
  identity->id = static_cast<uint32_t>(index);
  identity->ttc_index = fallback_font.ttc_index;
  identity->str_representation = fallback_font.filename;

  std::move(callback).Run(std::move(identity), fallback_font.name,
                          fallback_font.is_bold, fallback_font.is_italic);
}

void FontServiceApp::FontRenderStyleForStrike(
    const std::string& family,
    uint32_t size,
    bool is_bold,
    bool is_italic,
    float device_scale_factor,
    FontRenderStyleForStrikeCallback callback) {
  gfx::FontRenderParamsQuery query;

  query.device_scale_factor = device_scale_factor;
  query.families.push_back(family);
  query.pixel_size = size;
  query.style = is_italic ? gfx::Font::ITALIC : 0;
  query.weight = is_bold ? gfx::Font::Weight::BOLD : gfx::Font::Weight::NORMAL;
  const gfx::FontRenderParams params = gfx::GetFontRenderParams(query, nullptr);

  mojom::FontRenderStylePtr font_render_style(mojom::FontRenderStyle::New(
      params.use_bitmaps ? mojom::RenderStyleSwitch::ON
                         : mojom::RenderStyleSwitch::OFF,
      params.autohinter ? mojom::RenderStyleSwitch::ON
                        : mojom::RenderStyleSwitch::OFF,
      params.hinting != gfx::FontRenderParams::HINTING_NONE
          ? mojom::RenderStyleSwitch::ON
          : mojom::RenderStyleSwitch::OFF,
      ConvertHinting(params.hinting),
      params.antialiasing ? mojom::RenderStyleSwitch::ON
                          : mojom::RenderStyleSwitch::OFF,
      ConvertSubpixelRendering(params.subpixel_rendering),
      params.subpixel_positioning ? mojom::RenderStyleSwitch::ON
                                  : mojom::RenderStyleSwitch::OFF));
  std::move(callback).Run(std::move(font_render_style));
}

void FontServiceApp::MatchFontWithFallback(
    const std::string& family,
    bool is_bold,
    bool is_italic,
    uint32_t charset,
    uint32_t fallbackFamilyType,
    MatchFontWithFallbackCallback callback) {
#if BUILDFLAG(ENABLE_PLUGINS)
  base::File matched_font_file;
  int font_file_descriptor = MatchFontFaceWithFallback(
      family, is_bold, is_italic, charset, fallbackFamilyType);
  matched_font_file = base::File(font_file_descriptor);
  if (!matched_font_file.IsValid())
    matched_font_file = base::File();
  std::move(callback).Run(std::move(matched_font_file));
#else
  NOTREACHED();
#endif
}

void FontServiceApp::CreateSelf(mojom::FontServiceRequest request) {
  bindings_.AddBinding(this, std::move(request));
}

int FontServiceApp::FindOrAddPath(const SkString& path) {
  int count = paths_.size();
  for (int i = 0; i < count; ++i) {
    if (path == paths_[i])
      return i;
  }
  paths_.emplace_back(path);
  return count;
}

}  // namespace font_service
