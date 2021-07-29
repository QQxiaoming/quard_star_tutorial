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

#ifndef VULKANSERVERBUFFERINTEGRATION_H
#define VULKANSERVERBUFFERINTEGRATION_H

#include <QtWaylandClient/private/qwayland-wayland.h>
#include "qwayland-qt-vulkan-server-buffer-unstable-v1.h"
#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>

#include "vulkanserverbufferintegration.h"
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtCore/QTextStream>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class VulkanServerBufferIntegration;

class VulkanServerBuffer : public QWaylandServerBuffer
{
public:
    VulkanServerBuffer(VulkanServerBufferIntegration *integration, struct ::qt_server_buffer *id, int32_t fd, uint32_t width, uint32_t height, uint32_t memory_size, uint32_t format);
    ~VulkanServerBuffer() override;
    QOpenGLTexture* toOpenGlTexture() override;

private:
    void import();

    VulkanServerBufferIntegration *m_integration = nullptr;
    struct ::qt_server_buffer *m_server_buffer = nullptr;
    QOpenGLTexture *m_texture = nullptr;
    int m_fd = -1;
    uint m_memorySize = 0;
    uint m_internalFormat = 0;
    GLuint m_memoryObject = 0;
};

class VulkanServerBufferIntegration
    : public QWaylandServerBufferIntegration
    , public QtWayland::zqt_vulkan_server_buffer_v1
{
public:
    void initialize(QWaylandDisplay *display) override;

    QWaylandServerBuffer *serverBuffer(struct qt_server_buffer *buffer) override;

    void deleteGLTextureWhenPossible(QOpenGLTexture *texture) { orphanedTextures << texture; }
    void deleteOrphanedTextures();

protected:
    void zqt_vulkan_server_buffer_v1_server_buffer_created(qt_server_buffer *id, int32_t fd, uint32_t width, uint32_t height, uint32_t memory_size, uint32_t format) override;

private:
    static void wlDisplayHandleGlobal(void *data, struct ::wl_registry *registry, uint32_t id,
                                      const QString &interface, uint32_t version);
    QWaylandDisplay *m_display = nullptr;
    QVector<QOpenGLTexture *> orphanedTextures;
};

}

QT_END_NAMESPACE

#endif
