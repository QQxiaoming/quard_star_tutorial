/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef VULKANSERVERBUFFERINTEGRATION_H
#define VULKANSERVERBUFFERINTEGRATION_H

#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include "qwayland-server-qt-vulkan-server-buffer-unstable-v1.h"

#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>

#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/private/qwayland-server-server-buffer-extension.h>

QT_BEGIN_NAMESPACE

class VulkanServerBufferIntegration;
class VulkanWrapper;
struct VulkanImageWrapper;

class VulkanServerBuffer : public QtWayland::ServerBuffer, public QtWaylandServer::qt_server_buffer
{
public:
    VulkanServerBuffer(VulkanServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format);
    VulkanServerBuffer(VulkanServerBufferIntegration *integration, VulkanImageWrapper *vImage, uint glInternalFormat, const QSize &size);
    ~VulkanServerBuffer() override;

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    bool bufferInUse() override;
    QOpenGLTexture *toOpenGlTexture() override;
    void releaseOpenGlTexture() override;

protected:
    void server_buffer_release(Resource *resource) override;

private:
    VulkanServerBufferIntegration *m_integration = nullptr;

    int m_width;
    int m_height;
    int m_memorySize;
    int m_fd = -1;
    VulkanImageWrapper *m_vImage = nullptr;
    QOpenGLTexture *m_texture = nullptr;
    uint m_glInternalFormat;
    GLuint m_memoryObject;
};

class VulkanServerBufferIntegration :
    public QtWayland::ServerBufferIntegration,
    public QtWaylandServer::zqt_vulkan_server_buffer_v1
{
public:
    VulkanServerBufferIntegration();
    ~VulkanServerBufferIntegration() override;

    VulkanWrapper *vulkanWrapper() const { return m_vulkanWrapper; }

    void initializeHardware(QWaylandCompositor *) override;

    bool supportsFormat(QtWayland::ServerBuffer::Format format) const override;
    QtWayland::ServerBuffer *createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format) override;
    QtWayland::ServerBuffer *createServerBufferFromData(const QByteArray &data, const QSize &size, uint glInternalFormat) override;

private:
    VulkanWrapper *m_vulkanWrapper = nullptr;
};

QT_END_NAMESPACE

#endif
