/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include <QtCore/qhash.h>
#include "gltexture_p.h"

#include <private/qdebug_p.h>
#include <QDebug>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLPixelTransferOptions>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qtexturedata.h>
#include <Qt3DRender/qtextureimagedata.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/texturedatamanager_p.h>
#include <Qt3DRender/private/qabstracttexture_p.h>
#include <Qt3DRender/private/renderbuffer_p.h>
#include <Qt3DRender/private/qtextureimagedata_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {

GLTexture::GLTexture(TextureDataManager *texDataMgr,
                     TextureImageDataManager *texImgDataMgr,
                     const QTextureGeneratorPtr &texGen,
                     bool unique)
    : m_unique(unique)
    , m_gl(nullptr)
    , m_renderBuffer(nullptr)
    , m_textureDataManager(texDataMgr)
    , m_textureImageDataManager(texImgDataMgr)
    , m_dataFunctor(texGen)
    , m_pendingDataFunctor(nullptr)
    , m_externalRendering(false)
{
    // make sure texture generator is executed
    // this is needed when Texture have the TargetAutomatic
    // to ensure they are loaded before trying to instantiate the QOpenGLTexture
    if (!texGen.isNull())
        m_textureDataManager->requestData(texGen, this);
}

GLTexture::~GLTexture()
{
    destroyGLTexture();
}

void GLTexture::destroyResources()
{
    if (m_dataFunctor)
        m_textureDataManager->releaseData(m_dataFunctor, this);
}

void GLTexture::destroyGLTexture()
{
    delete m_gl;
    m_gl = nullptr;
    delete m_renderBuffer;
    m_renderBuffer = nullptr;

    m_dirtyFlags.store(0);

    destroyResources();
}

bool GLTexture::loadTextureDataFromGenerator()
{
    m_textureData = m_textureDataManager->getData(m_dataFunctor);
    // if there is a texture generator, most properties will be defined by it
    if (m_textureData) {
        if (m_properties.target != QAbstractTexture::TargetAutomatic)
            qWarning() << "[Qt3DRender::GLTexture] When a texture provides a generator, it's target is expected to be TargetAutomatic";

        m_actualTarget = m_textureData->target();
        m_properties.width = m_textureData->width();
        m_properties.height = m_textureData->height();
        m_properties.depth = m_textureData->depth();
        m_properties.layers = m_textureData->layers();
        m_properties.format = m_textureData->format();

        const QVector<QTextureImageDataPtr> imageData = m_textureData->imageData();

        if (imageData.size() > 0) {
            // Set the mips level based on the first image if autoMipMapGeneration is disabled
            if (!m_properties.generateMipMaps)
                m_properties.mipLevels = imageData.first()->mipLevels();
        }
    }
    return !m_textureData.isNull();
}

void GLTexture::loadTextureDataFromImages()
{
    int maxMipLevel = 0;
    for (const Image &img : qAsConst(m_images)) {
        const QTextureImageDataPtr imgData = m_textureImageDataManager->getData(img.generator);
        // imgData may be null in the following cases:
        // - Texture is created with TextureImages which have yet to be
        // loaded (skybox where you don't yet know the path, source set by
        // a property binding, queued connection ...)
        // - TextureImage whose generator failed to return a valid data
        // (invalid url, error opening file...)
        if (imgData.isNull())
            continue;

        m_imageData.push_back(imgData);
        maxMipLevel = qMax(maxMipLevel, img.mipLevel);

        // If the texture doesn't have a texture generator, we will
        // derive some properties from the first TextureImage (layer=0, miplvl=0, face=0)
        if (!m_textureData && img.layer == 0 && img.mipLevel == 0 && img.face == QAbstractTexture::CubeMapPositiveX) {
            if (imgData->width() != -1 && imgData->height() != -1 && imgData->depth() != -1) {
                m_properties.width = imgData->width();
                m_properties.height = imgData->height();
                m_properties.depth = imgData->depth();
            }
            // Set the format of the texture if the texture format is set to Automatic
            if (m_properties.format == QAbstractTexture::Automatic) {
                m_properties.format = static_cast<QAbstractTexture::TextureFormat>(imgData->format());
            }
            setDirtyFlag(Properties, true);
        }
    }

    // make sure the number of mip levels is set when there is no texture data generator
    if (!m_dataFunctor) {
        m_properties.mipLevels = maxMipLevel + 1;
        setDirtyFlag(Properties, true);
    }
}

