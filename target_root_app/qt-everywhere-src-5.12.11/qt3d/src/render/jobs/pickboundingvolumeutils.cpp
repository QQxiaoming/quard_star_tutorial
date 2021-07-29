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

#include "pickboundingvolumeutils_p.h"
#include <Qt3DRender/private/geometryrenderer_p.h>
#include <Qt3DRender/private/framegraphnode_p.h>
#include <Qt3DRender/private/cameralens_p.h>
#include <Qt3DRender/private/cameraselectornode_p.h>
#include <Qt3DRender/private/viewportnode_p.h>
#include <Qt3DRender/private/rendersurfaceselector_p.h>
#include <Qt3DRender/private/triangleboundingvolume_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/sphere_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/trianglesvisitor_p.h>
#include <Qt3DRender/private/segmentsvisitor_p.h>
#include <Qt3DRender/private/pointsvisitor_p.h>
#include <Qt3DRender/private/layer_p.h>

#include <vector>
#include <algorithm>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
using namespace Qt3DRender::RayCasting;

namespace Render {

namespace PickingUtils {

void ViewportCameraAreaGatherer::visit(FrameGraphNode *node)
{
    const auto children = node->children();
    for (Render::FrameGraphNode *n : children)
        visit(n);
    if (node->childrenIds().empty())
        m_leaves.push_back(node);
}

ViewportCameraAreaDetails ViewportCameraAreaGatherer::gatherUpViewportCameraAreas(Render::FrameGraphNode *node) const
{
    ViewportCameraAreaDetails vca;
    vca.viewport = QRectF(0.0f, 0.0f, 1.0f, 1.0f);

    while (node) {
        if (node->isEnabled()) {
            switch (node->nodeType()) {
            case FrameGraphNode::CameraSelector:
                vca.cameraId = static_cast<const CameraSelector *>(node)->cameraUuid();
                break;
            case FrameGraphNode::Viewport:
                vca.viewport = ViewportNode::computeViewport(vca.viewport, static_cast<const ViewportNode *>(node));
                break;
            case FrameGraphNode::Surface: {
                auto selector = static_cast<const RenderSurfaceSelector *>(node);
                vca.area = selector->renderTargetSize();
                vca.surface = selector->surface();
                break;
            }
            default:
                break;
            }
        }
        node = node->parent();
    }
    return vca;
}

QVector<ViewportCameraAreaDetails> ViewportCameraAreaGatherer::gather(FrameGraphNode *root)
{
    // Retrieve all leaves
    visit(root);
    QVector<ViewportCameraAreaDetails> vcaTriplets;
    vcaTriplets.reserve(m_leaves.count());

    // Find all viewport/camera pairs by traversing from leaf to root
    for (Render::FrameGraphNode *leaf : qAsConst(m_leaves)) {
        ViewportCameraAreaDetails vcaDetails = gatherUpViewportCameraAreas(leaf);
        if (!m_targetCamera.isNull() && vcaDetails.cameraId != m_targetCamera)
            continue;
        if (!vcaDetails.cameraId.isNull() && isUnique(vcaTriplets, vcaDetails))
            vcaTriplets.push_back(vcaDetails);
    }
    return vcaTriplets;
}

bool ViewportCameraAreaGatherer::isUnique(const QVector<ViewportCameraAreaDetails> &vcaList,
                                          const ViewportCameraAreaDetails &vca) const
{
    for (const ViewportCameraAreaDetails &listItem : vcaList) {
        if (vca.cameraId == listItem.cameraId &&
                vca.viewport == listItem.viewport &&
                vca.surface == listItem.surface &&
                vca.area == listItem.area)
            return false;
    }
    return true;
}

QVector<Entity *> gatherEntities(Entity *entity, QVector<Entity *> entities)
{
    if (entity != nullptr && entity->isEnabled()) {
        entities.push_back(entity);
        // Traverse children
        const auto children = entity->children();
        for (Entity *child : children)
            entities = gatherEntities(child, std::move(entities));
    }
    return entities;
}

EntityGatherer::EntityGatherer(Entity *root)
    : m_root(root)
    , m_needsRefresh(true)
{
}

QVector<Entity *> EntityGatherer::entities() const
{
    if (m_needsRefresh) {
        m_entities.clear();
        m_entities = gatherEntities(m_root, std::move(m_entities));
        m_needsRefresh = false;
    }
    return m_entities;
}

class TriangleCollisionVisitor : public TrianglesVisitor
{
public:
    HitList hits;

