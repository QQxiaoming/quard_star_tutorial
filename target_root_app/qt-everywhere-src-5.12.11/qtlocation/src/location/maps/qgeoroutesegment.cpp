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

#include "qgeoroutesegment.h"
#include "qgeoroutesegment_p.h"

#include "qgeocoordinate.h"
#include <QDateTime>

QT_BEGIN_NAMESPACE

template<>
QGeoRouteSegmentPrivate *QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate>::clone()
{
    return d->clone();
}

/*!
    \class QGeoRouteSegment
    \inmodule QtLocation
    \ingroup QtLocation-routing
    \since 5.6

    \brief The QGeoRouteSegment class represents a segment of a route.

    A QGeoRouteSegment instance has information about the physical layout
    of the route segment, the length of the route and estimated time required
    to traverse the route segment and an optional QGeoManeuver associated with
    the beginning of the route segment.

    QGeoRouteSegment instances can be thought of as edges on a routing
    graph, with QGeoManeuver instances as optional labels attached to the
    vertices of the graph.
*/

/*!
    Constructs an invalid route segment object.

    The route segment will remain invalid until one of setNextRouteSegment(),
    setTravelTime(), setDistance(), setPath() or setManeuver() is called.
*/
QGeoRouteSegment::QGeoRouteSegment()
    : d_ptr(new QGeoRouteSegmentPrivateDefault()) {}

/*!
    Constructs a route segment object from the contents of \a other.
*/
QGeoRouteSegment::QGeoRouteSegment(const QGeoRouteSegment &other)
    : d_ptr(other.d_ptr) {}

/*!
    \internal
*/
QGeoRouteSegment::QGeoRouteSegment(const QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> &dd)
    : d_ptr(dd) {}

/*!
    Returns the private implementation.
*/
QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> &QGeoRouteSegment::d()
{
    return d_ptr;
}

/*!
    Destroys this route segment object.
*/
QGeoRouteSegment::~QGeoRouteSegment() {}


/*!
    Assigns \a other to this route segment object and then returns a
    reference to this route segment object.
*/
QGeoRouteSegment &QGeoRouteSegment::operator= (const QGeoRouteSegment & other)
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns whether this route segment and \a other are equal.

    The value of nextRouteSegment() is not considered in the comparison.
*/
bool QGeoRouteSegment::operator ==(const QGeoRouteSegment &other) const
{
    return ( (d_ptr.constData() == other.d_ptr.constData())
            || (*d_ptr) == (*other.d_ptr));
}

/*!
    Returns whether this route segment and \a other are not equal.

    The value of nextRouteSegment() is not considered in the comparison.
*/
bool QGeoRouteSegment::operator !=(const QGeoRouteSegment &other) const
{
    return !(operator==(other));
}

/*!
    Returns whether this route segment is valid or not.

    If nextRouteSegment() is called on the last route segment of a route, the
    returned value will be an invalid route segment.
*/
bool QGeoRouteSegment::isValid() const
{
    return d_ptr->valid();
}

/*!
    Returns whether this route segment is the last segment of a route leg.

    \since 5.12
*/
bool QGeoRouteSegment::isLegLastSegment() const
{
    if (!d_ptr->valid())
        return false;

    if (!d_ptr->nextRouteSegment())
        return true;
    return d_ptr->isLegLastSegment();
}

/*!
    Sets the next route segment in the route to \a routeSegment.
*/
void QGeoRouteSegment::setNextRouteSegment(const QGeoRouteSegment &routeSegment)
{
    d_ptr->setValid(true);
    d_ptr->setNextRouteSegment(routeSegment.d_ptr);
}

/*!
    Returns the next route segment in the route.

    Will return an invalid route segment if this is the last route
    segment in the route.
*/
QGeoRouteSegment QGeoRouteSegment::nextRouteSegment() const
{
    if (d_ptr->valid() && d_ptr->nextRouteSegment())
        return QGeoRouteSegment(d_ptr->nextRouteSegment());

    return QGeoRouteSegment();
}

/*!
    Sets the estimated amount of time it will take to traverse this segment of
    the route, in seconds, to \a secs.
*/
void QGeoRouteSegment::setTravelTime(int secs)
{
    d_ptr->setValid(true);
    d_ptr->setTravelTime(secs);
}

/*!
    Returns the estimated amount of time it will take to traverse this segment
    of the route, in seconds.
*/
int QGeoRouteSegment::travelTime() const
{
    return d_ptr->travelTime();
}

/*!
    Sets the distance covered by this segment of the route, in meters, to \a distance.
*/
void QGeoRouteSegment::setDistance(qreal distance)
{
    d_ptr->setValid(true);
    d_ptr->setDistance(distance);
}

/*!
    Returns the distance covered by this segment of the route, in meters.
*/
qreal QGeoRouteSegment::distance() const
{
    return d_ptr->distance();
}

/*!
    Sets the geometric shape of this segment of the route to \a path.

    The coordinates in \a path should be listed in the order in which they
    would be traversed by someone traveling along this segment of the route.
*/
void QGeoRouteSegment::setPath(const QList<QGeoCoordinate> &path)
{
    d_ptr->setValid(true);
    d_ptr->setPath(path);
}

/*!
    Returns the geometric shape of this route segment of the route.

    The coordinates should be listed in the order in which they
    would be traversed by someone traveling along this segment of the route.
*/

QList<QGeoCoordinate> QGeoRouteSegment::path() const
{
    return d_ptr->path();
}

/*!
    Sets the maneuver for this route segment to \a maneuver.
*/
void QGeoRouteSegment::setManeuver(const QGeoManeuver &maneuver)
{
    d_ptr->setValid(true);
    d_ptr->setManeuver(maneuver);
}

