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

#include "qmappolylineobjectqsg_p_p.h"
#include <QtQuick/qsgnode.h>
#include <QtQuick/qsgsimplerectnode.h>

QT_BEGIN_NAMESPACE

/*
    Note: never use q, since this class is also used inside QMapRouteObjectPrivateQSG!
*/

QMapPolylineObjectPrivateQSG::QMapPolylineObjectPrivateQSG(QGeoMapObject *q)
    : QMapPolylineObjectPrivate(q)
{

}

QMapPolylineObjectPrivateQSG::QMapPolylineObjectPrivateQSG(const QMapPolylineObjectPrivate &other)
    : QMapPolylineObjectPrivate(other.q)
{
    // do the appropriate internal update and trigger map repaint
    setPath(other.path());
    setColor(other.color());
    setWidth(other.width());
}

QMapPolylineObjectPrivateQSG::~QMapPolylineObjectPrivateQSG()
{
    if (m_map)
        m_map->removeMapObject(q);
}

QList<QDoubleVector2D> QMapPolylineObjectPrivateQSG::projectPath()
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

void QMapPolylineObjectPrivateQSG::updateGeometry()
{
    if (!m_map || m_geoPath.path().length() == 0
            || m_map->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;

    QScopedValueRollback<bool> rollback(m_updatingGeometry);
    m_updatingGeometry = true;
    m_geometry.markSourceDirty();
    const QList<QDoubleVector2D> &geopathProjected = projectPath();
    m_geometry.setPreserveGeometry(true, m_geoPath.boundingGeoRectangle().topLeft());
    m_geometry.updateSourcePoints(*m_map.data(), geopathProjected, m_geoPath.boundingGeoRectangle().topLeft());
    m_geometry.updateScreenPoints(*m_map.data(), width(), false);

    QPointF origin = m_map->geoProjection().coordinateToItemPosition(m_geometry.origin(), false).toPointF();
    m_geometry.translate(origin - m_geometry.firstPointOffset());
}

QSGNode *QMapPolylineObjectPrivateQSG::updateMapObjectNode(QSGNode *oldNode,
                                                           VisibleNode **visibleNode,
                                                           QSGNode *root,
                                                           QQuickWindow * /*window*/)
{
    Q_UNUSED(visibleNode)
    MapPolylineNode *node = static_cast<MapPolylineNode *>(oldNode);

    bool created = false;
    if (!node) {
        if (!m_geometry.size()) // condition to block the subtree
            return nullptr;
        node = new MapPolylineNode();
        *visibleNode = static_cast<VisibleNode *>(node);
        created = true;
    }

    //TODO: update only material
    if (m_geometry.isScreenDirty() || !oldNode || created) {
        node->update(color(), &m_geometry);
        m_geometry.setPreserveGeometry(false);
        m_geometry.markClean();
    }

    if (created)
        root->appendChildNode(node);

    return node;
}

QList<QGeoCoordinate> QMapPolylineObjectPrivateQSG::path() const { return m_geoPath.path(); }

QColor QMapPolylineObjectPrivateQSG::color() const { return m_color; }

qreal QMapPolylineObjectPrivateQSG::width() const { return m_width; }

void QMapPolylineObjectPrivateQSG::setPath(const QList<QGeoCoordinate> &path)
{
    m_geoPath.setPath(path);
    updateGeometry();

    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapPolylineObjectPrivateQSG::setColor(const QColor &color)
{
    m_color = color;
    updateGeometry();

    if (m_map)
        emit m_map->sgNodeChanged();
}

void QMapPolylineObjectPrivateQSG::setWidth(qreal width)
{
    m_width = width;
    updateGeometry();

    if (m_map)
        emit m_map->sgNodeChanged();
}

QGeoMapObjectPrivate *QMapPolylineObjectPrivateQSG::clone()
{
    return new QMapPolylineObjectPrivateQSG(static_cast<QMapPolylineObjectPrivate &>(*this));
}

QT_END_NAMESPACE
