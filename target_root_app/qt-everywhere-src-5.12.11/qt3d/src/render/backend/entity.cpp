/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
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

#include "entity_p.h"
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/qabstractlight.h>
#include <Qt3DRender/qenvironmentlight.h>
#include <Qt3DRender/qlayer.h>
#include <Qt3DRender/qlevelofdetail.h>
#include <Qt3DRender/qraycaster.h>
#include <Qt3DRender/qscreenraycaster.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <Qt3DRender/private/sphere_p.h>
#include <Qt3DRender/qshaderdata.h>
#include <Qt3DRender/qgeometryrenderer.h>
#include <Qt3DRender/qobjectpicker.h>
#include <Qt3DRender/qcomputecommand.h>
#include <Qt3DRender/private/geometryrenderermanager_p.h>
#include <Qt3DRender/private/armature_p.h>

#include <Qt3DRender/qcameralens.h>
#include <Qt3DCore/qarmature.h>
#include <Qt3DCore/qcomponentaddedchange.h>
#include <Qt3DCore/qcomponentremovedchange.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DCore/private/qentity_p.h>
#include <Qt3DCore/qnodecreatedchange.h>

#include <QMatrix4x4>
#include <QString>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {

Entity::Entity()
    : BackendNode()
    , m_nodeManagers(nullptr)
    , m_boundingDirty(false)
    , m_treeEnabled(true)
{
}

Entity::~Entity()
{
    cleanup();
}

void Entity::cleanup()
{
    if (m_nodeManagers != nullptr) {
        m_nodeManagers->worldMatrixManager()->releaseResource(peerId());
        qCDebug(Render::RenderNodes) << Q_FUNC_INFO;
    }
    if (!m_parentEntityId.isNull())
        markDirty(AbstractRenderer::EntityHierarchyDirty);

    m_parentEntityId = Qt3DCore::QNodeId();
    m_worldTransform = HMatrix();
    // Release all component will have to perform their own release when they receive the
    // NodeDeleted notification
    // Clear components
    m_transformComponent = Qt3DCore::QNodeId();
    m_cameraComponent = Qt3DCore::QNodeId();
    m_materialComponent = Qt3DCore::QNodeId();
    m_geometryRendererComponent = Qt3DCore::QNodeId();
    m_objectPickerComponent = QNodeId();
    m_boundingVolumeDebugComponent = QNodeId();
    m_computeComponent = QNodeId();
    m_armatureComponent = QNodeId();
    m_childrenHandles.clear();
    m_layerComponents.clear();
    m_levelOfDetailComponents.clear();
    m_rayCasterComponents.clear();
    m_shaderDataComponents.clear();
    m_lightComponents.clear();
    m_environmentLightComponents.clear();
    m_localBoundingVolume.reset();
    m_worldBoundingVolume.reset();
    m_worldBoundingVolumeWithChildren.reset();
    m_boundingDirty = false;
    QBackendNode::setEnabled(false);
}

void Entity::setParentHandle(HEntity parentHandle)
{
    Q_ASSERT(m_nodeManagers);
    m_parentHandle = parentHandle;
    auto parent = m_nodeManagers->renderNodesManager()->data(parentHandle);
    if (parent != nullptr && !parent->m_childrenHandles.contains(m_handle))
        parent->m_childrenHandles.append(m_handle);
}

void Entity::setNodeManagers(NodeManagers *manager)
{
    m_nodeManagers = manager;
}

void Entity::setHandle(HEntity handle)
{
    m_handle = handle;
}

