/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef TEXTURESHARINGEXTENSION_H
#define TEXTURESHARINGEXTENSION_H

#include <qpa/qwindowsysteminterface.h>
#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/qwaylandclientextension.h>
#include "qwayland-qt-texture-sharing-unstable-v1.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {
    class QWaylandServerBuffer;
    class QWaylandServerBufferIntegration;
};

class TextureSharingExtension : public QWaylandClientExtensionTemplate<TextureSharingExtension>
        , public QtWayland::zqt_texture_sharing_v1
{
    Q_OBJECT
public:
    TextureSharingExtension();

public slots:
    void requestImage(const QString &key);
    void abandonImage(const QString &key);

signals:
    void bufferReceived(QtWaylandClient::QWaylandServerBuffer *buffer, const QString &key);

private:
    void zqt_texture_sharing_v1_provide_buffer(struct ::qt_server_buffer *buffer, const QString &key) override;
    void zqt_texture_sharing_v1_image_failed(const QString &key, const QString &message) override;
    QtWaylandClient::QWaylandServerBufferIntegration *m_server_buffer_integration = nullptr;
};

QT_END_NAMESPACE

#endif // TEXTURESHARINGEXTENSION_H
