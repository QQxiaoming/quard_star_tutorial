/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qquickevents_p_p.h"
#include <QtCore/qmap.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/private/qtouchdevice_p.h>
#include <QtGui/private/qevent_p.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickpointerhandler_p.h>
#include <QtQuick/private/qquickwindow_p.h>
#include <private/qdebug_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcPointerEvents, "qt.quick.pointer.events")
Q_LOGGING_CATEGORY(lcPointerGrab, "qt.quick.pointer.grab")

/*!
    \qmltype KeyEvent
    \instantiates QQuickKeyEvent
    \inqmlmodule QtQuick
    \ingroup qtquick-input-events

    \brief Provides information about a key event.

    For example, the following changes the Item's state property when the Enter
    key is pressed:
    \qml
Item {
    focus: true
    Keys.onPressed: { if (event.key == Qt.Key_Enter) state = 'ShowDetails'; }
}
    \endqml
*/

/*!
    \qmlproperty int QtQuick::KeyEvent::key

    This property holds the code of the key that was pressed or released.

    See \l {Qt::Key}{Qt.Key} for the list of keyboard codes. These codes are
    independent of the underlying window system. Note that this
    function does not distinguish between capital and non-capital
    letters; use the \l {KeyEvent::}{text} property for this purpose.

    A value of either 0 or \l {Qt::Key_unknown}{Qt.Key_Unknown} means that the event is not
    the result of a known key; for example, it may be the result of
    a compose sequence, a keyboard macro, or due to key event
    compression.
*/

/*!
    \qmlproperty string QtQuick::KeyEvent::text

    This property holds the Unicode text that the key generated.
    The text returned can be an empty string in cases where modifier keys,
    such as Shift, Control, Alt, and Meta, are being pressed or released.
    In such cases \c key will contain a valid value
*/

/*!
    \qmlproperty bool QtQuick::KeyEvent::isAutoRepeat

    This property holds whether this event comes from an auto-repeating key.
*/

/*!
    \qmlproperty quint32 QtQuick::KeyEvent::nativeScanCode

    This property contains the native scan code of the key that was pressed. It is
    passed through from QKeyEvent unchanged.

    \sa QKeyEvent::nativeScanCode()
*/

/*!
    \qmlproperty int QtQuick::KeyEvent::count

    This property holds the number of keys involved in this event. If \l KeyEvent::text
    is not empty, this is simply the length of the string.
*/

/*!
    \qmlproperty bool QtQuick::KeyEvent::accepted

    Setting \a accepted to true prevents the key event from being
    propagated to the item's parent.

    Generally, if the item acts on the key event then it should be accepted
    so that ancestor items do not also respond to the same event.
*/

/*!
    \qmlproperty int QtQuick::KeyEvent::modifiers

    This property holds the keyboard modifier flags that existed immediately
    before the event occurred.

    It contains a bitwise combination of:
    \list
    \li \l {Qt::NoModifier} {Qt.NoModifier} - No modifier key is pressed.
    \li \l {Qt::ShiftModifier} {Qt.ShiftModifier} - A Shift key on the keyboard is pressed.
    \li \l {Qt::ControlModifier} {Qt.ControlModifier} - A Ctrl key on the keyboard is pressed.
    \li \l {Qt::AltModifier} {Qt.AltModifier} - An Alt key on the keyboard is pressed.
    \li \l {Qt::MetaModifier} {Qt.MetaModifier} - A Meta key on the keyboard is pressed.
    \li \l {Qt::KeypadModifier} {Qt.KeypadModifier} - A keypad button is pressed.
    \li \l {Qt::GroupSwitchModifier} {Qt.GroupSwitchModifier} - X11 only. A Mode_switch key on the keyboard is pressed.
    \endlist

    For example, to react to a Shift key + Enter key combination:
    \qml
    Item {
        focus: true
        Keys.onPressed: {
            if ((event.key == Qt.Key_Enter) && (event.modifiers & Qt.ShiftModifier))
                doSomething();
        }
    }
    \endqml
*/

/*!
    \qmlmethod bool QtQuick::KeyEvent::matches(StandardKey key)
    \since 5.2

    Returns \c true if the key event matches the given standard \a key; otherwise returns \c false.

    \qml
    Item {
        focus: true
        Keys.onPressed: {
            if (event.matches(StandardKey.Undo))
                myModel.undo();
            else if (event.matches(StandardKey.Redo))
                myModel.redo();
        }
    }
    \endqml

    \sa QKeySequence::StandardKey
*/

/*!
    \qmltype MouseEvent
    \instantiates QQuickMouseEvent
    \inqmlmodule QtQuick
    \ingroup qtquick-input-events

    \brief Provides information about a mouse event.

    The position of the mouse can be found via the \l {Item::x} {x} and \l {Item::y} {y} properties.
    The button that caused the event is available via the \l button property.

    \sa MouseArea
*/

/*!
    \internal
    \class QQuickMouseEvent
*/

/*!
    \qmlproperty real QtQuick::MouseEvent::x
    \qmlproperty real QtQuick::MouseEvent::y

    These properties hold the coordinates of the position supplied by the mouse event.
*/


/*!
    \qmlproperty bool QtQuick::MouseEvent::accepted

    Setting \a accepted to true prevents the mouse event from being
    propagated to items below this item.

    Generally, if the item acts on the mouse event then it should be accepted
    so that items lower in the stacking order do not also respond to the same event.
*/

/*!
    \qmlproperty enumeration QtQuick::MouseEvent::button

    This property holds the button that caused the event.  It can be one of:
    \list
    \li \l {Qt::LeftButton} {Qt.LeftButton}
    \li \l {Qt::RightButton} {Qt.RightButton}
    \li \l {Qt::MiddleButton} {Qt.MiddleButton}
    \endlist
*/

/*!
    \qmlproperty bool QtQuick::MouseEvent::wasHeld

    This property is true if the mouse button has been held pressed longer
    than the threshold (800ms).
*/

/*!
    \qmlproperty int QtQuick::MouseEvent::buttons

    This property holds the mouse buttons pressed when the event was generated.
    For mouse move events, this is all buttons that are pressed down. For mouse
    press and double click events this includes the button that caused the event.
    For mouse release events this excludes the button that caused the event.

    It contains a bitwise combination of:
    \list
    \li \l {Qt::LeftButton} {Qt.LeftButton}
    \li \l {Qt::RightButton} {Qt.RightButton}
    \li \l {Qt::MiddleButton} {Qt.MiddleButton}
    \endlist
*/

/*!
    \qmlproperty int QtQuick::MouseEvent::modifiers

    This property holds the keyboard modifier flags that existed immediately
    before the event occurred.

    It contains a bitwise combination of:
    \list
    \li \l {Qt::NoModifier} {Qt.NoModifier} - No modifier key is pressed.
    \li \l {Qt::ShiftModifier} {Qt.ShiftModifier} - A Shift key on the keyboard is pressed.
    \li \l {Qt::ControlModifier} {Qt.ControlModifier} - A Ctrl key on the keyboard is pressed.
    \li \l {Qt::AltModifier} {Qt.AltModifier} - An Alt key on the keyboard is pressed.
    \li \l {Qt::MetaModifier} {Qt.MetaModifier} - A Meta key on the keyboard is pressed.
    \li \l {Qt::KeypadModifier} {Qt.KeypadModifier} - A keypad button is pressed.
    \endlist

    For example, to react to a Shift key + Left mouse button click:
    \qml
    MouseArea {
        onClicked: {
            if ((mouse.button == Qt.LeftButton) && (mouse.modifiers & Qt.ShiftModifier))
                doSomething();
        }
    }
    \endqml
*/

/*!
    \qmlproperty int QtQuick::MouseEvent::source
    \since 5.7

    This property holds the source of the mouse event.

    The mouse event source can be used to distinguish between genuine and
    artificial mouse events. When using other pointing devices such as
    touchscreens and graphics tablets, if the application does not make use of
    the actual touch or tablet events, mouse events may be synthesized by the
    operating system or by Qt itself.

    The value can be one of:

    \list
    \li \l{Qt::MouseEventNotSynthesized} {Qt.MouseEventNotSynthesized}
    - The most common value. On platforms where such information is
    available, this value indicates that the event represents a genuine
    mouse event from the system.

    \li \l{Qt::MouseEventSynthesizedBySystem} {Qt.MouseEventSynthesizedBySystem} - Indicates that the mouse event was
    synthesized from a touch or tablet event by the platform.

    \li \l{Qt::MouseEventSynthesizedByQt} {Qt.MouseEventSynthesizedByQt}
    - Indicates that the mouse event was synthesized from an unhandled
    touch or tablet event by Qt.

    \li \l{Qt::MouseEventSynthesizedByApplication} {Qt.MouseEventSynthesizedByApplication}
    - Indicates that the mouse event was synthesized by the application.
    This allows distinguishing application-generated mouse events from
    the ones that are coming from the system or are synthesized by Qt.
    \endlist

    For example, to react only to events which come from an actual mouse:
    \qml
    MouseArea {
        onPressed: if (mouse.source !== Qt.MouseEventNotSynthesized) {
            mouse.accepted = false
        }

        onClicked: doSomething()
    }
    \endqml

    If the handler for the press event rejects the event, it will be propagated
    further, and then another Item underneath can handle synthesized events
    from touchscreens. For example, if a Flickable is used underneath (and the
    MouseArea is not a child of the Flickable), it can be useful for the
    MouseArea to handle genuine mouse events in one way, while allowing touch
    events to fall through to the Flickable underneath, so that the ability to
    flick on a touchscreen is retained. In that case the ability to drag the
    Flickable via mouse would be lost, but it does not prevent Flickable from
    receiving mouse wheel events.
*/

