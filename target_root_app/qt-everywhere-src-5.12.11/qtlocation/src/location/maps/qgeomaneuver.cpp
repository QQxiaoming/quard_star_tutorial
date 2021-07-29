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

#include "qgeomaneuver.h"
#include "qgeomaneuver_p.h"

#include "qgeocoordinate.h"

QT_BEGIN_NAMESPACE

template<>
QGeoManeuverPrivate *QSharedDataPointer<QGeoManeuverPrivate>::clone()
{
    return d->clone();
}

/*!
    \class QGeoManeuver
    \inmodule QtLocation
    \ingroup QtLocation-routing
    \since 5.6

    \brief The QGeoManeuver class represents the information relevant to the
    point at which two QGeoRouteSegments meet.

    QGeoRouteSegment instances can be thought of as edges on a routing
    graph, with QGeoManeuver instances as optional labels attached to the
    vertices of the graph.

    The most interesting information help in a QGeoManeuver instance is
    normally the textual navigation to provide and the position at which to
    provide it, accessible by instructionText() and position() respectively.

    It is also possible to determine if a routing waypoint has been passed by
    checking if waypoint() returns a valid QGeoCoordinate.
*/

/*!
\enum QGeoManeuver::InstructionDirection

Describes the change in direction associated with the instruction text
that is associated with a QGeoManaeuver.

\value NoDirection
There is no direction associated with the instruction text.

\value DirectionForward
The instruction indicates that the direction of travel does not need to change.

\value DirectionBearRight
The instruction indicates that the direction of travel should bear to the right.

\value DirectionLightRight
The instruction indicates that a light turn to the right is required.

\value DirectionRight
The instruction indicates that a turn to the right is required.

\value DirectionHardRight
The instruction indicates that a hard turn to the right is required.

\value DirectionUTurnRight
The instruction indicates that a u-turn to the right is required.

\value DirectionUTurnLeft
The instruction indicates that a u-turn to the left is required.

\value DirectionHardLeft
The instruction indicates that a hard turn to the left is required.

\value DirectionLeft
The instruction indicates that a turn to the left is required.

\value DirectionLightLeft
The instruction indicates that a light turn to the left is required.

\value DirectionBearLeft
The instruction indicates that the direction of travel should bear to the left.

*/

/*!
    Constructs a invalid maneuver object.

    The maneuver will remain invalid until one of
    setPosition(), setInstructionText(), setDirection(),
    setTimeToNextInstruction(), setDistanceToNextInstruction() or
    setWaypoint() is called.
*/
QGeoManeuver::QGeoManeuver()
    : d_ptr(new QGeoManeuverPrivateDefault()) {}

/*!
    Constructs a maneuver object from the contents of \a other.
*/
QGeoManeuver::QGeoManeuver(const QGeoManeuver &other)
    : d_ptr(other.d_ptr) {}

/*!
    Destroys this maneuver object.
*/
QGeoManeuver::~QGeoManeuver() {}

/*!
    Assigns \a other to this maneuver object and then returns
    a reference to this maneuver object.
*/
QGeoManeuver &QGeoManeuver::operator= (const QGeoManeuver & other)
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns whether this maneuver is equal to \a other.
*/
bool QGeoManeuver::operator== (const QGeoManeuver &other) const
{
    return ( (d_ptr.constData() == other.d_ptr.constData())
            ||  (*(d_ptr.constData()) == *(other.d_ptr.constData())) );
}

/*!
    Returns whether this maneuver is not equal to \a other.
*/
bool QGeoManeuver::operator!= (const QGeoManeuver &other) const
{
    return !(operator==(other));
}

/*!
    Returns whether this maneuver is valid or not.

    Invalid maneuvers are used when there is no information
    that needs to be attached to the endpoint of a QGeoRouteSegment instance.
*/
bool QGeoManeuver::isValid() const
{
    return d_ptr->valid();
}

/*!
    Sets the position where instructionText() should be displayed to \a
    position.
*/
void QGeoManeuver::setPosition(const QGeoCoordinate &position)
{
    d_ptr->setValid(true);
    d_ptr->setPosition(position);
}

/*!
    Returns the position where instructionText() should be displayed.
*/
QGeoCoordinate QGeoManeuver::position() const
{
    return d_ptr->position();
}

/*!
    Sets the textual navigation instructions to \a instructionText.
*/
void QGeoManeuver::setInstructionText(const QString &instructionText)
{
    d_ptr->setValid(true);
    d_ptr->setText(instructionText);
}

/*!
    Returns the textual navigation instructions.
*/
QString QGeoManeuver::instructionText() const
{
    return d_ptr->text();
}