    TriangleCollisionVisitor(NodeManagers* manager, const Entity *root, const RayCasting::QRay3D& ray,
                     bool frontFaceRequested, bool backFaceRequested)
        : TrianglesVisitor(manager), m_root(root), m_ray(ray), m_triangleIndex(0)
        , m_frontFaceRequested(frontFaceRequested), m_backFaceRequested(backFaceRequested)
    {
    }

private:
    const Entity *m_root;
    RayCasting::QRay3D m_ray;
    uint m_triangleIndex;
    bool m_frontFaceRequested;
    bool m_backFaceRequested;

    void visit(uint andx, const Vector3D &a,
               uint bndx, const Vector3D &b,
               uint cndx, const Vector3D &c) override;
    bool intersectsSegmentTriangle(uint andx, const Vector3D &a,
                                   uint bndx, const Vector3D &b,
                                   uint cndx, const Vector3D &c);
};

void TriangleCollisionVisitor::visit(uint andx, const Vector3D &a, uint bndx, const Vector3D &b, uint cndx, const Vector3D &c)
{
    const Matrix4x4 &mat = *m_root->worldTransform();
    const Vector3D tA = mat * a;
    const Vector3D tB = mat * b;
    const Vector3D tC = mat * c;

    bool intersected = m_frontFaceRequested &&
            intersectsSegmentTriangle(cndx, tC, bndx, tB, andx, tA);    // front facing
    if (!intersected && m_backFaceRequested) {
        intersected = intersectsSegmentTriangle(andx, tA, bndx, tB, cndx, tC);    // back facing
    }

    m_triangleIndex++;
}


bool TriangleCollisionVisitor::intersectsSegmentTriangle(uint andx, const Vector3D &a, uint bndx, const Vector3D &b, uint cndx, const Vector3D &c)
{
    float t = 0.0f;
    Vector3D uvw;
    bool intersected = Render::intersectsSegmentTriangle(m_ray, a, b, c, uvw, t);
    if (intersected) {
        QCollisionQueryResult::Hit queryResult;
        queryResult.m_type = QCollisionQueryResult::Hit::Triangle;
        queryResult.m_entityId = m_root->peerId();
        queryResult.m_primitiveIndex = m_triangleIndex;
        queryResult.m_vertexIndex[0] = andx;
        queryResult.m_vertexIndex[1] = bndx;
        queryResult.m_vertexIndex[2] = cndx;
        queryResult.m_uvw = uvw;
        queryResult.m_intersection = m_ray.point(t * m_ray.distance());
        queryResult.m_distance = m_ray.projectedDistance(queryResult.m_intersection);
        hits.push_back(queryResult);
    }
    return intersected;
}

class LineCollisionVisitor : public SegmentsVisitor
{
public:
    HitList hits;

    LineCollisionVisitor(NodeManagers* manager, const Entity *root, const RayCasting::QRay3D& ray,
                         float pickWorldSpaceTolerance)
        : SegmentsVisitor(manager), m_root(root), m_ray(ray)
        , m_segmentIndex(0), m_pickWorldSpaceTolerance(pickWorldSpaceTolerance)
    {
    }

private:
    const Entity *m_root;
    RayCasting::QRay3D m_ray;
    uint m_segmentIndex;
    float m_pickWorldSpaceTolerance;

