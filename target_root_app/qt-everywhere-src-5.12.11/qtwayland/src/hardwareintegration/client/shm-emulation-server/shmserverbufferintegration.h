/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef SHMSERVERBUFFERINTEGRATION_H
#define SHMSERVERBUFFERINTEGRATION_H

#include <QtWaylandClient/private/qwayland-wayland.h>
#include "qwayland-shm-emulation-server-buffer.h"
#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>

#include "shmserverbufferintegration.h"
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtCore/QTextStream>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class ShmServerBufferIntegration;

class ShmServerBuffer : public QWaylandServerBuffer
{
public:
    ShmServerBuffer(const QString &key, const QSize &size, int bytesPerLine, QWaylandServerBuffer::Format format);
    ~ShmServerBuffer() override;
    QOpenGLTexture* toOpenGlTexture() override;
private:
    QOpenGLTexture *m_texture = nullptr;
    QString m_key;
    int m_bpl;
};

class ShmServerBufferIntegration
    : public QWaylandServerBufferIntegration
    , public QtWayland::qt_shm_emulation_server_buffer
{
public:
    void initialize(QWaylandDisplay *display) override;

    QWaylandServerBuffer *serverBuffer(struct qt_server_buffer *buffer) override;

protected:
    void shm_emulation_server_buffer_server_buffer_created(qt_server_buffer *id, const QString &key, int32_t width, int32_t height, int32_t bytes_per_line, int32_t format) override;

private:
    static void wlDisplayHandleGlobal(void *data, struct ::wl_registry *registry, uint32_t id,
                                      const QString &interface, uint32_t version);
    QWaylandDisplay *m_display = nullptr;
};

}

QT_END_NAMESPACE

#endif
