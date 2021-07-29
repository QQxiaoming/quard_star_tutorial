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

#include "qabstractraycaster.h"
#include "qabstractraycaster_p.h"
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>
#include <Qt3DCore/private/qcomponent_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DRender/qlayer.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

QAbstractRayCasterPrivate::QAbstractRayCasterPrivate()
    : QComponentPrivate()
{
    m_enabled = false;
    m_shareable = false;
}

QAbstractRayCasterPrivate *QAbstractRayCasterPrivate::get(QAbstractRayCaster *obj)
{
    return obj->d_func();
}

const QAbstractRayCasterPrivate *QAbstractRayCasterPrivate::get(const QAbstractRayCaster *obj)
{
    return obj->d_func();
}

void QAbstractRayCasterPrivate::updateHitEntites(QAbstractRayCaster::Hits &hits, Qt3DCore::QScene *scene)
{
    for (int i = 0; i < hits.size(); i++)
        hits[i].setEntity(qobject_cast<Qt3DCore::QEntity *>(scene->lookupNode(hits[i].entityId())));
}

void QAbstractRayCasterPrivate::dispatchHits(const QAbstractRayCaster::Hits &hits)
{
    Q_Q(QAbstractRayCaster);
    m_hits = hits;
    updateHitEntites(m_hits, m_scene);
    bool v = q->blockNotifications(true);
    emit q->hitsChanged(m_hits);
    q->blockNotifications(v);
}

/*!
    \class Qt3DRender::QAbstractRayCaster
    \brief An abstract base class for ray casting in 3d scenes.
    \inmodule Qt3DRender
    \since 5.11
    \inherits QComponent

    Qt3DRender::QAbstractRayCaster is an abstract base class for casting rays into a 3d scene.
    Qt3DRender::QAbstractRayCaster can not be directly instantiated, but rather
    through its subclasses. QAbstractRayCaster specifies common properties
    for all ray casters, such as run mode and layer handling, while leaving the actual
    ray casting details to the subclasses.

    Ray castings differs from picking (using Qt3DRender::QObjectPicker) in that it does not
    require mouse events to trigger.

    By default, the instances of Qt3DRender::QAbstractRayCaster are disabled. When enabled,
    the specified ray will be tested for intersecting objects at every frame. The
    QAbstractRayCaster::hits property will be updated with the results of the ray casting,
    even if no objects are found.

    The Qt3DRender::QPickingSettings can be used to control the ray casting, such as which
    primitives are tested and how the results are returned.

    Furthermore, Qt3DRender::QLayer components can be used to control how entities, or entity
    sub-graphs, react to ray casting.

    \note Components derived from QAbstractRayCaster should not be shared amount multiple entities.

    \sa Qt3DRender::QRayCaster, Qt3DRender::QScreenRayCaster, Qt3DRender::QObjectPicker, Qt3DRender::QPickingSettings
*/
/*!
    \qmltype AbstractRayCaster
    \brief An abstract base class for ray casting in 3d scenes.
    \inqmlmodule Qt3D.Render
    \since 5.11
    \instantiates Qt3DRender::QAbstractRayCaster

    AbstractRayCaster is an abstract base class for casting rays into a 3d scene.
    AbstractRayCaster can not be directly instantiated, but rather
    through its subclasses. QAbstractRayCaster specifies common properties
    for all ray casters, such as run mode and layer handling, while leaving the actual
    ray casting details to the subclasses.

    Ray castings differs from picking (using ObjectPicker) in that it does not
    require mouse events to trigger.

    By default, the instances of AbstractRayCaster are disabled. When enabled,
    the specified ray will be tested for intersecting objects at every frame. The
    AbstractRayCaster.hits property will be updated with the results of the ray casting,
    even if no objects are found.

    The Qt3D.Render::PickingSettings can be used to control the ray casting, such as which
    primitives are tested and how the results are returned.

    Furthermore, Qt3D.Render::Layer components can be used to control how entities, or entity
    sub-graphs, react to ray casting.

    Note: components derived from AbstractRayCaster should not be shared amount multiple entities.

    \sa Qt3D.Render::RayCaster, Qt3D.Render::ScreenRayCaster, Qt3D.Render::ObjectPicker, Qt3D.Render::PickingSettings
*/

