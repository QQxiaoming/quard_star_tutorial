// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_AUDIO_LOG_FACTORY_MANAGER_H_
#define SERVICES_AUDIO_LOG_FACTORY_MANAGER_H_

#include <memory>

#include "mojo/public/cpp/bindings/binding_set.h"
#include "services/audio/log_factory_adapter.h"
#include "services/audio/public/mojom/log_factory_manager.mojom.h"
#include "services/audio/traced_service_ref.h"

namespace media {
class AudioLogFactory;
}

namespace audio {

// This class is used to provide the LogFactoryManager interface. It will
// typically be instantiated when needed and remain for the lifetime of the
// service.
class LogFactoryManager final : public mojom::LogFactoryManager {
 public:
  LogFactoryManager();
  ~LogFactoryManager() final;

  void Bind(mojom::LogFactoryManagerRequest request,
            TracedServiceRef context_ref);

  // LogFactoryManager implementation.
  void SetLogFactory(media::mojom::AudioLogFactoryPtr log_factory) final;
  media::AudioLogFactory* GetLogFactory();

 private:
  mojo::BindingSet<mojom::LogFactoryManager, TracedServiceRef> bindings_;
  LogFactoryAdapter log_factory_adapter_;
  SEQUENCE_CHECKER(owning_sequence_);

  DISALLOW_COPY_AND_ASSIGN(LogFactoryManager);
};

}  // namespace audio

#endif  // SERVICES_AUDIO_LOG_FACTORY_MANAGER_H_
