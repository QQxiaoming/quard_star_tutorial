// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVTOOLS_TARGET_REGISTRY_H_
#define CONTENT_BROWSER_DEVTOOLS_TARGET_REGISTRY_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/containers/flat_map.h"
#include "base/values.h"
#include "content/browser/devtools/devtools_agent_host_impl.h"

namespace content {

class DevToolsSession;

class TargetRegistry {
 public:
  explicit TargetRegistry(DevToolsSession* root_session);
  ~TargetRegistry();

  void AttachSubtargetSession(const std::string& session_id,
                              DevToolsAgentHostImpl* agent_host,
                              DevToolsAgentHostClient* client);
  void DetachSubtargetSession(const std::string& session_id);
  bool DispatchMessageOnAgentHost(const std::string& message,
                                  base::DictionaryValue* parsed_message);
  void SendMessageToClient(const std::string& session_id,
                           const std::string& message);

 private:
  DevToolsSession* root_session_;
  base::flat_map<
      std::string,
      std::pair<scoped_refptr<DevToolsAgentHostImpl>, DevToolsAgentHostClient*>>
      sessions_;
  DISALLOW_COPY_AND_ASSIGN(TargetRegistry);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVTOOLS_TARGET_REGISTRY_H_