    void visit(uint andx, const Vector3D &a,
               uint bndx, const Vector3D &b) override;
    bool intersectsSegmentSegment(uint andx, const Vector3D &a,
                                  uint bndx, const Vector3D &b);
    bool rayToLineSegment(const Vector3D& lineStart,const Vector3D& lineEnd,
                          float &distance, Vector3D &intersection) const;
};

void LineCollisionVisitor::visit(uint andx, const Vector3D &a, uint bndx, const Vector3D &b)
{
    const Matrix4x4 &mat = *m_root->worldTransform();
    const Vector3D tA = mat * a;
    const Vector3D tB = mat * b;

    intersectsSegmentSegment(andx, tA, bndx, tB);

    m_segmentIndex++;
}

bool LineCollisionVisitor::intersectsSegmentSegment(uint andx, const Vector3D &a,
                                                    uint bndx, const Vector3D &b)
{
    float distance = 0.f;
    Vector3D intersection;
    bool res = rayToLineSegment(a, b, distance, intersection);
    if (res) {
        QCollisionQueryResult::Hit queryResult;
        queryResult.m_type = QCollisionQueryResult::Hit::Edge;
        queryResult.m_entityId = m_root->peerId();
        queryResult.m_primitiveIndex = m_segmentIndex;
        queryResult.m_vertexIndex[0] = andx;
        queryResult.m_vertexIndex[1] = bndx;
        queryResult.m_intersection = intersection;
        queryResult.m_distance = m_ray.projectedDistance(queryResult.m_intersection);
        hits.push_back(queryResult);
        return true;
    }
    return false;
}

bool LineCollisionVisitor::rayToLineSegment(const Vector3D& lineStart,const Vector3D& lineEnd,
                                            float &distance, Vector3D &intersection) const
{
    const float epsilon = 0.00000001f;

    const Vector3D u = m_ray.direction() * m_ray.distance();
    const Vector3D v = lineEnd - lineStart;
    const Vector3D w = m_ray.origin() - lineStart;
    const float a = Vector3D::dotProduct(u, u);
    const float b = Vector3D::dotProduct(u, v);
    const float c = Vector3D::dotProduct(v, v);
    const float d = Vector3D::dotProduct(u, w);
    const float e = Vector3D::dotProduct(v, w);
    const float D = a * c - b * b;
    float sc, sN, sD = D;
    float tc, tN, tD = D;

    if (D < epsilon) {
        sN = 0.0;
        sD = 1.0;
        tN = e;
        tD = c;
    } else {
        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < 0.0) {
            sN = 0.0;
            tN = e;
            tD = c;
        }
    }

    if (tN < 0.0) {
        tN = 0.0;
        if (-d < 0.0)
            sN = 0.0;
        else {
            sN = -d;
            sD = a;
        }
    } else if (tN > tD) {
        tN = tD;
        if ((-d + b) < 0.0)
            sN = 0;
        else {
            sN = (-d + b);
            sD = a;
        }
    }

    sc = (qAbs(sN) < epsilon ? 0.0f : sN / sD);
    tc = (qAbs(tN) < epsilon ? 0.0f : tN / tD);

    const Vector3D dP = w + (sc * u) - (tc * v);
    const float f = dP.length();
    if (f < m_pickWorldSpaceTolerance) {
        distance = sc * u.length();
        intersection = lineStart + v * tc;
        return true;
    }
    return false;
}

class PointCollisionVisitor : public PointsVisitor
{
public:
    HitList hits;

    PointCollisionVisitor(NodeManagers* manager, const Entity *root, const RayCasting::QRay3D& ray,
                          float pickWorldSpaceTolerance)
        : PointsVisitor(manager), m_root(root), m_ray(ray)
        , m_pointIndex(0), m_pickWorldSpaceTolerance(pickWorldSpaceTolerance)
    {
    }

private:
    const Entity *m_root;
    RayCasting::QRay3D m_ray;
    uint m_pointIndex;
    float m_pickWorldSpaceTolerance;

    void visit(uint ndx, const Vector3D &p) override;

