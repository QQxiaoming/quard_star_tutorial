/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DEVTOOLS_FRONTEND_QT_H
#define DEVTOOLS_FRONTEND_QT_H

#include <memory>

#include "web_contents_delegate_qt.h"

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/web_contents_observer.h"
#include "net/url_request/url_fetcher_delegate.h"

namespace base {
class Value;
}

namespace content {
class NavigationHandle;
class RenderViewHost;
class WebContents;
}  // namespace content

class PersistentPrefStore;

namespace QtWebEngineCore {

class DevToolsFrontendQt : public content::WebContentsObserver
                         , public content::DevToolsAgentHostClient
                         , public net::URLFetcherDelegate {
public:
    static DevToolsFrontendQt *Show(QSharedPointer<WebContentsAdapter> frontendAdapter, content::WebContents *inspectedContents);

    void Activate();
    void Focus();
    void InspectElementAt(int x, int y);
    void Close();

    void DisconnectFromTarget();

    void CallClientFunction(const std::string& function_name,
                            const base::Value* arg1,
                            const base::Value* arg2,
                            const base::Value* arg3);

    WebContentsDelegateQt *frontendDelegate() const
    {
        return m_frontendDelegate;
    }

protected:
    DevToolsFrontendQt(QSharedPointer<WebContentsAdapter> webContentsAdapter, content::WebContents *inspectedContents);
    ~DevToolsFrontendQt() override;

    // content::DevToolsAgentHostClient implementation.
    void AgentHostClosed(content::DevToolsAgentHost* agent_host) override;
    void DispatchProtocolMessage(content::DevToolsAgentHost* agent_host, const std::string& message) override;

    void SetPreferences(const std::string& json);
    virtual void HandleMessageFromDevToolsFrontend(const std::string& message);

private:
    // WebContentsObserver overrides
    void ReadyToCommitNavigation(content::NavigationHandle* navigation_handle) override;
    void DocumentAvailableInMainFrame() override;
    void WebContentsDestroyed() override;

    // net::URLFetcherDelegate overrides.
    void OnURLFetchComplete(const net::URLFetcher* source) override;

    void SendMessageAck(int request_id, const base::Value* arg1);
    void SetPreference(const std::string &name, const std::string &value);
    void RemovePreference(const std::string &name);
    void ClearPreferences();
    void CreateJsonPreferences(bool clear);

    // We shouldn't be keeping it alive
    QWeakPointer<WebContentsAdapter> m_webContentsAdapter;
    WebContentsDelegateQt *m_frontendDelegate;
    content::WebContents *m_inspectedContents;
    scoped_refptr<content::DevToolsAgentHost> m_agentHost;
    int m_inspect_element_at_x;
    int m_inspect_element_at_y;
    std::unique_ptr<content::DevToolsFrontendHost> m_frontendHost;
    using PendingRequestsMap = std::map<const net::URLFetcher*, int>;
    PendingRequestsMap m_pendingRequests;
    base::DictionaryValue m_preferences;
    scoped_refptr<PersistentPrefStore> m_prefStore;
    base::WeakPtrFactory<DevToolsFrontendQt> m_weakFactory;

    DISALLOW_COPY_AND_ASSIGN(DevToolsFrontendQt);
};

} // namespace QtWebEngineCore

#endif  // DEVTOOLS_FRONTEND_QT_H
