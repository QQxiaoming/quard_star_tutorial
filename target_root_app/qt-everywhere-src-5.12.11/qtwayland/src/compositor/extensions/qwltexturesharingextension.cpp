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

#include "qwltexturesharingextension_p.h"

#include <QWaylandSurface>

#include <QDebug>

#include <QQuickWindow>

#include <QPainter>
#include <QPen>
#include <QTimer>

#include <QtGui/private/qtexturefilereader_p.h>
#include <QtGui/QOpenGLTexture>
#include <QtGui/QImageReader>

#include <QtQuick/QSGTexture>
#include <QQmlContext>
#include <QThread>

QT_BEGIN_NAMESPACE

class SharedTexture : public QSGTexture
{
    Q_OBJECT
public:
    SharedTexture(QtWayland::ServerBuffer *buffer);

    int textureId() const override;
    QSize textureSize() const override;
    bool hasAlphaChannel() const override;
    bool hasMipmaps() const override;

    void bind() override;

private:
    void updateGLTexture() const;
    QtWayland::ServerBuffer *m_buffer = nullptr;
    mutable QOpenGLTexture *m_tex = nullptr;
};

SharedTexture::SharedTexture(QtWayland::ServerBuffer *buffer)
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
    SharedTextureFactory(const QtWayland::ServerBuffer *buffer)
        : m_buffer(buffer)
    {
    }

    ~SharedTextureFactory() override
    {
        if (m_buffer && !QCoreApplication::closingDown())
            const_cast<QtWayland::ServerBuffer*>(m_buffer)->releaseOpenGlTexture();
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
        return new SharedTexture(const_cast<QtWayland::ServerBuffer *>(m_buffer));
    }

private:
    const QtWayland::ServerBuffer *m_buffer = nullptr;
};

class SharedTextureImageResponse : public QQuickImageResponse
{
    Q_OBJECT
public:
    SharedTextureImageResponse(QWaylandTextureSharingExtension *extension, const QString &id)
        : m_id(id)
    {
        if (extension)
            doRequest(extension);
    }

    void doRequest(QWaylandTextureSharingExtension *extension)
    {
        m_extension = extension;
        connect(extension, &QWaylandTextureSharingExtension::bufferResult, this, &SharedTextureImageResponse::doResponse);
        QMetaObject::invokeMethod(extension, [this] { m_extension->requestBuffer(m_id); }, Qt::AutoConnection);
    }

    QQuickTextureFactory *textureFactory() const override
    {
        if (m_buffer) {
//            qDebug() << "Creating shared buffer texture for" << m_id;
            return new SharedTextureFactory(m_buffer);
        }
//        qDebug() << "Shared buffer NOT found for" << m_id;
        m_errorString = QLatin1Literal("Shared buffer not found");
        return nullptr;
    }

    QString errorString() const override
    {
        return m_errorString;
    }

public slots:
    void doResponse(const QString &key, QtWayland::ServerBuffer *buffer)
    {
        if (key != m_id)
            return; //somebody else's texture

        m_buffer = buffer;

        if (m_extension)
            disconnect(m_extension, &QWaylandTextureSharingExtension::bufferResult, this, &SharedTextureImageResponse::doResponse);

        emit finished();
    }

private:
    QString m_id;
    QWaylandTextureSharingExtension *m_extension = nullptr;
    mutable QString m_errorString;
    QtWayland::ServerBuffer *m_buffer = nullptr;
};

QWaylandSharedTextureProvider::QWaylandSharedTextureProvider()
{
}

QWaylandSharedTextureProvider::~QWaylandSharedTextureProvider()
{
}

QQuickImageResponse *QWaylandSharedTextureProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);

//    qDebug() << "Provider: got request for" << id;

    auto *extension = QWaylandTextureSharingExtension::self();
    auto *response = new SharedTextureImageResponse(extension, id);
    if (!extension)
        m_pendingResponses << response;

    return response;
}

void QWaylandSharedTextureProvider::setExtensionReady(QWaylandTextureSharingExtension *extension)
{
    for (auto *response : qAsConst(m_pendingResponses))
        response->doRequest(extension);
    m_pendingResponses.clear();
    m_pendingResponses.squeeze();
}

QWaylandTextureSharingExtension *QWaylandTextureSharingExtension::s_self = nullptr; // theoretical race conditions, but OK as long as we don't delete it while we are running

QWaylandTextureSharingExtension::QWaylandTextureSharingExtension()
{
    s_self = this;
}

