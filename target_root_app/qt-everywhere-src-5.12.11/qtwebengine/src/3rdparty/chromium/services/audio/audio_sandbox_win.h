// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_AUDIO_AUDIO_SANDBOX_WIN_H_
#define SERVICES_AUDIO_AUDIO_SANDBOX_WIN_H_

namespace sandbox {
class TargetPolicy;
}

// These sandbox-config extension functions should be called from
// UtilitySandboxedProcessLauncherDelegate on Windows (or the appropriate
// Delegate if SANDBOX_TYPE_AUDIO is removed from SANDBOX_TYPE_UTILITY).
//
// NOTE: changes to this code need to be reviewed by the security team.

namespace audio {

// PreSpawnTarget extension.
bool AudioPreSpawnTarget(sandbox::TargetPolicy* policy);

}  // namespace audio

#endif  // SERVICES_AUDIO_AUDIO_SANDBOX_WIN_H_
