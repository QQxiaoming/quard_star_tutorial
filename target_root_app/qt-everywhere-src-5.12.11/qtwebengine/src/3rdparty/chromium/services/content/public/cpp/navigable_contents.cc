// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/content/public/cpp/navigable_contents.h"

#include "services/content/public/cpp/buildflags.h"

#if BUILDFLAG(ENABLE_AURA_CONTENT_VIEW_EMBEDDING)
#include "services/ui/public/interfaces/window_tree_constants.mojom.h"  // nogncheck
#include "ui/views/layout/fill_layout.h"                // nogncheck
#include "ui/views/mus/remote_view/remote_view_host.h"  // nogncheck
#include "ui/views/view.h"                              // nogncheck
#endif

namespace content {

NavigableContents::NavigableContents(mojom::NavigableContentsFactory* factory)
    : client_binding_(this) {
  mojom::NavigableContentsClientPtr client;
  client_binding_.Bind(mojo::MakeRequest(&client));
  factory->CreateContents(mojom::NavigableContentsParams::New(),
                          mojo::MakeRequest(&contents_), std::move(client));
}

NavigableContents::~NavigableContents() = default;

views::View* NavigableContents::GetView() {
#if BUILDFLAG(ENABLE_AURA_CONTENT_VIEW_EMBEDDING)
  if (!view_) {
    view_ = std::make_unique<views::View>();
    view_->set_owned_by_client();
    view_->SetLayoutManager(std::make_unique<views::FillLayout>());

    DCHECK(!remote_view_host_);
    remote_view_host_ = new views::RemoteViewHost;
    view_->AddChildView(remote_view_host_);

    contents_->CreateView(base::BindOnce(
        &NavigableContents::OnEmbedTokenReceived, base::Unretained(this)));
  }
  return view_.get();
#else
  return nullptr;
#endif
}

void NavigableContents::Navigate(const GURL& url) {
  contents_->Navigate(url);
}

void NavigableContents::DidStopLoading() {
  if (did_stop_loading_callback_)
    did_stop_loading_callback_.Run();
}

void NavigableContents::OnEmbedTokenReceived(
    const base::UnguessableToken& token) {
#if BUILDFLAG(ENABLE_AURA_CONTENT_VIEW_EMBEDDING)
  const uint32_t kEmbedFlags = ui::mojom::kEmbedFlagEmbedderInterceptsEvents |
                               ui::mojom::kEmbedFlagEmbedderControlsVisibility;
  remote_view_host_->EmbedUsingToken(token, kEmbedFlags, base::DoNothing());
#endif  // defined(USE_AURA)
}

}  // namespace content
