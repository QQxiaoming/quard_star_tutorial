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

#include "qgeomap_p.h"
#include "qgeomap_p_p.h"
#include "qgeocameracapabilities_p.h"
#include "qgeomappingmanagerengine_p.h"
#include "qdeclarativegeomapitembase_p.h"
#include "qgeomapobject_p.h"
#include "qgeomapobject_p_p.h"
#include <QDebug>
#include <QRectF>

QT_BEGIN_NAMESPACE

QGeoMap::QGeoMap(QGeoMapPrivate &dd, QObject *parent)
    : QObject(dd,parent)
{
}

QGeoMap::~QGeoMap()
{
    Q_D(QGeoMap);
    clearParameters();
    for (QGeoMapObject *p : d->mapObjects())
        p->setMap(nullptr); // forces replacing pimpls with the default ones.
}

void QGeoMap::setViewportSize(const QSize& size)
{
    Q_D(QGeoMap);
    if (size == d->m_viewportSize)
        return;
    d->m_viewportSize = size;
    d->m_geoProjection->setViewportSize(size);
    d->changeViewportSize(size);
}

QSize QGeoMap::viewportSize() const
{
    Q_D(const QGeoMap);
    return d->m_viewportSize;
}

int QGeoMap::viewportWidth() const
{
    Q_D(const QGeoMap);
    return d->m_viewportSize.width();
}

int QGeoMap::viewportHeight() const
{
    Q_D(const QGeoMap);
    return d->m_viewportSize.height();
}

void QGeoMap::setCameraData(const QGeoCameraData &cameraData)
{
    Q_D(QGeoMap);
    if (cameraData == d->m_cameraData)
        return;
    d->m_cameraData = cameraData;
    d->m_geoProjection->setCameraData(cameraData, false);
    d->changeCameraData(cameraData);
    emit cameraDataChanged(d->m_cameraData);
}

void QGeoMap::setCameraCapabilities(const QGeoCameraCapabilities &cameraCapabilities)
{
    Q_D(QGeoMap);
    d->setCameraCapabilities(cameraCapabilities);
}

bool QGeoMap::handleEvent(QEvent *event)
{
    Q_UNUSED(event)
    return false;
}

bool QGeoMap::setBearing(qreal bearing, const QGeoCoordinate &coordinate) //FIXME visibleArea
{
    Q_D(QGeoMap);
    bool res = d->m_geoProjection->setBearing(bearing, coordinate);
    if (!res)
        return false;

    setCameraData(geoProjection().cameraData());
    return true;
}

bool QGeoMap::anchorCoordinateToPoint(const QGeoCoordinate &coordinate, const QPointF &anchorPoint)
{
    Q_D(QGeoMap);
    QGeoCoordinate newCenter = geoProjection().anchorCoordinateToPoint(coordinate, anchorPoint);
    newCenter.setLatitude(qBound(d->m_minimumViewportLatitude, newCenter.latitude(), d->m_maximumViewportLatitude));
    QGeoCameraData data = cameraData();
    if (data.center() != newCenter) {
        data.setCenter(newCenter);
        setCameraData(data);
        return true;
    }
    return false;
}

bool QGeoMap::fitViewportToGeoRectangle(const QGeoRectangle &rectangle)
{
    Q_UNUSED(rectangle)
    return false;
}

QGeoShape QGeoMap::visibleRegion() const
{
    return geoProjection().visibleRegion();
}

const QGeoCameraData &QGeoMap::cameraData() const
{
    Q_D(const QGeoMap);
    return d->m_cameraData;
}

void QGeoMap::setActiveMapType(const QGeoMapType type)
{
    Q_D(QGeoMap);
    if (type == d->m_activeMapType)
        return;
    d->m_activeMapType = type;
    d->setCameraCapabilities(d->m_engine->cameraCapabilities(type.mapId())); // emits
    d->changeActiveMapType(type);
    emit activeMapTypeChanged();
}

const QGeoMapType QGeoMap::activeMapType() const
{
    Q_D(const QGeoMap);
    return d->m_activeMapType;
}

double QGeoMap::minimumZoom() const
{
    Q_D(const QGeoMap);
    return d->m_geoProjection->minimumZoom();
}

double QGeoMap::maximumCenterLatitudeAtZoom(const QGeoCameraData &cameraData) const
{
    Q_D(const QGeoMap);
    return d->maximumCenterLatitudeAtZoom(cameraData);
}