/*!
    \enum QAbstractRayCaster::RunMode

    This enumeration specifies how often ray casting is performed
    \value Continuous Ray casting is performed at every frame as long as the component is enabled.
    \value SingleShot Ray casting is done once then the component disables itself. This is the default
*/
/*!
    \enum QAbstractRayCaster::FilterMode

    Specifies the rules for selecting entities to test for raycasting.

    \value AcceptAnyMatchingLayers
    Accept entities that reference one or more \l QLayer objects added to this
    QAbstractRayCaster. This is the default

    \value AcceptAllMatchingLayers
    Accept entities that reference all the \l QLayer objects added to this QAbstractRayCaster

    \value DiscardAnyMatchingLayers
    Discard entities that reference one or more \l QLayer objects added to this QAbstractRayCaster

    \value DiscardAllMatchingLayers
    Discard entities that reference all \l QLayer objects added to this QAbstractRayCaster
*/

/*!
    \property Qt3DRender::QAbstractRayCaster::filterMode

    Holds the filter mode specifying the entities to select for ray casting tests.

    The default value is AcceptMatchingLayers.
*/
/*!
    \qmlproperty enumeration AbstractRayCaster::filterMode

    Holds the filter mode specifying the entities to select for ray casting tests.

    The default value is \c {AbstractRayCaster.AcceptMatchingLayers}.

    \value AcceptAnyMatchingLayers
    Accept entities that reference one or more \l Layer objects added to this
    AbstractRayCaster. This is the default

    \value AcceptAllMatchingLayers
    Accept entities that reference all the \l Layer objects added to this AbstractRayCaster

    \value DiscardAnyMatchingLayers
    Discard entities that reference one or more \l Layer objects added to this AbstractRayCaster

    \value DiscardAllMatchingLayers
    Discard entities that reference all \l Layer objects added to this AbstractRayCaster
*/

/*!
    \property Qt3DRender::QAbstractRayCaster::runMode

    Holds the run mode controlling how often ray casting tests are performed.

    If set to SingleShot (the default), when the component is enabled, a single ray casting
    test will be performed and the component will automatically disable itself.

    If set to Continuous, ray casting tests will be performed at every frame as long as
    the component is enabled.
*/
/*!
    \qmlproperty enumeration AbstractRayCaster::runMode
    Holds the run mode controlling how often ray casting tests are performed.

    \value Continuous Ray casting is performed at every frame as long as the component is enabled.

    \value SingleShot Ray casting is done once then the component disables itself. This is the default
*/

/*!
    \property Qt3DRender::QAbstractRayCaster::hits

    Holds the results of last ray casting test as a vector of Qt3DRender::QRayCasterHit instances.

    Note that even if successive tests return the exact same results (or empty results), a
    change notification will be emitted at every test.
*/

/*!
    \qmlproperty array AbstractRayCaster::hits

    Holds the results of last ray casting test as an array of javascript objects. The fields
    defined on the objects are defined below.

    \code
    {
        type // enum value of RayCasterHit.HitType
        entity // entity that was intersected
        distance // distance from ray origin to intersection
        localIntersection.x: // coordinate of intersection in the entity's coordinate system
        localIntersection.y
        localIntersection.z
        worldIntersection.x // coordinate of intersection in the model's coordinate system
        worldIntersection.y
        worldIntersection.z
        primitiveIndex // index of the primitive (triangle, line, point) that was intersected;
                       // (undefined if the picking mode is set to bounding volume)
        vertex1Index // index of the first point of the triangle or line that was intersected
                     // (undefined if the picking mode is set to bounding volume or points)
        vertex2Index // index of the second point of the triangle or line that was intersected
                     // (undefined if the picking mode is set to bounding volume or points)
        vertex3Index // index of the second point of the triangle that was intersected
                     // (undefined if the picking mode is set to bounding volume, points or lines)
    }
    \endcode

    Note that even if successive tests return the exact same results (or empty results), a
    change notification will be emitted at every test.
*/


QAbstractRayCaster::QAbstractRayCaster(Qt3DCore::QNode *parent)
    : Qt3DCore::QComponent(*new QAbstractRayCasterPrivate(), parent)
{
}

