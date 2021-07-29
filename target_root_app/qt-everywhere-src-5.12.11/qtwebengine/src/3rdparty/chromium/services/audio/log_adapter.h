// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_AUDIO_LOG_ADAPTER_H_
#define SERVICES_AUDIO_LOG_ADAPTER_H_

#include <string>

#include "media/audio/audio_logging.h"
#include "media/mojo/interfaces/audio_logging.mojom.h"

namespace media {
class AudioParameters;
}

namespace audio {

// This class wraps a media::mojom::AudioLogPtr into a media::AudioLog.
class LogAdapter : public media::AudioLog {
 public:
  explicit LogAdapter(media::mojom::AudioLogPtr audio_log);
  ~LogAdapter() override;

  // media::AudioLog implementation.
  void OnCreated(const media::AudioParameters& params,
                 const std::string& device_id) override;
  void OnStarted() override;
  void OnStopped() override;
  void OnClosed() override;
  void OnError() override;
  void OnSetVolume(double volume) override;
  void OnLogMessage(const std::string& message) override;

 private:
  media::mojom::AudioLogPtr audio_log_;

  DISALLOW_COPY_AND_ASSIGN(LogAdapter);
};

}  // namespace audio

#endif  // SERVICES_AUDIO_LOG_ADAPTER_H_