void Entity::initializeFromPeer(const QNodeCreatedChangeBasePtr &change)
{
    const auto typedChange = qSharedPointerCast<QNodeCreatedChange<Qt3DCore::QEntityData>>(change);
    const auto &data = typedChange->data;

    // Note this is *not* the parentId as that is the ID of the parent QNode, which is not
    // necessarily the same as the parent QEntity (which may be further up the tree).
    m_parentEntityId = data.parentEntityId;
    qCDebug(Render::RenderNodes) << "Creating Entity id =" << peerId() << "parentId =" << m_parentEntityId;

    // TODO: Store string id instead and only in debug mode
    //m_objectName = peer->objectName();
    m_worldTransform = m_nodeManagers->worldMatrixManager()->getOrAcquireHandle(peerId());

    // TODO: Suboptimal -> Maybe have a Hash<QComponent, QEntityList> instead
    m_transformComponent = QNodeId();
    m_materialComponent = QNodeId();
    m_cameraComponent = QNodeId();
    m_geometryRendererComponent = QNodeId();
    m_objectPickerComponent = QNodeId();
    m_boundingVolumeDebugComponent = QNodeId();
    m_computeComponent = QNodeId();
    m_layerComponents.clear();
    m_levelOfDetailComponents.clear();
    m_rayCasterComponents.clear();
    m_shaderDataComponents.clear();
    m_lightComponents.clear();
    m_environmentLightComponents.clear();
    m_localBoundingVolume = QSharedPointer<Sphere>::create(peerId());
    m_worldBoundingVolume = QSharedPointer<Sphere>::create(peerId());
    m_worldBoundingVolumeWithChildren = QSharedPointer<Sphere>::create(peerId());

    for (const auto &idAndType : qAsConst(data.componentIdsAndTypes))
        addComponent(idAndType);

    markDirty(AbstractRenderer::EntityHierarchyDirty);
}

void Entity::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    switch (e->type()) {

    case ComponentAdded: {
        QComponentAddedChangePtr change = qSharedPointerCast<QComponentAddedChange>(e);
        const auto componentIdAndType = QNodeIdTypePair(change->componentId(), change->componentMetaObject());
        addComponent(componentIdAndType);
        qCDebug(Render::RenderNodes) << Q_FUNC_INFO << "Component Added. Id =" << change->componentId();
        markDirty(AbstractRenderer::AllDirty);
        break;
    }

    case ComponentRemoved: {
        QComponentRemovedChangePtr change = qSharedPointerCast<QComponentRemovedChange>(e);
        removeComponent(change->componentId());
        qCDebug(Render::RenderNodes) << Q_FUNC_INFO << "Component Removed. Id =" << change->componentId();
        markDirty(AbstractRenderer::AllDirty);
        break;
    }

    case PropertyUpdated: {
        QPropertyUpdatedChangePtr change = qSharedPointerCast<QPropertyUpdatedChange>(e);
        if (change->propertyName() == QByteArrayLiteral("enabled")) {
            // We only mark as dirty the renderer
            markDirty(AbstractRenderer::EntityEnabledDirty);
            // We let QBackendNode::sceneChangeEvent change the enabled property
        } else if (change->propertyName() == QByteArrayLiteral("parentEntityUpdated")) {
            auto newParent = change->value().value<Qt3DCore::QNodeId>();
            qCDebug(Render::RenderNodes) << "Setting parent for " << peerId() << ", new parentId =" << newParent;
            if (m_parentEntityId != newParent) {
                m_parentEntityId = newParent;
                // TODO: change to EventHierarchyDirty and update renderer to
                //       ensure all jobs are run that depend on Entity hierarchy.
                markDirty(AbstractRenderer::AllDirty);
            }
        }

        break;
    }

    default:
        break;
    }
    BackendNode::sceneChangeEvent(e);
}

void Entity::dump() const
{
    static int depth = 0;
    QString indent(2 * depth++, QChar::fromLatin1(' '));
    qCDebug(Backend) << indent + m_objectName;
    const auto children_ = children();
    for (const Entity *child : children_)
        child->dump();
    --depth;
}

Entity *Entity::parent() const
{
    return m_nodeManagers->renderNodesManager()->data(m_parentHandle);
}


// clearEntityHierarchy and rebuildEntityHierarchy should only be called
// from UpdateEntityHierarchyJob to update the entity hierarchy for the
// entire scene at once
void Entity::clearEntityHierarchy()
{
    m_childrenHandles.clear();
    m_parentHandle = HEntity();
}

// clearEntityHierarchy and rebuildEntityHierarchy should only be called
// from UpdateEntityHierarchyJob to update the entity hierarchy for the
// entire scene at once
void Entity::rebuildEntityHierarchy()
{
    if (!m_parentEntityId.isNull())
        setParentHandle(m_nodeManagers->renderNodesManager()->lookupHandle(m_parentEntityId));
    else
        qCDebug(Render::RenderNodes) << Q_FUNC_INFO << "No parent entity found for Entity" << peerId();
}

