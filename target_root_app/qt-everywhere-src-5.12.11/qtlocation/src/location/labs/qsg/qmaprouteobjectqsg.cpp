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

#include "qmaprouteobjectqsg_p_p.h"

QT_BEGIN_NAMESPACE

QMapRouteObjectPrivateQSG::QMapRouteObjectPrivateQSG(QGeoMapObject *q)
    : QMapRouteObjectPrivate(q)
{
    QScopedPointer<QMapPolylineObjectPrivateQSG> poly(new QMapPolylineObjectPrivateQSG(q));
    m_polyline.swap(poly);
    m_polyline->m_componentCompleted = true;
}

QMapRouteObjectPrivateQSG::QMapRouteObjectPrivateQSG(const QMapRouteObjectPrivate &other)
    : QMapRouteObjectPrivate(other)
{
    QScopedPointer<QMapPolylineObjectPrivateQSG> poly(new QMapPolylineObjectPrivateQSG(other.q));
    m_polyline.swap(poly);
    m_polyline->m_componentCompleted = true;
    setRoute(other.declarativeGeoRoute());
}

QMapRouteObjectPrivateQSG::~QMapRouteObjectPrivateQSG()
{
    if (m_map)
        m_map->removeMapObject(q);
}

void QMapRouteObjectPrivateQSG::updateGeometry()
{
    m_polyline->updateGeometry();
}

QSGNode *QMapRouteObjectPrivateQSG::updateMapObjectNode(QSGNode *oldNode,
                                                        VisibleNode **visibleNode,
                                                        QSGNode *root,
                                                        QQuickWindow *window)
{
    return m_polyline->updateMapObjectNode(oldNode, visibleNode, root, window);
}

void QMapRouteObjectPrivateQSG::setRoute(const QDeclarativeGeoRoute *route)
{
    const QList<QGeoCoordinate> &path = route->route().path();
    m_polyline->setColor(QColor("deepskyblue")); // ToDo: support MapParameters for this
    m_polyline->setWidth(4);
    m_polyline->setPath(path); // SGNodeChanged emitted by m_polyline
}

QGeoMapObjectPrivate *QMapRouteObjectPrivateQSG::clone()
{
    return new QMapRouteObjectPrivateQSG(static_cast<QMapRouteObjectPrivate &>(*this));
}

void QMapRouteObjectPrivateQSG::setMap(QGeoMap *map)
{
    QGeoMapObjectPrivate::setMap(map);
    m_polyline->setMap(map);
}


void QMapRouteObjectPrivateQSG::setVisible(bool visible)
{
    m_visible = visible;
    m_polyline->setVisible(visible);
}

QT_END_NAMESPACE
