// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DOWNLOAD_WEB_UI_DOWNLOAD_URL_LOADER_FACTORY_GETTER_H_
#define CONTENT_BROWSER_DOWNLOAD_WEB_UI_DOWNLOAD_URL_LOADER_FACTORY_GETTER_H_

#include "base/files/file_path.h"
#include "components/download/public/common/download_url_loader_factory_getter.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

class GURL;

namespace content {
class RenderFrameHost;

// Class for retrieving the URLLoaderFactory for a webui URL.
class WebUIDownloadURLLoaderFactoryGetter
    : public download::DownloadURLLoaderFactoryGetter {
 public:
  WebUIDownloadURLLoaderFactoryGetter(RenderFrameHost* rfh, const GURL& url);

  // download::DownloadURLLoaderFactoryGetter implementation.
  scoped_refptr<network::SharedURLLoaderFactory> GetURLLoaderFactory() override;

 protected:
  ~WebUIDownloadURLLoaderFactoryGetter() override;

 private:
  network::mojom::URLLoaderFactoryPtrInfo factory_info_;

  // Lives on the UI thread and must be deleted there.
  std::unique_ptr<network::mojom::URLLoaderFactory> factory_;

  DISALLOW_COPY_AND_ASSIGN(WebUIDownloadURLLoaderFactoryGetter);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DOWNLOAD_WEB_UI_DOWNLOAD_URL_LOADER_FACTORY_GETTER_H_
