// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webrunner/browser/frame_impl.h"

#include "base/logging.h"
#include "content/public/browser/web_contents.h"
#include "ui/aura/layout_manager.h"
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host_platform.h"
#include "ui/platform_window/platform_window_init_properties.h"
#include "url/gurl.h"

namespace webrunner {

namespace {

// Layout manager that allows only one child window and stretches it to fill the
// parent.
class LayoutManagerImpl : public aura::LayoutManager {
 public:
  LayoutManagerImpl() = default;
  ~LayoutManagerImpl() override = default;

  // aura::LayoutManager.
  void OnWindowResized() override {
    // Resize the child to match the size of the parent
    if (child_) {
      SetChildBoundsDirect(child_,
                           gfx::Rect(child_->parent()->bounds().size()));
    }
  }
  void OnWindowAddedToLayout(aura::Window* child) override {
    DCHECK(!child_);
    child_ = child;
    SetChildBoundsDirect(child_, gfx::Rect(child_->parent()->bounds().size()));
  }

  void OnWillRemoveWindowFromLayout(aura::Window* child) override {
    DCHECK_EQ(child, child_);
    child_ = nullptr;
  }

  void OnWindowRemovedFromLayout(aura::Window* child) override {}
  void OnChildWindowVisibilityChanged(aura::Window* child,
                                      bool visible) override {}
  void SetChildBounds(aura::Window* child,
                      const gfx::Rect& requested_bounds) override {}

 private:
  aura::Window* child_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(LayoutManagerImpl);
};

}  // namespace

FrameImpl::FrameImpl(std::unique_ptr<content::WebContents> web_contents,
                     chromium::web::FrameObserverPtr observer)
    : web_contents_(std::move(web_contents)), observer_(std::move(observer)) {
  Observe(web_contents.get());
}

FrameImpl::~FrameImpl() {
  window_tree_host_->Hide();
  window_tree_host_->compositor()->SetVisible(false);
}

void FrameImpl::CreateView(
    fidl::InterfaceRequest<fuchsia::ui::views_v1_token::ViewOwner> view_owner,
    fidl::InterfaceRequest<fuchsia::sys::ServiceProvider> services) {
  ui::PlatformWindowInitProperties properties;
  properties.view_owner_request = std::move(view_owner);

  window_tree_host_ =
      std::make_unique<aura::WindowTreeHostPlatform>(std::move(properties));
  window_tree_host_->InitHost();
  window_tree_host_->window()->SetLayoutManager(new LayoutManagerImpl());
  window_tree_host_->window()->AddChild(web_contents_->GetNativeView());
  window_tree_host_->window()->Show();
  window_tree_host_->Show();
  web_contents_->GetNativeView()->Show();
}

void FrameImpl::GetNavigationController(
    fidl::InterfaceRequest<chromium::web::NavigationController> controller) {
  controller_bindings_.AddBinding(this, std::move(controller));
}

void FrameImpl::LoadUrl(fidl::StringPtr url,
                        std::unique_ptr<chromium::web::LoadUrlParams> params) {
  GURL validated_url(*url);
  if (!validated_url.is_valid()) {
    DLOG(WARNING) << "Invalid URL: " << *url;
    return;
  }

  content::NavigationController::LoadURLParams params_converted(validated_url);
  params_converted.transition_type = ui::PageTransitionFromInt(
      ui::PAGE_TRANSITION_TYPED | ui::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  web_contents_->GetController().LoadURLWithParams(params_converted);
}

void FrameImpl::GoBack() {
  NOTIMPLEMENTED();
}

void FrameImpl::GoForward() {
  NOTIMPLEMENTED();
}

void FrameImpl::Stop() {
  NOTIMPLEMENTED();
}

void FrameImpl::Reload() {
  NOTIMPLEMENTED();
}

void FrameImpl::GetVisibleEntry(GetVisibleEntryCallback callback) {
  NOTIMPLEMENTED();
  callback(nullptr);
}

}  // namespace webrunner
