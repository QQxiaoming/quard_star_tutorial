/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qgeomapitemsoverlay.h"
#include "qgeomappingmanagerengineitemsoverlay.h"
#include <QtLocation/private/qgeomap_p_p.h>
#include <QtQuick/qsgnode.h>
#include <QtQuick/qsgrectanglenode.h>
#include <QtQuick/qquickwindow.h>

#ifdef LOCATIONLABS
#include <QtLocation/private/qmappolylineobjectqsg_p_p.h>
#include <QtLocation/private/qmappolygonobjectqsg_p_p.h>
#include <QtLocation/private/qmapcircleobjectqsg_p_p.h>
#include <QtLocation/private/qmaprouteobjectqsg_p_p.h>
#include <QtLocation/private/qmapiconobjectqsg_p_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/private/qgeomapobjectqsgsupport_p.h>
#endif

QT_BEGIN_NAMESPACE

class QGeoMapItemsOverlayPrivate : public QGeoMapPrivate
{
    Q_DECLARE_PUBLIC(QGeoMapItemsOverlay)
public:
    QGeoMapItemsOverlayPrivate(QGeoMappingManagerEngineItemsOverlay *engine, QGeoMapItemsOverlay *map);
    virtual ~QGeoMapItemsOverlayPrivate();

#ifdef LOCATIONLABS
    QGeoMapObjectPrivate *createMapObjectImplementation(QGeoMapObject *obj) override;
    virtual QList<QGeoMapObject *> mapObjects() const override;
    void removeMapObject(QGeoMapObject *obj);
    void updateMapObjects(QSGNode *root, QQuickWindow *window);

    QGeoMapObjectQSGSupport m_qsgSupport;
#endif

    void updateObjectsGeometry();

    void setVisibleArea(const QRectF &visibleArea) override;
    QRectF visibleArea() const override;

protected:
    void changeViewportSize(const QSize &size) override;
    void changeCameraData(const QGeoCameraData &oldCameraData) override;
    void changeActiveMapType(const QGeoMapType mapType) override;

    QRectF m_visibleArea;
};

QGeoMapItemsOverlay::QGeoMapItemsOverlay(QGeoMappingManagerEngineItemsOverlay *engine, QObject *parent)
    : QGeoMap(*(new QGeoMapItemsOverlayPrivate(engine, this)), parent)
{

}

QGeoMapItemsOverlay::~QGeoMapItemsOverlay()
{
}

QGeoMap::Capabilities QGeoMapItemsOverlay::capabilities() const
{
    return Capabilities(SupportsVisibleRegion
                        | SupportsSetBearing
                        | SupportsAnchoringCoordinate);
}

bool QGeoMapItemsOverlay::createMapObjectImplementation(QGeoMapObject *obj)
{
#ifndef LOCATIONLABS
    return false;
#else
    Q_D(QGeoMapItemsOverlay);
    return d->m_qsgSupport.createMapObjectImplementation(obj, d);
#endif
}

QSGNode *QGeoMapItemsOverlay::updateSceneGraph(QSGNode *node, QQuickWindow *window)
{
#ifndef LOCATIONLABS
    Q_UNUSED(window)
    return node;
#else
    Q_D(QGeoMapItemsOverlay);

    QSGRectangleNode *mapRoot = static_cast<QSGRectangleNode *>(node);
    if (!mapRoot)
        mapRoot = window->createRectangleNode();

    mapRoot->setRect(QRect(0, 0, viewportWidth(), viewportHeight()));
    mapRoot->setColor(QColor(0,0,0,0));

    d->updateMapObjects(mapRoot, window);
    return mapRoot;
#endif
}

void QGeoMapItemsOverlay::removeMapObject(QGeoMapObject *obj)
{
#ifdef LOCATIONLABS
    Q_D(QGeoMapItemsOverlay);
    d->removeMapObject(obj);
#endif
}

void QGeoMapItemsOverlayPrivate::setVisibleArea(const QRectF &visibleArea)
{
    Q_Q(QGeoMapItemsOverlay);
    const QRectF va = clampVisibleArea(visibleArea);
    if (va == m_visibleArea)
        return;

    m_visibleArea = va;
    m_geoProjection->setVisibleArea(va);

    q->sgNodeChanged();
}

QRectF QGeoMapItemsOverlayPrivate::visibleArea() const
{
    return m_visibleArea;
}

QGeoMapItemsOverlayPrivate::QGeoMapItemsOverlayPrivate(QGeoMappingManagerEngineItemsOverlay *engine, QGeoMapItemsOverlay *map)
    : QGeoMapPrivate(engine, new QGeoProjectionWebMercator)
{
    m_qsgSupport.m_map = map;
}

QGeoMapItemsOverlayPrivate::~QGeoMapItemsOverlayPrivate()
{
}

#ifdef LOCATIONLABS
QGeoMapObjectPrivate *QGeoMapItemsOverlayPrivate::createMapObjectImplementation(QGeoMapObject *obj)
{
    return m_qsgSupport.createMapObjectImplementationPrivate(obj);
}

QList<QGeoMapObject *> QGeoMapItemsOverlayPrivate::mapObjects() const
{
    return m_qsgSupport.mapObjects();
}

void QGeoMapItemsOverlayPrivate::removeMapObject(QGeoMapObject *obj)
{
    m_qsgSupport.removeMapObject(obj);
}

void QGeoMapItemsOverlayPrivate::updateMapObjects(QSGNode *root, QQuickWindow *window)
{
    m_qsgSupport.updateMapObjects(root, window);
}
#endif

void QGeoMapItemsOverlayPrivate::updateObjectsGeometry()
{
#ifdef LOCATIONLABS
    m_qsgSupport.updateObjectsGeometry();
#endif
}

void QGeoMapItemsOverlayPrivate::changeViewportSize(const QSize &/*size*/)
{
    updateObjectsGeometry();
}

void QGeoMapItemsOverlayPrivate::changeCameraData(const QGeoCameraData &/*oldCameraData*/)
{
    updateObjectsGeometry();
}

void QGeoMapItemsOverlayPrivate::changeActiveMapType(const QGeoMapType /*mapType*/)
{
    updateObjectsGeometry();
}

QT_END_NAMESPACE




