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

#include "textureimage_p.h"
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DRender/qtextureimage.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/qabstracttextureimage_p.h>
#include <Qt3DRender/private/texturedatamanager_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {

TextureImage::TextureImage()
    : BackendNode(ReadWrite)
    , m_dirty(false)
    , m_layer(0)
    , m_mipLevel(0)
    , m_face(QAbstractTexture::CubeMapPositiveX)
    , m_textureImageDataManager(nullptr)
{
}

TextureImage::~TextureImage()
{
}

void TextureImage::cleanup()
{
    if (m_generator) {
        m_textureImageDataManager->releaseData(m_generator, peerId());
        m_generator.reset();
    }
    m_dirty = false;
    m_layer = 0;
    m_mipLevel = 0;
    m_face = QAbstractTexture::CubeMapPositiveX;
}

void TextureImage::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change)
{
    const auto typedChange = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<QAbstractTextureImageData>>(change);
    const auto &data = typedChange->data;
    m_mipLevel = data.mipLevel;
    m_layer = data.layer;
    m_face = data.face;
    m_generator = data.generator;
    m_dirty = true;

    // Request functor upload
    if (m_generator)
        m_textureImageDataManager->requestData(m_generator, peerId());
}

void TextureImage::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    QPropertyUpdatedChangePtr propertyChange = qSharedPointerCast<QPropertyUpdatedChange>(e);

    if (e->type() == PropertyUpdated) {
        if (propertyChange->propertyName() == QByteArrayLiteral("layer")) {
            m_layer = propertyChange->value().toInt();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("mipLevel")) {
            m_mipLevel = propertyChange->value().toInt();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("face")) {
            m_face = static_cast<QAbstractTexture::CubeMapFace>(propertyChange->value().toInt());
        } else if (propertyChange->propertyName() == QByteArrayLiteral("dataGenerator")) {
            // Release ref to generator
            if (m_generator)
                m_textureImageDataManager->releaseData(m_generator, peerId());
            m_generator = propertyChange->value().value<QTextureImageDataGeneratorPtr>();
            // Request functor upload
            if (m_generator)
                m_textureImageDataManager->requestData(m_generator, peerId());
        }
        m_dirty = true;
    }

    markDirty(AbstractRenderer::AllDirty);
    BackendNode::sceneChangeEvent(e);
}

void TextureImage::unsetDirty()
{
    m_dirty = false;
}

TextureImageFunctor::TextureImageFunctor(AbstractRenderer *renderer,
                                         TextureImageManager *textureImageManager,
                                         TextureImageDataManager *textureImageDataManager)
    : m_renderer(renderer)
    , m_textureImageManager(textureImageManager)
    , m_textureImageDataManager(textureImageDataManager)
{
}

Qt3DCore::QBackendNode *TextureImageFunctor::create(const Qt3DCore::QNodeCreatedChangeBasePtr &change) const
{
    TextureImage *backend = m_textureImageManager->getOrCreateResource(change->subjectId());
    backend->setTextureImageDataManager(m_textureImageDataManager);
    backend->setRenderer(m_renderer);
    return backend;
}

Qt3DCore::QBackendNode *TextureImageFunctor::get(Qt3DCore::QNodeId id) const
{
    return m_textureImageManager->lookupResource(id);
}

void TextureImageFunctor::destroy(Qt3DCore::QNodeId id) const
{
    m_textureImageManager->releaseResource(id);
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