/*!
    \qmlproperty int QtQuick::MouseEvent::flags
    \since 5.11

    This property holds the flags that provide additional information about the
    mouse event.

    \list
    \li \l {Qt::MouseEventCreatedDoubleClick} {Qt.MouseEventCreatedDoubleClick}
    - Indicates that Qt has created a double click event from this event.
    This flag is set in the event originating from a button press, and not
    in the resulting double click event.
    \endlist
*/

/*!
    \qmltype WheelEvent
    \instantiates QQuickWheelEvent
    \inqmlmodule QtQuick
    \ingroup qtquick-input-events
    \brief Provides information about a mouse wheel event.

    The position of the mouse can be found via the
    \l {Item::x} {x} and \l {Item::y} {y} properties.

    \sa MouseArea
*/

/*!
    \internal
    \class QQuickWheelEvent
*/

/*!
    \qmlproperty real QtQuick::WheelEvent::x
    \qmlproperty real QtQuick::WheelEvent::y

    These properties hold the coordinates of the position supplied by the wheel event.
*/

/*!
    \qmlproperty bool QtQuick::WheelEvent::accepted

    Setting \a accepted to true prevents the wheel event from being
    propagated to items below this item.

    Generally, if the item acts on the wheel event then it should be accepted
    so that items lower in the stacking order do not also respond to the same event.
*/

/*!
    \qmlproperty int QtQuick::WheelEvent::buttons

    This property holds the mouse buttons pressed when the wheel event was generated.

    It contains a bitwise combination of:
    \list
    \li \l {Qt::LeftButton} {Qt.LeftButton}
    \li \l {Qt::RightButton} {Qt.RightButton}
    \li \l {Qt::MiddleButton} {Qt.MiddleButton}
    \endlist
*/

/*!
    \qmlproperty point QtQuick::WheelEvent::angleDelta

    This property holds the distance that the wheel is rotated in wheel degrees.
    The x and y cordinate of this property holds the delta in horizontal and
    vertical orientation.

    A positive value indicates that the wheel was rotated up/right;
    a negative value indicates that the wheel was rotated down/left.

    Most mouse types work in steps of 15 degrees, in which case the delta value is a
    multiple of 120; i.e., 120 units * 1/8 = 15 degrees.
*/

/*!
    \qmlproperty point QtQuick::WheelEvent::pixelDelta

    This property holds the delta in screen pixels and is available in platforms that
    have high-resolution trackpads, such as \macos.
    The x and y cordinate of this property holds the delta in horizontal and
    vertical orientation. The value should be used directly to scroll content on screen.

    For platforms without high-resolution trackpad support, pixelDelta will always be (0,0),
    and angleDelta should be used instead.
*/

/*!
    \qmlproperty int QtQuick::WheelEvent::modifiers

    This property holds the keyboard modifier flags that existed immediately
    before the event occurred.

    It contains a bitwise combination of:
    \list
    \li \l {Qt::NoModifier} {Qt.NoModifier} - No modifier key is pressed.
    \li \l {Qt::ShiftModifier} {Qt.ShiftModifier} - A Shift key on the keyboard is pressed.
    \li \l {Qt::ControlModifier} {Qt.ControlModifier} - A Ctrl key on the keyboard is pressed.
    \li \l {Qt::AltModifier} {Qt.AltModifier} - An Alt key on the keyboard is pressed.
    \li \l {Qt::MetaModifier} {Qt.MetaModifier} - A Meta key on the keyboard is pressed.
    \li \l {Qt::KeypadModifier} {Qt.KeypadModifier} - A keypad button is pressed.
    \endlist

    For example, to react to a Control key pressed during the wheel event:
    \qml
    MouseArea {
        onWheel: {
            if (wheel.modifiers & Qt.ControlModifier) {
                adjustZoom(wheel.angleDelta.y / 120);
            }
        }
    }
    \endqml
*/

/*!
    \qmlproperty int QtQuick::WheelEvent::inverted

    Returns whether the delta values delivered with the event are inverted.

    Normally, a vertical wheel will produce a WheelEvent with positive delta
    values if the top of the wheel is rotating away from the hand operating it.
    Similarly, a horizontal wheel movement will produce a QWheelEvent with
    positive delta values if the top of the wheel is moved to the left.

    However, on some platforms this is configurable, so that the same
    operations described above will produce negative delta values (but with the
    same magnitude). For instance, in a QML component (such as a tumbler or a
    slider) where it is appropriate to synchronize the movement or rotation of
    an item with the direction of the wheel, regardless of the system settings,
    the wheel event handler can use the inverted property to decide whether to
    negate the angleDelta or pixelDelta values.

    \note Many platforms provide no such information. On such platforms
    \l inverted always returns false.
*/

/*!
    \qmltype PointerDevice
    \instantiates QQuickPointerDevice
    \inqmlmodule QtQuick
    \ingroup qtquick-input-events

    \brief Provides information about a pointing device.

    A pointing device can be a mouse, a touchscreen, or a stylus on a graphics
    tablet.

    \sa PointerEvent, PointerHandler
*/

/*!
    \readonly
    \qmlproperty enumeration QtQuick::PointerDevice::type

    This property holds the type of the pointing device.

    Valid values are:

    \value DeviceType.UnknownDevice
        the device cannot be identified
    \value DeviceType.Mouse
        a mouse
    \value DeviceType.TouchScreen
        a touchscreen providing absolute coordinates
    \value DeviceType.TouchPad
        a trackpad or touchpad providing relative coordinates
    \value DeviceType.Stylus
        a pen-like device
    \value DeviceType.Airbrush
        a stylus with a thumbwheel to adjust
        \l {QTabletEvent::tangentialPressure}{tangentialPressure}
    \value DeviceType.Puck
        a device that is similar to a flat mouse with a
        transparent circle with cross-hairs
        (same as \l {QTabletEvent::Puck} {Puck})
    \value DeviceType.AllDevices
        any of the above; used as a default value for construction

    \sa QTouchDevice::DeviceType
*/

/*!
    \readonly
    \qmlproperty enumeration QtQuick::PointerDevice::pointerType

    This property holds a value indicating what is interacting with
    the device. Think of the device as having a planar 2D surface, and
    the value of this property as identifying what interacts with the
    device.

    There is some redundancy between this property and \l {PointerDevice::type}.
    If a tocuchscreen is used, then the device is TouchScreen and
    pointerType is Finger (always).

    Valid values are:

    \value PointerDevice.GenericPointer
        a mouse or something acting like a mouse (the core pointer on X11)
    \value PointerDevice.Finger
        the user's finger
    \value PointerDevice.Pen
        the drawing end of a stylus
    \value PointerDevice.Eraser
        the other end of the stylus (if it has a virtual eraser on the other end)
    \value PointerDevice.Cursor
        a cursor in the pre-computer sense of the word
    \value PointerDevice.AllPointerTypes
        any of the above (used as a default value in constructors)
*/


/*!
    \readonly
    \qmlproperty enumeration QtQuick::PointerDevice::capabilities

    This property holds a bitwise combination of the capabilities of the
    pointing device. It tells you under which conditions events are sent,
    and which properties of PointerEvent are expected to be valid.

    Valid values are:

    \value CapabilityFlag.Position
        the \l {QtQuick::EventPoint::position}{position} and
        \l {QtQuick::EventPoint::scenePosition}{scenePosition} properties
    \value CapabilityFlag.Area
        the \l {QtQuick::EventTouchPoint::ellipseDiameters}{ellipseDiameters} property
    \value CapabilityFlag.Pressure
        the \l {QtQuick::EventTouchPoint::pressure}{pressure} property
    \value CapabilityFlag.Velocity
        the \l {QtQuick::EventPoint::velocity}{velocity} property
    \value CapabilityFlag.Scroll
        a \l {QtQuick::PointerDevice::type}{Mouse} has a wheel, or the
        operating system recognizes scroll gestures on a
        \l {QtQuick::PointerDevice::type}{TouchPad}
    \value CapabilityFlag.Hover
        events are sent even when no button is pressed, or the finger or stylus
        is not in contact with the surface
    \value CapabilityFlag.Rotation
        the \l {QtQuick::EventTouchPoint::rotation}{rotation} property
    \value CapabilityFlag.XTilt
        horizontal angle between a stylus and the axis perpendicular to the surface
    \value CapabilityFlag.YTilt
        vertical angle between a stylus and the axis perpendicular to the surface

    \sa QTouchDevice::capabilities
*/

