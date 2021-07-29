/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qgeotiledmaplabs_p.h"
#include <QtLocation/private/qgeotiledmap_p_p.h>
#include <QtLocation/private/qgeomapobject_p.h>
#include <QtLocation/private/qmappolylineobjectqsg_p_p.h>
#include <QtLocation/private/qmappolygonobjectqsg_p_p.h>
#include <QtLocation/private/qmapcircleobjectqsg_p_p.h>
#include <QtLocation/private/qmaprouteobjectqsg_p_p.h>
#include <QtLocation/private/qmapiconobjectqsg_p_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/private/qgeomapobjectqsgsupport_p.h>

QT_BEGIN_NAMESPACE

class QGeoTiledMapLabsPrivate : public QGeoTiledMapPrivate
{
    Q_DECLARE_PUBLIC(QGeoTiledMapLabs)
public:
    QGeoTiledMapLabsPrivate(QGeoTiledMappingManagerEngine *engine, QGeoTiledMapLabs *map);
    virtual ~QGeoTiledMapLabsPrivate();

    QGeoMapObjectPrivate *createMapObjectImplementation(QGeoMapObject *obj) override;
    virtual QList<QGeoMapObject *> mapObjects() const override;
    void removeMapObject(QGeoMapObject *obj);

    void updateMapObjects(QSGNode *root, QQuickWindow *window);
    void updateObjectsGeometry();

protected:
    void changeViewportSize(const QSize &size) override;
    void changeCameraData(const QGeoCameraData &oldCameraData) override;
    void changeActiveMapType(const QGeoMapType mapType) override;

    QGeoMapObjectQSGSupport m_qsgSupport;
};

QGeoTiledMapLabsPrivate::QGeoTiledMapLabsPrivate(QGeoTiledMappingManagerEngine *engine, QGeoTiledMapLabs *map)
    : QGeoTiledMapPrivate(engine)
{
    m_qsgSupport.m_map = map;
}

QGeoTiledMapLabsPrivate::~QGeoTiledMapLabsPrivate()
{

}

QGeoMapObjectPrivate *QGeoTiledMapLabsPrivate::createMapObjectImplementation(QGeoMapObject *obj)
{
    return m_qsgSupport.createMapObjectImplementationPrivate(obj);
}

QList<QGeoMapObject *> QGeoTiledMapLabsPrivate::mapObjects() const
{
    return m_qsgSupport.mapObjects();
}

void QGeoTiledMapLabsPrivate::removeMapObject(QGeoMapObject *obj)
{
    m_qsgSupport.removeMapObject(obj);
}

void QGeoTiledMapLabsPrivate::updateMapObjects(QSGNode *root, QQuickWindow *window)
{
    m_qsgSupport.updateMapObjects(root, window);
}

void QGeoTiledMapLabsPrivate::updateObjectsGeometry()
{
    m_qsgSupport.updateObjectsGeometry();
}

void QGeoTiledMapLabsPrivate::changeViewportSize(const QSize &size)
{
    updateObjectsGeometry();
    QGeoTiledMapPrivate::changeViewportSize(size);
}

void QGeoTiledMapLabsPrivate::changeCameraData(const QGeoCameraData &oldCameraData)
{
    updateObjectsGeometry();
    QGeoTiledMapPrivate::changeCameraData(oldCameraData);
}

void QGeoTiledMapLabsPrivate::changeActiveMapType(const QGeoMapType mapType)
{
    updateObjectsGeometry();
    QGeoTiledMapPrivate::changeActiveMapType(mapType);
}


/*
    QGeoTiledMapLabs
*/



QGeoTiledMapLabs::QGeoTiledMapLabs(QGeoTiledMappingManagerEngine *engine, QObject *parent)
    : QGeoTiledMap(*new QGeoTiledMapLabsPrivate(engine, this), engine, parent)
{

}

QGeoTiledMapLabs::~QGeoTiledMapLabs()
{

}

bool QGeoTiledMapLabs::createMapObjectImplementation(QGeoMapObject *obj)
{
    Q_D(QGeoTiledMapLabs);
    return d->m_qsgSupport.createMapObjectImplementation(obj, d);
}

QSGNode *QGeoTiledMapLabs::updateSceneGraph(QSGNode *node, QQuickWindow *window)
{
    Q_D(QGeoTiledMapLabs);
    QSGNode *root = QGeoTiledMap::updateSceneGraph(node, window);
    d->updateMapObjects(root, window);
    return root;
}

void QGeoTiledMapLabs::removeMapObject(QGeoMapObject *obj)
{
    Q_D(QGeoTiledMapLabs);
    d->removeMapObject(obj);
}

QGeoTiledMapLabs::QGeoTiledMapLabs(QGeoTiledMapLabsPrivate &dd, QGeoTiledMappingManagerEngine *engine, QObject *parent)
    : QGeoTiledMap(dd, engine, parent)
{

}

QT_END_NAMESPACE

