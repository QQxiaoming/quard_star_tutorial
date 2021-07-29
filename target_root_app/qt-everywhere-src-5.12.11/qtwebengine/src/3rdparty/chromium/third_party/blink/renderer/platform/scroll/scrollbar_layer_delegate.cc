// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/scroll/scrollbar_layer_delegate.h"

#include "third_party/blink/public/platform/web_point.h"
#include "third_party/blink/public/platform/web_rect.h"
#include "third_party/blink/renderer/platform/graphics/graphics_context.h"
#include "third_party/blink/renderer/platform/graphics/paint/paint_canvas.h"
#include "third_party/blink/renderer/platform/graphics/paint/paint_record_builder.h"
#include "third_party/blink/renderer/platform/scroll/scroll_types.h"
#include "third_party/blink/renderer/platform/scroll/scrollable_area.h"
#include "third_party/blink/renderer/platform/scroll/scrollbar.h"
#include "third_party/blink/renderer/platform/scroll/scrollbar_theme.h"
#include "ui/gfx/skia_util.h"

namespace blink {

namespace {

class ScopedScrollbarPainter {
 public:
  ScopedScrollbarPainter(cc::PaintCanvas& canvas, float device_scale_factor)
      : canvas_(canvas) {
    builder_.Context().SetDeviceScaleFactor(device_scale_factor);
  }
  ~ScopedScrollbarPainter() { canvas_.drawPicture(builder_.EndRecording()); }

  GraphicsContext& Context() { return builder_.Context(); }

