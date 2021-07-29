// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/accessibility/testing/accessibility_selection_test.h"

#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/html/html_element.h"
#include "third_party/blink/renderer/modules/accessibility/ax_object.h"
#include "third_party/blink/renderer/modules/accessibility/ax_object_cache_impl.h"
#include "third_party/blink/renderer/modules/accessibility/ax_position.h"
#include "third_party/blink/renderer/modules/accessibility/ax_selection.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/wtf/text/string_builder.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

namespace {

// Serialize accessibility subtree to selection text.
// Adds a '^' at the selection anchor offset and a '|' at the focus offset.
class AXSelectionSerializer final {
  STACK_ALLOCATED();

 public:
  explicit AXSelectionSerializer(const AXSelection& selection)
      : selection_(selection) {}

  std::string Serialize(const AXObject& subtree) {
    if (!selection_.IsValid())
      return {};
    SerializeSubtree(subtree);
    return builder_.ToString().Utf8().data();
  }

 private:
  void HandleTextObject(const AXObject& text_object) {
    builder_.Append('<');
    builder_.Append(AXObject::InternalRoleName(text_object.RoleValue()));
    builder_.Append(": ");
    const String name = text_object.ComputedName() + '>';
    const AXObject& base_container = *selection_.Base().ContainerObject();
    const AXObject& extent_container = *selection_.Extent().ContainerObject();

    if (base_container == text_object && extent_container == text_object) {
      DCHECK(selection_.Base().IsTextPosition() &&
             selection_.Extent().IsTextPosition());
      const int base_offset = selection_.Base().TextOffset();
      const int extent_offset = selection_.Extent().TextOffset();

      if (base_offset == extent_offset) {
        builder_.Append(name.Left(base_offset));
        builder_.Append('|');
        builder_.Append(name.Substring(base_offset));
        return;
      }

      if (base_offset < extent_offset) {
        builder_.Append(name.Left(base_offset));
        builder_.Append('^');
        builder_.Append(
            name.Substring(base_offset, extent_offset - base_offset));
        builder_.Append('|');
        builder_.Append(name.Substring(extent_offset));
        return;
      }

      builder_.Append(name.Left(extent_offset));
      builder_.Append('|');
      builder_.Append(
          name.Substring(extent_offset, base_offset - extent_offset));
      builder_.Append('^');
      builder_.Append(name.Substring(base_offset));
      return;
    }

    if (base_container == text_object) {
      DCHECK(selection_.Base().IsTextPosition());
      const int base_offset = selection_.Base().TextOffset();

      builder_.Append(name.Left(base_offset));
      builder_.Append('^');
      builder_.Append(name.Substring(base_offset));
      return;
    }

    if (extent_container == text_object) {
      DCHECK(selection_.Extent().IsTextPosition());
      const int extent_offset = selection_.Extent().TextOffset();

      builder_.Append(name.Left(extent_offset));
      builder_.Append('|');
      builder_.Append(name.Substring(extent_offset));
      return;
    }

    builder_.Append(name);
  }

  void HandleObject(const AXObject& object) {
    builder_.Append('<');
    builder_.Append(AXObject::InternalRoleName(object.RoleValue()));
    builder_.Append(": ");
    builder_.Append(object.ComputedName());
    builder_.Append('>');
    SerializeSubtree(object);
  }

  void HandleSelection(const AXPosition& position) {
    if (!position.IsValid())
      return;

    if (selection_.Extent() == position) {
      builder_.Append('|');
      return;
    }

    if (selection_.Base() != position)
      return;

    builder_.Append('^');
  }

  void SerializeSubtree(const AXObject& subtree) {
    for (const Member<AXObject>& child : subtree.Children()) {
      if (!child)
        continue;
      const auto position = AXPosition::CreatePositionBeforeObject(*child);
      HandleSelection(position);
      if (position.IsTextPosition()) {
        HandleTextObject(*child);
      } else {
        HandleObject(*child);
      }
    }
    HandleSelection(AXPosition::CreateLastPositionInObject(subtree));
  }

