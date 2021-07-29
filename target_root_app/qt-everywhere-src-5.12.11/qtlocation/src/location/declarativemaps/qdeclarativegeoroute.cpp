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

#include "qdeclarativegeoroute_p.h"
#include "locationvaluetypehelper_p.h"
#include <QtLocation/private/qgeomap_p.h>
#include <QtLocation/private/qgeoroute_p.h>
#include <QtLocation/private/qdeclarativegeoroutemodel_p.h>

#include <QtQml/QQmlEngine>
#include <QtQml/qqmlinfo.h>
#include <QtQml/private/qqmlengine_p.h>
#include <QtPositioning/QGeoRectangle>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Route
    \instantiates QDeclarativeGeoRoute
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-routing
    \since QtLocation 5.5

    \brief The Route type represents one geographical route.

    A Route type contains high level information about a route, such
    as the length the route, the estimated travel time for the route,
    and enough information to render a basic image of the route on a map.

    The QGeoRoute object also contains a list of \l RouteSegment objects which
    describe subsections of the route in greater detail.

    The primary means of acquiring Route objects is \l RouteModel.

    \section1 Example

    This example shows how to display a route's maneuvers in a ListView:

    \snippet declarative/routing.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/routing.qml Route Maneuver List1
    \snippet declarative/routing.qml Route Maneuver List2
    \snippet declarative/routing.qml Route Maneuver List3

*/

QDeclarativeGeoRoute::QDeclarativeGeoRoute(QObject *parent)
    : QObject(parent)
{
}

QDeclarativeGeoRoute::QDeclarativeGeoRoute(const QGeoRoute &route, QObject *parent)
    : QObject(parent), route_(route)
{
}

QDeclarativeGeoRoute::~QDeclarativeGeoRoute() {}

void QDeclarativeGeoRoute::initSegments(unsigned int lastIndex) // -1  turns it into unsigned int max
{
    if (!segmentsDirty_)
        return;

    const bool isLeg = qobject_cast<QDeclarativeGeoRoute *>(parent());
    QGeoRouteSegment segment = route_.firstRouteSegment();
    unsigned int idx = 0;
    unsigned int initialListSize = static_cast<unsigned int>(segments_.size());
    while (segment.isValid()) {
        if (idx >= initialListSize) {
            QDeclarativeGeoRouteSegment *routeSegment = new QDeclarativeGeoRouteSegment(segment, this);
            QQmlEngine::setContextForObject(routeSegment, QQmlEngine::contextForObject(this));
            segments_.append(routeSegment);
        }
        if (isLeg && segment.isLegLastSegment()) {
            segmentsDirty_ = false;
            return;
        }
        ++idx;
        segment = segment.nextRouteSegment();
        if (idx > lastIndex && segment.isValid()) // Do not clean segmentsDirty_ if there are still segments to initialize
            return;
    }
    segmentsDirty_ = false;
}

/*!
    \internal
*/
QList<QGeoCoordinate> QDeclarativeGeoRoute::routePath()
{
    return route_.path();
}

/*!
    \qmlproperty georectangle QtLocation::Route::bounds

    Read-only property which holds a bounding box which encompasses the entire route.

*/

QGeoRectangle QDeclarativeGeoRoute::bounds() const
{
    return route_.bounds();
}

/*!
    \qmlproperty int QtLocation::Route::travelTime

    Read-only property which holds the estimated amount of time it will take to
    traverse this route, in seconds.

*/

int QDeclarativeGeoRoute::travelTime() const
{
    return route_.travelTime();
}

/*!
    \qmlproperty real QtLocation::Route::distance

    Read-only property which holds distance covered by this route, in meters.
*/

qreal QDeclarativeGeoRoute::distance() const
{
    return route_.distance();
}

/*!
    \qmlproperty list<coordinate> QtLocation::Route::path

    Read-only property which holds the geographical coordinates of this route.
    Coordinates are listed in the order in which they would be traversed by someone
    traveling along this segment of the route.

    To access individual segments you can use standard list accessors: 'path.length'
    indicates the number of objects and 'path[index starting from zero]' gives
    the actual object.

    \sa QtPositioning::coordinate
*/

QJSValue QDeclarativeGeoRoute::path() const
{
    QQmlContext *context = QQmlEngine::contextForObject(parent());
    QQmlEngine *engine = context->engine();
    QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);

    QV4::Scope scope(v4);
    QV4::Scoped<QV4::ArrayObject> pathArray(scope, v4->newArrayObject(route_.path().length()));
    for (int i = 0; i < route_.path().length(); ++i) {
        const QGeoCoordinate &c = route_.path().at(i);

        QV4::ScopedValue cv(scope, v4->fromVariant(QVariant::fromValue(c)));
        pathArray->put(i, cv);
    }

    return QJSValue(v4, pathArray.asReturnedValue());
}

void QDeclarativeGeoRoute::setPath(const QJSValue &value)
{
    if (!value.isArray())
        return;

    QList<QGeoCoordinate> pathList;
    quint32 length = value.property(QStringLiteral("length")).toUInt();
    for (quint32 i = 0; i < length; ++i) {
        bool ok;
        QGeoCoordinate c = parseCoordinate(value.property(i), &ok);

        if (!ok || !c.isValid()) {
            qmlWarning(this) << "Unsupported path type";
            return;
        }

        pathList.append(c);
    }

    if (route_.path() == pathList)
        return;

    route_.setPath(pathList);

    emit pathChanged();
}

/*!
    \qmlproperty list<RouteSegment> QtLocation::Route::segments

    Read-only property which holds the list of \l RouteSegment objects of this route.

    To access individual segments you can use standard list accessors: 'segments.length'
    indicates the number of objects and 'segments[index starting from zero]' gives
    the actual objects.

    \sa RouteSegment
*/