    double pointToRayDistance(const Vector3D &a, Vector3D &p)
    {
        const Vector3D v = a - m_ray.origin();
        const double t = Vector3D::dotProduct(v, m_ray.direction());
        p = m_ray.origin() + t * m_ray.direction();
        return (p - a).length();
    }
};


void PointCollisionVisitor::visit(uint ndx, const Vector3D &p)
{
    const Matrix4x4 &mat = *m_root->worldTransform();
    const Vector3D tP = mat * p;
    Vector3D intersection;

    float d = pointToRayDistance(tP, intersection);
    if (d < m_pickWorldSpaceTolerance) {
        QCollisionQueryResult::Hit queryResult;
        queryResult.m_type = QCollisionQueryResult::Hit::Point;
        queryResult.m_entityId = m_root->peerId();
        queryResult.m_primitiveIndex = m_pointIndex;
        queryResult.m_vertexIndex[0] = ndx;
        queryResult.m_intersection = intersection;
        queryResult.m_distance = d;
        hits.push_back(queryResult);
    }

    m_pointIndex++;
}

HitList reduceToFirstHit(HitList &result, const HitList &intermediate)
{
    if (!intermediate.empty()) {
        if (result.empty())
            result.push_back(intermediate.front());
        float closest = result.front().m_distance;
        for (const auto &v : intermediate) {
            if (v.m_distance < closest) {
                result.push_front(v);
                closest = v.m_distance;
            }
        }

        while (result.size() > 1)
            result.pop_back();
    }
    return result;
}

HitList reduceToAllHits(HitList &results, const HitList &intermediate)
{
    if (!intermediate.empty())
        results << intermediate;
    return results;
}

AbstractCollisionGathererFunctor::AbstractCollisionGathererFunctor()
    : m_manager(nullptr)
{ }

AbstractCollisionGathererFunctor::~AbstractCollisionGathererFunctor()
{ }

HitList AbstractCollisionGathererFunctor::operator ()(const Entity *entity) const
{
    if (m_objectPickersRequired) {
        HObjectPicker objectPickerHandle = entity->componentHandle<ObjectPicker>();

        // If the Entity which actually received the hit doesn't have
        // an object picker component, we need to check the parent if it has one ...
        auto parentEntity = entity;
        while (objectPickerHandle.isNull() && parentEntity != nullptr) {
            parentEntity = parentEntity->parent();
            if (parentEntity != nullptr)
                objectPickerHandle = parentEntity->componentHandle<ObjectPicker>();
        }

        ObjectPicker *objectPicker = m_manager->objectPickerManager()->data(objectPickerHandle);
        if (objectPicker == nullptr || !objectPicker->isEnabled())
            return {};   // don't bother picking entities that don't
                         // have an object picker, or if it's disabled
    }

    return pick(entity);
}

bool AbstractCollisionGathererFunctor::rayHitsEntity(const Entity *entity) const
{
    QRayCastingService rayCasting;
    const QCollisionQueryResult::Hit queryResult = rayCasting.query(m_ray, entity->worldBoundingVolume());
    return queryResult.m_distance >= 0.f;
}

void AbstractCollisionGathererFunctor::sortHits(HitList &results)
{
    auto compareHitsDistance = [](const HitList::value_type &a,
                                  const HitList::value_type &b) {
        return a.m_distance < b.m_distance;
    };
    std::sort(results.begin(), results.end(), compareHitsDistance);
}

namespace {

// Workaround to avoid passing *this into the blockMappedReduce calls for the
// mapFunctor which would cause an SSE alignment error on Windows Also note
// that a lambda doesn't work since we need the typedef result_type defined to
// work with QtConcurrent
struct MapFunctorHolder
{
    MapFunctorHolder(const AbstractCollisionGathererFunctor *gatherer)
        : m_gatherer(gatherer)
    {}

    // This define is required to work with QtConcurrent
    typedef HitList result_type;
    HitList operator ()(const Entity *e) const { return m_gatherer->operator ()(e); }

