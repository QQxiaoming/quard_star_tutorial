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

#ifndef SHAREDTEXTUREPROVIDER_H
#define SHAREDTEXTUREPROVIDER_H

#include <QOpenGLFunctions>
#include <QQuickImageProvider>
#include <QtQuick/QSGTexture>
#include <QScopedPointer>
#include <QHash>

#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>

QT_BEGIN_NAMESPACE

class TextureSharingExtension;

class SharedTextureRegistry : public QObject
{
    Q_OBJECT
public:
    SharedTextureRegistry();
    ~SharedTextureRegistry() override;

    const QtWaylandClient::QWaylandServerBuffer *bufferForId(const QString &id) const;
    void requestBuffer(const QString &id);
    void abandonBuffer(const QString &id);

    static bool preinitialize();

public slots:
    void receiveBuffer(QtWaylandClient::QWaylandServerBuffer *buffer, const QString &id);

signals:
    void replyReceived(const QString &id);

private slots:
    void handleExtensionActive();

private:
    TextureSharingExtension *m_extension = nullptr;
    QHash<QString, QtWaylandClient::QWaylandServerBuffer *> m_buffers;
    QStringList m_pendingBuffers;
};

class SharedTextureProvider : public QQuickAsyncImageProvider
{
public:
    SharedTextureProvider();
    ~SharedTextureProvider() override;

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    SharedTextureRegistry *m_registry = nullptr;
    bool m_sharingAvailable = false;
};

class SharedTexture : public QSGTexture
{
    Q_OBJECT
public:
    SharedTexture(QtWaylandClient::QWaylandServerBuffer *buffer);

    int textureId() const override;
    QSize textureSize() const override;
    bool hasAlphaChannel() const override;
    bool hasMipmaps() const override;

    void bind() override;

private:
    void updateGLTexture() const;
    QtWaylandClient::QWaylandServerBuffer *m_buffer = nullptr;
    mutable QOpenGLTexture *m_tex = nullptr;
};


QT_END_NAMESPACE

#endif // SHAREDTEXTUREPROVIDER_H
