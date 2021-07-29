/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickhandlerpoint_p.h"
#include "private/qquickevents_p_p.h"

QT_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(DBG_TOUCH_TARGET)

/*!
    \qmltype HandlerPoint
    \instantiates QQuickHandlerPoint
    \inqmlmodule QtQuick
    \brief An event point.

    A QML representation of a QQuickEventPoint.

    It's possible to make bindings to properties of a handler's current
    \l {SinglePointHandler::point}{point} or
    \l {MultiPointHandler::centroid}{centroid}. For example:

    \snippet pointerHandlers/dragHandlerNullTarget.qml 0

    The point is kept up-to-date when the DragHandler is actively responding to
    an EventPoint; but after the point is released, or when the current point is
    being handled by a different handler, \c position.x and \c position.y are 0.

    \note This is practically identical to QtQuick::EventPoint; however an
    EventPoint is a long-lived QObject which is invalidated between gestures
    and reused for subsequent event deliveries. Continuous bindings to its
    properties are not possible, and an individual handler cannot rely on it
    outside the period when that point is part of an active gesture which that
    handler is handling. HandlerPoint is a Q_GADGET that the handler owns.
    This allows you to make lifetime bindings to its properties.

    \sa SinglePointHandler::point, MultiPointHandler::centroid
*/

QQuickHandlerPoint::QQuickHandlerPoint()
{}

void QQuickHandlerPoint::localize(QQuickItem *item)
{
    m_pressPosition = item->mapFromScene(m_scenePressPosition);
}

void QQuickHandlerPoint::reset()
{
    m_id = 0;
    m_uniqueId = QPointingDeviceUniqueId();
    m_position = QPointF();
    m_scenePosition = QPointF();
    m_pressPosition = QPointF();
    m_scenePressPosition = QPointF();
    m_sceneGrabPosition = QPointF();
    m_velocity = QVector2D();
    m_rotation = 0;
    m_pressure = 0;
    m_ellipseDiameters = QSizeF();
    m_pressedButtons = Qt::NoButton;
    m_pressedModifiers = Qt::NoModifier;
}

void QQuickHandlerPoint::reset(const QQuickEventPoint *point)
{
    m_id = point->pointId();
    const QQuickPointerEvent *event = point->pointerEvent();
    switch (point->state()) {
    case QQuickEventPoint::Pressed:
        m_pressPosition = point->position();
        m_scenePressPosition = point->scenePosition();
        m_pressedButtons = event->buttons();
        break;
    default:
        break;
    }
    m_scenePressPosition = point->scenePressPosition();
    m_pressedButtons = event->buttons();
    m_pressedModifiers = event->modifiers();
    if (event->asPointerTouchEvent()) {
        const QQuickEventTouchPoint *tp = static_cast<const QQuickEventTouchPoint *>(point);
        m_uniqueId = tp->uniqueId();
        m_rotation = tp->rotation();
        m_pressure = tp->pressure();
        m_ellipseDiameters = tp->ellipseDiameters();
    } else if (event->asPointerTabletEvent()) {
        // TODO
    } else {
        m_uniqueId = event->device()->uniqueId();
        m_rotation = 0;
        m_pressure = event->buttons() ? 1 : 0;
        m_ellipseDiameters = QSizeF();
    }
    m_position = point->position();
    m_scenePosition = point->scenePosition();
    if (point->state() == QQuickEventPoint::Updated)
        m_velocity = point->velocity();
}

void QQuickHandlerPoint::reset(const QVector<QQuickHandlerPoint> &points)
{
    if (points.isEmpty()) {
        qWarning("reset: no points");
        return;
    }
    if (points.count() == 1) {
        *this = points.first(); // copy all values
        return;
    }
    // all points are required to be from the same event
    QPointF posSum;
    QPointF scenePosSum;
    QPointF pressPosSum;
    QPointF scenePressPosSum;
    QVector2D velocitySum;
    qreal pressureSum = 0;
    QSizeF ellipseDiameterSum;
    for (const QQuickHandlerPoint &point : points) {
        posSum += point.position();
        scenePosSum += point.scenePosition();
        pressPosSum += point.pressPosition();
        scenePressPosSum += point.scenePressPosition();
        velocitySum += point.velocity();
        pressureSum += point.pressure();
        ellipseDiameterSum += point.ellipseDiameters();
    }
    m_id = 0;
    m_uniqueId = QPointingDeviceUniqueId();
    // all points are required to be from the same event, so pressed buttons and modifiers should be the same
    m_pressedButtons = points.first().pressedButtons();
    m_pressedModifiers = points.first().modifiers();
    m_position = posSum / points.size();
    m_scenePosition = scenePosSum / points.size();
    m_pressPosition = pressPosSum / points.size();
    m_scenePressPosition = scenePressPosSum / points.size();
    m_velocity = velocitySum / points.size();
    m_rotation = 0; // averaging the rotations of all the points isn't very sensible
    m_pressure = pressureSum / points.size();
    m_ellipseDiameters = ellipseDiameterSum / points.size();
}

/*!
    \readonly
    \qmlproperty int QtQuick::HandlerPoint::id
    \brief The ID number of the point

    During a touch gesture, from the time that the first finger is pressed
    until the last finger is released, each touchpoint will have a unique ID
    number. Likewise, if input from multiple devices occurs (for example
    simultaneous mouse and touch presses), all the current event points from
    all the devices will have unique IDs.

    \note Do not assume that id numbers start at zero or that they are
    sequential. Such an assumption is often false due to the way the underlying
    drivers work.

    \sa QTouchEvent::TouchPoint::id
*/

