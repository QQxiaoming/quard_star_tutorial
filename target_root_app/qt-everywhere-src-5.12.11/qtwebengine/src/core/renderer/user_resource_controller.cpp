/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "user_resource_controller.h"

#include "base/memory/weak_ptr.h"
#include "base/pending_task.h"
#include "base/strings/pattern.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_view_observer.h"
#include "extensions/common/url_pattern.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_script_source.h"
#include "third_party/blink/public/web/web_view.h"
#include "v8/include/v8.h"

#include "common/qt_messages.h"
#include "common/user_script_data.h"
#include "type_conversion.h"
#include "user_script.h"

#include <QRegularExpression>

#include <bitset>

Q_GLOBAL_STATIC(UserResourceController, qt_webengine_userResourceController)

static content::RenderView * const globalScriptsIndex = 0;

// Scripts meant to run after the load event will be run 500ms after DOMContentLoaded if the load event doesn't come within that delay.
static const int afterLoadTimeout = 500;

static int validUserScriptSchemes()
{
    return URLPattern::SCHEME_HTTP | URLPattern::SCHEME_HTTPS | URLPattern::SCHEME_FILE;
}

static bool regexMatchesURL(const std::string &pat, const GURL &url) {
    QRegularExpression qre(QtWebEngineCore::toQt(pat));
    qre.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    if (!qre.isValid())
        return false;
    return qre.match(QtWebEngineCore::toQt(url.spec())).hasMatch();
}

static bool includeRuleMatchesURL(const std::string &pat, const GURL &url)
{
    // Match patterns for greasemonkey's @include and @exclude rules which can
    // be either strings with wildcards or regular expressions.
    if (pat.front() == '/' && pat.back() == '/') {
        std::string re(++pat.cbegin(), --pat.cend());
        if (regexMatchesURL(re, url))
            return true;
    } else if (base::MatchPattern(url.spec(), pat)) {
        return true;
    }
    return false;
}

static bool scriptMatchesURL(const UserScriptData &scriptData, const GURL &url) {
    // Logic taken from Chromium (extensions/common/user_script.cc)
    bool matchFound;
    if (!scriptData.urlPatterns.empty()) {
        matchFound = false;
        for (auto it = scriptData.urlPatterns.begin(), end = scriptData.urlPatterns.end(); it != end; ++it) {
            URLPattern urlPattern(validUserScriptSchemes());
            if (urlPattern.Parse(*it) == URLPattern::PARSE_SUCCESS && urlPattern.MatchesURL(url))
                matchFound = true;
        }
        if (!matchFound)
            return false;
    }

    if (!scriptData.globs.empty()) {
        matchFound = false;
        for (auto it = scriptData.globs.begin(), end = scriptData.globs.end(); it != end; ++it) {
            if (includeRuleMatchesURL(*it, url))
                matchFound = true;
        }
        if (!matchFound)
            return false;
    }

    if (!scriptData.excludeGlobs.empty()) {
        for (auto it = scriptData.excludeGlobs.begin(), end = scriptData.excludeGlobs.end(); it != end; ++it) {
            if (includeRuleMatchesURL(*it, url))
                return false;
        }
    }

    return true;
}

class UserResourceController::RenderFrameObserverHelper : public content::RenderFrameObserver
{
public:
    RenderFrameObserverHelper(content::RenderFrame* render_frame);

private:
    // RenderFrameObserver implementation.
    void DidCommitProvisionalLoad(bool is_new_navigation, bool is_same_document_navigation) override;
    void DidFinishDocumentLoad() override;
    void DidFinishLoad() override;
    void FrameDetached() override;
    void OnDestruct() override;
    bool OnMessageReceived(const IPC::Message& message) override;

    void onUserScriptAdded(const UserScriptData &);
    void onUserScriptRemoved(const UserScriptData &);
    void onScriptsCleared();

    class Runner;
    QScopedPointer<Runner> m_runner;
};

// Helper class to create WeakPtrs so the AfterLoad tasks can be canceled and to
// avoid running scripts more than once per injection point.
class UserResourceController::RenderFrameObserverHelper::Runner : public base::SupportsWeakPtr<Runner> {
public:
    explicit Runner(blink::WebLocalFrame *frame)
        : m_frame(frame)
    {
    }

