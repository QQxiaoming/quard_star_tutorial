/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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

#include "qwaylandseat.h"
#include "qwaylandseat_p.h"

#include "qwaylandcompositor.h"
#include "qwaylandinputmethodcontrol.h"
#include "qwaylandview.h"
#if QT_CONFIG(draganddrop)
#include <QtWaylandCompositor/QWaylandDrag>
#endif
#include <QtWaylandCompositor/QWaylandTouch>
#include <QtWaylandCompositor/QWaylandPointer>
#include <QtWaylandCompositor/QWaylandKeymap>
#include <QtWaylandCompositor/private/qwaylandseat_p.h>
#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#if QT_CONFIG(wayland_datadevice)
#include <QtWaylandCompositor/private/qwldatadevice_p.h>
#endif

#include "extensions/qwlqtkey_p.h"
#include "extensions/qwaylandtextinput.h"

QT_BEGIN_NAMESPACE

QWaylandSeatPrivate::QWaylandSeatPrivate(QWaylandSeat *seat) :
#if QT_CONFIG(wayland_datadevice)
    drag_handle(new QWaylandDrag(seat)),
#endif
    keymap(new QWaylandKeymap())
{
}

QWaylandSeatPrivate::~QWaylandSeatPrivate()
{
}

void QWaylandSeatPrivate::setCapabilities(QWaylandSeat::CapabilityFlags caps)
{
    Q_Q(QWaylandSeat);
    if (capabilities != caps) {
        QWaylandSeat::CapabilityFlags changed = caps ^ capabilities;

        if (changed & QWaylandSeat::Pointer) {
            pointer.reset(pointer.isNull() ? QWaylandCompositorPrivate::get(compositor)->callCreatePointerDevice(q) : nullptr);
        }

        if (changed & QWaylandSeat::Keyboard) {
            keyboard.reset(keyboard.isNull() ? QWaylandCompositorPrivate::get(compositor)->callCreateKeyboardDevice(q) : nullptr);
        }

        if (changed & QWaylandSeat::Touch) {
            touch.reset(touch.isNull() ? QWaylandCompositorPrivate::get(compositor)->callCreateTouchDevice(q) : nullptr);
        }

        capabilities = caps;
        QList<Resource *> resources = resourceMap().values();
        for (int i = 0; i < resources.size(); i++) {
            wl_seat::send_capabilities(resources.at(i)->handle, (uint32_t)capabilities);
        }

        if ((changed & caps & QWaylandSeat::Keyboard) && keyboardFocus != nullptr)
            keyboard->setFocus(keyboardFocus);
    }
}

#if QT_CONFIG(wayland_datadevice)
void QWaylandSeatPrivate::clientRequestedDataDevice(QtWayland::DataDeviceManager *, struct wl_client *client, uint32_t id)
{
    Q_Q(QWaylandSeat);
    if (!data_device)
        data_device.reset(new QtWayland::DataDevice(q));
    data_device->add(client, id, 1);
}
#endif

void QWaylandSeatPrivate::seat_destroy_resource(wl_seat::Resource *)
{
//    cleanupDataDeviceForClient(resource->client(), true);
}

void QWaylandSeatPrivate::seat_bind_resource(wl_seat::Resource *resource)
{
    // The order of capabilities matches the order defined in the wayland protocol
    wl_seat::send_capabilities(resource->handle, (uint32_t)capabilities);
}

void QWaylandSeatPrivate::seat_get_pointer(wl_seat::Resource *resource, uint32_t id)
{
    if (!pointer.isNull()) {
        pointer->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
    }
}

void QWaylandSeatPrivate::seat_get_keyboard(wl_seat::Resource *resource, uint32_t id)
{
    if (!keyboard.isNull()) {
        keyboard->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
    }
}