 private:
  cc::PaintCanvas& canvas_;
  PaintRecordBuilder builder_;
};

}  // namespace

ScrollbarLayerDelegate::ScrollbarLayerDelegate(blink::Scrollbar& scrollbar,
                                               float device_scale_factor)
    : scrollbar_(&scrollbar),
      theme_(scrollbar.GetTheme()),
      device_scale_factor_(device_scale_factor) {}

ScrollbarLayerDelegate::~ScrollbarLayerDelegate() = default;

cc::ScrollbarOrientation ScrollbarLayerDelegate::Orientation() const {
  if (scrollbar_->Orientation() == kHorizontalScrollbar)
    return cc::HORIZONTAL;
  return cc::VERTICAL;
}

bool ScrollbarLayerDelegate::IsLeftSideVerticalScrollbar() const {
  return scrollbar_->IsLeftSideVerticalScrollbar();
}

bool ScrollbarLayerDelegate::HasThumb() const {
  return theme_.HasThumb(*scrollbar_);
}

bool ScrollbarLayerDelegate::IsOverlay() const {
  return scrollbar_->IsOverlayScrollbar();
}

gfx::Point ScrollbarLayerDelegate::Location() const {
  return scrollbar_->Location();
}

int ScrollbarLayerDelegate::ThumbThickness() const {
  IntRect thumb_rect = theme_.ThumbRect(*scrollbar_);
  if (scrollbar_->Orientation() == kHorizontalScrollbar)
    return thumb_rect.Height();
  return thumb_rect.Width();
}

int ScrollbarLayerDelegate::ThumbLength() const {
  IntRect thumb_rect = theme_.ThumbRect(*scrollbar_);
  if (scrollbar_->Orientation() == kHorizontalScrollbar)
    return thumb_rect.Width();
  return thumb_rect.Height();
}

gfx::Rect ScrollbarLayerDelegate::TrackRect() const {
  return theme_.TrackRect(*scrollbar_);
}

float ScrollbarLayerDelegate::ThumbOpacity() const {
  return theme_.ThumbOpacity(*scrollbar_);
}

bool ScrollbarLayerDelegate::NeedsPaintPart(cc::ScrollbarPart part) const {
  if (part == cc::THUMB)
    return scrollbar_->ThumbNeedsRepaint();
  return scrollbar_->TrackNeedsRepaint();
}

bool ScrollbarLayerDelegate::UsesNinePatchThumbResource() const {
  return theme_.UsesNinePatchThumbResource();
}

gfx::Size ScrollbarLayerDelegate::NinePatchThumbCanvasSize() const {
  DCHECK(theme_.UsesNinePatchThumbResource());
  return static_cast<gfx::Size>(theme_.NinePatchThumbCanvasSize(*scrollbar_));
}

gfx::Rect ScrollbarLayerDelegate::NinePatchThumbAperture() const {
  DCHECK(theme_.UsesNinePatchThumbResource());
  return theme_.NinePatchThumbAperture(*scrollbar_);
}

bool ScrollbarLayerDelegate::HasTickmarks() const {
  // TODO(crbug.com/860499): Remove this condition, it should not occur.
  // Layers may exist and be painted for a |scrollbar_| that has had its
  // ScrollableArea detached. This seems weird because if the area is detached
  // the layer should be destroyed but here we are. https://crbug.com/860499.
  if (!scrollbar_->GetScrollableArea())
    return false;
  // When the frame is throttled, the scrollbar will not be painted because
  // the frame has not had its lifecycle updated. Thus the actual value of
  // HasTickmarks can't be known and may change once the frame is unthrottled.
  if (scrollbar_->GetScrollableArea()->IsThrottled())
    return false;

  Vector<IntRect> tickmarks;
  scrollbar_->GetTickmarks(tickmarks);
  return !tickmarks.IsEmpty();
}

void ScrollbarLayerDelegate::PaintPart(cc::PaintCanvas* canvas,
                                       cc::ScrollbarPart part,
                                       const gfx::Rect& content_rect) {
  PaintCanvasAutoRestore auto_restore(canvas, true);
  blink::Scrollbar& scrollbar = *scrollbar_;

  // TODO(crbug.com/860499): Remove this condition, it should not occur.
  // Layers may exist and be painted for a |scrollbar_| that has had its
  // ScrollableArea detached. This seems weird because if the area is detached
  // the layer should be destroyed but here we are.
  if (!scrollbar_->GetScrollableArea())
    return;
  // When the frame is throttled, the scrollbar will not be painted because
  // the frame has not had its lifecycle updated.
  if (scrollbar.GetScrollableArea()->IsThrottled())
    return;

  if (part == cc::THUMB) {
    ScopedScrollbarPainter painter(*canvas, device_scale_factor_);
    theme_.PaintThumb(painter.Context(), scrollbar, IntRect(content_rect));
    if (!theme_.ShouldRepaintAllPartsOnInvalidation())
      scrollbar.ClearThumbNeedsRepaint();
    return;
  }

  if (part == cc::TICKMARKS) {
    ScopedScrollbarPainter painter(*canvas, device_scale_factor_);
    theme_.PaintTickmarks(painter.Context(), scrollbar, IntRect(content_rect));
    return;
  }

  canvas->clipRect(gfx::RectToSkRect(content_rect));
  ScopedScrollbarPainter painter(*canvas, device_scale_factor_);
  GraphicsContext& context = painter.Context();

  theme_.PaintScrollbarBackground(context, scrollbar);

  if (theme_.HasButtons(scrollbar)) {
    theme_.PaintButton(context, scrollbar,
                       theme_.BackButtonRect(scrollbar, kBackButtonStartPart),
                       kBackButtonStartPart);
    theme_.PaintButton(context, scrollbar,
                       theme_.BackButtonRect(scrollbar, kBackButtonEndPart),
                       kBackButtonEndPart);
    theme_.PaintButton(
        context, scrollbar,
        theme_.ForwardButtonRect(scrollbar, kForwardButtonStartPart),
        kForwardButtonStartPart);
    theme_.PaintButton(
        context, scrollbar,
        theme_.ForwardButtonRect(scrollbar, kForwardButtonEndPart),
        kForwardButtonEndPart);
  }

  IntRect track_paint_rect = theme_.TrackRect(scrollbar);
  theme_.PaintTrackBackground(context, scrollbar, track_paint_rect);

  if (theme_.HasThumb(scrollbar)) {
    theme_.PaintTrackPiece(painter.Context(), scrollbar, track_paint_rect,
                           kForwardTrackPart);
    theme_.PaintTrackPiece(painter.Context(), scrollbar, track_paint_rect,
                           kBackTrackPart);
  }

  theme_.PaintTickmarks(painter.Context(), scrollbar, track_paint_rect);

  if (!theme_.ShouldRepaintAllPartsOnInvalidation())
    scrollbar.ClearTrackNeedsRepaint();
}

}  // namespace blink