    void run(UserScriptData::InjectionPoint p)
    {
        DCHECK_LT(p, m_ran.size());
        if (!m_ran[p]) {
            UserResourceController::instance()->runScripts(p, m_frame);
            m_ran[p] = true;
        }
    }

private:
    blink::WebLocalFrame *m_frame;
    std::bitset<3> m_ran;
};

// Used only for script cleanup on RenderView destruction.
class UserResourceController::RenderViewObserverHelper : public content::RenderViewObserver
{
public:
    RenderViewObserverHelper(content::RenderView* render_view);
private:
    // RenderViewObserver implementation.
    void OnDestruct() override;
};

void UserResourceController::runScripts(UserScriptData::InjectionPoint p, blink::WebLocalFrame *frame)
{
    content::RenderFrame *renderFrame = content::RenderFrame::FromWebFrame(frame);
    if (!renderFrame)
        return;
    const bool isMainFrame = renderFrame->IsMainFrame();

    content::RenderView *renderView = renderFrame->GetRenderView();
    if (!renderView)
        return;

    QList<uint64_t> scriptsToRun = m_viewUserScriptMap.value(0).toList();
    scriptsToRun.append(m_viewUserScriptMap.value(renderView).toList());

    for (uint64_t id : qAsConst(scriptsToRun)) {
        const UserScriptData &script = m_scripts.value(id);
        if (script.injectionPoint != p
                || (!script.injectForSubframes && !isMainFrame))
            continue;
        if (!scriptMatchesURL(script, frame->GetDocument().Url()))
            continue;
        blink::WebScriptSource source(blink::WebString::FromUTF8(script.source), script.url);
        if (script.worldId)
            frame->ExecuteScriptInIsolatedWorld(script.worldId, source);
        else
            frame->ExecuteScript(source);
    }
}

void UserResourceController::RunScriptsAtDocumentEnd(content::RenderFrame *render_frame)
{
    runScripts(UserScriptData::DocumentLoadFinished, render_frame->GetWebFrame());
}

UserResourceController::RenderFrameObserverHelper::RenderFrameObserverHelper(content::RenderFrame *render_frame)
    : content::RenderFrameObserver(render_frame)
{
}

UserResourceController::RenderViewObserverHelper::RenderViewObserverHelper(content::RenderView *render_view)
    : content::RenderViewObserver(render_view)
{
}

void UserResourceController::RenderFrameObserverHelper::DidCommitProvisionalLoad(bool /* is_new_navigation */,
                                                                                 bool is_same_document_navigation)
{
    if (is_same_document_navigation)
        return;

    // We are almost ready to run scripts. We still have to wait until the host
    // process has been notified of the DidCommitProvisionalLoad event to ensure
    // that the WebChannelTransportHost is ready to receive messages.

    m_runner.reset(new Runner(render_frame()->GetWebFrame()));

    base::ThreadTaskRunnerHandle::Get()->PostTask(
            FROM_HERE,
            base::BindOnce(&Runner::run, m_runner->AsWeakPtr(), UserScriptData::DocumentElementCreation));
}

void UserResourceController::RenderFrameObserverHelper::DidFinishDocumentLoad()
{
    // Don't run scripts if provisional load failed (DidFailProvisionalLoad
    // called instead of DidCommitProvisionalLoad).
    if (m_runner)
        base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
            FROM_HERE,
            base::BindOnce(&Runner::run, m_runner->AsWeakPtr(), UserScriptData::AfterLoad),
            base::TimeDelta::FromMilliseconds(afterLoadTimeout));

}

void UserResourceController::RenderFrameObserverHelper::DidFinishLoad()
{
    if (m_runner)
        base::ThreadTaskRunnerHandle::Get()->PostTask(
            FROM_HERE,
            base::BindOnce(&Runner::run, m_runner->AsWeakPtr(), UserScriptData::AfterLoad));
}

void UserResourceController::RenderFrameObserverHelper::FrameDetached()
{
    m_runner.reset();
}

