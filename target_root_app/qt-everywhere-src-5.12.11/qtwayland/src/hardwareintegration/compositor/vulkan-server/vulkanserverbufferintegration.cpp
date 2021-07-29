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

#define  GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "vulkanserverbufferintegration.h"

#include "vulkanwrapper.h"

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLTexture>
#include <QtGui/QOffscreenSurface>

#include <unistd.h>
#include <fcntl.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE
static constexpr bool extraDebug = false;

#define DECL_GL_FUNCTION(name, type) \
    type name

#define FIND_GL_FUNCTION(name, type) \
    do { \
        name = reinterpret_cast<type>(glContext->getProcAddress(#name)); \
        if (!name) {                                                    \
            qWarning() << "ERROR in GL proc lookup. Could not find " #name; \
            return false;                                               \
        } \
    } while (0)

struct VulkanServerBufferGlFunctions
{
    DECL_GL_FUNCTION(glCreateMemoryObjectsEXT, PFNGLCREATEMEMORYOBJECTSEXTPROC);
    DECL_GL_FUNCTION(glImportMemoryFdEXT, PFNGLIMPORTMEMORYFDEXTPROC);
    //DECL_GL_FUNCTION(glTextureStorageMem2DEXT, PFNGLTEXTURESTORAGEMEM2DEXTPROC);
    DECL_GL_FUNCTION(glTexStorageMem2DEXT, PFNGLTEXSTORAGEMEM2DEXTPROC);
    DECL_GL_FUNCTION(glDeleteMemoryObjectsEXT, PFNGLDELETEMEMORYOBJECTSEXTPROC);

    bool init(QOpenGLContext *glContext)
    {
        FIND_GL_FUNCTION(glCreateMemoryObjectsEXT, PFNGLCREATEMEMORYOBJECTSEXTPROC);
        FIND_GL_FUNCTION(glImportMemoryFdEXT, PFNGLIMPORTMEMORYFDEXTPROC);
        //FIND_GL_FUNCTION(glTextureStorageMem2DEXT, PFNGLTEXTURESTORAGEMEM2DEXTPROC);
        FIND_GL_FUNCTION(glTexStorageMem2DEXT, PFNGLTEXSTORAGEMEM2DEXTPROC);
        FIND_GL_FUNCTION(glDeleteMemoryObjectsEXT, PFNGLDELETEMEMORYOBJECTSEXTPROC);

        return true;
    }
    static bool create(QOpenGLContext *glContext);
};

static VulkanServerBufferGlFunctions *funcs = nullptr;

//RAII
class CurrentContext
{
public:
    CurrentContext()
    {
        if (!QOpenGLContext::currentContext()) {
            if (QOpenGLContext::globalShareContext()) {
                if (!localContext) {
                    localContext = new QOpenGLContext;
                    localContext->setShareContext(QOpenGLContext::globalShareContext());
                    localContext->create();
                }
                if (!offscreenSurface) {
                    offscreenSurface = new QOffscreenSurface;
                    offscreenSurface->setFormat(localContext->format());
                    offscreenSurface->create();
                }
                localContext->makeCurrent(offscreenSurface);
                localContextInUse = true;
            } else {
                qCritical("VulkanServerBufferIntegration: no globalShareContext");
            }
        }
    }
    ~CurrentContext()
    {
        if (localContextInUse)
            localContext->doneCurrent();
    }
    QOpenGLContext *context() { return localContextInUse ? localContext : QOpenGLContext::currentContext(); }
private:
    static QOpenGLContext *localContext;
    static QOffscreenSurface *offscreenSurface;
    bool localContextInUse = false;
};

QOpenGLContext *CurrentContext::localContext = nullptr;
QOffscreenSurface *CurrentContext::offscreenSurface = nullptr;

bool VulkanServerBufferGlFunctions::create(QOpenGLContext *glContext)
{
    if (funcs)
        return true;
    funcs = new VulkanServerBufferGlFunctions;
    if (!funcs->init(glContext)) {
        delete funcs;
        funcs = nullptr;
        return false;
    }
    return true;
}

VulkanServerBuffer::VulkanServerBuffer(VulkanServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format)
    : QtWayland::ServerBuffer(qimage.size(),format)
    , m_integration(integration)
    , m_width(qimage.width())
    , m_height(qimage.height())
{
    m_format = format;
    switch (m_format) {
        case RGBA32:
            m_glInternalFormat = GL_RGBA8;
            break;
        // case A8:
        //     m_glInternalFormat = GL_R8;
        //     break;
        default:
            qWarning("VulkanServerBuffer: unsupported format");
            m_glInternalFormat = GL_RGBA8;
            break;
    }

    auto vulkanWrapper = m_integration->vulkanWrapper();
    m_vImage = vulkanWrapper->createTextureImage(qimage);
    if (m_vImage)
        m_fd = vulkanWrapper->getImageInfo(m_vImage, &m_memorySize);
}

VulkanServerBuffer::VulkanServerBuffer(VulkanServerBufferIntegration *integration, VulkanImageWrapper *vImage, uint glInternalFormat, const QSize &size)
    : QtWayland::ServerBuffer(size, QtWayland::ServerBuffer::Custom)
    , m_integration(integration)
    , m_width(size.width())
    , m_height(size.height())
    , m_vImage(vImage)
    , m_glInternalFormat(glInternalFormat)
{
    auto vulkanWrapper = m_integration->vulkanWrapper();
    m_fd = vulkanWrapper->getImageInfo(m_vImage, &m_memorySize);
}

VulkanServerBuffer::~VulkanServerBuffer()
{
    delete m_texture; //this is always nullptr for now
    auto vulkanWrapper = m_integration->vulkanWrapper();
    vulkanWrapper->freeTextureImage(m_vImage);
}

struct ::wl_resource *VulkanServerBuffer::resourceForClient(struct ::wl_client *client)
{
    auto *bufferResource = resourceMap().value(client);
    if (!bufferResource) {
        auto integrationResource = m_integration->resourceMap().value(client);
        if (!integrationResource) {
            qWarning("VulkanServerBuffer::resourceForClient: Trying to get resource for ServerBuffer. But client is not bound to the vulkan interface");
            return nullptr;
        }
        struct ::wl_resource *shm_integration_resource = integrationResource->handle;
        Resource *resource = add(client, 1);
        m_integration->send_server_buffer_created(shm_integration_resource, resource->handle, m_fd, m_width, m_height, m_memorySize, m_glInternalFormat);
        return resource->handle;
    }
    return bufferResource->handle;
}

QOpenGLTexture *VulkanServerBuffer::toOpenGlTexture()
{
    if (m_texture && m_texture->isCreated())
        return m_texture;

    CurrentContext current;

    if (!funcs && !VulkanServerBufferGlFunctions::create(current.context()))
        return nullptr;

    funcs->glCreateMemoryObjectsEXT(1, &m_memoryObject);
    if (extraDebug) qDebug() << "glCreateMemoryObjectsEXT" << hex << glGetError();


    int dupfd = fcntl(m_fd, F_DUPFD_CLOEXEC, 0);
    if (dupfd < 0) {
        perror("VulkanServerBuffer::toOpenGlTexture() Could not dup fd:");
        return nullptr;
    }

    funcs->glImportMemoryFdEXT(m_memoryObject, m_memorySize, GL_HANDLE_TYPE_OPAQUE_FD_EXT, dupfd);
    if (extraDebug) qDebug() << "glImportMemoryFdEXT" << hex << glGetError();


    if (!m_texture)
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_texture->create();

    GLuint texId = m_texture->textureId();
    if (extraDebug) qDebug() << "created texture" << texId << hex << glGetError();

    m_texture->bind();
    if (extraDebug) qDebug() << "bound texture" << texId << hex << glGetError();
    funcs->glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, m_glInternalFormat, m_size.width(), m_size.height(), m_memoryObject, 0 );
    if (extraDebug) qDebug() << "glTexStorageMem2DEXT" << hex << glGetError();
    if (extraDebug) qDebug() << "format" << hex  << m_glInternalFormat << GL_RGBA8;


    return m_texture;
}

void VulkanServerBuffer::releaseOpenGlTexture()
{
    if (!m_texture || !m_texture->isCreated())
        return;

    CurrentContext current;
    m_texture->destroy();
    funcs->glDeleteMemoryObjectsEXT(1, &m_memoryObject);
}


bool VulkanServerBuffer::bufferInUse()
{
    return (m_texture && m_texture->isCreated()) || resourceMap().count() > 0;
}

void VulkanServerBuffer::server_buffer_release(Resource *resource)
{
    qCDebug(qLcWaylandCompositorHardwareIntegration) << "server_buffer RELEASE resource" << resource->handle << wl_resource_get_id(resource->handle) << "for client" << resource->client();
    wl_resource_destroy(resource->handle);
}

VulkanServerBufferIntegration::VulkanServerBufferIntegration()
{
}

VulkanServerBufferIntegration::~VulkanServerBufferIntegration()
{
}

void VulkanServerBufferIntegration::initializeHardware(QWaylandCompositor *compositor)
{
    Q_ASSERT(QGuiApplication::platformNativeInterface());

    QtWaylandServer::zqt_vulkan_server_buffer_v1::init(compositor->display(), 1);
}

bool VulkanServerBufferIntegration::supportsFormat(QtWayland::ServerBuffer::Format format) const
{
    switch (format) {
        case QtWayland::ServerBuffer::RGBA32:
            return true;
        case QtWayland::ServerBuffer::A8:
            return false;
        default:
            return false;
    }
}

QtWayland::ServerBuffer *VulkanServerBufferIntegration::createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format)
{
    if (!m_vulkanWrapper) {
        CurrentContext current;
        m_vulkanWrapper = new VulkanWrapper(current.context());
    }
    return new VulkanServerBuffer(this, qimage, format);
}

QtWayland::ServerBuffer *VulkanServerBufferIntegration::createServerBufferFromData(const QByteArray &data, const QSize &size, uint glInternalFormat)
{
    if (!m_vulkanWrapper) {
        CurrentContext current;
        m_vulkanWrapper = new VulkanWrapper(current.context());
    }

    auto *vImage = m_vulkanWrapper->createTextureImageFromData(reinterpret_cast<const uchar*>(data.constData()), data.size(), size, glInternalFormat);

    if (vImage)
        return new VulkanServerBuffer(this, vImage, glInternalFormat, size);

    qCWarning(qLcWaylandCompositorHardwareIntegration) << "could not load compressed texture";
    return nullptr;
}

QT_END_NAMESPACE