void QWaylandSeatPrivate::seat_get_touch(wl_seat::Resource *resource, uint32_t id)
{
    if (!touch.isNull()) {
        touch->addClient(QWaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
    }
}

/*!
 * \qmltype WaylandSeat
 * \inqmlmodule QtWayland.Compositor
 * \since 5.8
 * \brief Provides access to keyboard, mouse, and touch input.
 *
 * The WaylandSeat type provides access to different types of user input and maintains
 * a keyboard focus and a mouse pointer. It corresponds to the wl_seat interface in the Wayland
 * protocol.
 */

/*!
 * \class QWaylandSeat
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandSeat class provides access to keyboard, mouse, and touch input.
 *
 * The QWaylandSeat provides access to different types of user input and maintains
 * a keyboard focus and a mouse pointer. It corresponds to the wl_seat interface in the Wayland protocol.
 */

/*!
 * \enum QWaylandSeat::CapabilityFlag
 *
 * This enum type describes the capabilities of a QWaylandSeat.
 *
 * \value Pointer The QWaylandSeat supports pointer input.
 * \value Keyboard The QWaylandSeat supports keyboard input.
 * \value Touch The QWaylandSeat supports touch input.
 * \value DefaultCapabilities The QWaylandSeat has the default capabilities.
 */

/*!
 * Constructs a QWaylandSeat for the given \a compositor and with the given \a capabilityFlags.
 */
QWaylandSeat::QWaylandSeat(QWaylandCompositor *compositor, CapabilityFlags capabilityFlags)
    : QWaylandObject(*new QWaylandSeatPrivate(this))
{
    Q_D(QWaylandSeat);
    d->compositor = compositor;
    d->capabilities = capabilityFlags;
    if (compositor->isCreated())
        initialize();
}

/*!
 * Destroys the QWaylandSeat
 */
QWaylandSeat::~QWaylandSeat()
{
}

void QWaylandSeat::initialize()
{
    Q_D(QWaylandSeat);
    d->init(d->compositor->display(), 4);

    if (d->capabilities & QWaylandSeat::Pointer)
        d->pointer.reset(QWaylandCompositorPrivate::get(d->compositor)->callCreatePointerDevice(this));
    if (d->capabilities & QWaylandSeat::Touch)
        d->touch.reset(QWaylandCompositorPrivate::get(d->compositor)->callCreateTouchDevice(this));
    if (d->capabilities & QWaylandSeat::Keyboard)
        d->keyboard.reset(QWaylandCompositorPrivate::get(d->compositor)->callCreateKeyboardDevice(this));

    d->isInitialized = true;
}

bool QWaylandSeat::isInitialized() const
{
    Q_D(const QWaylandSeat);
    return d->isInitialized;
}

/*!
 * Sends a mouse press event for \a button to the QWaylandSeat's pointer device.
 */
void QWaylandSeat::sendMousePressEvent(Qt::MouseButton button)
{
    Q_D(QWaylandSeat);
    d->pointer->sendMousePressEvent(button);
}

/*!
 * Sends a mouse release event for \a button to the QWaylandSeat's pointer device.
 */
void QWaylandSeat::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_D(QWaylandSeat);
    d->pointer->sendMouseReleaseEvent(button);
}

/*!
 * Sets the mouse focus to \a view and sends a mouse move event to the pointer device with the
 * local position \a localPos and output space position \a outputSpacePos.
 **/
void QWaylandSeat::sendMouseMoveEvent(QWaylandView *view, const QPointF &localPos, const QPointF &outputSpacePos)
{
    Q_D(QWaylandSeat);
    d->pointer->sendMouseMoveEvent(view, localPos, outputSpacePos);
}

/*!
 * Sends a mouse wheel event to the QWaylandSeat's pointer device with the given \a orientation and \a delta.
 */
void QWaylandSeat::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    Q_D(QWaylandSeat);
    d->pointer->sendMouseWheelEvent(orientation, delta);
}

/*!
 * Sends a key press event with the key \a code to the keyboard device.
 */
void QWaylandSeat::sendKeyPressEvent(uint code)
{
    Q_D(QWaylandSeat);
    d->keyboard->sendKeyPressEvent(code);
}

/*!
 * Sends a key release event with the key \a code to the keyboard device.
 */
void QWaylandSeat::sendKeyReleaseEvent(uint code)
{
    Q_D(QWaylandSeat);
    d->keyboard->sendKeyReleaseEvent(code);
}

/*!
 * Sends a touch point event to the \a surface on a touch device with the given
 * \a id, \a point and \a state.
 *
 * Returns the serial for the touch up or touch down event.
 */
uint QWaylandSeat::sendTouchPointEvent(QWaylandSurface *surface, int id, const QPointF &point, Qt::TouchPointState state)
{
    Q_D(QWaylandSeat);

    if (d->touch.isNull())
        return 0;

    return d->touch->sendTouchPointEvent(surface, id, point,state);
}

/*!
 * \qmlmethod uint QtWaylandCompositor::WaylandSeat::sendTouchPointPressed(WaylandSurface surface, int id, point position)
 *
 * Sends a touch pressed event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * Returns the serial for the touch down event.
 */

/*!
 * Sends a touch pressed event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * Returns the serial for the touch down event.
 */