typedef QHash<const QTouchDevice *, QQuickPointerDevice *> PointerDeviceForTouchDeviceHash;
Q_GLOBAL_STATIC(PointerDeviceForTouchDeviceHash, g_touchDevices)

struct ConstructableQQuickPointerDevice : public QQuickPointerDevice
{
    ConstructableQQuickPointerDevice(DeviceType devType, PointerType pType, Capabilities caps,
                              int maxPoints, int buttonCount, const QString &name,
                              qint64 uniqueId = 0)
        : QQuickPointerDevice(devType, pType, caps, maxPoints, buttonCount, name, uniqueId) {}

};
Q_GLOBAL_STATIC_WITH_ARGS(ConstructableQQuickPointerDevice, g_genericMouseDevice,
                            (QQuickPointerDevice::Mouse,
                             QQuickPointerDevice::GenericPointer,
                             QQuickPointerDevice::Position | QQuickPointerDevice::Scroll | QQuickPointerDevice::Hover,
                             1, 3, QLatin1String("core pointer"), 0))

typedef QHash<qint64, QQuickPointerDevice *> PointerDeviceForDeviceIdHash;
Q_GLOBAL_STATIC(PointerDeviceForDeviceIdHash, g_tabletDevices)

// debugging helpers
static const char *pointStateString(const QQuickEventPoint *point)
{
    static const QMetaEnum stateMetaEnum = point->metaObject()->enumerator(point->metaObject()->indexOfEnumerator("State"));
    return stateMetaEnum.valueToKey(point->state());
}

static const QString pointDeviceName(const QQuickEventPoint *point)
{
    auto device = static_cast<const QQuickPointerEvent *>(point->parent())->device();
    QString deviceName = (device ? device->name() : QLatin1String("null device"));
    deviceName.resize(16, ' '); // shorten, and align in case of sequential output
    return deviceName;
}


QQuickPointerDevice *QQuickPointerDevice::touchDevice(const QTouchDevice *d)
{
    if (g_touchDevices->contains(d))
        return g_touchDevices->value(d);

    QQuickPointerDevice::DeviceType type = QQuickPointerDevice::TouchScreen;
    QString name;
    int maximumTouchPoints = 10;
    QQuickPointerDevice::Capabilities caps = QQuickPointerDevice::Capabilities(QTouchDevice::Position);
    if (d) {
        caps = static_cast<QQuickPointerDevice::Capabilities>(static_cast<int>(d->capabilities()) & 0xFF);
        if (d->type() == QTouchDevice::TouchPad) {
            type = QQuickPointerDevice::TouchPad;
            caps |= QQuickPointerDevice::Scroll;
        }
        name = d->name();
        maximumTouchPoints = d->maximumTouchPoints();
    } else {
        qWarning() << "QQuickWindowPrivate::touchDevice: creating touch device from nullptr device in QTouchEvent";
    }

    QQuickPointerDevice *dev = new QQuickPointerDevice(type, QQuickPointerDevice::Finger,
        caps, maximumTouchPoints, 0, name, 0);
    g_touchDevices->insert(d, dev);
    return dev;
}

const QTouchDevice *QQuickPointerDevice::qTouchDevice() const
{
    return g_touchDevices->key(const_cast<QQuickPointerDevice *>(this));
}

QList<QQuickPointerDevice*> QQuickPointerDevice::touchDevices()
{
    return g_touchDevices->values();
}

QQuickPointerDevice *QQuickPointerDevice::genericMouseDevice()
{
    return g_genericMouseDevice;
}

QQuickPointerDevice *QQuickPointerDevice::tabletDevice(qint64 id)
{
    auto it = g_tabletDevices->find(id);
    if (it != g_tabletDevices->end())
        return it.value();

    // ### Figure out how to populate the tablet devices
    return nullptr;
}

/*!
    \qmltype EventPoint
    \qmlabstract
    \instantiates QQuickEventPoint
    \inqmlmodule QtQuick
    \ingroup qtquick-input-events
    \brief Provides information about an individual point within a PointerEvent.

    A PointerEvent contains an EventPoint for each point of contact: one corresponding
    to the mouse cursor, or one for each finger touching a touchscreen.

    \sa PointerEvent, PointerHandler
*/

/*!
    \readonly
    \qmlproperty point QtQuick::EventPoint::position

    This property holds the coordinates of the position supplied by the event,
    relative to the upper-left corner of the Item which has the PointerHandler.
    If a contact patch is available from the pointing device, this point
    represents its centroid.
*/

/*!
    \readonly
    \qmlproperty point QtQuick::EventPoint::scenePosition

    This property holds the coordinates of the position supplied by the event,
    relative to the scene. If a contact patch is available from the
    \l {QtQuick::PointerEvent::device} {device}, this point represents its centroid.
*/

/*!
    \readonly
    \qmlproperty point QtQuick::EventPoint::scenePressPosition

    This property holds the scene-relative position at which the press event
    (on a touch device) or most recent change in QQuickPointerEvent::buttons()
    (on a mouse or tablet stylus) occurred.
*/

/*!
    \readonly
    \qmlproperty point QtQuick::EventPoint::sceneGrabPosition

    This property holds the scene-relative position at which the EventPoint was
    located when setGrabber() was called most recently.
*/

/*!
    \readonly
    \qmlproperty vector2d QtQuick::EventPoint::velocity

    This property holds average recent velocity: how fast and in which
    direction the event point has been moving recently.
*/

/*!
    \readonly
    \qmlproperty int QtQuick::EventPoint::state

    This property tells what the user is currently doing at this point.

    It can be one of:
    \value Pressed
        The user's finger is now pressing a touchscreen, button or stylus
        which was not pressed already
    \value Updated
        The touchpoint or position is being moved, with no change in pressed state
    \value Stationary
        The touchpoint or position is not being moved, and there is also
        no change in pressed state
    \value Released
        The user's finger has now released a touch point, button or stylus
        which was pressed
*/

/*!
    \readonly
    \qmlproperty int QtQuick::EventPoint::pointId

    This property holds the ID of the event, if any.

    Touchpoints have automatically-incrementing IDs: each time the user
    presses a finger against the touchscreen, it will be a larger number.
    In other cases, it will be -1.

    \sa {QtQuick::EventTouchPoint::uniqueId}{uniqueId}
*/

/*!
    \readonly
    \qmlproperty bool QtQuick::EventPoint::accepted

    Setting \a accepted to true prevents the event from being propagated to
    Items below the PointerHandler's Item.

    Generally, if the handler acts on the mouse event, then it should be
    accepted so that items lower in the stacking order do not also respond to
    the same event.
*/

/*!
    \readonly
    \qmlproperty real QtQuick::EventPoint::timeHeld

    This property holds the amount of time in seconds that the button or touchpoint has
    been held.  It can be used to detect a "long press", and can drive an
    animation to show progress toward activation of the "long press" action.
*/

void QQuickEventPoint::reset(Qt::TouchPointState state, const QPointF &scenePos, int pointId, ulong timestamp, const QVector2D &velocity)
{
    m_scenePos = scenePos;
    m_pointId = pointId;
    m_accept = false;
    m_state = static_cast<QQuickEventPoint::State>(state);
    m_timestamp = timestamp;
    if (state == Qt::TouchPointPressed) {
        m_pressTimestamp = timestamp;
        m_scenePressPos = scenePos;
    }
    m_velocity = (Q_LIKELY(velocity.isNull()) ? estimatedVelocity() : velocity);
}

void QQuickEventPoint::localizePosition(QQuickItem *target)
{
    if (target)
        m_pos = target->mapFromScene(scenePosition());
    else
        m_pos = QPointF();
}

/*!
    If this point has an exclusive grabber, returns a pointer to it; else
    returns null, if there is no grabber.  The grabber could be either
    an Item or a PointerHandler.
*/
QObject *QQuickEventPoint::exclusiveGrabber() const
{
    return m_exclusiveGrabber.data();
}

