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

#ifndef WEB_UI_CONTROLLER_FACTORY_QT_H_
#define WEB_UI_CONTROLLER_FACTORY_QT_H_

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "components/favicon_base/favicon_callback.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_controller_factory.h"
#include "ui/base/layout.h"

class Profile;

namespace base {
class RefCountedMemory;
}

namespace QtWebEngineCore {

class WebUIControllerFactoryQt : public content::WebUIControllerFactory {
public:
    content::WebUI::TypeID GetWebUIType(content::BrowserContext *browserContext, const GURL &url) const override;
    bool UseWebUIForURL(content::BrowserContext *browserContext, const GURL &url) const override;
    bool UseWebUIBindingsForURL(content::BrowserContext *browserContext, const GURL &url) const override;
    std::unique_ptr<content::WebUIController> CreateWebUIControllerForURL(content::WebUI *webUi, const GURL &url) const override;

    static WebUIControllerFactoryQt *GetInstance();

protected:
    WebUIControllerFactoryQt();
    ~WebUIControllerFactoryQt() override;

private:
    friend struct base::DefaultSingletonTraits<WebUIControllerFactoryQt>;

    DISALLOW_COPY_AND_ASSIGN(WebUIControllerFactoryQt);
};

} // namespace QtWebEngineCore

#endif  // WEB_UI_CONTROLLER_FACTORY_QT_H_
