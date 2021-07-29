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

#include "qmappolygonobjectqsg_p_p.h"
#include <QtQuick/qsgnode.h>
#include <QtQuick/qsgsimplerectnode.h>

QT_BEGIN_NAMESPACE

QMapPolygonObjectPrivateQSG::QMapPolygonObjectPrivateQSG(QGeoMapObject *q)
    : QMapPolygonObjectPrivate(q)
{

}

QMapPolygonObjectPrivateQSG::QMapPolygonObjectPrivateQSG(const QMapPolygonObjectPrivate &other)
    : QMapPolygonObjectPrivate(other.q)
{
    setPath(other.path());
    setFillColor(other.fillColor());
    setBorderColor(other.borderColor());
    setBorderWidth(other.borderWidth());
}

QMapPolygonObjectPrivateQSG::~QMapPolygonObjectPrivateQSG()
{
    if (m_map)
        m_map->removeMapObject(q);
}

QList<QDoubleVector2D> QMapPolygonObjectPrivateQSG::projectPath()
{
    QList<QDoubleVector2D> geopathProjected_;
    if (!m_map || m_map->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return geopathProjected_;

    const QGeoProjectionWebMercator &p =
            static_cast<const QGeoProjectionWebMercator&>(m_map->geoProjection());
    geopathProjected_.reserve(m_geoPath.path().size());
    for (const QGeoCoordinate &c : m_geoPath.path())
        geopathProjected_ << p.geoToMapProjection(c);
    return geopathProjected_;
}

QSGNode *QMapPolygonObjectPrivateQSG::updateMapObjectNode(QSGNode *oldNode,
                                                          VisibleNode **visibleNode,
                                                          QSGNode *root,
                                                          QQuickWindow * /*window*/)
{
    Q_UNUSED(visibleNode)
    MapPolygonNode *node = static_cast<MapPolygonNode *>(oldNode);

    bool created = false;
    if (!node) {
        if (!m_geometry.size() && !m_borderGeometry.size())
            return nullptr;
        node = new MapPolygonNode();
        *visibleNode = static_cast<VisibleNode *>(node);
        created = true;
    }

    //TODO: update only material
    if (m_geometry.isScreenDirty() || !m_borderGeometry.isScreenDirty() || !oldNode || created) {
        node->update(fillColor(), borderColor(), &m_geometry, &m_borderGeometry);
        m_geometry.setPreserveGeometry(false);
        m_borderGeometry.setPreserveGeometry(false);
        m_geometry.markClean();
        m_borderGeometry.markClean();
    }

    if (created)
        root->appendChildNode(node);

    return node;
}

QList<QGeoCoordinate> QMapPolygonObjectPrivateQSG::path() const
{
    return m_geoPath.path();
}

QColor QMapPolygonObjectPrivateQSG::fillColor() const
{
    return m_fillColor;
}

QColor QMapPolygonObjectPrivateQSG::borderColor() const
{
    return m_borderColor;
}

qreal QMapPolygonObjectPrivateQSG::borderWidth() const
{
    return m_borderWidth;
}

void QMapPolygonObjectPrivateQSG::setPath(const QList<QGeoCoordinate> &path)
{
    m_geoPath.setPath(path);
    updateGeometry();

    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapPolygonObjectPrivateQSG::setFillColor(const QColor &color)
{
    m_fillColor = color;
    updateGeometry();

    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapPolygonObjectPrivateQSG::setBorderColor(const QColor &color)
{
    m_borderColor = color;
    updateGeometry();

    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapPolygonObjectPrivateQSG::setBorderWidth(qreal width)
{
    m_borderWidth = width;
    updateGeometry();

    if (m_map)
        emit m_map->sgNodeChanged();
}

QGeoMapObjectPrivate *QMapPolygonObjectPrivateQSG::clone()
{
    return new QMapPolygonObjectPrivateQSG(static_cast<QMapPolygonObjectPrivate &>(*this));
}

void QMapPolygonObjectPrivateQSG::updateGeometry()
{
    if (!m_map || m_geoPath.path().length() == 0
            || m_map->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;

    QScopedValueRollback<bool> rollback(m_updatingGeometry);
    m_updatingGeometry = true;

    const QList<QDoubleVector2D> &geopathProjected = projectPath();

    m_geometry.markSourceDirty();
    m_geometry.setPreserveGeometry(true, m_geoPath.boundingGeoRectangle().topLeft());
    m_geometry.updateSourcePoints(*m_map, geopathProjected);
    m_geometry.updateScreenPoints(*m_map);

    m_borderGeometry.clear();

    //if (border_.color() != Qt::transparent && border_.width() > 0)
    {
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_map->geoProjection());
        QList<QDoubleVector2D> closedPath = geopathProjected;
        closedPath << closedPath.first();

        m_borderGeometry.markSourceDirty();
        m_borderGeometry.setPreserveGeometry(true, m_geoPath.boundingGeoRectangle().topLeft());

        const QGeoCoordinate &geometryOrigin = m_geometry.origin();

        m_borderGeometry.clearSource();

        QDoubleVector2D borderLeftBoundWrapped;
        QList<QList<QDoubleVector2D > > clippedPaths =
                m_borderGeometry.clipPath(*m_map.data(), closedPath, borderLeftBoundWrapped);

        if (clippedPaths.size()) {
            borderLeftBoundWrapped = p.geoToWrappedMapProjection(geometryOrigin);
            m_borderGeometry.pathToScreen(*m_map.data(), clippedPaths, borderLeftBoundWrapped);
            m_borderGeometry.updateScreenPoints(*m_map.data(), borderWidth(), false);
        } else {
            m_borderGeometry.clear();
        }
    }

    QPointF origin = m_map->geoProjection().coordinateToItemPosition(m_geometry.origin(), false).toPointF();
    m_geometry.translate(origin - m_geometry.firstPointOffset());
    m_borderGeometry.translate(origin - m_borderGeometry.firstPointOffset());
}

QT_END_NAMESPACE
