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


#include "sharedtextureprovider.h"

#include <QFile>
#include <QDebug>
#include <qopenglfunctions.h>
#include <QQuickWindow>

#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>
#include <QtGui/QGuiApplication>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QWindow>
#include <QOpenGLTexture>
#include <QImageReader>

#include <QTimer>

#include "texturesharingextension.h"

QT_BEGIN_NAMESPACE

SharedTexture::SharedTexture(QtWaylandClient::QWaylandServerBuffer *buffer)
    : m_buffer(buffer), m_tex(nullptr)
{
}

int SharedTexture::textureId() const
{
    updateGLTexture();
    return m_tex ? m_tex->textureId() : 0;
}

QSize SharedTexture::textureSize() const
{
    updateGLTexture();
    return m_tex ? QSize(m_tex->width(), m_tex->height()) : QSize();
}

bool SharedTexture::hasAlphaChannel() const
{
    return true;
}

bool SharedTexture::hasMipmaps() const
{
    updateGLTexture();
    return m_tex ? (m_tex->mipLevels() > 1) : false;
}

void SharedTexture::bind()
{
    updateGLTexture();
    if (m_tex)
        m_tex->bind();
}

inline void SharedTexture::updateGLTexture() const
{
    if (!m_tex && m_buffer)
        m_tex = m_buffer->toOpenGlTexture();
}

class SharedTextureFactory : public QQuickTextureFactory
{
public:
    SharedTextureFactory(const QtWaylandClient::QWaylandServerBuffer *buffer, const QString &id, SharedTextureRegistry *registry)
        : m_buffer(buffer), m_id(id), m_registry(registry)
    {
    }

    ~SharedTextureFactory() override
    {
        //qDebug() << "====> DESTRUCTOR SharedTextureFactory" << this;
        if (m_registry)
            m_registry->abandonBuffer(m_id);
        delete m_buffer; // TODO: make sure we are not keeping references to this elsewhere
        //qDebug() << "buffer deleted";
    }

    QSize textureSize() const override
    {
        return m_buffer ? m_buffer->size() : QSize();
    }

    int textureByteCount() const override
    {
        return m_buffer ? (m_buffer->size().width() * m_buffer->size().height() * 4) : 0;
    }

    QSGTexture *createTexture(QQuickWindow *) const override
    {
        return new SharedTexture(const_cast<QtWaylandClient::QWaylandServerBuffer *>(m_buffer));
    }

private:
    const QtWaylandClient::QWaylandServerBuffer *m_buffer = nullptr;
    QString m_id;
    QPointer<SharedTextureRegistry> m_registry;
};


SharedTextureRegistry::SharedTextureRegistry()
    : m_extension(new TextureSharingExtension)
{
    connect(m_extension, &TextureSharingExtension::bufferReceived, this, &SharedTextureRegistry::receiveBuffer);
    connect(m_extension, &TextureSharingExtension::activeChanged, this, &SharedTextureRegistry::handleExtensionActive);
}

SharedTextureRegistry::~SharedTextureRegistry()
{
    delete m_extension;
}

const QtWaylandClient::QWaylandServerBuffer *SharedTextureRegistry::bufferForId(const QString &id) const
{
    return m_buffers.value(id);
}

void SharedTextureRegistry::requestBuffer(const QString &id)
{
    if (!m_extension->isActive()) {
        //qDebug() << "Extension not active, adding" << id << "to queue";
        m_pendingBuffers << id;
        return;
    }
    m_extension->requestImage(id);
}

void SharedTextureRegistry::abandonBuffer(const QString &id)
{
    m_buffers.remove(id);
    m_extension->abandonImage(id);
}


void SharedTextureRegistry::handleExtensionActive()
{
    //qDebug() << "handleExtensionActive, queue:" << m_pendingBuffers;
    if (m_extension->isActive())
        while (!m_pendingBuffers.isEmpty())
            requestBuffer(m_pendingBuffers.takeFirst());
}

bool SharedTextureRegistry::preinitialize()
{
    auto *serverBufferIntegration = QGuiApplicationPrivate::platformIntegration()->nativeInterface()->nativeResourceForIntegration("server_buffer_integration");

    if (!serverBufferIntegration) {
        qWarning() << "Wayland Server Buffer Integration not available.";
        return false;
    }

    return true;
}