QWaylandTextureSharingExtension::QWaylandTextureSharingExtension(QWaylandCompositor *compositor)
    :QWaylandCompositorExtensionTemplate(compositor)
{
    s_self = this;
}

QWaylandTextureSharingExtension::~QWaylandTextureSharingExtension()
{
    //qDebug() << Q_FUNC_INFO;
    //dumpBufferInfo();

    for (auto b : m_server_buffers)
        delete b.buffer;

    if (s_self == this)
        s_self = nullptr;
}

void QWaylandTextureSharingExtension::setImageSearchPath(const QString &path)
{
    m_image_dirs = path.split(QLatin1Char(';'));

    for (auto it = m_image_dirs.begin(); it != m_image_dirs.end(); ++it)
        if (!(*it).endsWith(QLatin1Char('/')))
            (*it) += QLatin1Char('/');
}

void QWaylandTextureSharingExtension::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    init(compositor->display(), 1);

    QString image_search_path = qEnvironmentVariable("QT_WAYLAND_SHAREDTEXTURE_SEARCH_PATH");
    if (!image_search_path.isEmpty())
        setImageSearchPath(image_search_path);

    if (m_image_dirs.isEmpty())
        m_image_dirs << QLatin1Literal(":/") << QLatin1Literal("./");

    auto suffixes = QTextureFileReader::supportedFileFormats();
    suffixes.append(QImageReader::supportedImageFormats());
    for (auto ext : qAsConst(suffixes))
        m_image_suffixes << QLatin1Char('.') + QString::fromLatin1(ext);

    //qDebug() << "m_image_suffixes" << m_image_suffixes << "m_image_dirs" << m_image_dirs;

    auto *ctx = QQmlEngine::contextForObject(this);
    if (ctx) {
        QQmlEngine *engine = ctx->engine();
        if (engine) {
            auto *provider = static_cast<QWaylandSharedTextureProvider*>(engine->imageProvider(QLatin1Literal("wlshared")));
            if (provider)
                provider->setExtensionReady(this);
        }
    }
}

QString QWaylandTextureSharingExtension::getExistingFilePath(const QString &key) const
{
    // The default search path blocks absolute pathnames, but this does not prevent relative
    // paths containing '../'. We handle that here, at the price of also blocking directory
    // names ending with two or more dots.

    if (key.contains(QLatin1Literal("../")))
        return QString();

    for (auto dir : m_image_dirs) {
        QString path = dir + key;
        if (QFileInfo::exists(path))
            return path;
    }

    for (auto dir : m_image_dirs) {
        for (auto ext : m_image_suffixes) {
            QString fp = dir + key + ext;
            //qDebug() << "trying" << fp;
            if (QFileInfo::exists(fp))
                return fp;
        }
    }
    return QString();
}

QtWayland::ServerBuffer *QWaylandTextureSharingExtension::getBuffer(const QString &key)
{
    if (!initServerBufferIntegration())
        return nullptr;

//qDebug() << "getBuffer" << key;

    QtWayland::ServerBuffer *buffer = nullptr;

    if ((buffer = m_server_buffers.value(key).buffer))
        return buffer;

    QByteArray pixelData;
    QSize size;
    uint glInternalFormat = GL_NONE;

    if (customPixelData(key, &pixelData, &size, &glInternalFormat)) {
        if (!pixelData.isEmpty()) {
            buffer = m_server_buffer_integration->createServerBufferFromData(pixelData, size, glInternalFormat);
            if (!buffer)
                qWarning() << "QWaylandTextureSharingExtension: could not create buffer from custom data for key:" << key;
        }
    } else {
        QString pathName = getExistingFilePath(key);
        //qDebug() << "pathName" << pathName;
        if (pathName.isEmpty())
            return nullptr;

        buffer = getCompressedBuffer(pathName);
        //qDebug() << "getCompressedBuffer" << buffer;

        if (!buffer) {
            QImage img(pathName);
            if (!img.isNull()) {
                img = img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
                buffer = m_server_buffer_integration->createServerBufferFromImage(img, QtWayland::ServerBuffer::RGBA32);
            }
            //qDebug() << "createServerBufferFromImage" << buffer;
        }
    }
    if (buffer)
        m_server_buffers.insert(key, BufferInfo(buffer));

    //qDebug() << ">>>>" << key << buffer;

    return buffer;
}