/*!
    Set the given Item or PointerHandler as the exclusive grabber of this point.
    If there was already an exclusive grab, it will be canceled.  If there
    were passive grabbers, they will continue to lurk, but the exclusive grab
    is a behavioral override of the passive grab as long as it remains.
    If you already know whether the grabber is to be an Item or a PointerHandler,
    you should instead call setGrabberItem() or setGrabberPointerHandler(),
    because it is slightly more efficient.
*/
void QQuickEventPoint::setExclusiveGrabber(QObject *grabber)
{
    if (QQuickPointerHandler *phGrabber = qmlobject_cast<QQuickPointerHandler *>(grabber))
        setGrabberPointerHandler(phGrabber, true);
    else
        setGrabberItem(static_cast<QQuickItem *>(grabber));
}

/*!
    If the exclusive grabber of this point is an Item, returns a
    pointer to that Item; else returns null, if there is no grabber or if
    the grabber is a PointerHandler.
*/
QQuickItem *QQuickEventPoint::grabberItem() const
{
    return (m_grabberIsHandler ? nullptr : static_cast<QQuickItem *>(m_exclusiveGrabber.data()));
}

/*!
    Set the given Item \a grabber as the exclusive grabber of this point.
    If there was already an exclusive grab, it will be canceled.  If there
    were passive grabbers, they will continue to lurk, but the exclusive grab
    is a behavioral override of the passive grab as long as it remains.
*/
void QQuickEventPoint::setGrabberItem(QQuickItem *grabber)
{
    if (grabber != m_exclusiveGrabber.data()) {
        QQuickPointerHandler *oldGrabberHandler = grabberPointerHandler();
        if (oldGrabberHandler && !oldGrabberHandler->approveGrabTransition(this, grabber))
            return;
        if (Q_UNLIKELY(lcPointerGrab().isDebugEnabled())) {
            qCDebug(lcPointerGrab) << pointDeviceName(this) << "point" << hex << m_pointId << pointStateString(this) << "@" << m_scenePos
                                   << ": grab" << m_exclusiveGrabber << "->" << grabber;
        }
        QQuickItem *oldGrabberItem = grabberItem();
        m_exclusiveGrabber = QPointer<QObject>(grabber);
        m_grabberIsHandler = false;
        m_sceneGrabPos = m_scenePos;
        if (oldGrabberHandler) {
            oldGrabberHandler->onGrabChanged(oldGrabberHandler, (grabber ? CancelGrabExclusive : UngrabExclusive), this);
        } else if (oldGrabberItem && oldGrabberItem != grabber && grabber && grabber->window()) {
            QQuickWindowPrivate *windowPriv = QQuickWindowPrivate::get(grabber->window());
            windowPriv->sendUngrabEvent(oldGrabberItem, windowPriv->isDeliveringTouchAsMouse());
        }
        if (grabber) {
            for (QPointer<QQuickPointerHandler> passiveGrabber : m_passiveGrabbers)
                if (passiveGrabber)
                    passiveGrabber->onGrabChanged(passiveGrabber, OverrideGrabPassive, this);
        }
    }
}

/*!
    If the exclusive grabber of this point is a PointerHandler, returns a
    pointer to that handler; else returns null, if there is no grabber or if
    the grabber is an Item.
*/
QQuickPointerHandler *QQuickEventPoint::grabberPointerHandler() const
{
    return (m_grabberIsHandler ? static_cast<QQuickPointerHandler *>(m_exclusiveGrabber.data()) : nullptr);
}

/*!
    Set the given PointerHandler \a grabber as grabber of this point. If \a
    exclusive is true, it will override any other grabs; if false, \a grabber
    will be added to the list of passive grabbers of this point.
*/
void QQuickEventPoint::setGrabberPointerHandler(QQuickPointerHandler *grabber, bool exclusive)
{
    if (Q_UNLIKELY(lcPointerGrab().isDebugEnabled())) {
        if (exclusive) {
            if (m_exclusiveGrabber != grabber)
                qCDebug(lcPointerGrab) << pointDeviceName(this) << "point" << hex << m_pointId << pointStateString(this)
                                       << ": grab (exclusive)" << m_exclusiveGrabber << "->" << grabber;
        } else {
            qCDebug(lcPointerGrab) << pointDeviceName(this) << "point" << hex << m_pointId << pointStateString(this)
                                   << ": grab (passive)" << grabber;
        }
    }
    if (exclusive) {
        if (grabber != m_exclusiveGrabber.data()) {
            QQuickPointerHandler *oldGrabberHandler = grabberPointerHandler();
            QQuickItem *oldGrabberItem = grabberItem();
            m_exclusiveGrabber = QPointer<QObject>(grabber);
            m_grabberIsHandler = true;
            m_sceneGrabPos = m_scenePos;
            if (grabber) {
                grabber->onGrabChanged(grabber, GrabExclusive, this);
                for (QPointer<QQuickPointerHandler> passiveGrabber : m_passiveGrabbers) {
                    if (!passiveGrabber.isNull() && passiveGrabber != grabber)
                        passiveGrabber->onGrabChanged(grabber, OverrideGrabPassive, this);
                }
            }
            if (oldGrabberHandler) {
                oldGrabberHandler->onGrabChanged(oldGrabberHandler, (grabber ? CancelGrabExclusive : UngrabExclusive), this);
            } else if (oldGrabberItem) {
                if (pointerEvent()->asPointerTouchEvent())
                    oldGrabberItem->touchUngrabEvent();
                else if (pointerEvent()->asPointerMouseEvent())
                    oldGrabberItem->mouseUngrabEvent();
            }
            // touchUngrabEvent() can result in the grabber being set to null (MPTA does that, for example).
            // So set it again to ensure that final state is what we want.
            m_exclusiveGrabber = QPointer<QObject>(grabber);
            m_grabberIsHandler = true;
            m_sceneGrabPos = m_scenePos;
        }
    } else {
        if (!grabber) {
            qDebug() << "can't set passive grabber to null";
            return;
        }
        auto ptr = QPointer<QQuickPointerHandler>(grabber);
        if (!m_passiveGrabbers.contains(ptr)) {
            m_passiveGrabbers.append(ptr);
            grabber->onGrabChanged(grabber, GrabPassive, this);
        }
    }
}

/*!
    If this point has an existing exclusive grabber (Item or PointerHandler),
    inform the grabber that its grab is canceled, and remove it as grabber.
    This normally happens when the grab is stolen by another Item.
*/
void QQuickEventPoint::cancelExclusiveGrab()
{
    if (m_exclusiveGrabber.isNull())
        qWarning("cancelGrab: no grabber");
    else
        cancelExclusiveGrabImpl();
}

void QQuickEventPoint::cancelExclusiveGrabImpl(QTouchEvent *cancelEvent)
{
    if (m_exclusiveGrabber.isNull())
        return;
    if (Q_UNLIKELY(lcPointerGrab().isDebugEnabled())) {
        qCDebug(lcPointerGrab) << pointDeviceName(this) << "point" << hex << m_pointId << pointStateString(this)
                               << ": grab (exclusive)" << m_exclusiveGrabber << "-> nullptr";
    }
    if (auto handler = grabberPointerHandler()) {
        handler->onGrabChanged(handler, CancelGrabExclusive, this);
    } else if (auto item = grabberItem()) {
        if (cancelEvent)
            QCoreApplication::sendEvent(item, cancelEvent);
        else
            item->touchUngrabEvent();
    }
    m_exclusiveGrabber.clear();
}

/*!
    If this point has the given \a handler as a passive grabber,
    inform the grabber that its grab is canceled, and remove it as grabber.
    This normally happens when another Item or PointerHandler does an exclusive grab.
*/
void QQuickEventPoint::cancelPassiveGrab(QQuickPointerHandler *handler)
{
    if (removePassiveGrabber(handler)) {
        if (Q_UNLIKELY(lcPointerGrab().isDebugEnabled())) {
            qCDebug(lcPointerGrab) << pointDeviceName(this) << "point" << hex << m_pointId << pointStateString(this)
                                   << ": grab (passive)" << handler << "removed";
        }
        handler->onGrabChanged(handler, CancelGrabPassive, this);
    }
}

/*!
    If this point has the given \a handler as a passive grabber, remove it as grabber.
    Returns true if it was removed, false if it wasn't a grabber.
*/
bool QQuickEventPoint::removePassiveGrabber(QQuickPointerHandler *handler)
{
    return m_passiveGrabbers.removeOne(handler);
}

/*!
    If the given \a handler is grabbing this point passively, exclusively
    or both, cancel the grab and remove it as grabber.
    This normally happens when the handler decides that the behavior of this
    point can no longer satisfy the handler's behavioral constraints within
    the remainder of the gesture which the user is performing: for example
    the handler tries to detect a tap but a drag is occurring instead, or
    it tries to detect a drag in one direction but the drag is going in
    another direction.  In such cases the handler no longer needs or wants
    to be informed of any further movements of this point.
*/
void QQuickEventPoint::cancelAllGrabs(QQuickPointerHandler *handler)
{
    if (m_exclusiveGrabber == handler) {
        handler->onGrabChanged(handler, CancelGrabExclusive, this);
        m_exclusiveGrabber.clear();
    }
    cancelPassiveGrab(handler);
}