GLTexture::TextureUpdateInfo GLTexture::createOrUpdateGLTexture()
{
    QMutexLocker locker(&m_textureMutex);
    bool needUpload = false;
    TextureUpdateInfo textureInfo;

    m_properties.status = QAbstractTexture::Error;

    // on the first invocation in the render thread, make sure to
    // evaluate the texture data generator output
    // (this might change some property values)
    if (m_dataFunctor && !m_textureData) {
        const bool successfullyLoadedTextureData = loadTextureDataFromGenerator();
        if (successfullyLoadedTextureData) {
            setDirtyFlag(Properties, true);
            needUpload = true;
        } else {
            if (m_pendingDataFunctor != m_dataFunctor.get()) {
                qWarning() << "[Qt3DRender::GLTexture] No QTextureData generated from Texture Generator yet. Texture will be invalid for this frame";
                m_pendingDataFunctor = m_dataFunctor.get();
            }
            textureInfo.properties.status = QAbstractTexture::Loading;
            return textureInfo;
        }
    }

    // additional texture images may be defined through image data generators
    if (testDirtyFlag(TextureData)) {
        m_imageData.clear();
        loadTextureDataFromImages();
        needUpload = true;
    }

    // don't try to create the texture if the format was not set
    if (m_properties.format == QAbstractTexture::Automatic) {
        textureInfo.properties.status = QAbstractTexture::Error;
        return textureInfo;
    }

    // if the properties changed, we need to re-allocate the texture
    if (testDirtyFlag(Properties)) {
        delete m_gl;
        m_gl = nullptr;
        textureInfo.wasUpdated = true;
        // If we are destroyed because of some property change but still our content data
        // make sure we are marked for upload
        if (m_textureData || !m_imageData.empty())
            needUpload = true;
    }


    if (!m_gl) {
        m_gl = buildGLTexture();
        if (!m_gl) {
            textureInfo.properties.status = QAbstractTexture::Error;
            return textureInfo;
        }

        m_gl->allocateStorage();
        if (!m_gl->isStorageAllocated()) {
            textureInfo.properties.status = QAbstractTexture::Error;
            return textureInfo;
        }
    }
    m_properties.status = QAbstractTexture::Ready;

    textureInfo.properties = m_properties;
    textureInfo.texture = m_gl;

    // need to (re-)upload texture data?
    if (needUpload) {
        uploadGLTextureData();
        setDirtyFlag(TextureData, false);
    }

    // need to set texture parameters?
    if (testDirtyFlag(Properties) || testDirtyFlag(Parameters)) {
        updateGLTextureParameters();
    }

    // un-set properties and parameters. The TextureData flag might have been set by another thread
    // in the meantime, so don't clear that.
    setDirtyFlag(Properties, false);
    setDirtyFlag(Parameters, false);

    return textureInfo;
}

RenderBuffer *GLTexture::getOrCreateRenderBuffer()
{
    QMutexLocker locker(&m_textureMutex);

    if (m_dataFunctor && !m_textureData) {
        m_textureData = m_textureDataManager->getData(m_dataFunctor);
        if (m_textureData) {
            if (m_properties.target != QAbstractTexture::TargetAutomatic)
                qWarning() << "[Qt3DRender::GLTexture] [renderbuffer] When a texture provides a generator, it's target is expected to be TargetAutomatic";

            m_properties.width = m_textureData->width();
            m_properties.height = m_textureData->height();
            m_properties.format = m_textureData->format();

            setDirtyFlag(Properties);
        } else {
            if (m_pendingDataFunctor != m_dataFunctor.get()) {
                qWarning() << "[Qt3DRender::GLTexture] [renderbuffer] No QTextureData generated from Texture Generator yet. Texture will be invalid for this frame";
                m_pendingDataFunctor = m_dataFunctor.get();
            }
            return nullptr;
        }
    }

    if (testDirtyFlag(Properties)) {
        delete m_renderBuffer;
        m_renderBuffer = nullptr;
    }

    if (!m_renderBuffer)
        m_renderBuffer = new RenderBuffer(m_properties.width, m_properties.height, m_properties.format);

    setDirtyFlag(Properties, false);
    setDirtyFlag(Parameters, false);

    return m_renderBuffer;
}

void GLTexture::setParameters(const TextureParameters &params)
{
    QMutexLocker locker(&m_textureMutex);
    if (m_parameters != params) {
        m_parameters = params;
        setDirtyFlag(Parameters);
    }
}

void GLTexture::setProperties(const TextureProperties &props)
{
    QMutexLocker locker(&m_textureMutex);
    if (m_properties != props) {
        m_properties = props;
        m_actualTarget = props.target;
        setDirtyFlag(Properties);
    }
}