void SharedTextureRegistry::receiveBuffer(QtWaylandClient::QWaylandServerBuffer *buffer, const QString& id)
{
    //qDebug() << "ReceiveBuffer for id" << id;
    if (buffer)
        m_buffers.insert(id, buffer);
    emit replyReceived(id);
}

class SharedTextureImageResponse : public QQuickImageResponse
{
    Q_OBJECT
public:
    SharedTextureImageResponse(SharedTextureRegistry *registry, const QString &id)
        : m_id(id), m_registry(registry)
    {
        if (!m_registry || m_registry->bufferForId(id)) {
            // Shortcut: no server roundtrip needed, just let the event loop call the slot
            QMetaObject::invokeMethod(this, "doResponse", Qt::QueuedConnection, Q_ARG(QString, id));

        } else {
            // TBD: timeout?
            connect(registry, &SharedTextureRegistry::replyReceived, this, &SharedTextureImageResponse::doResponse);
            registry->requestBuffer(id);
        }
    }

    QQuickTextureFactory *textureFactory() const override
    {
        if (m_registry) {
            const QtWaylandClient::QWaylandServerBuffer *buffer = m_registry->bufferForId(m_id);
            if (buffer) {
                //qDebug() << "Creating shared buffer texture for" << m_id;
                return new SharedTextureFactory(buffer, m_id, m_registry);
            }
            //qDebug() << "Shared buffer NOT found for" << m_id;
        }

        // No shared buffer, do fallback
        QString fbPath = fallbackPath();
        if (fbPath.isEmpty()) {
            m_errorString = QStringLiteral("Shared buffer not found, and no fallback path set.");
            return nullptr;
        }

        QImageReader reader(fbPath + m_id);
        QImage img = reader.read();
        if (img.isNull()) {
            qWarning() << "Could not load local image from id/path" << reader.fileName();
            m_errorString = QStringLiteral("Shared buffer not found, and fallback local file loading failed: ") + reader.errorString();
            return nullptr;
        }
        return QQuickTextureFactory::textureFactoryForImage(img);
    }

    QString errorString() const override
    {
        return m_errorString;
    }

    static QString fallbackPath()
    {
        static QString fbPath;
        static bool isInit = false;
        if (!isInit) {
            isInit = true;
            QByteArray envVal = qgetenv("QT_SHAREDTEXTURE_FALLBACK_DIR");
            if (!envVal.isEmpty()) {
                fbPath = QString::fromLocal8Bit(envVal);
                if (!fbPath.endsWith(QLatin1Char('/')))
                    fbPath.append(QLatin1Char('/'));
            }
        }
        return fbPath;
    }


public slots:
    void doResponse(const QString &key) {
        if (key != m_id)
            return; // not our buffer

        // No need to be called again
        if (m_registry)
            disconnect(m_registry, &SharedTextureRegistry::replyReceived, this, &SharedTextureImageResponse::doResponse);

        emit finished();
    }

private:
    QString m_id;
    SharedTextureRegistry *m_registry = nullptr;
    mutable QString m_errorString;
};


SharedTextureProvider::SharedTextureProvider()
{
    m_sharingAvailable = SharedTextureRegistry::preinitialize();
    if (!m_sharingAvailable) {
        if (SharedTextureImageResponse::fallbackPath().isEmpty())
            qWarning() << "Shared buffer images not available, and no fallback directory set.";
        else
            qWarning() << "Shared buffer images not available, will fallback to local image files from" << SharedTextureImageResponse::fallbackPath();
    }
}

SharedTextureProvider::~SharedTextureProvider()
{
    delete m_registry;
}

QQuickImageResponse *SharedTextureProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);

    //qDebug() << "Provider: got request for" << id;

    if (m_sharingAvailable && !m_registry)
        m_registry = new SharedTextureRegistry;

    return new SharedTextureImageResponse(m_registry, id);
}

QT_END_NAMESPACE

#include "sharedtextureprovider.moc"