/*!
    Sets this point as \a accepted (true) or rejected (false).

    During delivery of the current event to the Items in the scene, each Item
    or Pointer Handler should accept the points for which it is taking
    responsibility. As soon as all points within the event are accepted, event
    propagation stops. However accepting the point does not imply any kind of
    grab, passive or exclusive.

    \sa setExclusiveGrabber, QQuickPointerHandler::setPassiveGrab, QQuickPointerHandler::setExclusiveGrab
*/
void QQuickEventPoint::setAccepted(bool accepted)
{
    if (m_accept != accepted) {
        qCDebug(lcPointerEvents) << this << m_accept << "->" << accepted;
        m_accept = accepted;
    }
}


/*!
    \qmltype EventTouchPoint
    \qmlabstract
    \instantiates QQuickEventTouchPoint
    \inqmlmodule QtQuick
    \ingroup qtquick-input-events
    \brief Provides information about an individual touch point within a PointerEvent.

    \sa PointerEvent, PointerHandler
*/

/*!
    \readonly
    \qmlproperty QPointerUniqueId QtQuick::EventTouchPoint::uniqueId

    This property holds the unique ID of the fiducial or stylus in use, if any.

    On touchscreens that can track physical objects (such as knobs or game
    pieces) in addition to fingers, each object usually has a unique ID.
    Likewise, each stylus that can be used with a graphics tablet usually has a
    unique serial number. Qt so far only supports numeric IDs. You can get the
    actual number as uniqueId.numeric, but that is a device-specific detail.
    In the future, there may be support for non-numeric IDs, so you should
    not assume that the number is meaningful.

    If you need to identify specific objects, your application should provide
    UI for registering objects and mapping them to functionality: allow the
    user to select a meaning, virtual tool, or action, prompt the user to bring
    the object into proximity, and store a mapping from uniqueId to its
    purpose, for example in \l Settings.
*/

/*!
    \readonly
    \qmlproperty qreal QtQuick::EventTouchPoint::rotation

    This property holds the rotation angle of the stylus on a graphics tablet
    or the contact patch of a touchpoint on a touchscreen.

    It is valid only with certain tablet stylus devices and touchscreens that
    can measure the rotation angle. Otherwise, it will be zero.
*/

/*!
    \readonly
    \qmlproperty qreal QtQuick::EventTouchPoint::pressure

    This property tells how hard the user is pressing the stylus on a graphics
    tablet or the finger against a touchscreen, in the range from \c 0 (no
    measurable pressure) to \c 1.0 (maximum pressure which the device can
    measure).

    It is valid only with certain tablets and touchscreens that can measure
    pressure. Otherwise, it will be \c 1.0 when pressed.
*/

/*!
    \readonly
    \qmlproperty size QtQuick::EventTouchPoint::ellipseDiameters

    This property holds the diameters of the contact patch, if the event
    comes from a touchpoint and the \l {QtQuick::PointerEvent::device} {device}
    provides this information.

    A touchpoint is modeled as an elliptical area where the finger is
    pressed against the touchscreen. (In fact, it could also be
    modeled as a bitmap; but in that case we expect an elliptical
    bounding estimate to be fitted to the contact patch before the
    event is sent.) The harder the user presses, the larger the
    contact patch; so, these diameters provide an alternate way of
    detecting pressure, in case the device does not include a separate
    pressure sensor. The ellipse is centered on
    \l {QtQuick::EventPoint::scenePosition} {scenePosition}
    (\l {QtQuick::EventPoint::position} {position} in the PointerHandler's
    Item's local coordinates).  The \l rotation property provides the
    rotation of the ellipse, if known.  It is expected that if the
    \l rotation is zero, the verticalDiameter of the ellipse is the
    larger one (the major axis), because of the usual hand position,
    reaching upward or outward across the surface.

    If the contact patch is unknown, or the \l {QtQuick::PointerEvent::device} {device}
    is not a touchscreen, these values will be zero.
*/

QQuickEventTouchPoint::QQuickEventTouchPoint(QQuickPointerTouchEvent *parent)
    : QQuickEventPoint(parent), m_rotation(0), m_pressure(0)
{}

void QQuickEventTouchPoint::reset(const QTouchEvent::TouchPoint &tp, ulong timestamp)
{
    QQuickEventPoint::reset(tp.state(), tp.scenePos(), tp.id(), timestamp, tp.velocity());
    m_exclusiveGrabber.clear();
    m_passiveGrabbers.clear();
    m_rotation = tp.rotation();
    m_pressure = tp.pressure();
    m_ellipseDiameters = tp.ellipseDiameters();
    m_uniqueId = tp.uniqueId();
}

struct PointVelocityData {
    QVector2D velocity;
    QPointF pos;
    ulong timestamp = 0;
};

typedef QMap<quint64, PointVelocityData> PointDataForPointIdMap;
Q_GLOBAL_STATIC(PointDataForPointIdMap, g_previousPointData)
static const int PointVelocityAgeLimit = 500; // milliseconds

/*!
    \internal
    Estimates the velocity based on a weighted average of all previous velocities.
    The older the velocity is, the less significant it becomes for the estimate.
*/
QVector2D QQuickEventPoint::estimatedVelocity() const
{
    auto prevPointIt = g_previousPointData->find(m_pointId);
    auto end = g_previousPointData->end();
    if (prevPointIt == end) {
        // cleanup events older than PointVelocityAgeLimit
        for (auto it = g_previousPointData->begin(); it != end; ) {
            if (m_timestamp - it->timestamp > PointVelocityAgeLimit)
                it = g_previousPointData->erase(it);
            else
                ++it;
        }
        prevPointIt = g_previousPointData->insert(m_pointId, PointVelocityData());
    }

    auto &prevPoint = prevPointIt.value();
    const ulong timeElapsed = m_timestamp - prevPoint.timestamp;
    if (timeElapsed == 0)   // in case we call estimatedVelocity() twice on the same QQuickEventPoint
        return m_velocity;

    QVector2D newVelocity;
    if (prevPoint.timestamp != 0)
        newVelocity = QVector2D(m_scenePos - prevPoint.pos) / timeElapsed;

    // VERY simple kalman filter: does a weighted average
    // where the older velocities get less and less significant
    static const float KalmanGain = 0.7f;
    QVector2D filteredVelocity = newVelocity * KalmanGain + m_velocity * (1.0f - KalmanGain);

    prevPoint.velocity = filteredVelocity;
    prevPoint.pos = m_scenePos;
    prevPoint.timestamp = m_timestamp;
    return filteredVelocity;
}

/*!
    \qmltype PointerEvent
    \instantiates QQuickPointerEvent
    \inqmlmodule QtQuick
    \ingroup qtquick-input-events

    \brief Provides information about an event from a pointing device.

    A PointerEvent is an event describing contact or movement across a surface,
    provided by a mouse, a touchpoint (single finger on a touchscreen), or a
    stylus on a graphics tablet. The \l {QtQuick::PointerEvent::device} {device}
    property provides more information about where the event came from.

    \sa PointerHandler

    \image touchpoint-metrics.png
*/

/*!
    \internal
    \class QQuickPointerEvent

    QQuickPointerEvent is used as a long-lived object to store data related to
    an event from a pointing device, such as a mouse, touch or tablet event,
    during event delivery. It also provides properties which may be used later
    to expose the event to QML, the same as is done with QQuickMouseEvent,
    QQuickTouchPoint, QQuickKeyEvent, etc. Since only one event can be
    delivered at a time, this class is effectively a singleton.  We don't worry
    about the QObject overhead because the instances are long-lived: we don't
    dynamically create and destroy objects of this type for each event.
*/

/*!
    \readonly
    \qmlproperty enumeration QtQuick::PointerEvent::button

    This property holds the \l {Qt::MouseButton}{button} that caused the event,
    if any. If the \l {QtQuick::PointerEvent::device} {device} does not have
    buttons, or the event is a hover event, it will be \c Qt.NoButton.
*/

/*!
    \readonly
    \qmlproperty int QtQuick::PointerEvent::buttons

    This property holds the combination of mouse or stylus
    \l {Qt::MouseButton}{buttons} pressed when the event was generated. For move
    events, this is all buttons that are pressed down.  For press events, this
    includes the button that caused the event, as well as any others that were
    already held.  For release events, this excludes the button that caused the
    event.
*/