/*!
    Sets the direction associated with the associated instruction to \a
    direction.
*/
void QGeoManeuver::setDirection(QGeoManeuver::InstructionDirection direction)
{
    d_ptr->setValid(true);
    d_ptr->setDirection(direction);
}

/*!
    Returns the direction associated with the associated instruction.
*/
QGeoManeuver::InstructionDirection QGeoManeuver::direction() const
{
    return d_ptr->direction();
}

/*!
    Sets the estimated time it will take to travel from the point at which the
    associated instruction was issued and the point that the next instruction
    should be issued, in seconds, to \a secs.
*/
void QGeoManeuver::setTimeToNextInstruction(int secs)
{
    d_ptr->setValid(true);
    d_ptr->setTimeToNextInstruction(secs);
}

/*!
    Returns the estimated time it will take to travel from the point at which
    the associated instruction was issued and the point that the next
    instruction should be issued, in seconds.
*/
int QGeoManeuver::timeToNextInstruction() const
{
    return d_ptr->timeToNextInstruction();
}

/*!
    Sets the distance, in meters, between the point at which the associated
    instruction was issued and the point that the next instruction should be
    issued to \a distance.
*/
void QGeoManeuver::setDistanceToNextInstruction(qreal distance)
{
    d_ptr->setValid(true);
    d_ptr->setDistanceToNextInstruction(distance);
}

/*!
    Returns the distance, in meters, between the point at which the associated
    instruction was issued and the point that the next instruction should be
    issued.
*/
qreal QGeoManeuver::distanceToNextInstruction() const
{
    return d_ptr->distanceToNextInstruction();
}

/*!
    Sets the waypoint associated with this maneuver to \a coordinate.
*/
void QGeoManeuver::setWaypoint(const QGeoCoordinate &coordinate)
{
    d_ptr->setValid(true);
    d_ptr->setWaypoint(coordinate);
}

/*!
    Returns the waypoint associated with this maneuver.

    If there is not waypoint associated with this maneuver an invalid
    QGeoCoordinate will be returned.
*/
QGeoCoordinate QGeoManeuver::waypoint() const
{
    return d_ptr->waypoint();
}

QGeoManeuver::QGeoManeuver(const QSharedDataPointer<QGeoManeuverPrivate> &dd)
    : d_ptr(dd) {}

/*!
    Sets the extended attributes \a extendedAttributes associated with this maneuver.

    \since QtLocation 5.11
*/
void QGeoManeuver::setExtendedAttributes(const QVariantMap &extendedAttributes)
{
    d_ptr->setValid(true);
    d_ptr->setExtendedAttributes(extendedAttributes);
}

/*!
    Returns the extended attributes associated with this maneuver.

    \since QtLocation 5.11
*/
QVariantMap QGeoManeuver::extendedAttributes() const
{
    return d_ptr->extendedAttributes();
}

/*******************************************************************************
*******************************************************************************/

QGeoManeuverPrivate::QGeoManeuverPrivate()
{

}

QGeoManeuverPrivate::QGeoManeuverPrivate(const QGeoManeuverPrivate &other)
    : QSharedData(other)
{

}

QGeoManeuverPrivate::~QGeoManeuverPrivate()
{

}

bool QGeoManeuverPrivate::operator==(const QGeoManeuverPrivate &other) const
{
    return equals(other);
}

bool QGeoManeuverPrivate::equals(const QGeoManeuverPrivate &other) const
{
    return ((valid() == other.valid())
            && (position() == other.position())
            && (text() == other.text())
            && (direction() == other.direction())
            && (timeToNextInstruction() == other.timeToNextInstruction())
            && (distanceToNextInstruction() == other.distanceToNextInstruction())
            && (waypoint() == other.waypoint()));
}

bool QGeoManeuverPrivate::valid() const
{
    return false;
}

void QGeoManeuverPrivate::setValid(bool valid)
{
    Q_UNUSED(valid)
}

QString QGeoManeuverPrivate::id() const
{
    return QString();
}

void QGeoManeuverPrivate::setId(const QString id)
{
    Q_UNUSED(id)
}

QGeoCoordinate QGeoManeuverPrivate::position() const
{
    return QGeoCoordinate();
}

void QGeoManeuverPrivate::setPosition(const QGeoCoordinate &position)
{
    Q_UNUSED(position)
}

QString QGeoManeuverPrivate::text() const
{
    return QString();
}

void QGeoManeuverPrivate::setText(const QString &text)
{
    Q_UNUSED(text)
}

