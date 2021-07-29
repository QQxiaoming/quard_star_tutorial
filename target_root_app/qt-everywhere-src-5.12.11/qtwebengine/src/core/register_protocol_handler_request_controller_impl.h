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

#ifndef REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H
#define REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H

#include "register_protocol_handler_request_controller.h"

#include "chrome/browser/custom_handlers/protocol_handler_registry.h"
#include "chrome/common/custom_handlers/protocol_handler.h"
#include "content/public/browser/web_contents_observer.h"

class ProtocolHandlerRegistry;

namespace QtWebEngineCore {

class RegisterProtocolHandlerRequestControllerImpl final : public RegisterProtocolHandlerRequestController,
                                                           private content::WebContentsObserver {
public:
    RegisterProtocolHandlerRequestControllerImpl(
        content::WebContents *webContents,
        ProtocolHandler handler);

    ~RegisterProtocolHandlerRequestControllerImpl();

protected:
    void accepted() override;
    void rejected() override;

private:
    ProtocolHandlerRegistry *protocolHandlerRegistry();
    ProtocolHandler m_handler;
};

} // namespace QtWebEngineCore

#endif // REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H