/*!
    \readonly
    \qmlproperty int QtQuick::PointerEvent::modifiers

    This property holds the \l {Qt::KeyboardModifier}{keyboard modifier} flags
    that existed immediately before the event occurred.

    It contains a bitwise combination of the following flags:
    \value Qt.NoModifier
        No modifier key is pressed.
    \value Qt.ShiftModifier
        A Shift key on the keyboard is pressed.
    \value Qt.ControlModifier
        A Ctrl key on the keyboard is pressed.
    \value Qt.AltModifier
        An Alt key on the keyboard is pressed.
    \value Qt.MetaModifier
        A Meta key on the keyboard is pressed.
    \value Qt.KeypadModifier
        A keypad button is pressed.

    For example, to react to a Shift key + Left mouse button click:
    \qml
    Item {
        TapHandler {
            onTapped: {
                if ((event.button == Qt.LeftButton) && (event.modifiers & Qt.ShiftModifier))
                    doSomething();
            }
        }
    }
    \endqml
*/

/*!
    \readonly
    \qmlproperty PointerDevice QtQuick::PointerEvent::device

    This property holds the device that generated the event.
*/

QQuickPointerEvent::~QQuickPointerEvent()
{}

QQuickPointerEvent *QQuickPointerMouseEvent::reset(QEvent *event)
{
    auto ev = static_cast<QMouseEvent*>(event);
    m_event = ev;
    if (!event)
        return this;

    m_device = QQuickPointerDevice::genericMouseDevice();
    m_device->eventDeliveryTargets().clear();
    m_button = ev->button();
    m_pressedButtons = ev->buttons();
    Qt::TouchPointState state = Qt::TouchPointStationary;
    switch (ev->type()) {
    case QEvent::MouseButtonPress:
        m_point->clearPassiveGrabbers();
        Q_FALLTHROUGH();
    case QEvent::MouseButtonDblClick:
        state = Qt::TouchPointPressed;
        break;
    case QEvent::MouseButtonRelease:
        state = Qt::TouchPointReleased;
        break;
    case QEvent::MouseMove:
        state = Qt::TouchPointMoved;
        break;
    default:
        break;
    }
    m_point->reset(state, ev->windowPos(), quint64(1) << 24, ev->timestamp());  // mouse has device ID 1
    return this;
}

void QQuickSinglePointEvent::localize(QQuickItem *target)
{
    m_point->localizePosition(target);
}

QQuickPointerEvent *QQuickPointerTouchEvent::reset(QEvent *event)
{
    auto ev = static_cast<QTouchEvent*>(event);
    m_event = ev;
    if (!event)
        return this;

    m_device = QQuickPointerDevice::touchDevice(ev->device());
    m_device->eventDeliveryTargets().clear();
    m_button = Qt::NoButton;
    m_pressedButtons = Qt::NoButton;

    const QList<QTouchEvent::TouchPoint> &tps = ev->touchPoints();
    int newPointCount = tps.count();
    m_touchPoints.reserve(newPointCount);

    for (int i = m_touchPoints.size(); i < newPointCount; ++i)
        m_touchPoints.insert(i, new QQuickEventTouchPoint(this));

    // Make sure the grabbers and on-pressed values are right from one event to the next
    struct ToPreserve {
        int pointId; // just for double-checking
        ulong pressTimestamp;
        QPointF scenePressPos;
        QPointF sceneGrabPos;
        QObject * grabber;
        QVector <QPointer <QQuickPointerHandler> > passiveGrabbers;

        ToPreserve() : pointId(0), pressTimestamp(0), grabber(nullptr) {}
    };
    QVector<ToPreserve> preserves(newPointCount); // jar of pickled touchpoints, in order of points in the _new_ event

    // Copy stuff we need to preserve, because the order of points might have changed in the event.
    // The ID is all that we can rely on (release might remove the first point etc).
    for (int i = 0; i < newPointCount; ++i) {
        int pid = tps.at(i).id();
        if (auto point = pointById(pid)) {
            preserves[i].pointId = pid;
            preserves[i].pressTimestamp = point->m_pressTimestamp;
            preserves[i].scenePressPos = point->scenePressPosition();
            preserves[i].sceneGrabPos = point->sceneGrabPosition();
            preserves[i].grabber = point->exclusiveGrabber();
            preserves[i].passiveGrabbers = point->passiveGrabbers();
        }
    }

    for (int i = 0; i < newPointCount; ++i) {
        auto point = m_touchPoints.at(i);
        point->reset(tps.at(i), ev->timestamp());
        const auto &preserved = preserves.at(i);
        if (point->state() == QQuickEventPoint::Pressed) {
            if (preserved.grabber)
                qWarning() << "TouchPointPressed without previous release event" << point;
            point->setGrabberItem(nullptr);
            point->clearPassiveGrabbers();
        } else {
            // Restore the grabbers without notifying (don't call onGrabChanged)
            Q_ASSERT(preserved.pointId == 0 || preserved.pointId == point->pointId());
            point->m_pressTimestamp = preserved.pressTimestamp;
            point->m_scenePressPos = preserved.scenePressPos;
            point->m_sceneGrabPos = preserved.sceneGrabPos;
            point->m_exclusiveGrabber = preserved.grabber;
            point->m_grabberIsHandler = (qmlobject_cast<QQuickPointerHandler *>(point->m_exclusiveGrabber) != nullptr);
            point->m_passiveGrabbers = preserved.passiveGrabbers;
        }
    }
    m_pointCount = newPointCount;
    return this;
}

void QQuickPointerTouchEvent::localize(QQuickItem *target)
{
    for (auto point : qAsConst(m_touchPoints))
        point->localizePosition(target);
}

#if QT_CONFIG(gestures)
QQuickPointerEvent *QQuickPointerNativeGestureEvent::reset(QEvent *event)
{
    auto ev = static_cast<QNativeGestureEvent*>(event);
    m_event = ev;
    if (!event)
        return this;

    m_device = QQuickPointerDevice::touchDevice(ev->device());
    m_device->eventDeliveryTargets().clear();
    Qt::TouchPointState state = Qt::TouchPointMoved;
    switch (type()) {
    case Qt::BeginNativeGesture:
        state = Qt::TouchPointPressed;
        break;
    case Qt::EndNativeGesture:
        state = Qt::TouchPointReleased;
        break;
    default:
        break;
    }
    quint64 deviceId = QTouchDevicePrivate::get(const_cast<QTouchDevice *>(ev->device()))->id; // a bit roundabout since QTouchDevice::mTouchDeviceId is protected
    m_point->reset(state, ev->windowPos(), deviceId << 24, ev->timestamp());
    return this;
}
#endif // QT_CONFIG(gestures)

QQuickEventPoint *QQuickSinglePointEvent::point(int i) const
{
    if (i == 0)
        return m_point;
    return nullptr;
}

QQuickPointerEvent *QQuickPointerScrollEvent::reset(QEvent *event)
{
    m_event = static_cast<QInputEvent*>(event);
    if (!event)
        return this;
#if QT_CONFIG(wheelevent)
    if (event->type() == QEvent::Wheel) {
        auto ev = static_cast<QWheelEvent*>(event);
        m_device = QQuickPointerDevice::genericMouseDevice();
        m_device->eventDeliveryTargets().clear();
        // m_button = Qt::NoButton;
        m_pressedButtons = ev->buttons();
        m_angleDelta = QVector2D(ev->angleDelta());
        m_pixelDelta = QVector2D(ev->pixelDelta());
        m_phase = ev->phase();
        m_synthSource = ev->source();
        m_inverted = ev->inverted();

        m_point->reset(Qt::TouchPointMoved, ev->posF(), quint64(1) << 24, ev->timestamp()); // mouse has device ID 1
    }
#endif
    // TODO else if (event->type() == QEvent::Scroll) ...
    return this;
}

void QQuickPointerScrollEvent::localize(QQuickItem *target)
{
    m_point->localizePosition(target);
}

QQuickEventPoint *QQuickPointerTouchEvent::point(int i) const
{
    if (i >= 0 && i < m_pointCount)
        return m_touchPoints.at(i);
    return nullptr;
}

QQuickEventPoint::QQuickEventPoint(QQuickPointerEvent *parent)
  : QObject(parent), m_pointId(0), m_exclusiveGrabber(nullptr), m_timestamp(0), m_pressTimestamp(0),
    m_state(QQuickEventPoint::Released), m_accept(false), m_grabberIsHandler(false)
{
    Q_UNUSED(m_reserved);
}

QQuickPointerEvent *QQuickEventPoint::pointerEvent() const
{
    return static_cast<QQuickPointerEvent *>(parent());
}

bool QQuickSinglePointEvent::allPointsAccepted() const
{
    return m_point->isAccepted();
}

bool QQuickSinglePointEvent::allUpdatedPointsAccepted() const
{
    return m_point->state() == QQuickEventPoint::Pressed || m_point->isAccepted();
}

bool QQuickSinglePointEvent::allPointsGrabbed() const
{
    return m_point->exclusiveGrabber() != nullptr;
}

