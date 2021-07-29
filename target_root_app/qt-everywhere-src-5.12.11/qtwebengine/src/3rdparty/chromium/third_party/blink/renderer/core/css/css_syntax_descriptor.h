// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_CSS_CSS_SYNTAX_DESCRIPTOR_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_CSS_CSS_SYNTAX_DESCRIPTOR_H_

#include "third_party/blink/renderer/core/css/parser/css_parser_token_range.h"

namespace blink {

class CSSParserContext;
class CSSValue;

enum class CSSSyntaxType {
  kTokenStream,
  kIdent,
  kLength,
  kNumber,
  kPercentage,
  kLengthPercentage,
  kColor,
  kImage,
  kUrl,
  kInteger,
  kAngle,
  kTime,
  kResolution,
  kTransformFunction,
  kTransformList,
  kCustomIdent,
};

enum class CSSSyntaxRepeat { kNone, kSpaceSeparated, kCommaSeparated };

struct CSSSyntaxComponent {
  CSSSyntaxComponent(CSSSyntaxType type,
                     const String& string,
                     CSSSyntaxRepeat repeat)
      : type_(type), string_(string), repeat_(repeat) {}

  bool operator==(const CSSSyntaxComponent& a) const {
    return type_ == a.type_ && string_ == a.string_ && repeat_ == a.repeat_;
  }

  bool IsRepeatable() const { return repeat_ != CSSSyntaxRepeat::kNone; }

  CSSSyntaxType type_;
  String string_;  // Only used when type_ is CSSSyntaxType::kIdent
  CSSSyntaxRepeat repeat_;
};

class CORE_EXPORT CSSSyntaxDescriptor {
 public:
  explicit CSSSyntaxDescriptor(const String& syntax);

  const CSSValue* Parse(CSSParserTokenRange,
                        const CSSParserContext*,
                        bool is_animation_tainted) const;
  bool IsValid() const { return !syntax_components_.IsEmpty(); }
  bool IsTokenStream() const {
    return syntax_components_.size() == 1 &&
           syntax_components_[0].type_ == CSSSyntaxType::kTokenStream;
  }
  bool HasUrlSyntax() const {
    for (const CSSSyntaxComponent& component : syntax_components_) {
      if (component.type_ == CSSSyntaxType::kUrl)
        return true;
    }
    return false;
  }
  const Vector<CSSSyntaxComponent>& Components() const {
    return syntax_components_;
  }
  bool operator==(const CSSSyntaxDescriptor& a) const {
    return Components() == a.Components();
  }
  bool operator!=(const CSSSyntaxDescriptor& a) const {
    return Components() != a.Components();
  }

 private:
  Vector<CSSSyntaxComponent> syntax_components_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_CSS_CSS_SYNTAX_DESCRIPTOR_H_
