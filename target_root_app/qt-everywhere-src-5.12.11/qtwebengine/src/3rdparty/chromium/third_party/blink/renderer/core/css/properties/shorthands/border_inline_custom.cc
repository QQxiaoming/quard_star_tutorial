// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/css/properties/shorthands/border_inline.h"

#include "third_party/blink/renderer/core/css/parser/css_property_parser_helpers.h"
#include "third_party/blink/renderer/core/style_property_shorthand.h"

namespace blink {
namespace CSSShorthand {

bool BorderInline::ParseShorthand(
    bool important,
    CSSParserTokenRange& range,
    const CSSParserContext& context,
    const CSSParserLocalContext&,
    HeapVector<CSSPropertyValue, 256>& properties) const {
  const CSSValue* width = nullptr;
  const CSSValue* style = nullptr;
  const CSSValue* color = nullptr;

  if (!CSSPropertyParserHelpers::ConsumeBorderShorthand(range, context, width,
                                                        style, color)) {
    return false;
  };

  CSSPropertyParserHelpers::AddExpandedPropertyForValue(
      CSSPropertyBorderInlineWidth, *width, important, properties);
  CSSPropertyParserHelpers::AddExpandedPropertyForValue(
      CSSPropertyBorderInlineStyle, *style, important, properties);
  CSSPropertyParserHelpers::AddExpandedPropertyForValue(
      CSSPropertyBorderInlineColor, *color, important, properties);

  return range.AtEnd();
}

}  // namespace CSSShorthand
}  // namespace blink