QMouseEvent *QQuickPointerMouseEvent::asMouseEvent(const QPointF &localPos) const
{
    auto event = static_cast<QMouseEvent *>(m_event);
    event->setLocalPos(localPos);
    return event;
}

/*!
    Returns the exclusive grabber of this event, if any, in a vector.
*/
QVector<QObject *> QQuickSinglePointEvent::exclusiveGrabbers() const
{
    QVector<QObject *> result;
    if (QObject *grabber = m_point->exclusiveGrabber())
        result << grabber;
    return result;
}

/*!
    Remove all passive and exclusive grabbers of this event, without notifying.
*/
void QQuickSinglePointEvent::clearGrabbers() const
{
    m_point->setGrabberItem(nullptr);
    m_point->clearPassiveGrabbers();
}

/*!
    Returns whether the given \a handler is the exclusive grabber of this event.
*/
bool QQuickSinglePointEvent::hasExclusiveGrabber(const QQuickPointerHandler *handler) const
{
    return handler && (m_point->exclusiveGrabber() == handler);
}

bool QQuickPointerMouseEvent::isPressEvent() const
{
    auto me = static_cast<QMouseEvent*>(m_event);
    return ((me->type() == QEvent::MouseButtonPress || me->type() == QEvent::MouseButtonDblClick) &&
            (me->buttons() & me->button()) == me->buttons());
}

bool QQuickPointerMouseEvent::isDoubleClickEvent() const
{
    auto me = static_cast<QMouseEvent*>(m_event);
    return (me->type() == QEvent::MouseButtonDblClick);
}

bool QQuickPointerMouseEvent::isUpdateEvent() const
{
    auto me = static_cast<QMouseEvent*>(m_event);
    return me->type() == QEvent::MouseMove;
}

bool QQuickPointerMouseEvent::isReleaseEvent() const
{
    auto me = static_cast<QMouseEvent*>(m_event);
    return me && me->type() == QEvent::MouseButtonRelease;
}

bool QQuickPointerTouchEvent::allPointsAccepted() const
{
    for (int i = 0; i < m_pointCount; ++i) {
        if (!m_touchPoints.at(i)->isAccepted())
            return false;
    }
    return true;
}

bool QQuickPointerTouchEvent::allUpdatedPointsAccepted() const
{
    for (int i = 0; i < m_pointCount; ++i) {
        auto point = m_touchPoints.at(i);
        if (point->state() != QQuickEventPoint::Pressed && !point->isAccepted())
            return false;
    }
    return true;
}

bool QQuickPointerTouchEvent::allPointsGrabbed() const
{
    for (int i = 0; i < m_pointCount; ++i) {
        if (!m_touchPoints.at(i)->exclusiveGrabber())
            return false;
    }
    return true;
}

/*!
    Returns the exclusive grabbers of all points in this event, if any, in a vector.
*/
QVector<QObject *> QQuickPointerTouchEvent::exclusiveGrabbers() const
{
    QVector<QObject *> result;
    for (int i = 0; i < m_pointCount; ++i) {
        if (QObject *grabber = m_touchPoints.at(i)->exclusiveGrabber()) {
            if (!result.contains(grabber))
                result << grabber;
        }
    }
    return result;
}

/*!
    Remove all passive and exclusive grabbers of all touchpoints in this event,
    without notifying.
*/
void QQuickPointerTouchEvent::clearGrabbers() const
{
    for (auto point: m_touchPoints) {
        point->setGrabberItem(nullptr);
        point->clearPassiveGrabbers();
    }
}

Qt::TouchPointStates QQuickPointerTouchEvent::touchPointStates() const
{
    return m_event
        ? static_cast<QTouchEvent*>(m_event)->touchPointStates()
        : Qt::TouchPointStates();
}

/*!
    Returns whether the given \a handler is the exclusive grabber of any
    touchpoint within this event.
*/
bool QQuickPointerTouchEvent::hasExclusiveGrabber(const QQuickPointerHandler *handler) const
{
    for (auto point: m_touchPoints)
        if (point->exclusiveGrabber() == handler)
            return true;
    return false;
}

bool QQuickPointerTouchEvent::isPressEvent() const
{
    return touchPointStates() & Qt::TouchPointPressed;
}

bool QQuickPointerTouchEvent::isUpdateEvent() const
{
    return touchPointStates() & (Qt::TouchPointMoved | Qt::TouchPointStationary);
}

bool QQuickPointerTouchEvent::isReleaseEvent() const
{
    return touchPointStates() & Qt::TouchPointReleased;
}

QVector<QPointF> QQuickPointerEvent::unacceptedPressedPointScenePositions() const
{
    QVector<QPointF> points;
    for (int i = 0; i < pointCount(); ++i) {
        if (!point(i)->isAccepted() && point(i)->state() == QQuickEventPoint::Pressed)
            points << point(i)->scenePosition();
    }
    return points;
}

/*!
    \internal
    Populate the reusable synth-mouse event from one touchpoint.
    It's required that isTouchEvent() be true when this is called.
    If the touchpoint cannot be found, this returns nullptr.
    Ownership of the event is NOT transferred to the caller.
*/
QMouseEvent *QQuickPointerTouchEvent::syntheticMouseEvent(int pointID, QQuickItem *relativeTo) const
{
    const QTouchEvent::TouchPoint *p = touchPointById(pointID);
    if (!p)
        return nullptr;
    QEvent::Type type;
    Qt::MouseButton buttons = Qt::LeftButton;
    switch (p->state()) {
    case Qt::TouchPointPressed:
        type = QEvent::MouseButtonPress;
        break;
    case Qt::TouchPointMoved:
    case Qt::TouchPointStationary:
        type = QEvent::MouseMove;
        break;
    case Qt::TouchPointReleased:
        type = QEvent::MouseButtonRelease;
        buttons = Qt::NoButton;
        break;
    default:
        Q_ASSERT(false);
        return nullptr;
    }
    m_synthMouseEvent = QMouseEvent(type, relativeTo->mapFromScene(p->scenePos()),
        p->scenePos(), p->screenPos(), Qt::LeftButton, buttons, m_event->modifiers());
    m_synthMouseEvent.setAccepted(true);
    m_synthMouseEvent.setTimestamp(m_event->timestamp());
    // In the future we will try to always have valid velocity in every QQuickEventPoint.
    // QQuickFlickablePrivate::handleMouseMoveEvent() checks for QTouchDevice::Velocity
    // and if it is set, then it does not need to do its own velocity calculations.
    // That's probably the only usecase for this, so far.  Some day Flickable should handle
    // pointer events, and then passing touchpoint velocity via QMouseEvent will be obsolete.
    // Conveniently (by design), QTouchDevice::Velocity == QQuickPointerDevice.Velocity
    // so that we don't need to convert m_device->capabilities().
    if (m_device)
        QGuiApplicationPrivate::setMouseEventCapsAndVelocity(&m_synthMouseEvent, m_device->capabilities(), p->velocity());
    QGuiApplicationPrivate::setMouseEventSource(&m_synthMouseEvent, Qt::MouseEventSynthesizedByQt);
    return &m_synthMouseEvent;
}

#if QT_CONFIG(gestures)
bool QQuickPointerNativeGestureEvent::isPressEvent() const
{
    return type() == Qt::BeginNativeGesture;
}

bool QQuickPointerNativeGestureEvent::isUpdateEvent() const
{
    switch (type()) {
    case Qt::BeginNativeGesture:
    case Qt::EndNativeGesture:
        return false;
    default:
        return true;
    }
}

bool QQuickPointerNativeGestureEvent::isReleaseEvent() const
{
    return type() == Qt::EndNativeGesture;
}

Qt::NativeGestureType QQuickPointerNativeGestureEvent::type() const
{
    return static_cast<QNativeGestureEvent *>(m_event)->gestureType();
}

qreal QQuickPointerNativeGestureEvent::value() const
{
    return static_cast<QNativeGestureEvent *>(m_event)->value();
}
#endif // QT_CONFIG(gestures)

/*!
    Returns whether the scroll event has Qt::ScrollBegin phase. On touchpads
    which provide phase information, this is true when the fingers are placed
    on the touchpad and scrolling begins. On other devices where this
    information is not available, it remains false.
*/
bool QQuickPointerScrollEvent::isPressEvent() const
{
    return phase() == Qt::ScrollBegin;
}

/*!
    Returns true when the scroll event has Qt::ScrollUpdate phase, or when the
    phase is unknown. Some multi-touch-capable touchpads and trackpads provide
    phase information; whereas ordinary mouse wheels and other types of
    trackpads do not, and in such cases this is always true.
*/
bool QQuickPointerScrollEvent::isUpdateEvent() const
{
    return phase() == Qt::ScrollUpdate || phase() == Qt::NoScrollPhase;
}