QGeoManeuver::InstructionDirection QGeoManeuverPrivate::direction() const
{
    return QGeoManeuver::NoDirection;
}

void QGeoManeuverPrivate::setDirection(QGeoManeuver::InstructionDirection direction)
{
    Q_UNUSED(direction)
}

int QGeoManeuverPrivate::timeToNextInstruction() const
{
    return 0;
}

void QGeoManeuverPrivate::setTimeToNextInstruction(int timeToNextInstruction)
{
    Q_UNUSED(timeToNextInstruction)
}

qreal QGeoManeuverPrivate::distanceToNextInstruction() const
{
    return 0;
}

void QGeoManeuverPrivate::setDistanceToNextInstruction(qreal distanceToNextInstruction)
{
    Q_UNUSED(distanceToNextInstruction)
}

QGeoCoordinate QGeoManeuverPrivate::waypoint() const
{
    return QGeoCoordinate();
}

void QGeoManeuverPrivate::setWaypoint(const QGeoCoordinate &waypoint)
{
    Q_UNUSED(waypoint)
}

QVariantMap QGeoManeuverPrivate::extendedAttributes() const
{
    return QVariantMap();
}

void QGeoManeuverPrivate::setExtendedAttributes(const QVariantMap &extendedAttributes)
{
    Q_UNUSED(extendedAttributes)
}



/*******************************************************************************
*******************************************************************************/

QGeoManeuverPrivateDefault::QGeoManeuverPrivateDefault()
    : m_valid(false),
      m_direction(QGeoManeuver::NoDirection),
      m_timeToNextInstruction(0),
      m_distanceToNextInstruction(0.0) {}

QGeoManeuverPrivateDefault::QGeoManeuverPrivateDefault(const QGeoManeuverPrivateDefault &other)
    : QGeoManeuverPrivate(other),
      m_valid(other.m_valid),
      m_position(other.m_position),
      m_text(other.m_text),
      m_direction(other.m_direction),
      m_timeToNextInstruction(other.m_timeToNextInstruction),
      m_distanceToNextInstruction(other.m_distanceToNextInstruction),
      m_waypoint(other.m_waypoint) {}

QGeoManeuverPrivateDefault::~QGeoManeuverPrivateDefault() {}

QGeoManeuverPrivate *QGeoManeuverPrivateDefault::clone()
{
    return new QGeoManeuverPrivateDefault(*this);
}

bool QGeoManeuverPrivateDefault::valid() const
{
    return m_valid;
}

void QGeoManeuverPrivateDefault::setValid(bool valid)
{
    m_valid = valid;
}

QString QGeoManeuverPrivateDefault::id() const
{
    return m_id;
}

void QGeoManeuverPrivateDefault::setId(const QString id)
{
    m_id = id;
}

QGeoCoordinate QGeoManeuverPrivateDefault::position() const
{
    return m_position;
}

void QGeoManeuverPrivateDefault::setPosition(const QGeoCoordinate &position)
{
    m_position = position;
}

QString QGeoManeuverPrivateDefault::text() const
{
    return m_text;
}

void QGeoManeuverPrivateDefault::setText(const QString &text)
{
    m_text = text;
}

QGeoManeuver::InstructionDirection QGeoManeuverPrivateDefault::direction() const
{
    return m_direction;
}

void QGeoManeuverPrivateDefault::setDirection(QGeoManeuver::InstructionDirection direction)
{
    m_direction = direction;
}

int QGeoManeuverPrivateDefault::timeToNextInstruction() const
{
    return m_timeToNextInstruction;
}

void QGeoManeuverPrivateDefault::setTimeToNextInstruction(int timeToNextInstruction)
{
    m_timeToNextInstruction = timeToNextInstruction;
}

qreal QGeoManeuverPrivateDefault::distanceToNextInstruction() const
{
    return m_distanceToNextInstruction;
}

void QGeoManeuverPrivateDefault::setDistanceToNextInstruction(qreal distanceToNextInstruction)
{
    m_distanceToNextInstruction = distanceToNextInstruction;
}

QGeoCoordinate QGeoManeuverPrivateDefault::waypoint() const
{
    return m_waypoint;
}

void QGeoManeuverPrivateDefault::setWaypoint(const QGeoCoordinate &waypoint)
{
    m_waypoint = waypoint;
}

QVariantMap QGeoManeuverPrivateDefault::extendedAttributes() const
{
    return m_extendedAttributes;
}

void QGeoManeuverPrivateDefault::setExtendedAttributes(const QVariantMap &extendedAttributes)
{
    m_extendedAttributes = extendedAttributes;
}

QT_END_NAMESPACE
