// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/css/properties/longhands/text_indent.h"

#include "third_party/blink/renderer/core/css/css_value_list.h"
#include "third_party/blink/renderer/core/css/parser/css_parser_context.h"
#include "third_party/blink/renderer/core/css/parser/css_property_parser_helpers.h"
#include "third_party/blink/renderer/core/css/properties/computed_style_utils.h"
#include "third_party/blink/renderer/core/style/computed_style.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

namespace blink {
namespace CSSLonghand {

const CSSValue* TextIndent::ParseSingleValue(
    CSSParserTokenRange& range,
    const CSSParserContext& context,
    const CSSParserLocalContext&) const {
  // [ <length> | <percentage> ] && hanging? && each-line?
  // Keywords only allowed when css3Text is enabled.
  CSSValue* length_percentage = nullptr;
  CSSValue* hanging = nullptr;
  CSSValue* each_line = nullptr;
  do {
    if (!length_percentage) {
      length_percentage = CSSPropertyParserHelpers::ConsumeLengthOrPercent(
          range, context.Mode(), kValueRangeAll,
          CSSPropertyParserHelpers::UnitlessQuirk::kAllow);
      if (length_percentage) {
        continue;
      }
    }

    if (RuntimeEnabledFeatures::CSS3TextEnabled()) {
      CSSValueID id = range.Peek().Id();
      if (!hanging && id == CSSValueHanging) {
        hanging = CSSPropertyParserHelpers::ConsumeIdent(range);
        continue;
      }
      if (!each_line && id == CSSValueEachLine) {
        each_line = CSSPropertyParserHelpers::ConsumeIdent(range);
        continue;
      }
    }
    return nullptr;
  } while (!range.AtEnd());

  if (!length_percentage)
    return nullptr;
  CSSValueList* list = CSSValueList::CreateSpaceSeparated();
  list->Append(*length_percentage);

  if (hanging)
    list->Append(*hanging);

  if (each_line)
    list->Append(*each_line);

  return list;
}

const CSSValue* TextIndent::CSSValueFromComputedStyleInternal(
    const ComputedStyle& style,
    const SVGComputedStyle&,
    const LayoutObject*,
    Node* styled_node,
    bool allow_visited_style) const {
  CSSValueList* list = CSSValueList::CreateSpaceSeparated();
  list->Append(*ComputedStyleUtils::ZoomAdjustedPixelValueForLength(
      style.TextIndent(), style));
  if (RuntimeEnabledFeatures::CSS3TextEnabled()) {
    if (style.GetTextIndentType() == TextIndentType::kHanging)
      list->Append(*CSSIdentifierValue::Create(CSSValueHanging));
    if (style.GetTextIndentLine() == TextIndentLine::kEachLine)
      list->Append(*CSSIdentifierValue::Create(CSSValueEachLine));
  }
  return list;
}

void TextIndent::ApplyInitial(StyleResolverState& state) const {
  state.Style()->SetTextIndent(ComputedStyleInitialValues::InitialTextIndent());
  state.Style()->SetTextIndentLine(
      ComputedStyleInitialValues::InitialTextIndentLine());
  state.Style()->SetTextIndentType(
      ComputedStyleInitialValues::InitialTextIndentType());
}

void TextIndent::ApplyInherit(StyleResolverState& state) const {
  state.Style()->SetTextIndent(state.ParentStyle()->TextIndent());
  state.Style()->SetTextIndentLine(state.ParentStyle()->GetTextIndentLine());
  state.Style()->SetTextIndentType(state.ParentStyle()->GetTextIndentType());
}

void TextIndent::ApplyValue(StyleResolverState& state,
                            const CSSValue& value) const {
  Length length_or_percentage_value;
  TextIndentLine text_indent_line_value =
      ComputedStyleInitialValues::InitialTextIndentLine();
  TextIndentType text_indent_type_value =
      ComputedStyleInitialValues::InitialTextIndentType();

  for (auto& list_value : ToCSSValueList(value)) {
    if (list_value->IsPrimitiveValue()) {
      length_or_percentage_value =
          ToCSSPrimitiveValue(*list_value)
              .ConvertToLength(state.CssToLengthConversionData());
    } else if (ToCSSIdentifierValue(*list_value).GetValueID() ==
               CSSValueEachLine) {
      text_indent_line_value = TextIndentLine::kEachLine;
    } else if (ToCSSIdentifierValue(*list_value).GetValueID() ==
               CSSValueHanging) {
      text_indent_type_value = TextIndentType::kHanging;
    } else {
      NOTREACHED();
    }
  }

  state.Style()->SetTextIndent(length_or_percentage_value);
  state.Style()->SetTextIndentLine(text_indent_line_value);
  state.Style()->SetTextIndentType(text_indent_type_value);
}

}  // namespace CSSLonghand
}  // namespace blink