double QGeoMap::minimumCenterLatitudeAtZoom(const QGeoCameraData &cameraData) const
{
    Q_D(const QGeoMap);
    return d->minimumCenterLatitudeAtZoom(cameraData);
}

double QGeoMap::mapWidth() const
{
    Q_D(const QGeoMap);
    return d->mapWidth();
}

double QGeoMap::mapHeight() const
{
    Q_D(const QGeoMap);
    return d->mapHeight();
}

const QGeoProjection &QGeoMap::geoProjection() const
{
    Q_D(const QGeoMap);
    return *(d->m_geoProjection);
}

QGeoCameraCapabilities QGeoMap::cameraCapabilities() const
{
    Q_D(const QGeoMap);
    return d->m_cameraCapabilities;
}

QGeoMap::Capabilities QGeoMap::capabilities() const
{
    return Capabilities(QGeoMap::SupportsNothing);
}

void QGeoMap::prefetchData()
{

}

void QGeoMap::clearData()
{

}

void QGeoMap::addParameter(QGeoMapParameter *param)
{
    Q_D(QGeoMap);
    if (param && !d->m_mapParameters.contains(param)) {
        d->m_mapParameters.append(param);
        d->addParameter(param);
    }
}

void QGeoMap::removeParameter(QGeoMapParameter *param)
{
    Q_D(QGeoMap);
    if (param && d->m_mapParameters.contains(param)) {
        d->removeParameter(param);
        d->m_mapParameters.removeOne(param);
    }
}

void QGeoMap::clearParameters()
{
    Q_D(QGeoMap);
    for (QGeoMapParameter *p : qAsConst(d->m_mapParameters))
        d->removeParameter(p);
    d->m_mapParameters.clear();
}

QGeoMap::ItemTypes QGeoMap::supportedMapItemTypes() const
{
    Q_D(const QGeoMap);
    return d->supportedMapItemTypes();
}

void QGeoMap::addMapItem(QDeclarativeGeoMapItemBase *item)
{
    Q_D(QGeoMap);
    if (item && !d->m_mapItems.contains(item) && d->supportedMapItemTypes() & item->itemType()) {
        d->m_mapItems.append(item);
        d->addMapItem(item);
    }
}

void QGeoMap::removeMapItem(QDeclarativeGeoMapItemBase *item)
{
    Q_D(QGeoMap);
    if (item && d->m_mapItems.contains(item)) {
        d->removeMapItem(item);
        d->m_mapItems.removeOne(item);
    }
}

void QGeoMap::clearMapItems()
{
    Q_D(QGeoMap);
    for (QDeclarativeGeoMapItemBase *p : d->m_mapItems)
        d->removeMapItem(p);
    d->m_mapItems.clear();
}

/*!
    Fills obj with a backend-specific pimpl.
*/
bool QGeoMap::createMapObjectImplementation(QGeoMapObject *obj)
{
    Q_D(QGeoMap);
    QExplicitlySharedDataPointer<QGeoMapObjectPrivate> pimpl =
            QExplicitlySharedDataPointer<QGeoMapObjectPrivate>(d->createMapObjectImplementation(obj));
    if (pimpl.constData())
        return obj->setImplementation(pimpl);
    return false;
}

/*!
    To be called in ~QGeoMapObjectPrivate overrides, if needed
*/
void QGeoMap::removeMapObject(QGeoMapObject * /*obj*/)
{
}

void QGeoMap::setVisibleArea(const QRectF &visibleArea)
{
    Q_D(QGeoMap);
    const QRectF &va = d->visibleArea();
    d->setVisibleArea(visibleArea);
    if (va != d->visibleArea())
        emit visibleAreaChanged();
}

QRectF QGeoMap::visibleArea() const
{
    Q_D(const QGeoMap);
    return d->visibleArea();
}

QList<QGeoMapObject *> QGeoMap::mapObjects() const
{
    Q_D(const QGeoMap);
    return d->mapObjects();
}

QString QGeoMap::copyrightsStyleSheet() const
{
    return QStringLiteral("#copyright-root { background: rgba(255, 255, 255, 128) }");
}

void QGeoMap::setAcceptedGestures(bool pan, bool flick, bool pinch, bool rotate, bool tilt)
{
    Q_UNUSED(pan)
    Q_UNUSED(flick)
    Q_UNUSED(pinch)
    Q_UNUSED(rotate)
    Q_UNUSED(tilt)
}

void QGeoMap::setCopyrightVisible(bool visible)
{
    Q_D(QGeoMap);
    if (d->m_copyrightVisible == visible)
        return;

    d->m_copyrightVisible = visible;
}

