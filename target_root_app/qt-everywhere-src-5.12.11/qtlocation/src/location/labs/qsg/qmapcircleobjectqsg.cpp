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

#include "qmapcircleobjectqsg_p_p.h"

QT_BEGIN_NAMESPACE

static const int CircleSamples = 128;

QMapCircleObjectPrivateQSG::QMapCircleObjectPrivateQSG(QGeoMapObject *q)
    : QMapCircleObjectPrivateDefault(q)
{

}

QMapCircleObjectPrivateQSG::QMapCircleObjectPrivateQSG(const QMapCircleObjectPrivate &other)
    : QMapCircleObjectPrivateDefault(other)
{
    // Data already cloned by the *Default copy constructor, but necessary
    // update operations triggered by setters overrides
    setCenter(center());
    setRadius(radius());
    setColor(color());
    setBorderColor(borderColor());
    setBorderWidth(borderWidth());
}

QMapCircleObjectPrivateQSG::~QMapCircleObjectPrivateQSG()
{
    if (m_map)
        m_map->removeMapObject(q);
}

void QMapCircleObjectPrivateQSG::updateCirclePath()
{
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_map->geoProjection());
    QList<QGeoCoordinate> path;
    QDeclarativeCircleMapItem::calculatePeripheralPoints(path, center(), radius(), CircleSamples, m_leftBound);
    m_circlePath.clear();
    for (const QGeoCoordinate &c : path)
        m_circlePath << p.geoToMapProjection(c);
}

void QMapCircleObjectPrivateQSG::updateGeometry()
{
    if (!m_map || m_map->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator
            || !qIsFinite(m_radius) || !m_center.isValid())
        return;

    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_map->geoProjection());
    QScopedValueRollback<bool> rollback(m_updatingGeometry);
    m_updatingGeometry = true;

    updateCirclePath();
    QList<QDoubleVector2D> circlePath = m_circlePath;

    int pathCount = circlePath.size();
    bool preserve = QDeclarativeCircleMapItem::preserveCircleGeometry(circlePath, center(), radius(), p);
    // using leftBound_ instead of the analytically calculated circle_.boundingGeoRectangle().topLeft());
    // to fix QTBUG-62154
    m_geometry.markSourceDirty();
    m_geometry.setPreserveGeometry(true, m_leftBound); // to set the geoLeftBound_
    m_geometry.setPreserveGeometry(preserve, m_leftBound);

    bool invertedCircle = false;
    if (QDeclarativeCircleMapItem::crossEarthPole(center(), radius()) && circlePath.size() == pathCount) {
        m_geometry.updateScreenPointsInvert(circlePath, *m_map); // invert fill area for really huge circles
        invertedCircle = true;
    } else {
        m_geometry.updateSourcePoints(*m_map, circlePath);
        m_geometry.updateScreenPoints(*m_map);
    }

    m_borderGeometry.clear();

    //if (borderColor() != Qt::transparent && borderWidth() > 0)
    {
        QList<QDoubleVector2D> closedPath = circlePath;
        closedPath << closedPath.first();

        if (invertedCircle) {
            closedPath = m_circlePath;
            closedPath << closedPath.first();
            std::reverse(closedPath.begin(), closedPath.end());
        }

        m_borderGeometry.markSourceDirty();
        m_borderGeometry.setPreserveGeometry(true, m_leftBound);
        m_borderGeometry.setPreserveGeometry(preserve, m_leftBound);

        // Use srcOrigin_ from fill geometry after clipping to ensure that translateToCommonOrigin won't fail.
        const QGeoCoordinate &geometryOrigin = m_geometry.origin();

        m_borderGeometry.clearSource();

        QDoubleVector2D borderLeftBoundWrapped;
        QList<QList<QDoubleVector2D > > clippedPaths =
                m_borderGeometry.clipPath(*m_map, closedPath, borderLeftBoundWrapped);
        if (clippedPaths.size()) {
            borderLeftBoundWrapped = p.geoToWrappedMapProjection(geometryOrigin);
            m_borderGeometry.pathToScreen(*m_map, clippedPaths, borderLeftBoundWrapped);
            m_borderGeometry.updateScreenPoints(*m_map, borderWidth(), false);
        } else {
            m_borderGeometry.clear();
        }
    }

    QPointF origin = m_map->geoProjection().coordinateToItemPosition(m_geometry.origin(), false).toPointF();
    m_geometry.translate(origin - m_geometry.firstPointOffset());
    m_borderGeometry.translate(origin - m_borderGeometry.firstPointOffset());
}

QGeoMapObjectPrivate *QMapCircleObjectPrivateQSG::clone()
{
    return new QMapCircleObjectPrivateQSG(static_cast<QMapCircleObjectPrivate &>(*this));
}

QSGNode *QMapCircleObjectPrivateQSG::updateMapObjectNode(QSGNode *oldNode,
                                                         VisibleNode **visibleNode,
                                                         QSGNode *root,
                                                         QQuickWindow * /*window*/)
{
//    Q_UNUSED(visibleNode) // coz of -Werror=unused-but-set-parameter
    MapPolygonNode *node = static_cast<MapPolygonNode *>(oldNode);

    bool created = false;
    if (!node) {
        node = new MapPolygonNode();
        *visibleNode = static_cast<VisibleNode *>(node);
        created = true;
    }

    //TODO: update only material
    if (m_geometry.isScreenDirty() || !m_borderGeometry.isScreenDirty() || !oldNode || created) {
        //QMapPolygonObject *p = static_cast<QMapPolygonObject *>(q);
        node->update(color(), borderColor(), &m_geometry, &m_borderGeometry);
        m_geometry.setPreserveGeometry(false);
        m_borderGeometry.setPreserveGeometry(false);
        m_geometry.markClean();
        m_borderGeometry.markClean();
    }

    if (created)
        root->appendChildNode(node);

    return node;
}


void QMapCircleObjectPrivateQSG::setCenter(const QGeoCoordinate &center)
{
    m_center = center;
    updateGeometry();
    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapCircleObjectPrivateQSG::setRadius(qreal radius)
{
    m_radius = radius;
    updateGeometry();
    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapCircleObjectPrivateQSG::setColor(const QColor &color)
{
    m_fillColor = color;
    updateGeometry();
    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapCircleObjectPrivateQSG::setBorderColor(const QColor &color)
{
    m_borderColor = color;
    updateGeometry();
    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapCircleObjectPrivateQSG::setBorderWidth(qreal width)
{
    m_borderWidth = width;
    updateGeometry();
    if (m_map)
        emit m_map->sgNodeChanged();
}


QT_END_NAMESPACE
