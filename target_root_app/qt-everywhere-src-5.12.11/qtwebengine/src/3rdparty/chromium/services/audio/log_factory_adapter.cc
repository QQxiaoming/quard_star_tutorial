// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/audio/log_factory_adapter.h"

#include <utility>

#include "services/audio/log_adapter.h"

namespace audio {

const int kMaxPendingLogRequests = 500;

struct LogFactoryAdapter::PendingLogRequest {
  PendingLogRequest(media::mojom::AudioLogComponent component,
                    int component_id,
                    media::mojom::AudioLogRequest request)
      : component(component),
        component_id(component_id),
        request(std::move(request)) {}
  PendingLogRequest(PendingLogRequest&& other) = default;
  PendingLogRequest& operator=(PendingLogRequest&& other) = default;
  PendingLogRequest(const PendingLogRequest& other) = delete;
  PendingLogRequest& operator=(const PendingLogRequest& other) = delete;
  ~PendingLogRequest() = default;

  media::mojom::AudioLogComponent component;
  int component_id;
  media::mojom::AudioLogRequest request;
};

LogFactoryAdapter::LogFactoryAdapter() = default;
LogFactoryAdapter::~LogFactoryAdapter() = default;

void LogFactoryAdapter::SetLogFactory(
    media::mojom::AudioLogFactoryPtr log_factory) {
  if (log_factory_) {
    LOG(WARNING) << "Attempting to set log factory more than once. Ignoring "
                    "request.";
    return;
  }

  log_factory_ = std::move(log_factory);
  while (!pending_requests_.empty()) {
    auto& front = pending_requests_.front();
    log_factory_->CreateAudioLog(front.component, front.component_id,
                                 std::move(front.request));
    pending_requests_.pop();
  }
}

std::unique_ptr<media::AudioLog> LogFactoryAdapter::CreateAudioLog(
    AudioComponent component,
    int component_id) {
  media::mojom::AudioLogPtr audio_log_ptr;
  media::mojom::AudioLogRequest audio_log_request =
      mojo::MakeRequest(&audio_log_ptr);
  media::mojom::AudioLogComponent mojo_component =
      static_cast<media::mojom::AudioLogComponent>(component);
  if (log_factory_) {
    log_factory_->CreateAudioLog(mojo_component, component_id,
                                 std::move(audio_log_request));
  } else if (pending_requests_.size() >= kMaxPendingLogRequests) {
    LOG(WARNING) << "Maximum number of queued log requests exceeded. Fulfilling"
                    " request with fake log.";
    return fake_log_factory_.CreateAudioLog(component, component_id);
  } else {
    pending_requests_.emplace(mojo_component, component_id,
                              std::move(audio_log_request));
  }
  return std::make_unique<LogAdapter>(std::move(audio_log_ptr));
}

}  // namespace audio