QGeoMapPrivate::QGeoMapPrivate(QGeoMappingManagerEngine *engine, QGeoProjection *geoProjection)
    : QObjectPrivate(),
      m_geoProjection(geoProjection),
      m_engine(engine),
      m_activeMapType(QGeoMapType())
{
    // Setting the default camera caps without emitting anything
    if (engine)
        m_cameraCapabilities = m_engine->cameraCapabilities(m_activeMapType.mapId());
}

QGeoMapPrivate::~QGeoMapPrivate()
{
    if (m_geoProjection)
        delete m_geoProjection;
}

void QGeoMapPrivate::setCameraCapabilities(const QGeoCameraCapabilities &cameraCapabilities)
{
    Q_Q(QGeoMap);
    if (m_cameraCapabilities == cameraCapabilities)
        return;
    QGeoCameraCapabilities oldCaps = m_cameraCapabilities;
    m_cameraCapabilities = cameraCapabilities;
    emit q->cameraCapabilitiesChanged(oldCaps);
}

const QGeoCameraCapabilities &QGeoMapPrivate::cameraCapabilities() const
{
    return m_cameraCapabilities;
}

const QGeoMapPrivate *QGeoMapPrivate::get(const QGeoMap &map)
{
    return map.d_func();
}

void QGeoMapPrivate::addParameter(QGeoMapParameter *param)
{
    Q_UNUSED(param)
}

void QGeoMapPrivate::removeParameter(QGeoMapParameter *param)
{
    Q_UNUSED(param)
}

QGeoMap::ItemTypes QGeoMapPrivate::supportedMapItemTypes() const
{
    return QGeoMap::NoItem;
}

void QGeoMapPrivate::addMapItem(QDeclarativeGeoMapItemBase *item)
{
    Q_UNUSED(item)
}

void QGeoMapPrivate::removeMapItem(QDeclarativeGeoMapItemBase *item)
{
    Q_UNUSED(item)
}

QGeoMapObjectPrivate *QGeoMapPrivate::createMapObjectImplementation(QGeoMapObject *obj)
{
    Q_UNUSED(obj)
    return nullptr;
}

QList<QGeoMapObject *> QGeoMapPrivate::mapObjects() const
{
    return QList<QGeoMapObject *>();
}

double QGeoMapPrivate::mapWidth() const
{
    if (m_geoProjection->projectionType() == QGeoProjection::ProjectionWebMercator)
        return static_cast<const QGeoProjectionWebMercator *>(m_geoProjection)->mapWidth();
    return 0; // override this for maps supporting other projections
}

double QGeoMapPrivate::mapHeight() const
{
    if (m_geoProjection->projectionType() == QGeoProjection::ProjectionWebMercator)
        return static_cast<const QGeoProjectionWebMercator *>(m_geoProjection)->mapHeight();
    return 0; // override this for maps supporting other projections
}

void QGeoMapPrivate::setCopyrightVisible(bool visible)
{
    m_copyrightVisible = visible;
}

bool QGeoMapPrivate::copyrightVisible() const
{
    return m_copyrightVisible;
}

double QGeoMapPrivate::maximumCenterLatitudeAtZoom(const QGeoCameraData &cameraData) const
{
    m_maximumViewportLatitude = m_geoProjection->maximumCenterLatitudeAtZoom(cameraData);
    return m_maximumViewportLatitude;
}

double QGeoMapPrivate::minimumCenterLatitudeAtZoom(const QGeoCameraData &cameraData) const
{
    m_minimumViewportLatitude = m_geoProjection->minimumCenterLatitudeAtZoom(cameraData);
    return m_minimumViewportLatitude;
}

void QGeoMapPrivate::setVisibleArea(const QRectF &/*visibleArea*/)
{

}

QRectF QGeoMapPrivate::visibleArea() const
{
    return QRectF();
}

QRectF QGeoMapPrivate::clampVisibleArea(const QRectF &visibleArea) const
{
    qreal xp = qMin<qreal>(visibleArea.x(), qMax(m_viewportSize.width() - 1, 0));
    qreal yp = qMin<qreal>(visibleArea.y(), qMax(m_viewportSize.height() - 1, 0));
    qreal w = qMin<qreal>(visibleArea.width(), qMax<qreal>(m_viewportSize.width() - xp, 0));
    qreal h = qMin<qreal>(visibleArea.height(), qMax<qreal>(m_viewportSize.height() - yp, 0));
    return QRectF(xp, yp, w, h);
}

QT_END_NAMESPACE