QQmlListProperty<QDeclarativeGeoRouteSegment> QDeclarativeGeoRoute::segments()
{
    return QQmlListProperty<QDeclarativeGeoRouteSegment>(this, 0, segments_append, segments_count,
                                                         segments_at, segments_clear);
}

/*!
    \internal
*/
void QDeclarativeGeoRoute::segments_append(QQmlListProperty<QDeclarativeGeoRouteSegment> *prop,
                                           QDeclarativeGeoRouteSegment *segment)
{
    QDeclarativeGeoRoute *declRoute = static_cast<QDeclarativeGeoRoute *>(prop->object);
    declRoute->initSegments();
    declRoute->appendSegment(segment);
}

/*!
    \internal
*/
int QDeclarativeGeoRoute::segments_count(QQmlListProperty<QDeclarativeGeoRouteSegment> *prop)
{
    QDeclarativeGeoRoute *declRoute = static_cast<QDeclarativeGeoRoute *>(prop->object);
    return declRoute->segmentsCount();
}

/*!
    \internal
*/
QDeclarativeGeoRouteSegment *QDeclarativeGeoRoute::segments_at(QQmlListProperty<QDeclarativeGeoRouteSegment> *prop, int index)
{
    QDeclarativeGeoRoute *declRoute = static_cast<QDeclarativeGeoRoute *>(prop->object);
    declRoute->initSegments(index); // init only what's needed.
    return declRoute->segments_.at(index);
}

/*!
    \internal
*/
void QDeclarativeGeoRoute::segments_clear(QQmlListProperty<QDeclarativeGeoRouteSegment> *prop)
{
    static_cast<QDeclarativeGeoRoute *>(prop->object)->clearSegments();
}

/*!
    \internal
*/
void QDeclarativeGeoRoute::appendSegment(QDeclarativeGeoRouteSegment *segment)
{
    segments_.append(segment);
}

/*!
    \internal
*/
void QDeclarativeGeoRoute::clearSegments()
{
    segments_.clear();
}

/*!
    \qmlmethod int QtLocation::Route::segmentsCount()

    Returns the number of segments in the route

    \sa RouteSegment

    \since 5.11
*/

int QDeclarativeGeoRoute::segmentsCount() const
{
    return qMax(route_.d_ptr->segmentsCount(), segments_.count());
}

const QGeoRoute &QDeclarativeGeoRoute::route() const
{
    return route_;
}

/*!
    \qmlproperty RouteQuery QtLocation::Route::routeQuery

    Returns the route query associated with this route.

    \since 5.11
*/
QDeclarativeGeoRouteQuery *QDeclarativeGeoRoute::routeQuery()
{
    if (!routeQuery_)
        routeQuery_ = new QDeclarativeGeoRouteQuery(route_.request(), this);
    return routeQuery_;
}

/*!
    \qmlproperty list<Route> QtLocation::Route::legs

    Returns the route legs associated with this route.
    Route legs are the sub-routes between each two adjacent waypoints.
    The result may be empty, if this level of detail is not supported by the
    backend.

    \since QtLocation 5.12
*/
QList<QObject *> QDeclarativeGeoRoute::legs()
{
    // route_.routeLegs() is expected not to change.
    // The following if condition is expected to be run only once.
    if (route_.routeLegs().size() != legs_.size()) {
        legs_.clear();
        QList<QGeoRouteLeg> rlegs = route_.routeLegs();
        for (const QGeoRouteLeg &r: rlegs) {
            QDeclarativeGeoRouteLeg *dr = new QDeclarativeGeoRouteLeg(r, this);
            legs_.append(dr);
        }
    }
    return legs_;
}

/*!
    \qmlmethod bool QtLocation::Route::equals(Route other)

    This method performs deep comparison.

    \since 5.12
*/
bool QDeclarativeGeoRoute::equals(QDeclarativeGeoRoute *other) const
{
    return route_ == other->route_;
}

/*!
    \qmltype RouteLeg
    \instantiates QDeclarativeGeoRouteLeg
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-routing
    \since QtLocation 5.12

    \brief The RouteLeg type represents a leg of a Route, that is the portion
    of a route between one waypoint and the next.

    \note Since RouteLeg is a subclass of Route, QtLocation::Route::legs will
    return an empty list if accessed on a route leg.
*/

/*!
    \qmlproperty int QtLocation::RouteLeg::legIndex

    Read-only property which holds the index of the leg within the containing Route's list of QtLocation::Route::legs .
*/

/*!
    \qmlproperty Route QtLocation::RouteLeg::overallRoute

    Read-only property which holds the Route that contains this leg.
*/


QDeclarativeGeoRouteLeg::QDeclarativeGeoRouteLeg(QObject *parent)
    : QDeclarativeGeoRoute(parent)
{

}

QDeclarativeGeoRouteLeg::QDeclarativeGeoRouteLeg(const QGeoRouteLeg &routeLeg, QObject *parent)
    : QDeclarativeGeoRoute(routeLeg, parent), m_routeLeg(routeLeg)
{

}

QDeclarativeGeoRouteLeg::~QDeclarativeGeoRouteLeg()
{

}

int QDeclarativeGeoRouteLeg::legIndex() const
{
    return m_routeLeg.legIndex();
}

QObject *QDeclarativeGeoRouteLeg::overallRoute() const
{
    QDeclarativeGeoRoute *containingRoute = qobject_cast<QDeclarativeGeoRoute *>(parent());
    if (Q_UNLIKELY(!containingRoute))
        return new QDeclarativeGeoRoute(m_routeLeg.overallRoute(), parent());
    return containingRoute;
}

QT_END_NAMESPACE
