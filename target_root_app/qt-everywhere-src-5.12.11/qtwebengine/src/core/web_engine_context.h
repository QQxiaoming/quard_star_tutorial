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

#ifndef WEB_ENGINE_CONTEXT_H
#define WEB_ENGINE_CONTEXT_H

#include "build_config_qt.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"
#include <QVector>

namespace base {
class RunLoop;
}

namespace content {
class BrowserMainRunner;
class ContentMainRunner;
}

#if QT_CONFIG(webengine_printing_and_pdf)
namespace printing {
class PrintJobManager;
}
#endif

QT_FORWARD_DECLARE_CLASS(QObject)

namespace QtWebEngineCore {

class ProfileAdapter;
class ContentMainDelegateQt;
class DevToolsServerQt;

bool usingSoftwareDynamicGL();

class WebEngineContext : public base::RefCounted<WebEngineContext> {
public:
    static WebEngineContext *current();
    static void destroyContextPostRoutine();

    ProfileAdapter *createDefaultProfileAdapter();
    ProfileAdapter *defaultProfileAdapter();

    QObject *globalQObject();
#if QT_CONFIG(webengine_printing_and_pdf)
    printing::PrintJobManager* getPrintJobManager();
#endif
    void destroyProfileAdapter();
    void addProfileAdapter(ProfileAdapter *profileAdapter);
    void removeProfileAdapter(ProfileAdapter *profileAdapter);
    void destroy();

private:
    friend class base::RefCounted<WebEngineContext>;
    friend class ProfileAdapter;
    WebEngineContext();
    ~WebEngineContext();

    std::unique_ptr<base::RunLoop> m_runLoop;
    std::unique_ptr<ContentMainDelegateQt> m_mainDelegate;
    std::unique_ptr<content::ContentMainRunner> m_contentRunner;
    std::unique_ptr<content::BrowserMainRunner> m_browserRunner;
    std::unique_ptr<QObject> m_globalQObject;
    std::unique_ptr<ProfileAdapter> m_defaultProfileAdapter;
    std::unique_ptr<DevToolsServerQt> m_devtoolsServer;
    QVector<ProfileAdapter*> m_profileAdapters;

#if QT_CONFIG(webengine_printing_and_pdf)
    std::unique_ptr<printing::PrintJobManager> m_printJobManager;
#endif
    static scoped_refptr<QtWebEngineCore::WebEngineContext> m_handle;
    static bool m_destroyed;
};

} // namespace

#endif // WEB_ENGINE_CONTEXT_H