void Entity::appendChildHandle(HEntity childHandle)
{
    if (!m_childrenHandles.contains(childHandle)) {
        m_childrenHandles.append(childHandle);
        Entity *child = m_nodeManagers->renderNodesManager()->data(childHandle);
        if (child != nullptr)
            child->m_parentHandle = m_handle;
    }
}

QVector<Entity *> Entity::children() const
{
    QVector<Entity *> childrenVector;
    childrenVector.reserve(m_childrenHandles.size());
    for (const HEntity &handle : m_childrenHandles) {
        Entity *child = m_nodeManagers->renderNodesManager()->data(handle);
        if (child != nullptr)
            childrenVector.append(child);
    }
    return childrenVector;
}

Matrix4x4 *Entity::worldTransform()
{
    return m_nodeManagers->worldMatrixManager()->data(m_worldTransform);
}

const Matrix4x4 *Entity::worldTransform() const
{
    return m_nodeManagers->worldMatrixManager()->data(m_worldTransform);
}

void Entity::addComponent(Qt3DCore::QNodeIdTypePair idAndType)
{
    // The backend element is always created when this method is called
    // If that's not the case something has gone wrong
    const auto type = idAndType.type;
    const auto id = idAndType.id;
    qCDebug(Render::RenderNodes) << Q_FUNC_INFO << "id =" << id << type->className();
    if (type->inherits(&Qt3DCore::QTransform::staticMetaObject)) {
        m_transformComponent = id;
    } else if (type->inherits(&QCameraLens::staticMetaObject)) {
        m_cameraComponent = id;
    } else if (type->inherits(&QLayer::staticMetaObject)) {
        m_layerComponents.append(id);
    } else if (type->inherits(&QLevelOfDetail::staticMetaObject)) {
        m_levelOfDetailComponents.append(id);
    } else if (type->inherits(&QRayCaster::staticMetaObject)) {
        m_rayCasterComponents.append(id);
    } else if (type->inherits(&QScreenRayCaster::staticMetaObject)) {
        m_rayCasterComponents.append(id);
    } else if (type->inherits(&QMaterial::staticMetaObject)) {
        m_materialComponent = id;
    } else if (type->inherits(&QAbstractLight::staticMetaObject)) { // QAbstractLight subclasses QShaderData
        m_lightComponents.append(id);
    } else if (type->inherits(&QEnvironmentLight::staticMetaObject)) {
        m_environmentLightComponents.append(id);
    } else if (type->inherits(&QShaderData::staticMetaObject)) {
        m_shaderDataComponents.append(id);
    } else if (type->inherits(&QGeometryRenderer::staticMetaObject)) {
        m_geometryRendererComponent = id;
        m_boundingDirty = true;
    } else if (type->inherits(&QObjectPicker::staticMetaObject)) {
        m_objectPickerComponent = id;
//    } else if (type->inherits(&QBoundingVolumeDebug::staticMetaObject)) {
//        m_boundingVolumeDebugComponent = id;
    } else if (type->inherits(&QComputeCommand::staticMetaObject)) {
        m_computeComponent = id;
    } else if (type->inherits(&QArmature::staticMetaObject)) {
        m_armatureComponent = id;
    }
}

void Entity::removeComponent(Qt3DCore::QNodeId nodeId)
{
    if (m_transformComponent == nodeId) {
        m_transformComponent = QNodeId();
    } else if (m_cameraComponent == nodeId) {
        m_cameraComponent = QNodeId();
    } else if (m_layerComponents.contains(nodeId)) {
        m_layerComponents.removeAll(nodeId);
    } else if (m_levelOfDetailComponents.contains(nodeId)) {
        m_levelOfDetailComponents.removeAll(nodeId);
    } else if (m_rayCasterComponents.contains(nodeId)) {
        m_rayCasterComponents.removeAll(nodeId);
    } else if (m_materialComponent == nodeId) {
        m_materialComponent = QNodeId();
    } else if (m_shaderDataComponents.contains(nodeId)) {
        m_shaderDataComponents.removeAll(nodeId);
    } else if (m_geometryRendererComponent == nodeId) {
        m_geometryRendererComponent = QNodeId();
        m_boundingDirty = true;
    } else if (m_objectPickerComponent == nodeId) {
        m_objectPickerComponent = QNodeId();
//    } else if (m_boundingVolumeDebugComponent == nodeId) {
//        m_boundingVolumeDebugComponent = QNodeId();
    } else if (m_lightComponents.contains(nodeId)) {
        m_lightComponents.removeAll(nodeId);
    } else if (m_environmentLightComponents.contains(nodeId)) {
        m_environmentLightComponents.removeAll(nodeId);
    } else if (m_computeComponent == nodeId) {
        m_computeComponent = QNodeId();
    } else if (m_armatureComponent == nodeId) {
        m_armatureComponent = QNodeId();
    }
}