void UserResourceController::RenderFrameObserverHelper::OnDestruct()
{
    delete this;
}

void UserResourceController::RenderViewObserverHelper::OnDestruct()
{
    // Remove all scripts associated with the render view.
    if (content::RenderView *view = render_view())
        UserResourceController::instance()->renderViewDestroyed(view);
    delete this;
}

bool UserResourceController::RenderFrameObserverHelper::OnMessageReceived(const IPC::Message &message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(UserResourceController::RenderFrameObserverHelper, message)
        IPC_MESSAGE_HANDLER(RenderFrameObserverHelper_AddScript, onUserScriptAdded)
        IPC_MESSAGE_HANDLER(RenderFrameObserverHelper_RemoveScript, onUserScriptRemoved)
        IPC_MESSAGE_HANDLER(RenderFrameObserverHelper_ClearScripts, onScriptsCleared)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
            return handled;
}

void UserResourceController::RenderFrameObserverHelper::onUserScriptAdded(const UserScriptData &script)
{
    if (content::RenderFrame *frame = render_frame())
        if (content::RenderView *view = frame->GetRenderView())
            UserResourceController::instance()->addScriptForView(script, view);
}

void UserResourceController::RenderFrameObserverHelper::onUserScriptRemoved(const UserScriptData &script)
{
    if (content::RenderFrame *frame = render_frame())
        if (content::RenderView *view = frame->GetRenderView())
            UserResourceController::instance()->removeScriptForView(script, view);
}

void UserResourceController::RenderFrameObserverHelper::onScriptsCleared()
{
    if (content::RenderFrame *frame = render_frame())
        if (content::RenderView *view = frame->GetRenderView())
            UserResourceController::instance()->clearScriptsForView(view);
}

UserResourceController *UserResourceController::instance()
{
    return qt_webengine_userResourceController();
}

bool UserResourceController::OnControlMessageReceived(const IPC::Message &message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(UserResourceController, message)
        IPC_MESSAGE_HANDLER(UserResourceController_AddScript, onAddScript)
        IPC_MESSAGE_HANDLER(UserResourceController_RemoveScript, onRemoveScript)
        IPC_MESSAGE_HANDLER(UserResourceController_ClearScripts, onClearScripts)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;
}

UserResourceController::UserResourceController()
{
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
    static bool onlyCalledOnce = true;
    DCHECK(onlyCalledOnce);
    onlyCalledOnce = false;
#endif // !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
}

void UserResourceController::renderFrameCreated(content::RenderFrame *renderFrame)
{
    // Will destroy itself when the RenderFrame is destroyed.
    new RenderFrameObserverHelper(renderFrame);
}

void UserResourceController::renderViewCreated(content::RenderView *renderView)
{
    // Will destroy itself when the RenderView is destroyed.
    new RenderViewObserverHelper(renderView);
}

void UserResourceController::renderViewDestroyed(content::RenderView *renderView)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(renderView);
    if (it == m_viewUserScriptMap.end()) // ASSERT maybe?
        return;
    for (uint64_t id : qAsConst(it.value())) {
        m_scripts.remove(id);
    }
    m_viewUserScriptMap.remove(renderView);
}

void UserResourceController::addScriptForView(const UserScriptData &script, content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        it = m_viewUserScriptMap.insert(view, UserScriptSet());

    (*it).insert(script.scriptId);
    m_scripts.insert(script.scriptId, script);
}

void UserResourceController::removeScriptForView(const UserScriptData &script, content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        return;

    (*it).remove(script.scriptId);
    m_scripts.remove(script.scriptId);
}

void UserResourceController::clearScriptsForView(content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        return;
    for (uint64_t id : qAsConst(it.value()))
        m_scripts.remove(id);

    m_viewUserScriptMap.remove(view);
}

void UserResourceController::onAddScript(const UserScriptData &script)
{
    addScriptForView(script, globalScriptsIndex);
}

void UserResourceController::onRemoveScript(const UserScriptData &script)
{
    removeScriptForView(script, globalScriptsIndex);
}

void UserResourceController::onClearScripts()
{
    clearScriptsForView(globalScriptsIndex);
}

