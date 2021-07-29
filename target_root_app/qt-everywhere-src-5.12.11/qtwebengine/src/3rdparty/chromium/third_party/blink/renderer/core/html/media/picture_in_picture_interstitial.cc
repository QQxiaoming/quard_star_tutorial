// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/html/media/picture_in_picture_interstitial.h"

#include "cc/layers/layer.h"
#include "third_party/blink/public/platform/web_localized_string.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/geometry/dom_rect_read_only.h"
#include "third_party/blink/renderer/core/html/html_image_element.h"
#include "third_party/blink/renderer/core/html/media/html_video_element.h"
#include "third_party/blink/renderer/core/html/media/media_controls.h"
#include "third_party/blink/renderer/core/resize_observer/resize_observer.h"
#include "third_party/blink/renderer/core/resize_observer/resize_observer_entry.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/text/platform_locale.h"

namespace {

constexpr TimeDelta kPictureInPictureStyleChangeTransitionDuration =
    TimeDelta::FromMilliseconds(200);
constexpr TimeDelta kPictureInPictureHiddenAnimationSeconds =
    TimeDelta::FromMilliseconds(300);

}  // namespace

namespace blink {

class PictureInPictureInterstitial::VideoElementResizeObserverDelegate final
    : public ResizeObserver::Delegate {
 public:
  explicit VideoElementResizeObserverDelegate(
      PictureInPictureInterstitial* interstitial)
      : interstitial_(interstitial) {
    DCHECK(interstitial);
  }
  ~VideoElementResizeObserverDelegate() override = default;

  void OnResize(
      const HeapVector<Member<ResizeObserverEntry>>& entries) override {
    DCHECK_EQ(1u, entries.size());
    DCHECK_EQ(entries[0]->target(), interstitial_->GetVideoElement());
    DCHECK(entries[0]->contentRect());
    interstitial_->NotifyElementSizeChanged(*entries[0]->contentRect());
  }

  void Trace(blink::Visitor* visitor) override {
    visitor->Trace(interstitial_);
    ResizeObserver::Delegate::Trace(visitor);
  }

 private:
  Member<PictureInPictureInterstitial> interstitial_;
};

PictureInPictureInterstitial::PictureInPictureInterstitial(
    HTMLVideoElement& videoElement)
    : HTMLDivElement(videoElement.GetDocument()),
      resize_observer_(
          ResizeObserver::Create(videoElement.GetDocument(),
                                 new VideoElementResizeObserverDelegate(this))),
      interstitial_timer_(
          videoElement.GetDocument().GetTaskRunner(TaskType::kInternalMedia),
          this,
          &PictureInPictureInterstitial::ToggleInterstitialTimerFired),
      video_element_(&videoElement) {
  SetShadowPseudoId(AtomicString("-internal-media-interstitial"));

  background_image_ = HTMLImageElement::Create(GetDocument());
  background_image_->SetShadowPseudoId(
      AtomicString("-internal-media-interstitial-background-image"));
  background_image_->SetSrc(videoElement.getAttribute(HTMLNames::posterAttr));
  ParserAppendChild(background_image_);

  message_element_ = HTMLDivElement::Create(GetDocument());
  message_element_->SetShadowPseudoId(
      AtomicString("-internal-picture-in-picture-interstitial-message"));
  message_element_->setInnerText(
      GetVideoElement().GetLocale().QueryString(
          WebLocalizedString::kPictureInPictureInterstitialText),
      ASSERT_NO_EXCEPTION);
  ParserAppendChild(message_element_);

  resize_observer_->observe(video_element_);
}

void PictureInPictureInterstitial::Show() {
  if (should_be_visible_)
    return;

  if (interstitial_timer_.IsActive())
    interstitial_timer_.Stop();
  should_be_visible_ = true;
  RemoveInlineStyleProperty(CSSPropertyDisplay);
  interstitial_timer_.StartOneShot(
      kPictureInPictureStyleChangeTransitionDuration, FROM_HERE);

  DCHECK(GetVideoElement().CcLayer());
  GetVideoElement().CcLayer()->SetIsDrawable(false);
}

void PictureInPictureInterstitial::Hide() {
  if (!should_be_visible_)
    return;

  if (interstitial_timer_.IsActive())
    interstitial_timer_.Stop();
  should_be_visible_ = false;
  SetInlineStyleProperty(CSSPropertyOpacity, 0,
                         CSSPrimitiveValue::UnitType::kNumber);
  interstitial_timer_.StartOneShot(kPictureInPictureHiddenAnimationSeconds,
                                   FROM_HERE);

  if (GetVideoElement().CcLayer())
    GetVideoElement().CcLayer()->SetIsDrawable(true);
}

Node::InsertionNotificationRequest PictureInPictureInterstitial::InsertedInto(
    ContainerNode* root) {
  if (GetVideoElement().isConnected() && !resize_observer_) {
    resize_observer_ =
        ResizeObserver::Create(GetVideoElement().GetDocument(),
                               new VideoElementResizeObserverDelegate(this));
    resize_observer_->observe(&GetVideoElement());
  }

  return HTMLDivElement::InsertedInto(root);
}

void PictureInPictureInterstitial::RemovedFrom(ContainerNode*) {
  DCHECK(!GetVideoElement().isConnected());

  if (resize_observer_) {
    resize_observer_->disconnect();
    resize_observer_.Clear();
  }
}

void PictureInPictureInterstitial::NotifyElementSizeChanged(
    const DOMRectReadOnly& new_size) {
  message_element_->setAttribute(
      "class", MediaControls::GetSizingCSSClass(
                   MediaControls::GetSizingClass(new_size.width())));
}

void PictureInPictureInterstitial::ToggleInterstitialTimerFired(TimerBase*) {
  interstitial_timer_.Stop();
  if (should_be_visible_) {
    SetInlineStyleProperty(CSSPropertyBackgroundColor, CSSValueBlack);
    SetInlineStyleProperty(CSSPropertyOpacity, 1,
                           CSSPrimitiveValue::UnitType::kNumber);
  } else {
    SetInlineStyleProperty(CSSPropertyDisplay, CSSValueNone);
  }
}

void PictureInPictureInterstitial::OnPosterImageChanged() {
  background_image_->SetSrc(
      GetVideoElement().getAttribute(HTMLNames::posterAttr));
}

void PictureInPictureInterstitial::Trace(blink::Visitor* visitor) {
  visitor->Trace(resize_observer_);
  visitor->Trace(video_element_);
  visitor->Trace(background_image_);
  visitor->Trace(message_element_);
  HTMLDivElement::Trace(visitor);
}

}  // namespace blink