/*!
    Returns the maneuver for this route segment.

    Will return an invalid QGeoManeuver if no information has been attached
    to the starting point of this route segment.
*/
QGeoManeuver QGeoRouteSegment::maneuver() const
{
    return d_ptr->maneuver();
}

/*******************************************************************************
*******************************************************************************/

QGeoRouteSegmentPrivate::QGeoRouteSegmentPrivate() {}

QGeoRouteSegmentPrivate::QGeoRouteSegmentPrivate(const QGeoRouteSegmentPrivate &other)
    : QSharedData(other), m_nextSegment(other.m_nextSegment) {}

QGeoRouteSegmentPrivate::~QGeoRouteSegmentPrivate()
{
    m_nextSegment.reset();
}

bool QGeoRouteSegmentPrivate::operator ==(const QGeoRouteSegmentPrivate &other) const
{
    return equals(other);
}

bool QGeoRouteSegmentPrivate::equals(const QGeoRouteSegmentPrivate &other) const
{
    return ((valid() == other.valid())
            && (travelTime() == other.travelTime())
            && (distance() == other.distance())
            && (path() == other.path())
            && (maneuver() == other.maneuver()));
}

bool QGeoRouteSegmentPrivate::valid() const
{
    return false;
}

void QGeoRouteSegmentPrivate::setValid(bool valid)
{
    Q_UNUSED(valid)
}

bool QGeoRouteSegmentPrivate::isLegLastSegment() const
{
    return false;
}

void QGeoRouteSegmentPrivate::setLegLastSegment(bool lastSegment)
{
    Q_UNUSED(lastSegment)
}

int QGeoRouteSegmentPrivate::travelTime() const
{
    return 0;
}

void QGeoRouteSegmentPrivate::setTravelTime(int travelTime)
{
    Q_UNUSED(travelTime)
}

qreal QGeoRouteSegmentPrivate::distance() const
{
    return 0;
}

void QGeoRouteSegmentPrivate::setDistance(qreal distance)
{
    Q_UNUSED(distance)
}

QList<QGeoCoordinate> QGeoRouteSegmentPrivate::path() const
{
    return QList<QGeoCoordinate>();
}

void QGeoRouteSegmentPrivate::setPath(const QList<QGeoCoordinate> &path)
{
    Q_UNUSED(path)
}

QGeoManeuver QGeoRouteSegmentPrivate::maneuver() const
{
    return QGeoManeuver();
}

void QGeoRouteSegmentPrivate::setManeuver(const QGeoManeuver &maneuver)
{
    Q_UNUSED(maneuver)
}

QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> QGeoRouteSegmentPrivate::nextRouteSegment() const
{
    return m_nextSegment;
}

void QGeoRouteSegmentPrivate::setNextRouteSegment(const QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> &next)
{
    m_nextSegment = next;
}

QGeoRouteSegmentPrivate *QGeoRouteSegmentPrivate::get(QGeoRouteSegment &segment)
{
    return segment.d_ptr.data();
}

/*******************************************************************************
*******************************************************************************/

QGeoRouteSegmentPrivateDefault::QGeoRouteSegmentPrivateDefault()
    : m_valid(false),
      m_travelTime(0),
      m_distance(0.0)
{

}

QGeoRouteSegmentPrivateDefault::QGeoRouteSegmentPrivateDefault(const QGeoRouteSegmentPrivateDefault &other)
    : QGeoRouteSegmentPrivate(other),
      m_valid(other.m_valid),
      m_travelTime(other.m_travelTime),
      m_distance(other.m_distance),
      m_path(other.m_path),
      m_maneuver(other.m_maneuver)
{

}

QGeoRouteSegmentPrivateDefault::~QGeoRouteSegmentPrivateDefault()
{

}

QGeoRouteSegmentPrivate *QGeoRouteSegmentPrivateDefault::clone()
{
    return new QGeoRouteSegmentPrivateDefault(*this);
}

bool QGeoRouteSegmentPrivateDefault::operator ==(const QGeoRouteSegmentPrivateDefault &other) const
{
    return QGeoRouteSegmentPrivate::operator ==(other);
}

bool QGeoRouteSegmentPrivateDefault::valid() const
{
    return m_valid;
}

void QGeoRouteSegmentPrivateDefault::setValid(bool valid)
{
    m_valid = valid;
}

bool QGeoRouteSegmentPrivateDefault::isLegLastSegment() const
{
    return m_legLastSegment;
}

void QGeoRouteSegmentPrivateDefault::setLegLastSegment(bool lastSegment)
{
    m_legLastSegment = lastSegment;
}

int QGeoRouteSegmentPrivateDefault::travelTime() const
{
    return m_travelTime;
}

void QGeoRouteSegmentPrivateDefault::setTravelTime(int travelTime)
{
    m_travelTime = travelTime;
}

qreal QGeoRouteSegmentPrivateDefault::distance() const
{
    return m_distance;
}

void QGeoRouteSegmentPrivateDefault::setDistance(qreal distance)
{
    m_distance = distance;
}

QList<QGeoCoordinate> QGeoRouteSegmentPrivateDefault::path() const
{
    return m_path;
}

void QGeoRouteSegmentPrivateDefault::setPath(const QList<QGeoCoordinate> &path)
{
    m_path = path;
}

QGeoManeuver QGeoRouteSegmentPrivateDefault::maneuver() const
{
    return m_maneuver;
}

void QGeoRouteSegmentPrivateDefault::setManeuver(const QGeoManeuver &maneuver)
{
    m_maneuver = maneuver;
}

/*******************************************************************************
*******************************************************************************/

QT_END_NAMESPACE
