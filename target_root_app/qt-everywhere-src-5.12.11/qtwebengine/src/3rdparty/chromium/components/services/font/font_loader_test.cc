// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/services/font/font_loader_test.h"

#include <utility>

#include "base/run_loop.h"
#include "components/services/font/public/interfaces/constants.mojom.h"
#include "components/services/font/public/interfaces/font_service.mojom.h"
#include "ppapi/buildflags/buildflags.h"
#include "services/service_manager/public/cpp/connector.h"
#include "third_party/skia/include/core/SkFontStyle.h"

#if BUILDFLAG(ENABLE_PLUGINS)
#include <ft2build.h>
#include <freetype/freetype.h>
#include "ppapi/c/private/pp_private_font_charset.h"  // nogncheck
#endif

namespace {
bool IsInTestFontDirectory(const char* path) {
  const char kTestFontsDir[] = "test_fonts";
  return std::string(path).find(kTestFontsDir) != std::string::npos;
}

#if BUILDFLAG(ENABLE_PLUGINS)
std::string GetPostscriptNameFromFile(base::File& font_file) {
  int64_t file_size = font_file.GetLength();
  if (!file_size)
    return "";

  std::vector<char> file_contents;
  file_contents.reserve(file_size);
  CHECK_EQ(file_size, font_file.Read(0, file_contents.data(), file_size));
  std::string font_family_name;
  FT_Library library;
  FT_Init_FreeType(&library);
  FT_Face font_face;
  FT_Open_Args open_args = {FT_OPEN_MEMORY,
                            reinterpret_cast<FT_Byte*>(file_contents.data()),
                            file_size};
  CHECK_EQ(FT_Err_Ok, FT_Open_Face(library, &open_args, 0, &font_face));
  font_family_name = FT_Get_Postscript_Name(font_face);
  FT_Done_Face(font_face);
  FT_Done_FreeType(library);
  return font_family_name;
}
#endif

}  // namespace

namespace font_service {

FontLoaderTest::FontLoaderTest() : ServiceTest("font_service_unittests") {}

FontLoaderTest::~FontLoaderTest() {}

void FontLoaderTest::SetUp() {
  ServiceTest::SetUp();
  font_loader_ = std::make_unique<FontLoader>(connector());
}

TEST_F(FontLoaderTest, BasicMatchingTest) {
  SkFontStyle styles[] = {
      SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                  SkFontStyle::kUpright_Slant),
      SkFontStyle(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width,
                  SkFontStyle::kUpright_Slant),
      SkFontStyle(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width,
                  SkFontStyle::kItalic_Slant)};
  // See kFontsConfTemplate[] in fontconfig_util_linux.cc for details of which
  // fonts can be picked. Arial, Times New Roman and Courier New are aliased to
  // Arimos, Tinos and Cousine ChromeOS open source alternatives when FontConfig
  // is set up for testing.
  std::vector<std::vector<std::string>> family_names_expected_names = {
      {"Arial", "Arimo"},
      {"Times New Roman", "Tinos"},
      {"Courier New", "Cousine"}};
  for (std::vector<std::string> request_family_name :
       family_names_expected_names) {
    for (auto& request_style : styles) {
      SkFontConfigInterface::FontIdentity font_identity;
      SkFontStyle result_style;
      SkString result_family_name;
      font_loader()->matchFamilyName(request_family_name[0].c_str(),
                                     request_style, &font_identity,
                                     &result_family_name, &result_style);
      EXPECT_EQ(request_family_name[1],
                std::string(result_family_name.c_str()));
      EXPECT_TRUE(IsInTestFontDirectory(font_identity.fString.c_str()));
      EXPECT_EQ(result_style, request_style);
    }
  }
}

TEST_F(FontLoaderTest, NotFoundTest) {
  std::string request_family_name = {"IMPROBABLE_FONT_NAME"};
  SkFontConfigInterface::FontIdentity font_identity;
  SkFontStyle result_style;
  SkString result_family_name;
  font_loader()->matchFamilyName(request_family_name.c_str(), SkFontStyle(),
                                 &font_identity, &result_family_name,
                                 &result_style);
  EXPECT_EQ("", std::string(result_family_name.c_str()));
  EXPECT_EQ(0u, font_identity.fID);
  EXPECT_EQ("", std::string(font_identity.fString.c_str()));
}

TEST_F(FontLoaderTest, EmptyFontName) {
  std::string request_family_name = {""};
  std::string kDefaultFontName = "DejaVu Sans";
  SkFontConfigInterface::FontIdentity font_identity;
  SkFontStyle result_style;
  SkString result_family_name;
  font_loader()->matchFamilyName(request_family_name.c_str(), SkFontStyle(),
                                 &font_identity, &result_family_name,
                                 &result_style);
  EXPECT_EQ(kDefaultFontName, std::string(result_family_name.c_str()));
  EXPECT_TRUE(IsInTestFontDirectory(font_identity.fString.c_str()));
}

