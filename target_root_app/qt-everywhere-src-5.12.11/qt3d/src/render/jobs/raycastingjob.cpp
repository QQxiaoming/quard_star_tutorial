/****************************************************************************
**
** Copyright (C) 2018 Klaralvdalens Datakonsult AB (KDAB).
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

#include "raycastingjob_p.h"
#include <Qt3DRender/qgeometryrenderer.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/geometryrenderer_p.h>
#include <Qt3DRender/private/job_common_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/pickboundingvolumeutils_p.h>
#include <Qt3DRender/private/qray3d_p.h>
#include <Qt3DRender/private/sphere_p.h>
#include <Qt3DRender/private/renderer_p.h>
#include <Qt3DRender/private/rendersettings_p.h>
#include <Qt3DRender/private/trianglesvisitor_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DRender;
using namespace Qt3DRender::RayCasting;
using namespace Render;

namespace {

class EntityCasterGatherer
{
public:
    using EntityCasterList = QVector<QPair<Entity *, RayCaster*>>;

    explicit EntityCasterGatherer(Entity *root) : m_root(root), m_needsRefresh(true) { }

    EntityCasterList result() const {
        if (m_needsRefresh) {
            m_result.clear();
            m_result = gatherEntities(m_root, std::move(m_result));
            m_needsRefresh = false;
        }
        return m_result;
    }

private:
    EntityCasterList gatherEntities(Entity *entity, EntityCasterList entities) const
    {
        if (entity != nullptr && entity->isEnabled()) {
            QVector<RayCaster *> components = entity->renderComponents<RayCaster>();
            for (const auto c: qAsConst(components)) {
                if (c->isEnabled())
                    entities.push_back(qMakePair(entity, c));
            }

            // Traverse children
            const auto children = entity->children();
            for (Entity *child : children)
                entities = gatherEntities(child, std::move(entities));
        }
        return entities;
    }

    Entity *m_root;
    mutable EntityCasterList m_result;
    mutable bool m_needsRefresh;
};

} // anonymous


RayCastingJob::RayCastingJob()
    : AbstractPickingJob()
    , m_castersDirty(true)
{
    SET_JOB_RUN_STAT_TYPE(this, JobTypes::RayCasting, 0);
}

void RayCastingJob::markCastersDirty()
{
    m_castersDirty = true;
}

bool RayCastingJob::runHelper()
{
    // Quickly look which caster settings we've got
    // NOTE: should not really cached, we're tracking the state of
    //       RayCaster components but not of the parent entities
    if (m_castersDirty) {
        m_castersDirty = false;
        m_oneEnabledAtLeast = false;

        const auto activeHandles = m_manager->rayCasterManager()->activeHandles();
        for (const auto &handle : activeHandles) {
            const auto caster = m_manager->rayCasterManager()->data(handle);
            m_oneEnabledAtLeast |= caster->isEnabled();
            if (m_oneEnabledAtLeast)
                break;
        }
    }

    // bail out early if no caster is enabled
    if (!m_oneEnabledAtLeast)
        return false;

    const bool trianglePickingRequested = (m_renderSettings->pickMethod() & QPickingSettings::TrianglePicking);
    const bool edgePickingRequested = (m_renderSettings->pickMethod() & QPickingSettings::LinePicking);
    const bool pointPickingRequested = (m_renderSettings->pickMethod() & QPickingSettings::PointPicking);
    const bool primitivePickingRequested = pointPickingRequested | edgePickingRequested | trianglePickingRequested;
    const bool frontFaceRequested =
            m_renderSettings->faceOrientationPickingMode() != QPickingSettings::BackFace;
    const bool backFaceRequested =
            m_renderSettings->faceOrientationPickingMode() != QPickingSettings::FrontFace;
    const float pickWorldSpaceTolerance = m_renderSettings->pickWorldSpaceTolerance();

    EntityCasterGatherer gatherer(m_node);
    const EntityCasterGatherer::EntityCasterList &entities = gatherer.result();

    PickingUtils::ViewportCameraAreaGatherer vcaGatherer;
    const QVector<PickingUtils::ViewportCameraAreaDetails> vcaDetails = vcaGatherer.gather(m_frameGraphRoot);

    const float sceneRayLength = m_node->worldBoundingVolumeWithChildren()->radius() * 3.f;

    for (const EntityCasterGatherer::EntityCasterList::value_type &pair: entities) {
        QVector<QRay3D> rays;

        switch (pair.second->type()) {
        case QAbstractRayCasterPrivate::WorldSpaceRayCaster:
            rays << QRay3D(Vector3D(pair.second->origin()),
                           Vector3D(pair.second->direction()),
                           pair.second->length() > 0.f ? pair.second->length() : sceneRayLength);
            rays.back().transform(*pair.first->worldTransform());
            break;
        case QAbstractRayCasterPrivate::ScreenScapeRayCaster:
            for (const PickingUtils::ViewportCameraAreaDetails &vca : vcaDetails) {
                auto ray = rayForViewportAndCamera(vca, nullptr, pair.second->position());
                if (ray.isValid())
                    rays << ray;
            }
            break;
        default:
            Q_UNREACHABLE();
        }

        for (const QRay3D &ray: qAsConst(rays)) {
            PickingUtils::HitList sphereHits;
            PickingUtils::HierarchicalEntityPicker entityPicker(ray, false);
            entityPicker.setFilterLayers(pair.second->layerIds(), pair.second->filterMode());
            if (entityPicker.collectHits(m_manager, m_node)) {
                if (trianglePickingRequested) {
                    PickingUtils::TriangleCollisionGathererFunctor gathererFunctor;
                    gathererFunctor.m_frontFaceRequested = frontFaceRequested;
                    gathererFunctor.m_backFaceRequested = backFaceRequested;
                    gathererFunctor.m_manager = m_manager;
                    gathererFunctor.m_ray = ray;
                    gathererFunctor.m_objectPickersRequired = false;
                    sphereHits << gathererFunctor.computeHits(entityPicker.entities(), true);
                }
                if (edgePickingRequested) {
                    PickingUtils::LineCollisionGathererFunctor gathererFunctor;
                    gathererFunctor.m_manager = m_manager;
                    gathererFunctor.m_ray = ray;
                    gathererFunctor.m_pickWorldSpaceTolerance = pickWorldSpaceTolerance;
                    gathererFunctor.m_objectPickersRequired = false;
                    sphereHits << gathererFunctor.computeHits(entityPicker.entities(), true);
                    PickingUtils::AbstractCollisionGathererFunctor::sortHits(sphereHits);
                }
                if (pointPickingRequested) {
                    PickingUtils::PointCollisionGathererFunctor gathererFunctor;
                    gathererFunctor.m_manager = m_manager;
                    gathererFunctor.m_ray = ray;
                    gathererFunctor.m_pickWorldSpaceTolerance = pickWorldSpaceTolerance;
                    gathererFunctor.m_objectPickersRequired = false;
                    sphereHits << gathererFunctor.computeHits(entityPicker.entities(), true);
                    PickingUtils::AbstractCollisionGathererFunctor::sortHits(sphereHits);
                }
                if (!primitivePickingRequested) {
                    sphereHits << entityPicker.hits();
                    PickingUtils::AbstractCollisionGathererFunctor::sortHits(sphereHits);
                }
            }

            dispatchHits(pair.second, sphereHits);
        }
    }

    return true;
}

void RayCastingJob::dispatchHits(RayCaster *rayCaster, const PickingUtils::HitList &sphereHits)
{
    QAbstractRayCaster::Hits hits;
    for (const PickingUtils::HitList::value_type &sphereHit: sphereHits) {
        Entity *entity = m_manager->renderNodesManager()->lookupResource(sphereHit.m_entityId);
        Vector3D localIntersection = sphereHit.m_intersection;
        if (entity && entity->worldTransform())
            localIntersection = entity->worldTransform()->inverted() * localIntersection;

        QRayCasterHit::HitType hitType = QRayCasterHit::EntityHit;
        switch (sphereHit.m_type) {
        case RayCasting::QCollisionQueryResult::Hit::Entity:
            break;
        case RayCasting::QCollisionQueryResult::Hit::Triangle:
            hitType = QRayCasterHit::TriangleHit;
            break;
        case RayCasting::QCollisionQueryResult::Hit::Edge:
            hitType = QRayCasterHit::LineHit;
            break;
        case RayCasting::QCollisionQueryResult::Hit::Point:
            hitType = QRayCasterHit::PointHit;
            break;
        default: Q_UNREACHABLE();
        }

        hits << QRayCasterHit{
                hitType,
                sphereHit.m_entityId,
                sphereHit.m_distance,
                convertToQVector3D(localIntersection),
                convertToQVector3D(sphereHit.m_intersection),
                sphereHit.m_primitiveIndex,
                sphereHit.m_vertexIndex[0],
                sphereHit.m_vertexIndex[1],
                sphereHit.m_vertexIndex[2]
        };
    }

    rayCaster->dispatchHits(hits);
}

QT_END_NAMESPACE
