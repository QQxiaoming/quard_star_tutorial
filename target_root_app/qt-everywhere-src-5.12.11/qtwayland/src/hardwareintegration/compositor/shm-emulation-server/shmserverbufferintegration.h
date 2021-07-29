/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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

#ifndef SHMSERVERBUFFERINTEGRATION_H
#define SHMSERVERBUFFERINTEGRATION_H

#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include "qwayland-server-shm-emulation-server-buffer.h"

#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>

#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/private/qwayland-server-server-buffer-extension.h>

QT_BEGIN_NAMESPACE

class ShmServerBufferIntegration;
class QSharedMemory;

class ShmServerBuffer : public QtWayland::ServerBuffer, public QtWaylandServer::qt_server_buffer
{
public:
    ShmServerBuffer(ShmServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format);
    ~ShmServerBuffer() override;

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    bool bufferInUse() override;
    QOpenGLTexture *toOpenGlTexture() override;

private:
    ShmServerBufferIntegration *m_integration = nullptr;

    QSharedMemory *m_shm = nullptr;
    int m_width;
    int m_height;
    int m_bpl;
    QOpenGLTexture *m_texture = nullptr;
    QtWaylandServer::qt_shm_emulation_server_buffer::format m_shm_format;
};

class ShmServerBufferIntegration :
    public QtWayland::ServerBufferIntegration,
    public QtWaylandServer::qt_shm_emulation_server_buffer
{
public:
    ShmServerBufferIntegration();
    ~ShmServerBufferIntegration() override;

    void initializeHardware(QWaylandCompositor *) override;

    bool supportsFormat(QtWayland::ServerBuffer::Format format) const override;
    QtWayland::ServerBuffer *createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format) override;


private:
};

QT_END_NAMESPACE

#endif