void GLTexture::setImages(const QVector<Image> &images)
{
    // check if something has changed at all
    bool same = (images.size() == m_images.size());
    if (same) {
        for (int i = 0; i < images.size(); i++) {
            if (images[i] != m_images[i]) {
                same = false;
                break;
            }
        }
    }


    if (!same) {
        m_images = images;
        requestUpload();
    }
}

void GLTexture::setGenerator(const QTextureGeneratorPtr &generator)
{
    // Note: we do not compare if the generator is different
    // as in some cases we may want to reset the same generator to force a reload
    // e.g when using remote urls for textures
    if (m_dataFunctor)
        m_textureDataManager->releaseData(m_dataFunctor, this);

    m_textureData.reset();
    m_dataFunctor = generator;

    if (m_dataFunctor) {
        m_textureDataManager->requestData(m_dataFunctor, this);
        requestUpload();
    }
}

// Return nullptr if
// - context cannot be obtained
// - texture hasn't yet been loaded
QOpenGLTexture *GLTexture::buildGLTexture()
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning() << Q_FUNC_INFO << "requires an OpenGL context";
        return nullptr;
    }

    if (m_actualTarget == QAbstractTexture::TargetAutomatic) {
        // If the target is automatic at this point, it means that the texture
        // hasn't been loaded yet (case of remote urls) and that loading failed
        // and that target format couldn't be deduced
        return nullptr;
    }

    QOpenGLTexture* glTex = new QOpenGLTexture(static_cast<QOpenGLTexture::Target>(m_actualTarget));

    // m_format may not be ES2 compatible. Now it's time to convert it, if necessary.
    QAbstractTexture::TextureFormat format = m_properties.format;
    if (ctx->isOpenGLES() && ctx->format().majorVersion() < 3) {
        switch (m_properties.format) {
        case QOpenGLTexture::RGBA8_UNorm:
        case QOpenGLTexture::RGBAFormat:
            format = QAbstractTexture::RGBAFormat;
            break;
        case QOpenGLTexture::RGB8_UNorm:
        case QOpenGLTexture::RGBFormat:
            format = QAbstractTexture::RGBFormat;
            break;
        case QOpenGLTexture::DepthFormat:
            format = QAbstractTexture::DepthFormat;
            break;
        default:
            auto warning = qWarning();
            warning << "Could not find a matching OpenGL ES 2.0 texture format:";
            QtDebugUtils::formatQEnum(warning, m_properties.format);
            break;
        }
    }

    // Map ETC1 to ETC2 when supported. This allows using features like
    // immutable storage as ETC2 is standard in GLES 3.0, while the ETC1 extension
    // is written against GLES 1.0.
    if (m_properties.format == QAbstractTexture::RGB8_ETC1) {
        if ((ctx->isOpenGLES() && ctx->format().majorVersion() >= 3)
                || ctx->hasExtension(QByteArrayLiteral("GL_OES_compressed_ETC2_RGB8_texture"))
                || ctx->hasExtension(QByteArrayLiteral("GL_ARB_ES3_compatibility")))
            format = m_properties.format = QAbstractTexture::RGB8_ETC2;
    }

    glTex->setFormat(m_properties.format == QAbstractTexture::Automatic ?
                         QOpenGLTexture::NoFormat :
                         static_cast<QOpenGLTexture::TextureFormat>(format));
    glTex->setSize(m_properties.width, m_properties.height, m_properties.depth);
    // Set layers count if texture array
    if (m_actualTarget == QAbstractTexture::Target1DArray ||
            m_actualTarget == QAbstractTexture::Target2DArray ||
            m_actualTarget == QAbstractTexture::Target2DMultisampleArray ||
            m_actualTarget == QAbstractTexture::TargetCubeMapArray) {
        glTex->setLayers(m_properties.layers);
    }

    if (m_actualTarget == QAbstractTexture::Target2DMultisample ||
            m_actualTarget == QAbstractTexture::Target2DMultisampleArray) {
        // Set samples count if multisampled texture
        // (multisampled textures don't have mipmaps)
        glTex->setSamples(m_properties.samples);
    } else if (m_properties.generateMipMaps) {
        glTex->setMipLevels(glTex->maximumMipLevels());
    } else {
        glTex->setAutoMipMapGenerationEnabled(false);
        if (glTex->hasFeature(QOpenGLTexture::TextureMipMapLevel)) {
            glTex->setMipBaseLevel(0);
            glTex->setMipMaxLevel(m_properties.mipLevels - 1);
        }
        glTex->setMipLevels(m_properties.mipLevels);
    }

    if (!glTex->create()) {
        qWarning() << Q_FUNC_INFO << "creating QOpenGLTexture failed";
        return nullptr;
    }

    return glTex;
}

