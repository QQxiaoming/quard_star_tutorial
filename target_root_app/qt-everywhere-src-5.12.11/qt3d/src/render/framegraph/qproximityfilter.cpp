/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
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

#include "qproximityfilter.h"
#include "qproximityfilter_p.h"
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qframegraphnodecreatedchange.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

QProximityFilterPrivate::QProximityFilterPrivate()
    : QFrameGraphNodePrivate()
    , m_entity(nullptr)
    , m_distanceThreshold(0.0f)
{
}

/*!
    \class Qt3DRender::QProximityFilter
    \inmodule Qt3DRender
    \since 5.10

    \brief Select entities which are within a distance threshold of a target
    entity.

    A \l Qt3DRender::QProximityFilter can be used to select entities to render
    when they are placed within a given distance threshold of another entity.
*/

/*!
    \property Qt3DRender::QProximityFilter::entity

    Holds the entity against which we should compare the distance to.
*/

/*!
    \property Qt3DRender::QProximityFilter::distanceThreshold

    Holds the distance to the target entity above which entities are filtered
    out.
*/

/*!
    \qmltype ProximityFilter
    \instantiates Qt3DRender::QProximityFilter
    \inherits FrameGraphNode
    \inqmlmodule Qt3D.Render
    \since 5.10

    \brief Select entities which are within a distance threshold of a target
    entity.

    A \l ProximityFilter can be used to select entities to render
    when they are placed within a given distance threshold of another entity.

    \badcode
    import Qt3DRender 2.10
    ...
    RenderSetting {
        Viewport {
            CameraSelector {
                camera: mainCamera
                ProximityFilter {
                    entity: mainCamera
                    distanceThreshold: 50 // select entities within 50m metre radius of mainCamera
                }
            }
        }
    }
    \endcode
*/

/*!
    \qmlproperty Entity Qt3D.Render::ProximityFilter::entity

    Holds the entity against which we should compare the distance to.
 */

/*!
    \qmlproperty real Qt3D.Render::ProximityFilter::distanceThreshold

    Holds the distance to the target entity above which entities are filtered
    out.
 */


QProximityFilter::QProximityFilter(Qt3DCore::QNode *parent)
    : QFrameGraphNode(*new QProximityFilterPrivate, parent)
{

}

/*! \internal */
QProximityFilter::QProximityFilter(QProximityFilterPrivate &dd, QNode *parent)
    : QFrameGraphNode(dd, parent)
{
}

/*! \internal */
QProximityFilter::~QProximityFilter()
{
}

Qt3DCore::QEntity *QProximityFilter::entity() const
{
    Q_D(const QProximityFilter);
    return d->m_entity;
}

float QProximityFilter::distanceThreshold() const
{
    Q_D(const QProximityFilter);
    return d->m_distanceThreshold;
}

void QProximityFilter::setEntity(Qt3DCore::QEntity *entity)
{
    Q_D(QProximityFilter);
    if (d->m_entity != entity) {

        if (d->m_entity)
            d->unregisterDestructionHelper(d->m_entity);

        if (entity && !entity->parent())
            entity->setParent(this);

        d->m_entity = entity;

        if (d->m_entity)
            d->registerDestructionHelper(d->m_entity, &QProximityFilter::setEntity, d->m_entity);

        emit entityChanged(entity);
    }
}

void QProximityFilter::setDistanceThreshold(float distanceThreshold)
{
    Q_D(QProximityFilter);
    if (d->m_distanceThreshold == distanceThreshold)
        return;

    d->m_distanceThreshold = distanceThreshold;
    emit distanceThresholdChanged(distanceThreshold);
}

Qt3DCore::QNodeCreatedChangeBasePtr QProximityFilter::createNodeCreationChange() const
{
    auto creationChange = QFrameGraphNodeCreatedChangePtr<QProximityFilterData>::create(this);
    QProximityFilterData &data = creationChange->data;
    Q_D(const QProximityFilter);
    data.entityId = Qt3DCore::qIdForNode(d->m_entity);
    data.distanceThreshold = d->m_distanceThreshold;
    return creationChange;
}

} // Qt3DRender

QT_END_NAMESPACE