// Compositor requesting image for its own UI
void QWaylandTextureSharingExtension::requestBuffer(const QString &key)
{
    //qDebug() << "requestBuffer" << key;

    if (thread() != QThread::currentThread())
        qWarning("QWaylandTextureSharingExtension::requestBuffer() called from outside main thread: possible race condition");

    auto *buffer = getBuffer(key);

    if (buffer)
        m_server_buffers[key].usedLocally = true;

    //dumpBufferInfo();

    emit bufferResult(key, buffer);
}

void QWaylandTextureSharingExtension::zqt_texture_sharing_v1_request_image(Resource *resource, const QString &key)
{
    //qDebug() << "texture_sharing_request_image" << key;
    auto *buffer = getBuffer(key);
    if (buffer) {
        struct ::wl_client *client = resource->client();
        struct ::wl_resource *buffer_resource = buffer->resourceForClient(client);
        //qDebug() << "          server_buffer resource" << buffer_resource;
        if (buffer_resource)
            send_provide_buffer(resource->handle, buffer_resource, key);
        else
            qWarning() << "QWaylandTextureSharingExtension: no buffer resource for client";
    } else {
        send_image_failed(resource->handle, key, QString());
    }
    //dumpBufferInfo();
}

void QWaylandTextureSharingExtension::zqt_texture_sharing_v1_abandon_image(Resource *resource, const QString &key)
{
    Q_UNUSED(resource);
    Q_UNUSED(key);
//    qDebug() << Q_FUNC_INFO << resource << key;
    QTimer::singleShot(100, this, &QWaylandTextureSharingExtension::cleanupBuffers);
}

// A client has disconnected
void QWaylandTextureSharingExtension::zqt_texture_sharing_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
//    qDebug() << "texture_sharing_destroy_resource" << resource->handle << resource->handle->object.id << "client" << resource->client();
//    dumpBufferInfo();
    QTimer::singleShot(1000, this, &QWaylandTextureSharingExtension::cleanupBuffers);
}

bool QWaylandTextureSharingExtension::initServerBufferIntegration()
{
    if (!m_server_buffer_integration) {
        QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());

        m_server_buffer_integration = QWaylandCompositorPrivate::get(compositor)->serverBufferIntegration();
        if (!m_server_buffer_integration) {
            qWarning("QWaylandTextureSharingExtension initialization failed: No Server Buffer Integration");
            if (qEnvironmentVariableIsEmpty("QT_WAYLAND_SERVER_BUFFER_INTEGRATION"))
                qWarning("Set the environment variable 'QT_WAYLAND_SERVER_BUFFER_INTEGRATION' to specify.");
            return false;
        }
    }
    return true;
}

QtWayland::ServerBuffer *QWaylandTextureSharingExtension::getCompressedBuffer(const QString &pathName)
{
    QFile f(pathName);
    if (!f.open(QIODevice::ReadOnly))
        return nullptr;

    QTextureFileReader r(&f, pathName);

    if (!r.canRead())
        return nullptr;

    QTextureFileData td(r.read());

    //qDebug() << "QWaylandTextureSharingExtension: reading compressed texture data" << td;

    if (!td.isValid()) {
        qWarning() << "VulkanServerBufferIntegration:" << pathName << "not valid compressed texture";
        return nullptr;
    }

    QByteArray pixelData = QByteArray::fromRawData(td.data().constData() + td.dataOffset(), td.dataLength());

    return m_server_buffer_integration->createServerBufferFromData(pixelData, td.size(), td.glInternalFormat());
}

void QWaylandTextureSharingExtension::cleanupBuffers()
{
    for (auto it = m_server_buffers.begin(); it != m_server_buffers.end(); ) {
        auto *buffer = it.value().buffer;
        if (!it.value().usedLocally && !buffer->bufferInUse()) {
            //qDebug() << "deleting buffer for" << it.key();
            it = m_server_buffers.erase(it);
            delete buffer;
        } else {
            ++it;
        }
    }
    //dumpBufferInfo();
}

void QWaylandTextureSharingExtension::dumpBufferInfo()
{
    qDebug() << "shared buffers:" << m_server_buffers.count();
    for (auto it = m_server_buffers.cbegin(); it != m_server_buffers.cend(); ++it)
        qDebug() << "    " << it.key() << ":" << it.value().buffer << "in use" << it.value().buffer->bufferInUse() << "usedLocally" << it.value().usedLocally ;
}

QT_END_NAMESPACE

#include "qwltexturesharingextension.moc"
