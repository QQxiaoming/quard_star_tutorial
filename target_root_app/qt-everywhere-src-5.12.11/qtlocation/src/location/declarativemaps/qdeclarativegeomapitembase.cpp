/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativegeomapitembase_p.h"
#include "qgeocameradata_p.h"
#include <QtLocation/private/qgeomap_p.h>
#include <QtQml/QQmlInfo>
#include <QtQuick/QSGOpacityNode>
#include <QtQuick/private/qquickmousearea_p.h>
#include <QtQuick/private/qquickitem_p.h>

QT_BEGIN_NAMESPACE

QGeoMapViewportChangeEvent::QGeoMapViewportChangeEvent()
    : zoomLevelChanged(false),
      centerChanged(false),
      mapSizeChanged(false),
      tiltChanged(false),
      bearingChanged(false),
      rollChanged(false)
{
}

QGeoMapViewportChangeEvent::QGeoMapViewportChangeEvent(const QGeoMapViewportChangeEvent &other)
{
    this->operator=(other);
}

QGeoMapViewportChangeEvent &QGeoMapViewportChangeEvent::operator=(const QGeoMapViewportChangeEvent &other)
{
    if (this == &other)
        return (*this);

    cameraData = other.cameraData;
    mapSize = other.mapSize;
    zoomLevelChanged = other.zoomLevelChanged;
    centerChanged = other.centerChanged;
    mapSizeChanged = other.mapSizeChanged;
    tiltChanged = other.tiltChanged;
    bearingChanged = other.bearingChanged;
    rollChanged = other.rollChanged;

    return (*this);
}

QDeclarativeGeoMapItemBase::QDeclarativeGeoMapItemBase(QQuickItem *parent)
:   QQuickItem(parent), map_(0), quickMap_(0), parentGroup_(0)
{
    setFiltersChildMouseEvents(true);
    connect(this, SIGNAL(childrenChanged()),
            this, SLOT(afterChildrenChanged()));
    // Changing opacity on a mapItemGroup should affect also the opacity on the children.
    // This must be notified to plugins, if they are to render the item.
    connect(this, &QQuickItem::opacityChanged, this, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged);
}

QDeclarativeGeoMapItemBase::~QDeclarativeGeoMapItemBase()
{
    disconnect(this, SLOT(afterChildrenChanged()));
    if (quickMap_)
        quickMap_->removeMapItem(this);
}

