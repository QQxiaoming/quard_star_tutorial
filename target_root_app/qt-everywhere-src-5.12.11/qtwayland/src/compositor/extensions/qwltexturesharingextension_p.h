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

#ifndef QWLTEXTURESHARINGEXTENSION_P_H
#define QWLTEXTURESHARINGEXTENSION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "wayland-util.h"

#include <QtCore/QMap>
#include <QtCore/QHash>

#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QQuickImageProvider>

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include <QtWaylandCompositor/private/qwayland-server-qt-texture-sharing-unstable-v1.h>

QT_BEGIN_NAMESPACE

namespace QtWayland
{
    class ServerBufferIntegration;
}

class QWaylandTextureSharingExtension;
class SharedTextureImageResponse;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandSharedTextureProvider : public QQuickAsyncImageProvider
{
public:
    QWaylandSharedTextureProvider();
    ~QWaylandSharedTextureProvider() override;

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    void setExtensionReady(QWaylandTextureSharingExtension *extension);

private:
    QVector<SharedTextureImageResponse*> m_pendingResponses;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandTextureSharingExtension
    : public QWaylandCompositorExtensionTemplate<QWaylandTextureSharingExtension>
    , public QtWaylandServer::zqt_texture_sharing_v1
{
    Q_OBJECT
    Q_PROPERTY(QString imageSearchPath WRITE setImageSearchPath)
public:
    QWaylandTextureSharingExtension();
    QWaylandTextureSharingExtension(QWaylandCompositor *compositor);
    ~QWaylandTextureSharingExtension() override;

    void initialize() override;

    void setImageSearchPath(const QString &path);

    static QWaylandTextureSharingExtension *self() { return s_self; }

public slots:
    void requestBuffer(const QString &key);

signals:
     void bufferResult(const QString &key, QtWayland::ServerBuffer *buffer);

protected slots:
    void cleanupBuffers();

protected:
    void zqt_texture_sharing_v1_request_image(Resource *resource, const QString &key) override;
    void zqt_texture_sharing_v1_abandon_image(Resource *resource, const QString &key) override;
    void zqt_texture_sharing_v1_destroy_resource(Resource *resource) override;

    virtual bool customPixelData(const QString &key, QByteArray *data, QSize *size, uint *glInternalFormat)
    {
        Q_UNUSED(key);
        Q_UNUSED(data);
        Q_UNUSED(size);
        Q_UNUSED(glInternalFormat);
        return false;
    }

private:
    QtWayland::ServerBuffer *getBuffer(const QString &key);
    bool initServerBufferIntegration();
    QtWayland::ServerBuffer *getCompressedBuffer(const QString &key);
    QString getExistingFilePath(const QString &key) const;
    void dumpBufferInfo();

    struct BufferInfo
    {
        BufferInfo(QtWayland::ServerBuffer *b = nullptr) : buffer(b) {}
        QtWayland::ServerBuffer *buffer = nullptr;
        bool usedLocally = false;
    };

    QStringList m_image_dirs;
    QStringList m_image_suffixes;
    QHash<QString, BufferInfo> m_server_buffers;
    QtWayland::ServerBufferIntegration *m_server_buffer_integration = nullptr;

    static QWaylandTextureSharingExtension *s_self;
};

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandTextureSharingExtension)

QT_END_NAMESPACE

#endif // QWLTEXTURESHARINGEXTENSION_P_H
