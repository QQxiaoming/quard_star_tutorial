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

#include "texture_p.h"

#include <QDebug>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>

#include <Qt3DRender/private/texture_p.h>
#include <Qt3DRender/private/qabstracttexture_p.h>
#include <Qt3DRender/private/gltexturemanager_p.h>
#include <Qt3DRender/private/managers_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {

Texture::Texture()
    // We need backend -> frontend notifications to update the status of the texture
    : BackendNode(ReadWrite)
    , m_dirty(DirtyImageGenerators|DirtyProperties|DirtyParameters|DirtyDataGenerator)
{
}

Texture::~Texture()
{
    // We do not abandon the api texture
    // because if the dtor is called that means
    // the manager was destroyed otherwise cleanup
    // would have been called
}

void Texture::addDirtyFlag(DirtyFlags flags)
{
    QMutexLocker lock(&m_flagsMutex);
    m_dirty |= flags;
    if (m_renderer)
        markDirty(AbstractRenderer::TexturesDirty);
}

Texture::DirtyFlags Texture::dirtyFlags()
{
    QMutexLocker lock(&m_flagsMutex);
    return m_dirty;
}

void Texture::unsetDirty()
{
    QMutexLocker lock(&m_flagsMutex);
    m_dirty = Texture::NotDirty;
}

void Texture::addTextureImage(Qt3DCore::QNodeId id)
{
    if (!m_textureImageIds.contains(id)) {
        m_textureImageIds.push_back(id);
        addDirtyFlag(DirtyImageGenerators);
    }
}

void Texture::removeTextureImage(Qt3DCore::QNodeId id)
{
    m_textureImageIds.removeAll(id);
    addDirtyFlag(DirtyImageGenerators);
}

// This is called by Renderer::updateGLResources
// when the texture has been marked for cleanup
void Texture::cleanup()
{
    // Whoever calls this must make sure to also check if this
    // texture is being referenced by a shared API specific texture (GLTexture)
    m_dataFunctor.reset();
    m_textureImageIds.clear();

    // set default values
    m_properties = {};
    m_parameters = {};

    m_dirty = NotDirty;
}

// ChangeArbiter/Aspect Thread
void Texture::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    DirtyFlags dirty;

    switch (e->type()) {
    case PropertyUpdated: {
        QPropertyUpdatedChangePtr propertyChange = qSharedPointerCast<QPropertyUpdatedChange>(e);
        if (propertyChange->propertyName() == QByteArrayLiteral("width")) {
            m_properties.width = propertyChange->value().toInt();
            dirty = DirtyProperties;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("height")) {
            m_properties.height = propertyChange->value().toInt();
            dirty = DirtyProperties;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("depth")) {
            m_properties.depth = propertyChange->value().toInt();
            dirty = DirtyProperties;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("format")) {
            m_properties.format = static_cast<QAbstractTexture::TextureFormat>(propertyChange->value().toInt());
            dirty = DirtyProperties;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("target")) {
            m_properties.target = static_cast<QAbstractTexture::Target>(propertyChange->value().toInt());
            dirty = DirtyProperties;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("mipmaps")) {
            m_properties.generateMipMaps = propertyChange->value().toBool();
            dirty = DirtyProperties;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("minificationFilter")) {
            m_parameters.minificationFilter = static_cast<QAbstractTexture::Filter>(propertyChange->value().toInt());
            dirty = DirtyParameters;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("magnificationFilter")) {
            m_parameters.magnificationFilter = static_cast<QAbstractTexture::Filter>(propertyChange->value().toInt());
            dirty = DirtyParameters;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("wrapModeX")) {
            m_parameters.wrapModeX = static_cast<QTextureWrapMode::WrapMode>(propertyChange->value().toInt());
            dirty = DirtyParameters;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("wrapModeY")) {
            m_parameters.wrapModeY = static_cast<QTextureWrapMode::WrapMode>(propertyChange->value().toInt());
            dirty = DirtyParameters;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("wrapModeZ")) {
            m_parameters.wrapModeZ =static_cast<QTextureWrapMode::WrapMode>(propertyChange->value().toInt());
            dirty = DirtyParameters;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("maximumAnisotropy")) {
            m_parameters.maximumAnisotropy = propertyChange->value().toFloat();
            dirty = DirtyParameters;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("comparisonFunction")) {
            m_parameters.comparisonFunction = propertyChange->value().value<QAbstractTexture::ComparisonFunction>();
            dirty = DirtyParameters;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("comparisonMode")) {
            m_parameters.comparisonMode = propertyChange->value().value<QAbstractTexture::ComparisonMode>();
            dirty = DirtyParameters;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("layers")) {
            m_properties.layers = propertyChange->value().toInt();
            dirty = DirtyProperties;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("samples")) {
            m_properties.samples = propertyChange->value().toInt();
            dirty = DirtyProperties;
        } else if (propertyChange->propertyName() == QByteArrayLiteral("generator")) {
            setDataGenerator(propertyChange->value().value<QTextureGeneratorPtr>());
        }
    }
        break;

    case PropertyValueAdded: {
        const auto change = qSharedPointerCast<QPropertyNodeAddedChange>(e);
        if (change->propertyName() == QByteArrayLiteral("textureImage")) {
            addTextureImage(change->addedNodeId());
        }
    }
        break;

    case PropertyValueRemoved: {
        const auto change = qSharedPointerCast<QPropertyNodeRemovedChange>(e);
        if (change->propertyName() == QByteArrayLiteral("textureImage")) {
            removeTextureImage(change->removedNodeId());
        }
    }
        break;

    default:
        break;

    }

    addDirtyFlag(dirty);
    BackendNode::sceneChangeEvent(e);
}