    const AbstractCollisionGathererFunctor *m_gatherer;
};

} // anonymous

HitList EntityCollisionGathererFunctor::computeHits(const QVector<Entity *> &entities, bool allHitsRequested)
{
    const auto reducerOp = allHitsRequested ? PickingUtils::reduceToAllHits : PickingUtils::reduceToFirstHit;
    const MapFunctorHolder holder(this);
#if QT_CONFIG(concurrent)
    return QtConcurrent::blockingMappedReduced<HitList>(entities, holder, reducerOp);
#else
    HitList sphereHits;
    QVector<PickingUtils::EntityCollisionGathererFunctor::result_type> results;
    for (const Entity *entity : entities)
        sphereHits = reducerOp(sphereHits, holder(entity));
    return sphereHits;
#endif
}

HitList EntityCollisionGathererFunctor::pick(const Entity *entity) const
{
    HitList result;

    QRayCastingService rayCasting;
    const QCollisionQueryResult::Hit queryResult = rayCasting.query(m_ray, entity->worldBoundingVolume());
    if (queryResult.m_distance >= 0.f)
        result.push_back(queryResult);

    return result;
}

HitList TriangleCollisionGathererFunctor::computeHits(const QVector<Entity *> &entities, bool allHitsRequested)
{
    const auto reducerOp = allHitsRequested ? PickingUtils::reduceToAllHits : PickingUtils::reduceToFirstHit;
    const MapFunctorHolder holder(this);
#if QT_CONFIG(concurrent)
    return QtConcurrent::blockingMappedReduced<HitList>(entities, holder, reducerOp);
#else
    HitList sphereHits;
    QVector<PickingUtils::TriangleCollisionGathererFunctor::result_type> results;
    for (const Entity *entity : entities)
        sphereHits = reducerOp(sphereHits, holder(entity));
        return sphereHits;
#endif
}

HitList TriangleCollisionGathererFunctor::pick(const Entity *entity) const
{
    HitList result;

    GeometryRenderer *gRenderer = entity->renderComponent<GeometryRenderer>();
    if (!gRenderer)
        return result;

    if (rayHitsEntity(entity)) {
        TriangleCollisionVisitor visitor(m_manager, entity, m_ray, m_frontFaceRequested, m_backFaceRequested);
        visitor.apply(gRenderer, entity->peerId());
        result = visitor.hits;

        sortHits(result);
    }

    return result;
}

HitList LineCollisionGathererFunctor::computeHits(const QVector<Entity *> &entities, bool allHitsRequested)
{
    const auto reducerOp = allHitsRequested ? PickingUtils::reduceToAllHits : PickingUtils::reduceToFirstHit;
    const MapFunctorHolder holder(this);
#if QT_CONFIG(concurrent)
    return QtConcurrent::blockingMappedReduced<HitList>(entities, holder, reducerOp);
#else
    HitList sphereHits;
    QVector<PickingUtils::LineCollisionGathererFunctor::result_type> results;
    for (const Entity *entity : entities)
        sphereHits = reducerOp(sphereHits, holder(entity));
    return sphereHits;
#endif
}

HitList LineCollisionGathererFunctor::pick(const Entity *entity) const
{
    HitList result;

    GeometryRenderer *gRenderer = entity->renderComponent<GeometryRenderer>();
    if (!gRenderer)
        return result;

    if (rayHitsEntity(entity)) {
        LineCollisionVisitor visitor(m_manager, entity, m_ray, m_pickWorldSpaceTolerance);
        visitor.apply(gRenderer, entity->peerId());
        result = visitor.hits;
        sortHits(result);
    }

    return result;
}

HitList PointCollisionGathererFunctor::computeHits(const QVector<Entity *> &entities, bool allHitsRequested)
{
    const auto reducerOp = allHitsRequested ? PickingUtils::reduceToAllHits : PickingUtils::reduceToFirstHit;
    const MapFunctorHolder holder(this);
#if QT_CONFIG(concurrent)
    return QtConcurrent::blockingMappedReduced<HitList>(entities, holder, reducerOp);
#else
    HitList sphereHits;
    QVector<PickingUtils::PointCollisionGathererFunctor::result_type> results;
    for (const Entity *entity : entities)
        sphereHits = reducerOp(sphereHits, holder(entity));
    return sphereHits;
#endif
}

HitList PointCollisionGathererFunctor::pick(const Entity *entity) const
{
    HitList result;

    GeometryRenderer *gRenderer = entity->renderComponent<GeometryRenderer>();
    if (!gRenderer)
        return result;

    if (gRenderer->primitiveType() != Qt3DRender::QGeometryRenderer::Points)
        return result;

    if (rayHitsEntity(entity)) {
        PointCollisionVisitor visitor(m_manager, entity, m_ray, m_pickWorldSpaceTolerance);
        visitor.apply(gRenderer, entity->peerId());
        result = visitor.hits;
        sortHits(result);
    }

    return result;
}

HierarchicalEntityPicker::HierarchicalEntityPicker(const QRay3D &ray, bool requireObjectPicker)
    : m_ray(ray)
    , m_objectPickersRequired(requireObjectPicker)
    , m_filterMode(QAbstractRayCaster::AcceptAnyMatchingLayers)
{

}

void HierarchicalEntityPicker::setFilterLayers(const Qt3DCore::QNodeIdVector &layerIds, QAbstractRayCaster::FilterMode mode)
{
    m_filterMode = mode;
    m_layerIds = layerIds;
    std::sort(m_layerIds.begin(), m_layerIds.end());
}

bool HierarchicalEntityPicker::collectHits(NodeManagers *manager, Entity *root)
{
    m_hits.clear();
    m_entities.clear();

    QRayCastingService rayCasting;
    struct EntityData {
        Entity* entity;
        bool hasObjectPicker;
        Qt3DCore::QNodeIdVector recursiveLayers;
    };
    std::vector<EntityData> worklist;
    worklist.push_back({root, !root->componentHandle<ObjectPicker>().isNull(), {}});

    LayerManager *layerManager = manager->layerManager();

    while (!worklist.empty()) {
        EntityData current = worklist.back();
        worklist.pop_back();

        bool accepted = true;
        if (m_layerIds.size()) {
            // TODO investigate reusing logic from LayerFilter job
            Qt3DCore::QNodeIdVector filterLayers = current.recursiveLayers + current.entity->componentsUuid<Layer>();

            // remove disabled layers
            filterLayers.erase(std::remove_if(filterLayers.begin(), filterLayers.end(),
                                              [layerManager](const Qt3DCore::QNodeId layerId) {
                Layer *layer = layerManager->lookupResource(layerId);
                return !layer || !layer->isEnabled();
            }), filterLayers.end());

            std::sort(filterLayers.begin(), filterLayers.end());

            Qt3DCore::QNodeIdVector commonIds;
            std::set_intersection(m_layerIds.cbegin(), m_layerIds.cend(),
                                  filterLayers.cbegin(), filterLayers.cend(),
                                  std::back_inserter(commonIds));

            switch (m_filterMode) {
            case QAbstractRayCaster::AcceptAnyMatchingLayers: {
                accepted = !commonIds.empty();
                break;
            }
            case QAbstractRayCaster::AcceptAllMatchingLayers: {
                accepted = commonIds == m_layerIds;
                break;
            }
            case QAbstractRayCaster::DiscardAnyMatchingLayers: {
                accepted = commonIds.empty();
                break;
            }
            case QAbstractRayCaster::DiscardAllMatchingLayers: {
                accepted = !(commonIds == m_layerIds);
                break;
            }
            default:
                Q_UNREACHABLE();
                break;
            }
        }

        // first pick entry sub-scene-graph
        QCollisionQueryResult::Hit queryResult =
                rayCasting.query(m_ray, current.entity->worldBoundingVolumeWithChildren());
        if (queryResult.m_distance < 0.f)
            continue;

        // if we get a hit, we check again for this specific entity
        queryResult = rayCasting.query(m_ray, current.entity->worldBoundingVolume());
        if (accepted && queryResult.m_distance >= 0.f && (current.hasObjectPicker || !m_objectPickersRequired)) {
            m_entities.push_back(current.entity);
            m_hits.push_back(queryResult);
        }

        Qt3DCore::QNodeIdVector recursiveLayers;
        const Qt3DCore::QNodeIdVector entityLayers = current.entity->componentsUuid<Layer>();
        for (const Qt3DCore::QNodeId layerId : entityLayers) {
            Layer *layer = layerManager->lookupResource(layerId);
            if (layer->recursive())
                recursiveLayers << layerId;
        }

        // and pick children
        const auto children = current.entity->children();
        for (auto child: children)
            worklist.push_back({child, current.hasObjectPicker || !child->componentHandle<ObjectPicker>().isNull(),
                                current.recursiveLayers + recursiveLayers});
    }

    return !m_hits.empty();
}

} // PickingUtils

} // Render

} // Qt3DRender

QT_END_NAMESPACE
