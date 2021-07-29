// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVTOOLS_BROWSER_DEVTOOLS_AGENT_HOST_H_
#define CONTENT_BROWSER_DEVTOOLS_BROWSER_DEVTOOLS_AGENT_HOST_H_

#include "base/containers/flat_map.h"
#include "content/browser/devtools/devtools_agent_host_impl.h"

namespace content {

class TargetRegistry;

class BrowserDevToolsAgentHost : public DevToolsAgentHostImpl {
 private:
  friend class DevToolsAgentHost;
  BrowserDevToolsAgentHost(
      scoped_refptr<base::SingleThreadTaskRunner> tethering_task_runner,
      const CreateServerSocketCallback& socket_callback,
      bool only_discovery);
  ~BrowserDevToolsAgentHost() override;

  // DevToolsAgentHostImpl overrides.
  bool AttachSession(DevToolsSession* session,
                     TargetRegistry* registry) override;
  void DetachSession(DevToolsSession* session) override;
  bool DispatchProtocolMessage(DevToolsAgentHostClient* client,
                               const std::string& message,
                               base::DictionaryValue* parsed_message) override;

  // DevToolsAgentHost implementation.
  std::string GetType() override;
  std::string GetTitle() override;
  GURL GetURL() override;
  bool Activate() override;
  void Reload() override;
  bool Close() override;

  scoped_refptr<base::SingleThreadTaskRunner> tethering_task_runner_;
  CreateServerSocketCallback socket_callback_;
  bool only_discovery_;
  base::flat_map<DevToolsAgentHostClient*, std::unique_ptr<TargetRegistry>>
      target_registries_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVTOOLS_BROWSER_DEVTOOLS_AGENT_HOST_H_