// Called by sceneChangeEvent or TextureDownloadRequest (both in AspectThread context)
void Texture::setDataGenerator(const QTextureGeneratorPtr &generator)
{
    m_dataFunctor = generator;
    addDirtyFlag(DirtyDataGenerator);
}

// Called by sendTextureChangesToFrontendJob once GLTexture and sharing
// has been performed
void Texture::updatePropertiesAndNotify(const TextureProperties &properties)
{
    // If we are Dirty, some property has changed and the properties we have
    // received are potentially already outdated
    if (m_dirty != NotDirty)
        return;

    // Note we don't update target has it is constant for frontend nodes

    if (properties.width != m_properties.width) {
        m_properties.width = properties.width;
        auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
        change->setDeliveryFlags(Qt3DCore::QSceneChange::Nodes);
        change->setPropertyName("width");
        change->setValue(properties.width);
        notifyObservers(change);
    }

    if (properties.height != m_properties.height) {
        m_properties.height = properties.height;
        auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
        change->setDeliveryFlags(Qt3DCore::QSceneChange::Nodes);
        change->setPropertyName("height");
        change->setValue(properties.height);
        notifyObservers(change);
    }

    if (properties.depth != m_properties.depth) {
        m_properties.depth = properties.depth;
        auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
        change->setDeliveryFlags(Qt3DCore::QSceneChange::Nodes);
        change->setPropertyName("depth");
        change->setValue(properties.depth);
        notifyObservers(change);
    }

    if (properties.layers != m_properties.layers) {
        m_properties.layers = properties.layers;
        auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
        change->setDeliveryFlags(Qt3DCore::QSceneChange::Nodes);
        change->setPropertyName("layers");
        change->setValue(properties.layers);
        notifyObservers(change);
    }

    if (properties.format != m_properties.format) {
        m_properties.format = properties.format;
        auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
        change->setDeliveryFlags(Qt3DCore::QSceneChange::Nodes);
        change->setPropertyName("format");
        change->setValue(properties.format);
        notifyObservers(change);
    }

    if (properties.status != m_properties.status) {
        m_properties.status = properties.status;
        auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
        change->setDeliveryFlags(Qt3DCore::QSceneChange::Nodes);
        change->setPropertyName("status");
        change->setValue(properties.status);
        notifyObservers(change);
    }
}

bool Texture::isValid(TextureImageManager *manager) const
{
    for (const QNodeId id : m_textureImageIds) {
        TextureImage *img = manager->lookupResource(id);
        if (img == nullptr)
            return false;
    }
    return true;
}

void Texture::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change)
{
    const auto typedChange = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<QAbstractTextureData>>(change);
    const auto &data = typedChange->data;

    m_properties.target = data.target;
    m_properties.format = data.format;
    m_properties.width = data.width;
    m_properties.height = data.height;
    m_properties.depth = data.depth;
    m_properties.generateMipMaps = data.autoMipMap;
    m_properties.layers = data.layers;
    m_properties.samples = data.samples;
    m_parameters.minificationFilter = data.minFilter;
    m_parameters.magnificationFilter = data.magFilter;
    m_parameters.wrapModeX = data.wrapModeX;
    m_parameters.wrapModeY = data.wrapModeY;
    m_parameters.wrapModeZ = data.wrapModeZ;
    m_parameters.maximumAnisotropy = data.maximumAnisotropy;
    m_parameters.comparisonFunction = data.comparisonFunction;
    m_parameters.comparisonMode = data.comparisonMode;
    m_dataFunctor = data.dataFunctor;

    for (const QNodeId imgId : data.textureImageIds)
        addTextureImage(imgId);

    addDirtyFlag(DirtyFlags(DirtyImageGenerators|DirtyProperties|DirtyParameters));
}


TextureFunctor::TextureFunctor(AbstractRenderer *renderer,
                               TextureManager *textureNodeManager)
    : m_renderer(renderer)
    , m_textureNodeManager(textureNodeManager)
{
}

Qt3DCore::QBackendNode *TextureFunctor::create(const Qt3DCore::QNodeCreatedChangeBasePtr &change) const
{
    Texture *backend = m_textureNodeManager->getOrCreateResource(change->subjectId());
    backend->setRenderer(m_renderer);
    // Remove id from cleanupList if for some reason we were in the dirty list of texture
    // (Can happen when a node destroyed is followed by a node created change
    // in the same loop, when changing parent for instance)
    m_textureNodeManager->removeTextureIdToCleanup(change->subjectId());
    return backend;
}

Qt3DCore::QBackendNode *TextureFunctor::get(Qt3DCore::QNodeId id) const
{
    return m_textureNodeManager->lookupResource(id);
}

void TextureFunctor::destroy(Qt3DCore::QNodeId id) const
{
    m_textureNodeManager->addTextureIdToCleanup(id);
    // We add ourselves to the dirty list to tell the shared texture managers
    // in the renderer that this texture has been destroyed
    m_textureNodeManager->releaseResource(id);
}


} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
