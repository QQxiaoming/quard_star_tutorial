// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/network/empty_url_loader_client.h"

#include <utility>

#include "base/bind.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace network {

// static
void EmptyURLLoaderClient::DrainURLRequest(
    mojom::URLLoaderClientRequest client_request,
    mojom::URLLoaderPtr url_loader) {
  // Raw |new| is okay, because the newly constructed EmptyURLLoaderClient will
  // delete itself after consuming all the data/callbacks.
  new EmptyURLLoaderClient(std::move(client_request), std::move(url_loader));
}

EmptyURLLoaderClient::EmptyURLLoaderClient(
    mojom::URLLoaderClientRequest request,
    mojom::URLLoaderPtr url_loader)
    : binding_(this, std::move(request)), url_loader_(std::move(url_loader)) {
  binding_.set_connection_error_handler(base::BindOnce(
      &EmptyURLLoaderClient::DeleteSelf, base::Unretained(this)));
}

EmptyURLLoaderClient::~EmptyURLLoaderClient() {}

void EmptyURLLoaderClient::DeleteSelf() {
  delete this;
}

void EmptyURLLoaderClient::OnReceiveResponse(const ResourceResponseHead& head) {
}

void EmptyURLLoaderClient::OnReceiveRedirect(
    const net::RedirectInfo& redirect_info,
    const ResourceResponseHead& head) {}

void EmptyURLLoaderClient::OnUploadProgress(int64_t current_position,
                                            int64_t total_size,
                                            OnUploadProgressCallback callback) {
  std::move(callback).Run();
}

void EmptyURLLoaderClient::OnReceiveCachedMetadata(
    const std::vector<uint8_t>& data) {}

void EmptyURLLoaderClient::OnTransferSizeUpdated(int32_t transfer_size_diff) {}

void EmptyURLLoaderClient::OnStartLoadingResponseBody(
    mojo::ScopedDataPipeConsumerHandle body) {
  DCHECK(!response_body_drainer_);
  response_body_drainer_ =
      std::make_unique<mojo::DataPipeDrainer>(this, std::move(body));
}

void EmptyURLLoaderClient::OnComplete(const URLLoaderCompletionStatus& status) {
  DeleteSelf();
}

void EmptyURLLoaderClient::OnDataAvailable(const void* data, size_t num_bytes) {
}

void EmptyURLLoaderClient::OnDataComplete() {}

}  // namespace network
