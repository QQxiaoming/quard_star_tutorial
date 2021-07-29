// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBRUNNER_BROWSER_FRAME_IMPL_H_
#define WEBRUNNER_BROWSER_FRAME_IMPL_H_

#include <lib/fidl/cpp/binding_set.h>
#include <memory>
#include <utility>

#include "base/macros.h"
#include "content/public/browser/web_contents_observer.h"
#include "url/gurl.h"
#include "webrunner/fidl/chromium/web/cpp/fidl.h"

namespace aura {
class WindowTreeHost;
}  // namespace aura

namespace content {
class BrowserContext;
class WebContents;
}  // namespace content

namespace webrunner {

// Implementation of Frame from //webrunner/fidl/frame.fidl.
// Implements a Frame service, which is a wrapper for a WebContents instance.
class FrameImpl : public chromium::web::Frame,
                  public chromium::web::NavigationController,
                  public content::WebContentsObserver {
 public:
  FrameImpl(std::unique_ptr<content::WebContents> web_contents,
            chromium::web::FrameObserverPtr observer);
  ~FrameImpl() override;

  // content::WebContentsObserver implementation.
  void Init(content::BrowserContext* browser_context, const GURL& url);

  // chromium::web::Frame implementation.
  void CreateView(
      fidl::InterfaceRequest<fuchsia::ui::views_v1_token::ViewOwner> view_owner,
      fidl::InterfaceRequest<fuchsia::sys::ServiceProvider> services) override;
  void GetNavigationController(
      fidl::InterfaceRequest<chromium::web::NavigationController> controller)
      override;

  // chromium::web::NavigationController implementation.
  void LoadUrl(fidl::StringPtr url,
               std::unique_ptr<chromium::web::LoadUrlParams> params) override;
  void GoBack() override;
  void GoForward() override;
  void Stop() override;
  void Reload() override;
  void GetVisibleEntry(GetVisibleEntryCallback callback) override;

 private:
  std::unique_ptr<aura::WindowTreeHost> window_tree_host_;
  std::unique_ptr<content::WebContents> web_contents_;

  chromium::web::FrameObserverPtr observer_;
  fidl::BindingSet<chromium::web::NavigationController> controller_bindings_;

  DISALLOW_COPY_AND_ASSIGN(FrameImpl);
};

}  // namespace webrunner

#endif  // WEBRUNNER_BROWSER_FRAME_IMPL_H_
