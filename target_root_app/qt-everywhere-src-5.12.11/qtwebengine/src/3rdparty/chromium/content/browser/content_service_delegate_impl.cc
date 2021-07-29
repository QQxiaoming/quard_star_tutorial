// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/content_service_delegate_impl.h"

#include "base/macros.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "services/content/navigable_contents_delegate.h"
#include "services/content/service.h"

namespace content {

namespace {

// Bridge between Content Service navigable contents delegation API and a
// WebContentsImpl.
class NavigableContentsDelegateImpl : public content::NavigableContentsDelegate,
                                      public WebContentsObserver {
 public:
  explicit NavigableContentsDelegateImpl(BrowserContext* browser_context,
                                         mojom::NavigableContentsClient* client)
      : client_(client) {
    WebContents::CreateParams params(browser_context);
    web_contents_ = WebContents::Create(params);
    WebContentsObserver::Observe(web_contents_.get());
  }

  ~NavigableContentsDelegateImpl() override {
    WebContentsObserver::Observe(nullptr);
  }

 private:
  // content::NavigableContentsDelegate:
  gfx::NativeView GetNativeView() override {
    return web_contents_->GetNativeView();
  }

  void Navigate(const GURL& url) override {
    NavigationController::LoadURLParams params(url);
    params.transition_type = ui::PAGE_TRANSITION_AUTO_TOPLEVEL;
    web_contents_->GetController().LoadURLWithParams(params);
  }

  // WebContentsObserver:
  void DidStopLoading() override { client_->DidStopLoading(); }

  std::unique_ptr<WebContents> web_contents_;
  mojom::NavigableContentsClient* const client_;

  DISALLOW_COPY_AND_ASSIGN(NavigableContentsDelegateImpl);
};

}  // namespace

ContentServiceDelegateImpl::ContentServiceDelegateImpl(
    BrowserContext* browser_context)
    : browser_context_(browser_context) {}

ContentServiceDelegateImpl::~ContentServiceDelegateImpl() {
  // This delegate is destroyed immediately before |browser_context_| is
  // destroyed. We force-kill any Content Service instances which depend on
  // |this|, since they will no longer be functional anyway.
  std::set<content::Service*> instances;
  std::swap(instances, service_instances_);
  for (content::Service* service : instances) {
    // Eventually destroys |service|. Ensures that no more calls into |this|
    // will occur.
    service->ForceQuit();
  }
}

void ContentServiceDelegateImpl::AddService(content::Service* service) {
  service_instances_.insert(service);
}

void ContentServiceDelegateImpl::WillDestroyServiceInstance(
    content::Service* service) {
  service_instances_.erase(service);
}

std::unique_ptr<content::NavigableContentsDelegate>
ContentServiceDelegateImpl::CreateNavigableContentsDelegate(
    mojom::NavigableContentsClient* client) {
  return std::make_unique<NavigableContentsDelegateImpl>(browser_context_,
                                                         client);
}

}  // namespace content