/*! \internal */
QAbstractRayCaster::QAbstractRayCaster(QAbstractRayCasterPrivate &dd, Qt3DCore::QNode *parent)
    : Qt3DCore::QComponent(dd, parent)
{
}

/*! \internal */
QAbstractRayCaster::~QAbstractRayCaster()
{
}

QAbstractRayCaster::RunMode QAbstractRayCaster::runMode() const
{
    Q_D(const QAbstractRayCaster);
    return d->m_runMode;
}

void QAbstractRayCaster::setRunMode(QAbstractRayCaster::RunMode runMode)
{
    Q_D(QAbstractRayCaster);
    if (d->m_runMode != runMode) {
        d->m_runMode = runMode;
        emit runModeChanged(d->m_runMode);
    }
}

QAbstractRayCaster::FilterMode QAbstractRayCaster::filterMode() const
{
    Q_D(const QAbstractRayCaster);
    return d->m_filterMode;
}

void QAbstractRayCaster::setFilterMode(QAbstractRayCaster::FilterMode filterMode)
{
    Q_D(QAbstractRayCaster);
    if (d->m_filterMode != filterMode) {
        d->m_filterMode = filterMode;
        emit filterModeChanged(d->m_filterMode);
    }
}

QAbstractRayCaster::Hits QAbstractRayCaster::hits() const
{
    Q_D(const QAbstractRayCaster);
    return d->m_hits;
}

/*!
    Add \a layer to the current list of layers
 */
void QAbstractRayCaster::addLayer(QLayer *layer)
{
    Q_ASSERT(layer);
    Q_D(QAbstractRayCaster);
    if (!d->m_layers.contains(layer)) {
        d->m_layers.append(layer);

        // Ensures proper bookkeeping
        d->registerDestructionHelper(layer, &QAbstractRayCaster::removeLayer, d->m_layers);

        // We need to add it as a child of the current node if it has been declared inline
        // Or not previously added as a child of the current node so that
        // 1) The backend gets notified about it's creation
        // 2) When the current node is destroyed, it gets destroyed as well
        if (!layer->parent())
            layer->setParent(this);

        if (d->m_changeArbiter != nullptr) {
            const auto change = Qt3DCore::QPropertyNodeAddedChangePtr::create(id(), layer);
            change->setPropertyName("layer");
            d->notifyObservers(change);
        }
    }
}

/*!
    Remove \a layer from the current list of layers
 */
void QAbstractRayCaster::removeLayer(QLayer *layer)
{
    Q_ASSERT(layer);
    Q_D(QAbstractRayCaster);
    if (d->m_changeArbiter != nullptr) {
        const auto change = Qt3DCore::QPropertyNodeRemovedChangePtr::create(id(), layer);
        change->setPropertyName("layer");
        d->notifyObservers(change);
    }
    d->m_layers.removeOne(layer);
    // Remove bookkeeping connection
    d->unregisterDestructionHelper(layer);
}

/*!
    \return the current list of layers
 */
QVector<QLayer *> QAbstractRayCaster::layers() const
{
    Q_D(const QAbstractRayCaster);
    return d->m_layers;
}

/*! \internal */
void QAbstractRayCaster::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change)
{
    Q_D(QAbstractRayCaster);
    Qt3DCore::QPropertyUpdatedChangePtr e = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(change);
    if (e->type() == Qt3DCore::PropertyUpdated) {
        const QByteArray propertyName = e->propertyName();
        if (propertyName == QByteArrayLiteral("hits")) {
            Hits hits = e->value().value<Hits>();
            d->dispatchHits(hits);
        }
    }

    QComponent::sceneChangeEvent(change);
}

/*! \internal */
Qt3DCore::QNodeCreatedChangeBasePtr QAbstractRayCaster::createNodeCreationChange() const
{
    auto creationChange = Qt3DCore::QNodeCreatedChangePtr<QAbstractRayCasterData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QAbstractRayCaster);
    data.casterType = d->m_rayCasterType;
    data.runMode = d->m_runMode;
    data.origin = d->m_origin;
    data.direction = d->m_direction;
    data.length = d->m_length;
    data.position = d->m_position;
    data.filterMode = d->m_filterMode;
    data.layerIds = qIdsForNodes(d->m_layers);
    return creationChange;
}

} // Qt3DRender

QT_END_NAMESPACE
