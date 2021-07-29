/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
#include "register_protocol_handler_request_controller_impl.h"

#include "chrome/browser/custom_handlers/protocol_handler_registry_factory.h"
#include "content/public/browser/web_contents.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

RegisterProtocolHandlerRequestControllerImpl::RegisterProtocolHandlerRequestControllerImpl(
    content::WebContents *webContents,
    ProtocolHandler handler)
    : RegisterProtocolHandlerRequestController(
        toQt(handler.url()),
        toQt(handler.protocol()))
    , content::WebContentsObserver(webContents)
    , m_handler(handler)
{}

RegisterProtocolHandlerRequestControllerImpl::~RegisterProtocolHandlerRequestControllerImpl()
{
    reject();
}

ProtocolHandlerRegistry *RegisterProtocolHandlerRequestControllerImpl::protocolHandlerRegistry()
{
    content::WebContents *webContents = web_contents();
    if (!webContents)
        return nullptr;
    content::BrowserContext *context = webContents->GetBrowserContext();
    return ProtocolHandlerRegistryFactory::GetForBrowserContext(context);
}

void RegisterProtocolHandlerRequestControllerImpl::accepted()
{
    if (ProtocolHandlerRegistry *registry = protocolHandlerRegistry())
        registry->OnAcceptRegisterProtocolHandler(m_handler);
}

void RegisterProtocolHandlerRequestControllerImpl::rejected()
{
    if (ProtocolHandlerRegistry *registry = protocolHandlerRegistry())
        registry->OnIgnoreRegisterProtocolHandler(m_handler);
}

} // namespace QtWebEngineCore
