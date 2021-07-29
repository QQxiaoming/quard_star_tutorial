/****************************************************************************
**
** Copyright (C) 2016 Paul Lemire
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

#include "filterlayerentityjob_p.h"
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/job_common_p.h>
#include <Qt3DRender/private/layerfilternode_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

namespace Render {

namespace {
int layerFilterJobCounter = 0;
} // anonymous

FilterLayerEntityJob::FilterLayerEntityJob()
    : Qt3DCore::QAspectJob()
    , m_manager(nullptr)
{
    SET_JOB_RUN_STAT_TYPE(this, JobTypes::LayerFiltering, layerFilterJobCounter++);
}


void FilterLayerEntityJob::run()
{

    m_filteredEntities.clear();
    if (hasLayerFilter()) // LayerFilter set -> filter
        filterLayerAndEntity();
    else // No LayerFilter set -> retrieve all
        selectAllEntities();

    // sort needed for set_intersection in RenderViewBuilder
    std::sort(m_filteredEntities.begin(), m_filteredEntities.end());
}

// We accept the entity if it contains any of the layers that are in the layer filter
void FilterLayerEntityJob::filterAcceptAnyMatchingLayers(Entity *entity,
                                                         const Qt3DCore::QNodeIdVector &layerIds)
{
    const Qt3DCore::QNodeIdVector entityLayers = entity->layerIds();

    for (const Qt3DCore::QNodeId id : entityLayers) {
        const bool layerAccepted = layerIds.contains(id);

        if (layerAccepted) {
            m_filteredEntities.push_back(entity);
            break;
        }
    }
}

// We accept the entity if it contains all the layers that are in the layer
// filter
void FilterLayerEntityJob::filterAcceptAllMatchingLayers(Entity *entity,
                                                         const Qt3DCore::QNodeIdVector &layerIds)
{
    const Qt3DCore::QNodeIdVector entityLayers = entity->layerIds();
    int layersAccepted = 0;

    for (const Qt3DCore::QNodeId id : entityLayers) {
        if (layerIds.contains(id))
            ++layersAccepted;
    }

    if (layersAccepted == layerIds.size())
        m_filteredEntities.push_back(entity);
}

// We discard the entity if it contains any of the layers that are in the layer
// filter
// In other words that means we select an entity if one of its layers is not on
// the layer filter
void FilterLayerEntityJob::filterDiscardAnyMatchingLayers(Entity *entity,
                                                          const Qt3DCore::QNodeIdVector &layerIds)
{
    const Qt3DCore::QNodeIdVector entityLayers = entity->layerIds();
    bool entityCanBeDiscarded = false;

    for (const Qt3DCore::QNodeId id : entityLayers) {
        if (layerIds.contains(id)) {
            entityCanBeDiscarded =  true;
            break;
        }
    }

    if (!entityCanBeDiscarded)
        m_filteredEntities.push_back(entity);
}

// We discard the entity if it contains all of the layers that are in the layer
// filter
// In other words that means we select an entity if none of its layers are on
// the layer filter
void FilterLayerEntityJob::filterDiscardAllMatchingLayers(Entity *entity,
                                                          const Qt3DCore::QNodeIdVector &layerIds)
{
    const Qt3DCore::QNodeIdVector entityLayers = entity->layerIds();

    int containedLayers = 0;

    for (const Qt3DCore::QNodeId id : layerIds) {
        if (entityLayers.contains(id))
            ++containedLayers;
    }

    if (containedLayers != layerIds.size())
        m_filteredEntities.push_back(entity);
}

void FilterLayerEntityJob::filterLayerAndEntity()
{
    EntityManager *entityManager = m_manager->renderNodesManager();
    const QVector<HEntity> handles = entityManager->activeHandles();

    QVector<Entity *> entitiesToFilter;
    entitiesToFilter.reserve(handles.size());

    for (const HEntity &handle : handles) {
        Entity *entity = entityManager->data(handle);

        if (entity->isTreeEnabled())
            entitiesToFilter.push_back(entity);
    }

    FrameGraphManager *frameGraphManager = m_manager->frameGraphManager();
    LayerManager *layerManager = m_manager->layerManager();

    for (const Qt3DCore::QNodeId layerFilterId : qAsConst(m_layerFilterIds)) {
        LayerFilterNode *layerFilter = static_cast<LayerFilterNode *>(frameGraphManager->lookupNode(layerFilterId));
        Qt3DCore::QNodeIdVector layerIds = layerFilter->layerIds();

        // Remove layerIds which are not active/enabled
        for (int i = layerIds.size() - 1; i >= 0; --i) {
            Layer *backendLayer = layerManager->lookupResource(layerIds.at(i));
            if (backendLayer == nullptr || !backendLayer->isEnabled())
                layerIds.removeAt(i);
        }

        const QLayerFilter::FilterMode filterMode = layerFilter->filterMode();

        // Perform filtering
        for (Entity *entity : entitiesToFilter) {
            switch (filterMode) {
            case QLayerFilter::AcceptAnyMatchingLayers: {
                filterAcceptAnyMatchingLayers(entity, layerIds);
                break;
            }
            case QLayerFilter::AcceptAllMatchingLayers: {
                filterAcceptAllMatchingLayers(entity, layerIds);
                break;
            }
            case QLayerFilter::DiscardAnyMatchingLayers: {
                filterDiscardAnyMatchingLayers(entity, layerIds);
                break;
            }
            case QLayerFilter::DiscardAllMatchingLayers: {
                filterDiscardAllMatchingLayers(entity, layerIds);
                break;
            }
            default:
                Q_UNREACHABLE();
            }
        }

        // Entities to filter for the next frame are the filtered result of the
        // current LayerFilter
        entitiesToFilter = std::move(m_filteredEntities);
    }
    m_filteredEntities = std::move(entitiesToFilter);
}

// No layer filter -> retrieve all entities
void FilterLayerEntityJob::selectAllEntities()
{
    EntityManager *entityManager = m_manager->renderNodesManager();
    const QVector<HEntity> handles = entityManager->activeHandles();

    m_filteredEntities.reserve(handles.size());
    for (const HEntity &handle : handles) {
        Entity *e = entityManager->data(handle);
        if (e->isTreeEnabled())
            m_filteredEntities.push_back(e);
    }
}

} // Render

} // Qt3DRender

QT_END_NAMESPACE