uint QWaylandSeat::sendTouchPointPressed(QWaylandSurface *surface, int id, const QPointF &position)
{
    return sendTouchPointEvent(surface, id, position, Qt::TouchPointPressed);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendTouchPointReleased(WaylandSurface surface, int id, point position)
 *
 * Sends a touch released event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * Returns the serial for the touch up event.
 */

/*!
 * Sends a touch released event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * Returns the serial for the touch up event.
 */
uint QWaylandSeat::sendTouchPointReleased(QWaylandSurface *surface, int id, const QPointF &position)
{
    return sendTouchPointEvent(surface, id, position, Qt::TouchPointReleased);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendTouchPointMoved(WaylandSurface surface, int id, point position)
 *
 * Sends a touch moved event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * Returns the serial for the touch motion event.
 */

/*!
 * Sends a touch moved event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * Returns the serial for the touch motion event.
 */
uint QWaylandSeat::sendTouchPointMoved(QWaylandSurface *surface, int id, const QPointF &position)
{
    return sendTouchPointEvent(surface, id, position, Qt::TouchPointMoved);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendTouchFrameEvent(WaylandClient client)
 *
 * Sends a frame event to the touch device of a \a client to indicate the end
 * of a series of touch up, down, and motion events.
 */

/*!
 * Sends a frame event to the touch device of a \a client to indicate the end
 * of a series of touch up, down, and motion events.
 */
void QWaylandSeat::sendTouchFrameEvent(QWaylandClient *client)
{
    Q_D(QWaylandSeat);
    if (!d->touch.isNull())
        d->touch->sendFrameEvent(client);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendTouchCancelEvent(WaylandClient client)
 *
 * Sends a cancel event to the touch device of a \a client.
 */

/*!
 * Sends a cancel event to the touch device of a \a client.
 */
void QWaylandSeat::sendTouchCancelEvent(QWaylandClient *client)
{
    Q_D(QWaylandSeat);
    if (!d->touch.isNull())
        d->touch->sendCancelEvent(client);
}

/*!
 * Sends the \a event to the specified \a surface on the touch device.
 */
void QWaylandSeat::sendFullTouchEvent(QWaylandSurface *surface, QTouchEvent *event)
{
    Q_D(QWaylandSeat);

    if (!d->touch)
        return;

    d->touch->sendFullTouchEvent(surface, event);
}

/*!
 * Sends the \a event to the keyboard device.
 */
void QWaylandSeat::sendFullKeyEvent(QKeyEvent *event)
{
    Q_D(QWaylandSeat);

    if (!keyboardFocus()) {
        qWarning("Cannot send key event, no keyboard focus, fix the compositor");
        return;
    }

#if QT_CONFIG(im)
    if (keyboardFocus()->inputMethodControl()->enabled()
        && event->nativeScanCode() == 0) {
        QWaylandTextInput *textInput = QWaylandTextInput::findIn(this);
        if (textInput) {
            textInput->sendKeyEvent(event);
            return;
        }
    }
#endif

    QtWayland::QtKeyExtensionGlobal *ext = QtWayland::QtKeyExtensionGlobal::findIn(d->compositor);
    if (ext && ext->postQtKeyEvent(event, keyboardFocus()))
        return;

    if (!d->keyboard.isNull() && !event->isAutoRepeat()) {

        uint scanCode = event->nativeScanCode();
        if (scanCode == 0)
            scanCode = d->keyboard->keyToScanCode(event->key());

        if (scanCode == 0) {
            qWarning() << "Can't send Wayland key event: Unable to get a valid scan code";
            return;
        }

        if (event->type() == QEvent::KeyPress)
            d->keyboard->sendKeyPressEvent(scanCode);
        else if (event->type() == QEvent::KeyRelease)
            d->keyboard->sendKeyReleaseEvent(scanCode);
    }
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendKeyEvent(int qtKey, bool pressed)
 * \since 5.12
 *
 * Sends a key press or release to the keyboard device.
 */

/*!
 * Sends a key press or release to the keyboard device.
 *
 * \since 5.12
 */
void QWaylandSeat::sendKeyEvent(int qtKey, bool pressed)
{
    Q_D(QWaylandSeat);
    if (!keyboardFocus()) {
        qWarning("Cannot send Wayland key event, no keyboard focus, fix the compositor");
        return;
    }

    if (auto scanCode = d->keyboard->keyToScanCode(qtKey)) {
        if (pressed)
            d->keyboard->sendKeyPressEvent(scanCode);
        else
            d->keyboard->sendKeyReleaseEvent(scanCode);
    } else {
        qWarning() << "Can't send Wayland key event: Unable to get scan code for" << Qt::Key(qtKey);
    }
}

/*!
 * Returns the keyboard for this input device.
 */
QWaylandKeyboard *QWaylandSeat::keyboard() const
{
    Q_D(const QWaylandSeat);
    return d->keyboard.data();
}

/*!
 * Returns the current focused surface for keyboard input.
 */
QWaylandSurface *QWaylandSeat::keyboardFocus() const
{
    Q_D(const QWaylandSeat);
    if (d->keyboard.isNull() || !d->keyboard->focus())
        return nullptr;

    return d->keyboard->focus();
}

/*!
 * Sets the current keyboard focus to \a surface.
 * Returns a boolean indicating if the operation
 * was successful.
 */
bool QWaylandSeat::setKeyboardFocus(QWaylandSurface *surface)
{
    Q_D(QWaylandSeat);
    if (surface && surface->isDestroyed())
        return false;

    QWaylandSurface *oldSurface = keyboardFocus();
    if (surface == oldSurface)
        return true;

    d->keyboardFocus = surface;
    if (!d->keyboard.isNull())
        d->keyboard->setFocus(surface);
#if QT_CONFIG(wayland_datadevice)
    if (d->data_device)
        d->data_device->setFocus(surface ? surface->client() : nullptr);
#endif
    emit keyboardFocusChanged(surface, oldSurface);
    return true;
}

QWaylandKeymap *QWaylandSeat::keymap()
{
    Q_D(const QWaylandSeat);
    return d->keymap.data();
}

/*!
 * Returns the pointer device for this QWaylandSeat.
 */
QWaylandPointer *QWaylandSeat::pointer() const
{
    Q_D(const QWaylandSeat);
    return d->pointer.data();
}

/*!
 * Returns the touch device for this QWaylandSeat.
 */
QWaylandTouch *QWaylandSeat::touch() const
{
    Q_D(const QWaylandSeat);
    return d->touch.data();
}

/*!
 * Returns the view that currently has mouse focus.
 */
QWaylandView *QWaylandSeat::mouseFocus() const
{
    Q_D(const QWaylandSeat);
    return d->mouseFocus;
}

/*!
 * Sets the current mouse focus to \a view.
 */
void QWaylandSeat::setMouseFocus(QWaylandView *view)
{
    Q_D(QWaylandSeat);
    if (view == d->mouseFocus)
        return;

    QWaylandView *oldFocus = d->mouseFocus;
    d->mouseFocus = view;

    if (oldFocus)
        disconnect(oldFocus, &QObject::destroyed, this, &QWaylandSeat::handleMouseFocusDestroyed);
    if (d->mouseFocus)
        connect(d->mouseFocus, &QObject::destroyed, this, &QWaylandSeat::handleMouseFocusDestroyed);

    emit mouseFocusChanged(d->mouseFocus, oldFocus);
}

/*!
 * Returns the compositor for this QWaylandSeat.
 */
QWaylandCompositor *QWaylandSeat::compositor() const
{
    Q_D(const QWaylandSeat);
    return d->compositor;
}

/*!
 * Returns the drag object for this QWaylandSeat.
 */
#if QT_CONFIG(draganddrop)
QWaylandDrag *QWaylandSeat::drag() const
{
    Q_D(const QWaylandSeat);
    return d->drag_handle.data();
}
#endif

/*!
 * Returns the capability flags for this QWaylandSeat.
 */
QWaylandSeat::CapabilityFlags QWaylandSeat::capabilities() const
{
    Q_D(const QWaylandSeat);
    return d->capabilities;
}

/*!
 * \internal
 */
bool QWaylandSeat::isOwner(QInputEvent *inputEvent) const
{
    Q_UNUSED(inputEvent);
    return true;
}

/*!
 * Returns the QWaylandSeat corresponding to the \a resource. The \a resource is expected
 * to have the type wl_seat.
 */
QWaylandSeat *QWaylandSeat::fromSeatResource(struct ::wl_resource *resource)
{
    if (auto *r = QWaylandSeatPrivate::Resource::fromResource(resource))
        return static_cast<QWaylandSeatPrivate *>(r->seat_object)->q_func();
    return nullptr;
}

/*!
 * \fn void QWaylandSeat::mouseFocusChanged(QWaylandView *newFocus, QWaylandView *oldFocus)
 *
 * This signal is emitted when the mouse focus has changed from \a oldFocus to \a newFocus.
 */

void QWaylandSeat::handleMouseFocusDestroyed()
{
    // This is triggered when the QWaylandView is destroyed, NOT the surface.
    // ... so this is for the rare case when the view that currently holds the mouse focus is
    // destroyed before its surface
    Q_D(QWaylandSeat);
    d->mouseFocus = nullptr;
    QWaylandView *oldFocus = nullptr; // we have to send nullptr because the old focus is already destroyed at this point
    emit mouseFocusChanged(d->mouseFocus, oldFocus);
}

QT_END_NAMESPACE
