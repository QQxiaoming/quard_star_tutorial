// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_MEDIA_AUDIO_LOG_FACTORY_H_
#define CONTENT_BROWSER_MEDIA_AUDIO_LOG_FACTORY_H_

#include "media/mojo/interfaces/audio_logging.mojom.h"

namespace content {

class AudioLogFactory : public media::mojom::AudioLogFactory {
 public:
  AudioLogFactory();
  ~AudioLogFactory() override;

  // media::mojom::AudioLogFactory implementation.
  void CreateAudioLog(media::mojom::AudioLogComponent component,
                      int32_t component_id,
                      media::mojom::AudioLogRequest audio_log_request) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(AudioLogFactory);
};

}  // namespace content

#endif  // CONTENT_BROWSER_MEDIA_AUDIO_LOG_FACTORY_H_