bool Entity::isBoundingVolumeDirty() const
{
    return m_boundingDirty;
}

void Entity::unsetBoundingVolumeDirty()
{
    m_boundingDirty = false;
}

void Entity::addRecursiveLayerId(const QNodeId layerId)
{
    if (!m_recursiveLayerComponents.contains(layerId) && !m_layerComponents.contains(layerId))
        m_recursiveLayerComponents.push_back(layerId);
}

void Entity::removeRecursiveLayerId(const QNodeId layerId)
{
    m_recursiveLayerComponents.removeOne(layerId);
}

ENTITY_COMPONENT_TEMPLATE_IMPL(Material, HMaterial, MaterialManager, m_materialComponent)
ENTITY_COMPONENT_TEMPLATE_IMPL(CameraLens, HCamera, CameraManager, m_cameraComponent)
ENTITY_COMPONENT_TEMPLATE_IMPL(Transform, HTransform, TransformManager, m_transformComponent)
ENTITY_COMPONENT_TEMPLATE_IMPL(GeometryRenderer, HGeometryRenderer, GeometryRendererManager, m_geometryRendererComponent)
ENTITY_COMPONENT_TEMPLATE_IMPL(ObjectPicker, HObjectPicker, ObjectPickerManager, m_objectPickerComponent)
ENTITY_COMPONENT_TEMPLATE_IMPL(ComputeCommand, HComputeCommand, ComputeCommandManager, m_computeComponent)
ENTITY_COMPONENT_TEMPLATE_IMPL(Armature, HArmature, ArmatureManager, m_armatureComponent)
ENTITY_COMPONENT_LIST_TEMPLATE_IMPL(Layer, HLayer, LayerManager, m_layerComponents)
ENTITY_COMPONENT_LIST_TEMPLATE_IMPL(LevelOfDetail, HLevelOfDetail, LevelOfDetailManager, m_levelOfDetailComponents)
ENTITY_COMPONENT_LIST_TEMPLATE_IMPL(RayCaster, HRayCaster, RayCasterManager, m_rayCasterComponents)
ENTITY_COMPONENT_LIST_TEMPLATE_IMPL(ShaderData, HShaderData, ShaderDataManager, m_shaderDataComponents)
ENTITY_COMPONENT_LIST_TEMPLATE_IMPL(Light, HLight, LightManager, m_lightComponents)
ENTITY_COMPONENT_LIST_TEMPLATE_IMPL(EnvironmentLight, HEnvironmentLight, EnvironmentLightManager, m_environmentLightComponents)

RenderEntityFunctor::RenderEntityFunctor(AbstractRenderer *renderer, NodeManagers *manager)
    : m_nodeManagers(manager)
    , m_renderer(renderer)
{
}

Qt3DCore::QBackendNode *RenderEntityFunctor::create(const Qt3DCore::QNodeCreatedChangeBasePtr &change) const
{
    HEntity renderNodeHandle = m_nodeManagers->renderNodesManager()->getOrAcquireHandle(change->subjectId());
    Entity *entity = m_nodeManagers->renderNodesManager()->data(renderNodeHandle);
    entity->setNodeManagers(m_nodeManagers);
    entity->setHandle(renderNodeHandle);
    entity->setRenderer(m_renderer);
    return entity;
}

Qt3DCore::QBackendNode *RenderEntityFunctor::get(Qt3DCore::QNodeId id) const
{
    return m_nodeManagers->renderNodesManager()->lookupResource(id);
}

void RenderEntityFunctor::destroy(Qt3DCore::QNodeId id) const
{
    m_nodeManagers->renderNodesManager()->releaseResource(id);
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
