// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/frame/link_highlights.h"

#include <memory>

#include "cc/layers/picture_layer.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/platform/web_layer_tree_view.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/layout/layout_object.h"
#include "third_party/blink/renderer/core/page/chrome_client.h"
#include "third_party/blink/renderer/core/page/page.h"
#include "third_party/blink/renderer/core/paint/link_highlight_impl.h"
#include "third_party/blink/renderer/platform/animation/compositor_animation_host.h"
#include "third_party/blink/renderer/platform/animation/compositor_animation_timeline.h"

namespace blink {

LinkHighlights::LinkHighlights(Page& owner) : page_(&owner) {}

LinkHighlights::~LinkHighlights() {
  RemoveAllHighlights();
}

void LinkHighlights::Trace(blink::Visitor* visitor) {
  visitor->Trace(page_);
}

void LinkHighlights::RemoveAllHighlights() {
  if (timeline_) {
    for (auto& highlight : link_highlights_)
      timeline_->AnimationDestroyed(*highlight);
  }
  link_highlights_.clear();
}

void LinkHighlights::ResetForPageNavigation() {
  RemoveAllHighlights();
}

void LinkHighlights::SetTapHighlights(
    HeapVector<Member<Node>>& highlight_nodes) {
  // Always clear any existing highlight when this is invoked, even if we
  // don't get a new target to highlight.
  RemoveAllHighlights();

  for (size_t i = 0; i < highlight_nodes.size(); ++i) {
    Node* node = highlight_nodes[i];

    if (!node || !node->GetLayoutObject())
      continue;

    Color highlight_color =
        node->GetLayoutObject()->Style()->TapHighlightColor();
    // Safari documentation for -webkit-tap-highlight-color says if the
    // specified color has 0 alpha, then tap highlighting is disabled.
    // http://developer.apple.com/library/safari/#documentation/appleapplications/reference/safaricssref/articles/standardcssproperties.html
    if (!highlight_color.Alpha())
      continue;

    link_highlights_.push_back(LinkHighlightImpl::Create(node));
    if (timeline_)
      timeline_->AnimationAttached(*link_highlights_.back());
  }
}

void LinkHighlights::UpdateGeometry() {
  for (auto& highlight : link_highlights_)
    highlight->UpdateGeometry();
}

LocalFrame* LinkHighlights::MainFrame() const {
  return GetPage().MainFrame() && GetPage().MainFrame()->IsLocalFrame()
             ? GetPage().DeprecatedLocalMainFrame()
             : nullptr;
}

void LinkHighlights::StartHighlightAnimationIfNeeded() {
  for (auto& highlight : link_highlights_)
    highlight->StartHighlightAnimationIfNeeded();

  if (auto* local_frame = MainFrame())
    GetPage().GetChromeClient().ScheduleAnimation(local_frame->View());
}

void LinkHighlights::LayerTreeViewInitialized(
    WebLayerTreeView& layer_tree_view) {
  if (Platform::Current()->IsThreadedAnimationEnabled()) {
    timeline_ = CompositorAnimationTimeline::Create();
    animation_host_ = std::make_unique<CompositorAnimationHost>(
        layer_tree_view.CompositorAnimationHost());
    animation_host_->AddTimeline(*timeline_);
  }
}

void LinkHighlights::WillCloseLayerTreeView(WebLayerTreeView& layer_tree_view) {
  RemoveAllHighlights();
  if (timeline_) {
    animation_host_->RemoveTimeline(*timeline_);
    timeline_.reset();
  }
  animation_host_ = nullptr;
}

}  // namespace blink