/*!
    \internal
*/
void QDeclarativeGeoMapItemBase::afterChildrenChanged()
{
    QList<QQuickItem *> kids = childItems();
    if (kids.size() > 0) {
        bool printedWarning = false;
        foreach (QQuickItem *i, kids) {
            if (i->flags() & QQuickItem::ItemHasContents
                    && !qobject_cast<QQuickMouseArea *>(i)) {
                if (!printedWarning) {
                    qmlWarning(this) << "Geographic map items do not support child items";
                    printedWarning = true;
                }

                qmlWarning(i) << "deleting this child";
                i->deleteLater();
            }
        }
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMapItemBase::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    if (quickMap == quickMap_)
        return;
    if (quickMap && quickMap_)
        return; // don't allow association to more than one map

    quickMap_ = quickMap;
    map_ = map;

    if (map_ && quickMap_) {
        // For performance reasons we're not connecting map_'s and quickMap_'s signals to this.
        // Rather, the handling of cameraDataChanged, visibleAreaChanged, heightChanged and widthChanged is done explicitly in QDeclarativeGeoMap by directly calling methods on the items.
        // See QTBUG-76950
        lastSize_ = QSizeF(quickMap_->width(), quickMap_->height());
        lastCameraData_ = map_->cameraData();
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMapItemBase::baseCameraDataChanged(const QGeoCameraData &cameraData)
{
    QGeoMapViewportChangeEvent evt;
    evt.cameraData = cameraData;
    evt.mapSize = QSizeF(quickMap_->width(), quickMap_->height());

    if (evt.mapSize != lastSize_)
        evt.mapSizeChanged = true;

    if (cameraData.bearing() != lastCameraData_.bearing())
        evt.bearingChanged = true;
    if (cameraData.center() != lastCameraData_.center())
        evt.centerChanged = true;
    if (cameraData.roll() != lastCameraData_.roll())
        evt.rollChanged = true;
    if (cameraData.tilt() != lastCameraData_.tilt())
        evt.tiltChanged = true;
    if (cameraData.zoomLevel() != lastCameraData_.zoomLevel())
        evt.zoomLevelChanged = true;

    lastSize_ = evt.mapSize;
    lastCameraData_ = cameraData;

    afterViewportChanged(evt);
}

void QDeclarativeGeoMapItemBase::visibleAreaChanged()
{
    QGeoMapViewportChangeEvent evt;
    evt.mapSize = QSizeF(quickMap_->width(), quickMap_->height());
    afterViewportChanged(evt);
}

/*!
    \internal
*/
void QDeclarativeGeoMapItemBase::setPositionOnMap(const QGeoCoordinate &coordinate, const QPointF &offset)
{
    if (!map_ || !quickMap_)
        return;

    QDoubleVector2D pos;
    if (map()->geoProjection().projectionType() == QGeoProjection::ProjectionWebMercator) {
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map()->geoProjection());
        QDoubleVector2D wrappedProjection = p.geoToWrappedMapProjection(coordinate);
        if (!p.isProjectable(wrappedProjection))
            return;
        pos = p.wrappedMapProjectionToItemPosition(wrappedProjection);
    } else {
        pos = map()->geoProjection().coordinateToItemPosition(coordinate, false);
        if (qIsNaN(pos.x()))
            return;
    }

    QPointF topLeft = pos.toPointF() - offset;

    setPosition(topLeft);
}

static const double opacityRampMin = 1.5;
static const double opacityRampMax = 2.5;
/*!
    \internal
*/
float QDeclarativeGeoMapItemBase::zoomLevelOpacity() const
{
    if (quickMap_->zoomLevel() > opacityRampMax)
        return 1.0;
    else if (quickMap_->zoomLevel() > opacityRampMin)
        return quickMap_->zoomLevel() - opacityRampMin;
    else
        return 0.0;
}

bool QDeclarativeGeoMapItemBase::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    Q_UNUSED(item)
    if (event->type() == QEvent::MouseButtonPress && !contains(static_cast<QMouseEvent*>(event)->pos())) {
        // In case of items that are not rectangles, this filter is used to test if the event has landed
        // inside the actual item shape.
        // If so, the method returns true, meaning that it prevents the event delivery to child "*item" (for example,
        // a mouse area that is on top of this map item).
        // However, this method sets "accepted" to false, so that the event can still be passed further up,
        // specifically to the parent Map, that is a sort of flickable.
        // Otherwise, if the event is not contained within the map item, the method returns false, meaning the event
        // is delivered to the child *item (like the mouse area associated).
        event->setAccepted(false);
        return true;
    }
    return false;
}

/*!
    \internal
*/
QSGNode *QDeclarativeGeoMapItemBase::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *pd)
{
    if (!map_ || !quickMap_ || map_->supportedMapItemTypes() & itemType()) {
        if (oldNode)
            delete oldNode;
        oldNode = 0;
        return 0;
    }

    QSGOpacityNode *opn = static_cast<QSGOpacityNode *>(oldNode);
    if (!opn)
        opn = new QSGOpacityNode();

    opn->setOpacity(zoomLevelOpacity());

    QSGNode *oldN = opn->childCount() ? opn->firstChild() : 0;
    opn->removeAllChildNodes();
    if (opn->opacity() > 0.0) {
        QSGNode *n = this->updateMapItemPaintNode(oldN, pd);
        if (n)
            opn->appendChildNode(n);
    } else {
        delete oldN;
    }

    return opn;
}

/*!
    \internal
*/
QSGNode *QDeclarativeGeoMapItemBase::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    delete oldNode;
    return 0;
}

QGeoMap::ItemType QDeclarativeGeoMapItemBase::itemType() const
{
    return m_itemType;
}

/*!
    \internal

    The actual combined opacity of the item. Needed by custom renderer to look like
    the scene-graph one.
*/
qreal QDeclarativeGeoMapItemBase::mapItemOpacity() const
{
    if (parentGroup_)
        return parentGroup_->mapItemOpacity() * opacity();
    return opacity();
}

void QDeclarativeGeoMapItemBase::setParentGroup(QDeclarativeGeoMapItemGroup &parentGroup)
{
    parentGroup_ = &parentGroup;
    if (parentGroup_) {
        connect(parentGroup_, &QDeclarativeGeoMapItemGroup::mapItemOpacityChanged,
                this, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged);
    }
}

bool QDeclarativeGeoMapItemBase::isPolishScheduled() const
{
    return QQuickItemPrivate::get(this)->polishScheduled;
}

void QDeclarativeGeoMapItemBase::polishAndUpdate()
{
    polish();
    update();
}

QT_END_NAMESPACE
