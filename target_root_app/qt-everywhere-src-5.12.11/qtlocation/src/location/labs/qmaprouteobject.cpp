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

#include <QtLocation/private/qdeclarativegeoroute_p.h>
#include "qmaprouteobject_p.h"
#include "qmaprouteobject_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapRouteObject
    \instantiates QMapRouteObject
    \inqmlmodule Qt.labs.location
    \ingroup qml-QtLocation5-maps
    \inherits QGeoMapObject

    \brief The MapRouteObject displays a geographical route on a Map.

    The MapRouteObject type displays a Route obtained through a RouteModel or
    other means, on the Map as a Polyline following the path of the Route.
*/

/*

    QGeoMapRoutePrivate

*/

QMapRouteObjectPrivate::QMapRouteObjectPrivate(QGeoMapObject *q) : QGeoMapObjectPrivate(q)
{

}

QMapRouteObjectPrivate::QMapRouteObjectPrivate(const QMapRouteObjectPrivate &other) : QGeoMapObjectPrivate(other)
{
    // QGeoMapRoutePrivate doesn't contain anything because QGeoRoute has already everything necessary.
}

QMapRouteObjectPrivate::~QMapRouteObjectPrivate()
{

}

QGeoMapObject::Type QMapRouteObjectPrivate::type() const
{
    return QGeoMapObject::RouteType;
}

QDeclarativeGeoRoute *QMapRouteObjectPrivate::declarativeGeoRoute() const
{
    const QMapRouteObject *r = static_cast<QMapRouteObject *>(q);
    return r->m_route;
}

/*!
    \qmlproperty Route Qt.labs.location::MapRouteObject::route

    This property holds the route to be drawn.
*/
QGeoRoute QMapRouteObjectPrivate::route() const
{
    const QDeclarativeGeoRoute *r = declarativeGeoRoute();
    if (r)
        return r->route();
    return {};
}

void QMapRouteObjectPrivate::setRoute(const QDeclarativeGeoRoute *route)
{
    Q_UNUSED(route)
}

bool QMapRouteObjectPrivate::equals(const QGeoMapObjectPrivate &other) const
{
    if (other.type() != type()) // This check might be unnecessary, depending on how equals gets used
        return false;

    const QMapRouteObjectPrivate &o = static_cast<const QMapRouteObjectPrivate &>(other);
    return (QGeoMapObjectPrivate::equals(o)
            && route() == o.route()); // Could also be done shallow, comparing declarativeGeoRoute()
}

QGeoMapObjectPrivate *QMapRouteObjectPrivate::clone()
{
    return new QMapRouteObjectPrivate(*this);
}


/*

    QGeoMapRoute

*/

QMapRouteObject::QMapRouteObject(QObject *parent)
    : QGeoMapObject(QExplicitlySharedDataPointer<QGeoMapObjectPrivate>(new QMapRouteObjectPrivate(this)), parent)
{

}

QMapRouteObject::~QMapRouteObject()
{

}

QDeclarativeGeoRoute *QMapRouteObject::route() const
{
    return m_route;
}

QGeoRoute QMapRouteObject::geoRoute() const
{
    if (m_route)
        return m_route->route();
    return {};
}

void QMapRouteObject::setRoute(QDeclarativeGeoRoute *route)
{
    if (route == m_route)
        return;
//    if ((!m_route && !route) || (m_route && route && m_route->route() == route->route()))
//        return;

    m_route = route;
    QMapRouteObjectPrivate *d = static_cast<QMapRouteObjectPrivate *>(d_ptr.data());
    d->setRoute(route);
    emit routeChanged(route);
}

void QMapRouteObject::setMap(QGeoMap *map)
{
    QMapRouteObjectPrivate *d = static_cast<QMapRouteObjectPrivate *>(d_ptr.data());
    if (d->m_map == map)
        return;

    QGeoMapObject::setMap(map); // This is where the specialized pimpl gets created and injected

    if (!map) {
        // Map was set, now it has ben re-set to NULL, but not inside d_ptr.
        // so m_map inside d_ptr can still be used to remove itself, inside the destructor.
        d_ptr = new QMapRouteObjectPrivate(*d);
        // Old pimpl deleted implicitly by QExplicitlySharedDataPointer
    }
}

QT_END_NAMESPACE