TEST_F(FontLoaderTest, CharacterFallback) {
  std::pair<uint32_t, std::string> fallback_characters_families[] = {
      // A selection of character hitting fonts from the third_party/test_fonts
      // portfolio.
      {0x662F /* CJK UNIFIED IDEOGRAPH-662F */, "Noto Sans CJK JP"},
      {0x1780 /* KHMER LETTER KA */, "Noto Sans Khmer"},
      {0xA05 /* GURMUKHI LETTER A */, "Lohit Gurmukhi"},
      {0xB85 /* TAMIL LETTER A */, "Lohit Tamil"},
      {0x904 /* DEVANAGARI LETTER SHORT A */, "Lohit Devanagari"},
      {0x985 /* BENGALI LETTER A */, "Mukti Narrow"},
      // Tests for not finding fallback:
      {0x13170 /* EGYPTIAN HIEROGLYPH G042 */, ""},
      {0x1817 /* MONGOLIAN DIGIT SEVEN */, ""}};
  for (auto& character_family : fallback_characters_families) {
    mojom::FontIdentityPtr font_identity;
    std::string result_family_name;
    bool is_bold;
    bool is_italic;
    font_loader()->FallbackFontForCharacter(
        std::move(character_family.first), "en_US", &font_identity,
        &result_family_name, &is_bold, &is_italic);
    EXPECT_EQ(result_family_name, character_family.second);
    EXPECT_FALSE(is_bold);
    EXPECT_FALSE(is_italic);
    if (character_family.second.size()) {
      EXPECT_TRUE(
          IsInTestFontDirectory(font_identity->str_representation.c_str()));
    } else {
      EXPECT_EQ(font_identity->str_representation.size(), 0u);
      EXPECT_EQ(result_family_name, "");
    }
  }
}

TEST_F(FontLoaderTest, RenderStyleForStrike) {
  // Use FontConfig configured test font aliases from kFontsConfTemplate in
  // fontconfig_util_linux.cc.
  std::pair<std::string, mojom::FontRenderStylePtr> families_styles[] = {
      {"NonAntiAliasedSans",
       mojom::FontRenderStyle::New(
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::OFF,
           mojom::RenderStyleSwitch::ON, 3, mojom::RenderStyleSwitch::OFF,
           mojom::RenderStyleSwitch::OFF, mojom::RenderStyleSwitch::OFF)},
      {"SlightHintedGeorgia",
       mojom::FontRenderStyle::New(
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::OFF,
           mojom::RenderStyleSwitch::ON, 1, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::OFF, mojom::RenderStyleSwitch::OFF)},
      {"NonHintedSans",
       mojom::FontRenderStyle::New(
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::OFF,
           mojom::RenderStyleSwitch::OFF, 0, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::OFF, mojom::RenderStyleSwitch::OFF)},
      {"AutohintedSerif",
       mojom::FontRenderStyle::New(
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::ON, 2, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::OFF, mojom::RenderStyleSwitch::OFF)},
      {"HintedSerif",
       mojom::FontRenderStyle::New(
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::OFF,
           mojom::RenderStyleSwitch::ON, 2, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::OFF, mojom::RenderStyleSwitch::OFF)},
      {"FullAndAutoHintedSerif",
       mojom::FontRenderStyle::New(
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::ON, 3, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::OFF, mojom::RenderStyleSwitch::OFF)},
      {"SubpixelEnabledArial",
       mojom::FontRenderStyle::New(
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::OFF,
           mojom::RenderStyleSwitch::ON, 3, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::OFF)},
      {"SubpixelDisabledArial",
       mojom::FontRenderStyle::New(
           mojom::RenderStyleSwitch::ON, mojom::RenderStyleSwitch::OFF,
           mojom::RenderStyleSwitch::ON, 3, mojom::RenderStyleSwitch::ON,
           mojom::RenderStyleSwitch::OFF, mojom::RenderStyleSwitch::OFF)}};

  for (auto& family_style : families_styles) {
    mojom::FontRenderStylePtr result_style;
    font_loader()->FontRenderStyleForStrike(std::move(family_style.first), 16,
                                            false, false, 1.0, &result_style);
    EXPECT_TRUE(result_style.Equals(family_style.second));
  }
}

TEST_F(FontLoaderTest, PPAPIFallback) {
#if BUILDFLAG(ENABLE_PLUGINS)
  std::tuple<std::string, uint32_t, std::string> family_charset_expectations[] =
      {
          {"", PP_PRIVATEFONTCHARSET_SHIFTJIS, "DejaVuSans"},
          {"", PP_PRIVATEFONTCHARSET_THAI, "Garuda"},
          {"", PP_PRIVATEFONTCHARSET_GB2312, "DejaVuSans"},
          {"", PP_PRIVATEFONTCHARSET_GREEK, "DejaVuSans"},
          {"Arial", PP_PRIVATEFONTCHARSET_DEFAULT, "Arimo-Regular"},
          {"Times New Roman", PP_PRIVATEFONTCHARSET_DEFAULT, "Tinos-Regular"},
          {"Courier New", PP_PRIVATEFONTCHARSET_DEFAULT, "Cousine-Regular"},
      };
  for (auto& family_charset_expectation : family_charset_expectations) {
    base::File result_file;
    font_loader()->MatchFontWithFallback(
        std::move(std::get<0>(family_charset_expectation)), false, false,
        std::move(std::get<1>(family_charset_expectation)), 0, &result_file);
    EXPECT_TRUE(result_file.IsValid());
    EXPECT_EQ(GetPostscriptNameFromFile(result_file),
              std::get<2>(family_charset_expectation));
  }
#endif
}

}  // namespace font_service
