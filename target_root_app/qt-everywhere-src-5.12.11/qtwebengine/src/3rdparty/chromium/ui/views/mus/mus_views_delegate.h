// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_MUS_MUS_VIEWS_DELEGATE_H_
#define UI_VIEWS_MUS_MUS_VIEWS_DELEGATE_H_

#include "base/macros.h"
#include "ui/views/layout/layout_provider.h"
#include "ui/views/mus/mus_export.h"
#include "ui/views/views_delegate.h"

namespace views {

class VIEWS_MUS_EXPORT MusViewsDelegate : public ViewsDelegate {
 public:
  MusViewsDelegate();
  ~MusViewsDelegate() override;

  // ViewsDelegate:
  void NotifyAccessibilityEvent(View* view,
                                ax::mojom::Event event_type) override;

 private:
  LayoutProvider layout_provider_;

  DISALLOW_COPY_AND_ASSIGN(MusViewsDelegate);
};

}  // namespace views

#endif  // UI_VIEWS_MUS_MUS_VIEWS_DELEGATE_H_