static void uploadGLData(QOpenGLTexture *glTex,
                         int level, int layer, QOpenGLTexture::CubeMapFace face,
                         const QByteArray &bytes, const QTextureImageDataPtr &data)
{
    if (data->isCompressed()) {
        glTex->setCompressedData(level, layer, face, bytes.size(), bytes.constData());
    } else {
        QOpenGLPixelTransferOptions uploadOptions;
        uploadOptions.setAlignment(1);
        glTex->setData(level, layer, face, data->pixelFormat(), data->pixelType(), bytes.constData(), &uploadOptions);
    }
}

void GLTexture::uploadGLTextureData()
{
    // Upload all QTexImageData set by the QTextureGenerator
    if (m_textureData) {
        const QVector<QTextureImageDataPtr> imgData = m_textureData->imageData();

        for (const QTextureImageDataPtr &data : imgData) {
            const int mipLevels = m_properties.generateMipMaps ? 1 : data->mipLevels();

            for (int layer = 0; layer < data->layers(); layer++) {
                for (int face = 0; face < data->faces(); face++) {
                    for (int level = 0; level < mipLevels; level++) {
                        // ensure we don't accidentally cause a detach / copy of the raw bytes
                        const QByteArray bytes(data->data(layer, face, level));
                        uploadGLData(m_gl, level, layer,
                                     static_cast<QOpenGLTexture::CubeMapFace>(QOpenGLTexture::CubeMapPositiveX + face),
                                     bytes, data);
                    }
                }
            }
        }
    }

    // Upload all QTexImageData references by the TextureImages
    for (int i = 0; i < std::min(m_images.size(), m_imageData.size()); i++) {
        const QTextureImageDataPtr &imgData = m_imageData.at(i);
        // Here the bytes in the QTextureImageData contain data for a single
        // layer, face or mip level, unlike the QTextureGenerator case where
        // they are in a single blob. Hence QTextureImageData::data() is not suitable.
        const QByteArray bytes(QTextureImageDataPrivate::get(imgData.get())->m_data);
        uploadGLData(m_gl, m_images[i].mipLevel, m_images[i].layer,
                     static_cast<QOpenGLTexture::CubeMapFace>(m_images[i].face),
                     bytes, imgData);
    }
}

void GLTexture::updateGLTextureParameters()
{
    const bool isMultisampledTexture = (m_actualTarget == QAbstractTexture::Target2DMultisample ||
                                        m_actualTarget == QAbstractTexture::Target2DMultisampleArray);
    // Multisampled textures can only be accessed by texelFetch in shaders
    // and don't support wrap modes and mig/mag filtes
    if (isMultisampledTexture)
        return;

    m_gl->setWrapMode(QOpenGLTexture::DirectionS, static_cast<QOpenGLTexture::WrapMode>(m_parameters.wrapModeX));
    if (m_actualTarget != QAbstractTexture::Target1D &&
            m_actualTarget != QAbstractTexture::Target1DArray &&
            m_actualTarget != QAbstractTexture::TargetBuffer)
        m_gl->setWrapMode(QOpenGLTexture::DirectionT, static_cast<QOpenGLTexture::WrapMode>(m_parameters.wrapModeY));
    if (m_actualTarget == QAbstractTexture::Target3D)
        m_gl->setWrapMode(QOpenGLTexture::DirectionR, static_cast<QOpenGLTexture::WrapMode>(m_parameters.wrapModeZ));
    m_gl->setMinMagFilters(static_cast<QOpenGLTexture::Filter>(m_parameters.minificationFilter),
                           static_cast<QOpenGLTexture::Filter>(m_parameters.magnificationFilter));
    if (m_gl->hasFeature(QOpenGLTexture::AnisotropicFiltering))
        m_gl->setMaximumAnisotropy(m_parameters.maximumAnisotropy);
    if (m_gl->hasFeature(QOpenGLTexture::TextureComparisonOperators)) {
        m_gl->setComparisonFunction(static_cast<QOpenGLTexture::ComparisonFunction>(m_parameters.comparisonFunction));
        m_gl->setComparisonMode(static_cast<QOpenGLTexture::ComparisonMode>(m_parameters.comparisonMode));
    }
}


} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