/*!
    Returns whether the scroll event has Qt::ScrollBegin phase. On touchpads
    which provide phase information, this is true when the fingers are lifted
    from the touchpad. On other devices where this information is not
    available, it remains false.
*/
bool QQuickPointerScrollEvent::isReleaseEvent() const
{
    return phase() == Qt::ScrollEnd;
}

/*!
    \internal
    Returns a pointer to the QQuickEventPoint which has the \a pointId as
    \l {QQuickEventPoint::pointId}{pointId}.
    Returns nullptr if there is no point with that ID.

    \fn QQuickPointerEvent::pointById(int pointId) const
*/
QQuickEventPoint *QQuickSinglePointEvent::pointById(int pointId) const
{
    if (m_point && pointId == m_point->pointId())
        return m_point;
    return nullptr;
}

QQuickEventPoint *QQuickPointerTouchEvent::pointById(int pointId) const
{
    auto it = std::find_if(m_touchPoints.constBegin(), m_touchPoints.constEnd(),
        [pointId](const QQuickEventTouchPoint *tp) { return tp->pointId() == pointId; } );
    if (it != m_touchPoints.constEnd())
        return *it;
    return nullptr;
}

/*!
    \internal
    Returns a pointer to the original TouchPoint which has the same
    \l {QTouchEvent::TouchPoint::id}{id} as \a pointId, if the original event is a
    QTouchEvent, and if that point is found. Otherwise, returns nullptr.
*/
const QTouchEvent::TouchPoint *QQuickPointerTouchEvent::touchPointById(int pointId) const
{
    const QTouchEvent *ev = asTouchEvent();
    if (!ev)
        return nullptr;
    const QList<QTouchEvent::TouchPoint> &tps = ev->touchPoints();
    auto it = std::find_if(tps.constBegin(), tps.constEnd(),
        [pointId](QTouchEvent::TouchPoint const& tp) { return tp.id() == pointId; } );
    // return the pointer to the actual TP in QTouchEvent::_touchPoints
    return (it == tps.constEnd() ? nullptr : it.operator->());
}

/*!
    \internal
    Make a new QTouchEvent, giving it a subset of the original touch points.

    Returns a nullptr if all points are stationary, or there are no points inside the item,
    or none of the points were pressed inside and the item was not grabbing any of them
    and isFiltering is false.  When isFiltering is true, it is assumed that the item
    cares about all points which are inside its bounds, because most filtering items
    need to monitor eventpoint movements until a drag threshold is exceeded or the
    requirements for a gesture to be recognized are met in some other way.
*/
QTouchEvent *QQuickPointerTouchEvent::touchEventForItem(QQuickItem *item, bool isFiltering) const
{
    QList<QTouchEvent::TouchPoint> touchPoints;
    Qt::TouchPointStates eventStates;
    // TODO maybe add QQuickItem::mapVector2DFromScene(QVector2D) to avoid needing QQuickItemPrivate here
    // Or else just document that velocity is always scene-relative and is not scaled and rotated with the item
    // but that would require changing tst_qquickwindow::touchEvent_velocity(): it expects transformed velocity

    bool anyPressOrReleaseInside = false;
    bool anyStationaryWithModifiedPropertyInside = false;
    bool anyGrabber = false;
    QMatrix4x4 transformMatrix(QQuickItemPrivate::get(item)->windowToItemTransform());
    for (int i = 0; i < m_pointCount; ++i) {
        auto p = m_touchPoints.at(i);
        if (p->isAccepted())
            continue;
        // include points where item is the grabber
        bool isGrabber = p->exclusiveGrabber() == item;
        if (isGrabber)
            anyGrabber = true;
        // include points inside the bounds if no other item is the grabber or if the item is filtering
        bool isInside = item->contains(item->mapFromScene(p->scenePosition()));
        bool hasAnotherGrabber = p->exclusiveGrabber() && p->exclusiveGrabber() != item;

        // filtering: (childMouseEventFilter) include points that are grabbed by children of the target item
        bool grabberIsChild = false;
        auto parent = p->grabberItem();
        while (isFiltering && parent) {
            if (parent == item) {
                grabberIsChild = true;
                break;
            }
            parent = parent->parentItem();
        }

        bool filterRelevant = isFiltering && grabberIsChild;
        if (!(isGrabber || (isInside && (!hasAnotherGrabber || isFiltering)) || filterRelevant))
            continue;
        if ((p->state() == QQuickEventPoint::Pressed || p->state() == QQuickEventPoint::Released) && isInside)
            anyPressOrReleaseInside = true;
        const QTouchEvent::TouchPoint *tp = touchPointById(p->pointId());
        if (tp) {
            if (isInside && tp->d->stationaryWithModifiedProperty)
                anyStationaryWithModifiedPropertyInside = true;
            eventStates |= tp->state();
            QTouchEvent::TouchPoint tpCopy = *tp;
            tpCopy.setPos(item->mapFromScene(tpCopy.scenePos()));
            tpCopy.setLastPos(item->mapFromScene(tpCopy.lastScenePos()));
            tpCopy.setStartPos(item->mapFromScene(tpCopy.startScenePos()));
            tpCopy.setRect(item->mapRectFromScene(tpCopy.sceneRect()));
            tpCopy.setVelocity(transformMatrix.mapVector(tpCopy.velocity()).toVector2D());
            touchPoints << tpCopy;
        }
    }

    // Now touchPoints will have only points which are inside the item.
    // But if none of them were just pressed inside, and the item has no other reason to care, ignore them anyway.
    if ((eventStates == Qt::TouchPointStationary && !anyStationaryWithModifiedPropertyInside) ||
            touchPoints.isEmpty() || (!anyPressOrReleaseInside && !anyGrabber && !isFiltering))
        return nullptr;

    // if all points have the same state, set the event type accordingly
    const QTouchEvent &event = *asTouchEvent();
    QEvent::Type eventType = event.type();
    switch (eventStates) {
    case Qt::TouchPointPressed:
        eventType = QEvent::TouchBegin;
        break;
    case Qt::TouchPointReleased:
        eventType = QEvent::TouchEnd;
        break;
    default:
        eventType = QEvent::TouchUpdate;
        break;
    }

    QTouchEvent *touchEvent = new QTouchEvent(eventType);
    touchEvent->setWindow(event.window());
    touchEvent->setTarget(item);
    touchEvent->setDevice(event.device());
    touchEvent->setModifiers(event.modifiers());
    touchEvent->setTouchPoints(touchPoints);
    touchEvent->setTouchPointStates(eventStates);
    touchEvent->setTimestamp(event.timestamp());
    touchEvent->accept();
    return touchEvent;
}

QTouchEvent *QQuickPointerTouchEvent::asTouchEvent() const
{
    return static_cast<QTouchEvent *>(m_event);
}

#ifndef QT_NO_DEBUG_STREAM

Q_QUICK_PRIVATE_EXPORT QDebug operator<<(QDebug dbg, const QQuickPointerDevice *dev)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    if (!dev) {
        dbg << "QQuickPointerDevice(0)";
        return dbg;
    }
    dbg << "QQuickPointerDevice("<< dev->name() << ' ';
    QtDebugUtils::formatQEnum(dbg, dev->type());
    dbg << ' ';
    QtDebugUtils::formatQEnum(dbg, dev->pointerType());
    dbg << " caps:";
    QtDebugUtils::formatQFlags(dbg, dev->capabilities());
    if (dev->type() == QQuickPointerDevice::TouchScreen ||
            dev->type() == QQuickPointerDevice::TouchPad)
        dbg << " maxTouchPoints:" << dev->maximumTouchPoints();
    else
        dbg << " buttonCount:" << dev->buttonCount();
    dbg << ')';
    return dbg;
}

Q_QUICK_PRIVATE_EXPORT QDebug operator<<(QDebug dbg, const QQuickPointerEvent *event)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "QQuickPointerEvent(";
    dbg << event->timestamp();
    dbg << " dev:";
    QtDebugUtils::formatQEnum(dbg, event->device()->type());
    if (event->buttons() != Qt::NoButton) {
        dbg << " buttons:";
        QtDebugUtils::formatQEnum(dbg, event->buttons());
    }
    dbg << " [";
    int c = event->pointCount();
    for (int i = 0; i < c; ++i)
        dbg << event->point(i) << ' ';
    dbg << "])";
    return dbg;
}

Q_QUICK_PRIVATE_EXPORT QDebug operator<<(QDebug dbg, const QQuickEventPoint *event)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "QQuickEventPoint(accepted:" << event->isAccepted()
        << " state:";
    QtDebugUtils::formatQEnum(dbg, event->state());
    dbg << " scenePos:" << event->scenePosition() << " id:" << hex << event->pointId() << dec
        << " timeHeld:" << event->timeHeld() << ')';
    return dbg;
}

#endif

QT_END_NAMESPACE