  StringBuilder builder_;
  AXSelection selection_;
};

// Deserializes an HTML snippet with or without selection markers to an
// accessibility tree. A '^' could be present at the selection anchor offset and
// a '|' at the focus offset. If multiple markers are present, the deserializer
// will DCHECK. If there are no markers, no selection will be made.
class AXSelectionDeserializer final {
  STACK_ALLOCATED();

 public:
  AXSelectionDeserializer(AXObjectCacheImpl& cache)
      : ax_object_cache_(&cache), base_(), extent_() {}
  ~AXSelectionDeserializer() = default;

  // Creates an accessibility tree rooted at the given HTML element from the
  // provided HTML snippet, selects the part of the tree indicated by the
  // selection markers in the snippet if present, and returns the tree's root.
  AXObject* Deserialize(const std::string& html_snippet, HTMLElement& element) {
    element.SetInnerHTMLFromString(String::FromUTF8(html_snippet.c_str()));
    AXObject* root = ax_object_cache_->GetOrCreate(&element);
    if (!root)
      return nullptr;

    FindSelectionMarkers(*root);
    if (base_ && extent_) {
      AXSelection::Builder builder;
      AXSelection ax_selection =
          builder.SetBase(base_).SetExtent(extent_).Build();
      ax_selection.Select();
    }

    return root;
  }

 private:
  void HandleCharacterData(const AXObject& text_object) {
    int base_offset = -1;
    int extent_offset = -1;
    String name = text_object.ComputedName();
    for (unsigned i = 0; i < name.length(); ++i) {
      const UChar character = name[i];
      if (character == '^') {
        DCHECK_EQ(base_offset, -1) << text_object;
        base_offset = static_cast<int>(i);
        continue;
      }
      if (character == '|') {
        DCHECK_EQ(extent_offset, -1) << text_object;
        extent_offset = static_cast<int>(i);
        continue;
      }
    }

    if (base_offset == -1 && extent_offset == -1)
      return;

    if (base_offset >= 0)
      base_ = AXPosition::CreatePositionInTextObject(text_object, base_offset);
    if (extent_offset >= 0) {
      extent_ =
          AXPosition::CreatePositionInTextObject(text_object, extent_offset);
    }
  }

  void HandleObject(const AXObject& object) {
    for (const Member<AXObject>& child : object.Children()) {
      if (!child)
        continue;
      FindSelectionMarkers(*child);
    }
  }

  void FindSelectionMarkers(const AXObject& root) {
    const Node* node = root.GetNode();
    if (node && node->IsCharacterDataNode()) {
      HandleCharacterData(root);
      return;
    }
    HandleObject(root);
  }

  Member<AXObjectCacheImpl> const ax_object_cache_;
  AXPosition base_;
  AXPosition extent_;
};

}  // namespace

AccessibilitySelectionTest::AccessibilitySelectionTest(
    LocalFrameClient* local_frame_client)
    : AccessibilityTest(local_frame_client) {}

std::string AccessibilitySelectionTest::GetSelectionText(
    const AXSelection& selection) const {
  const AXObject* root = GetAXRootObject();
  if (!root)
    return {};
  return AXSelectionSerializer(selection).Serialize(*root);
}

std::string AccessibilitySelectionTest::GetSelectionText(
    const AXSelection& selection,
    const AXObject& subtree) const {
  return AXSelectionSerializer(selection).Serialize(subtree);
}

AXObject* AccessibilitySelectionTest::SetSelectionText(
    const std::string& selection_text) const {
  HTMLElement* body = GetDocument().body();
  if (!body)
    return nullptr;
  return AXSelectionDeserializer(GetAXObjectCache())
      .Deserialize(selection_text, *body);
}

AXObject* AccessibilitySelectionTest::SetSelectionText(
    const std::string& selection_text,
    HTMLElement& element) const {
  return AXSelectionDeserializer(GetAXObjectCache())
      .Deserialize(selection_text, element);
}

}  // namespace blink
