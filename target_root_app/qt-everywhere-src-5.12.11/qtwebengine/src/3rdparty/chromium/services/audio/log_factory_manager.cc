// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/audio/log_factory_manager.h"

#include <utility>

#include "services/service_manager/public/cpp/service_context_ref.h"

namespace audio {

LogFactoryManager::LogFactoryManager() {}

LogFactoryManager::~LogFactoryManager() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(owning_sequence_);
}

void LogFactoryManager::Bind(mojom::LogFactoryManagerRequest request,
                             TracedServiceRef context_ref) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(owning_sequence_);
  bindings_.AddBinding(this, std::move(request), std::move(context_ref));
}

void LogFactoryManager::SetLogFactory(
    media::mojom::AudioLogFactoryPtr log_factory) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(owning_sequence_);
  log_factory_adapter_.SetLogFactory(std::move(log_factory));
}

media::AudioLogFactory* LogFactoryManager::GetLogFactory() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(owning_sequence_);
  return &log_factory_adapter_;
}

}  // namespace audio