/*!
    \readonly
    \qmlproperty PointingDeviceUniqueId QtQuick::HandlerPoint::uniqueId
    \brief The unique ID of the point, if any

    This is normally empty, because touchscreens cannot uniquely identify fingers.

    On some types of touchscreens, especially those using TUIO drivers,
    it's possible to use recognizable physical tokens (fiducial objects)
    in addition to fingers.  So if this point is a touch point, and
    uniqueId is set, it is the identifier for such an object.

    On a graphics tablet, each type of stylus or other tool often has a unique
    ID or serial number, which can be useful to respond in different ways to
    different tools.

    Interpreting the contents of this ID requires knowledge of the hardware and
    drivers in use.

    \sa QTabletEvent::uniqueId, QtQuick::TouchPoint::uniqueId, QtQuick::EventTouchPoint::uniqueId
*/

/*!
    \readonly
    \qmlproperty QPointF QtQuick::HandlerPoint::position
    \brief The position within the \c parent Item

    This is the position of the event point relative to the bounds of
    the \l {PointerHandler::parent} {parent}.
*/

/*!
    \readonly
    \qmlproperty QPointF QtQuick::HandlerPoint::scenePosition
    \brief The position within the scene

    This is the position of the event point relative to the bounds of the Qt
    Quick scene (typically the whole window).
*/

/*!
    \readonly
    \qmlproperty QPointF QtQuick::HandlerPoint::pressPosition
    \brief The pressed position within the \c parent Item

    This is the position at which this point was pressed, relative to the
    bounds of the \l {PointerHandler::parent} {parent}.
*/

/*!
    \readonly
    \qmlproperty QPointF QtQuick::HandlerPoint::scenePressPosition
    \brief The pressed position within the scene

    This is the position at which this point was pressed, in the coordinate
    system of the \l {Qt Quick Scene Graph}{scene graph}.
*/

/*!
    \readonly
    \qmlproperty QPointF QtQuick::HandlerPoint::sceneGrabPosition
    \brief The grabbed position within the scene

    If this point has been grabbed by a Pointer Handler or an Item, it means
    that object has taken sole responsibility for handling the movement and the
    release if this point. In that case, this is the position at which the grab
    occurred, in the coordinate system of the \l {Qt Quick Scene Graph}{scene graph}.
*/

/*!
    \readonly
    \qmlproperty enumeration QtQuick::HandlerPoint::pressedButtons
    \brief Which mouse or stylus buttons are currently pressed

    \sa MouseArea::pressedButtons
*/

/*!
    \readonly
    \qmlproperty enumeration QtQuick::HandlerPoint::modifiers
    \brief Which modifier keys are currently pressed

    This property holds the keyboard modifiers that were pressed at the time
    the event occurred.
*/

/*!
    \readonly
    \qmlproperty QVector2D QtQuick::HandlerPoint::velocity
    \brief A vector representing the average speed and direction of movement

    This is a velocity vector pointing in the direction of movement, in logical
    pixels per second. It has x and y components, at least one of which will be
    nonzero when this point is in motion. It holds the average recent velocity:
    how fast and in which direction the event point has been moving recently.

    \sa QtQuick::EventPoint::velocity, QtQuick::TouchPoint::velocity, QTouchEvent::TouchPoint::velocity
*/

/*!
    \readonly
    \qmlproperty qreal QtQuick::HandlerPoint::rotation

    This property holds the rotation angle of the stylus on a graphics tablet
    or the contact patch of a touchpoint on a touchscreen.

    It is valid only with certain tablet stylus devices and touchscreens that
    can measure the rotation angle. Otherwise, it will be zero.
*/

/*!
    \readonly
    \qmlproperty qreal QtQuick::HandlerPoint::pressure

    This property tells how hard the user is pressing the stylus on a graphics
    tablet or the finger against a touchscreen, in the range from \c 0 (no
    measurable pressure) to \c 1.0 (maximum pressure which the device can
    measure).

    It is valid only with certain tablets and touchscreens that can measure
    pressure. Otherwise, it will be zero.
*/

/*!
    \readonly
    \qmlproperty size QtQuick::HandlerPoint::ellipseDiameters

    This property holds the diameters of the contact patch, if the event
    comes from a touchpoint and the device provides this information.

    A touchpoint is modeled as an elliptical area where the finger is pressed
    against the touchscreen. (In fact, it could also be modeled as a bitmap;
    but in that case we expect an elliptical bounding estimate to be fitted to
    the contact patch before the event is sent.) The harder the user presses,
    the larger the contact patch; so, these diameters provide an alternate way
    of detecting pressure, in case the device does not include a separate
    pressure sensor. The ellipse is centered on \l scenePosition (\l position
    in the PointerHandler's Item's local coordinates). The \l rotation property
    provides the rotation of the ellipse, if known. It is expected that if the
    \l rotation is zero, the \l {QSize::height}{height} is the larger dimension
    (the major axis), because of the usual hand position, reaching upward or
    outward across the surface.

    If the contact patch is unknown, or the device is not a touchscreen,
    these values will be zero.

    \sa QtQuick::EventTouchPoint::ellipseDiameters, QtQuick::TouchPoint::ellipseDiameters, QTouchEvent::TouchPoint::ellipseDiameters
*/

QT_END_NAMESPACE
