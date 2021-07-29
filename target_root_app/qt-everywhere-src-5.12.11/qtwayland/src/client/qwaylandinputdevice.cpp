/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qwaylandinputdevice_p.h"

#include "qwaylandintegration_p.h"
#include "qwaylandwindow_p.h"
#include "qwaylandbuffer_p.h"
#if QT_CONFIG(wayland_datadevice)
#include "qwaylanddatadevice_p.h"
#include "qwaylanddatadevicemanager_p.h"
#endif
#include "qwaylandtouch_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylandcursor_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandshmbackingstore_p.h"
#include "../shared/qwaylandxkb_p.h"
#include "qwaylandinputcontext_p.h"

#include <QtGui/private/qpixmap_raster_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatforminputcontext.h>
#include <QDebug>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#if QT_CONFIG(cursor)
#include <wayland-cursor.h>
#endif

#include <QtGui/QGuiApplication>

#if QT_CONFIG(xkbcommon)
#include <xkbcommon/xkbcommon-compose.h>
#endif

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandInputDevice::Keyboard::Keyboard(QWaylandInputDevice *p)
    : mParent(p)
{
    connect(&mRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatKey()));
}

#if QT_CONFIG(xkbcommon)
bool QWaylandInputDevice::Keyboard::createDefaultKeyMap()
{
    if (mXkbContext && mXkbMap && mXkbState) {
        return true;
    }

    xkb_rule_names names;
    names.rules = strdup("evdev");
    names.model = strdup("pc105");
    names.layout = strdup("us");
    names.variant = strdup("");
    names.options = strdup("");

    mXkbContext = xkb_context_new(xkb_context_flags(0));
    if (mXkbContext) {
        mXkbMap = xkb_map_new_from_names(mXkbContext, &names, xkb_map_compile_flags(0));
        if (mXkbMap) {
            mXkbState = xkb_state_new(mXkbMap);
        }
    }

    if (!mXkbContext || !mXkbMap || !mXkbState) {
        qWarning() << "xkb_map_new_from_names failed, no key input";
        return false;
    }
    createComposeState();
    return true;
}

void QWaylandInputDevice::Keyboard::releaseKeyMap()
{
    if (mXkbState)
        xkb_state_unref(mXkbState);
    if (mXkbMap)
        xkb_map_unref(mXkbMap);
    if (mXkbContext)
        xkb_context_unref(mXkbContext);
}

void QWaylandInputDevice::Keyboard::createComposeState()
{
    static const char *locale = nullptr;
    if (!locale) {
        locale = getenv("LC_ALL");
        if (!locale)
            locale = getenv("LC_CTYPE");
        if (!locale)
            locale = getenv("LANG");
        if (!locale)
            locale = "C";
    }

    mXkbComposeTable = xkb_compose_table_new_from_locale(mXkbContext, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (mXkbComposeTable)
        mXkbComposeState = xkb_compose_state_new(mXkbComposeTable, XKB_COMPOSE_STATE_NO_FLAGS);
}

void QWaylandInputDevice::Keyboard::releaseComposeState()
{
    if (mXkbComposeState)
        xkb_compose_state_unref(mXkbComposeState);
    if (mXkbComposeTable)
        xkb_compose_table_unref(mXkbComposeTable);
    mXkbComposeState = nullptr;
    mXkbComposeTable = nullptr;
}

#endif

QWaylandInputDevice::Keyboard::~Keyboard()
{
#if QT_CONFIG(xkbcommon)
    releaseComposeState();
    releaseKeyMap();
#endif
    if (mFocus)
        QWindowSystemInterface::handleWindowActivated(nullptr);
    if (mParent->mVersion >= 3)
        wl_keyboard_release(object());
    else
        wl_keyboard_destroy(object());
}

void QWaylandInputDevice::Keyboard::stopRepeat()
{
    mRepeatTimer.stop();
}

QWaylandInputDevice::Pointer::Pointer(QWaylandInputDevice *p)
    : mParent(p)
{
}

QWaylandInputDevice::Pointer::~Pointer()
{
    if (mParent->mVersion >= 3)
        wl_pointer_release(object());
    else
        wl_pointer_destroy(object());
}

QWaylandInputDevice::Touch::Touch(QWaylandInputDevice *p)
    : mParent(p)
{
}

QWaylandInputDevice::Touch::~Touch()
{
    if (mParent->mVersion >= 3)
        wl_touch_release(object());
    else
        wl_touch_destroy(object());
}

QWaylandInputDevice::QWaylandInputDevice(QWaylandDisplay *display, int version, uint32_t id)
    : QtWayland::wl_seat(display->wl_registry(), id, qMin(version, 4))
    , mQDisplay(display)
    , mDisplay(display->wl_display())
    , mVersion(qMin(version, 4))
{
#if QT_CONFIG(wayland_datadevice)
    if (mQDisplay->dndSelectionHandler()) {
        mDataDevice = mQDisplay->dndSelectionHandler()->getDataDevice(this);
    }
#endif

    if (mQDisplay->textInputManager()) {
        mTextInput = new QWaylandTextInput(mQDisplay, mQDisplay->textInputManager()->get_text_input(wl_seat()));
    }
}

QWaylandInputDevice::~QWaylandInputDevice()
{
    delete mPointer;
    delete mKeyboard;
    delete mTouch;
}

void QWaylandInputDevice::seat_capabilities(uint32_t caps)
{
    mCaps = caps;

    if (caps & WL_SEAT_CAPABILITY_KEYBOARD && !mKeyboard) {
        mKeyboard = createKeyboard(this);
        mKeyboard->init(get_keyboard());
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && mKeyboard) {
        delete mKeyboard;
        mKeyboard = nullptr;
    }

    if (caps & WL_SEAT_CAPABILITY_POINTER && !mPointer) {
        mPointer = createPointer(this);
        mPointer->init(get_pointer());
        pointerSurface = mQDisplay->createSurface(this);
    } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && mPointer) {
        delete mPointer;
        mPointer = nullptr;
    }

    if (caps & WL_SEAT_CAPABILITY_TOUCH && !mTouch) {
        mTouch = createTouch(this);
        mTouch->init(get_touch());

        if (!mTouchDevice) {
            mTouchDevice = new QTouchDevice;
            mTouchDevice->setType(QTouchDevice::TouchScreen);
            mTouchDevice->setCapabilities(QTouchDevice::Position);
            QWindowSystemInterface::registerTouchDevice(mTouchDevice);
        }
    } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && mTouch) {
        delete mTouch;
        mTouch = nullptr;
    }
}

QWaylandInputDevice::Keyboard *QWaylandInputDevice::createKeyboard(QWaylandInputDevice *device)
{
    return new Keyboard(device);
}

QWaylandInputDevice::Pointer *QWaylandInputDevice::createPointer(QWaylandInputDevice *device)
{
    return new Pointer(device);
}

QWaylandInputDevice::Touch *QWaylandInputDevice::createTouch(QWaylandInputDevice *device)
{
    return new Touch(device);
}

void QWaylandInputDevice::handleWindowDestroyed(QWaylandWindow *window)
{
    if (mKeyboard && window == mKeyboard->mFocus)
        mKeyboard->stopRepeat();
}

void QWaylandInputDevice::handleEndDrag()
{
    if (mTouch)
        mTouch->releasePoints();
    if (mPointer)
        mPointer->releaseButtons();
}

#if QT_CONFIG(wayland_datadevice)
void QWaylandInputDevice::setDataDevice(QWaylandDataDevice *device)
{
    mDataDevice = device;
}

QWaylandDataDevice *QWaylandInputDevice::dataDevice() const
{
    return mDataDevice;
}
#endif

void QWaylandInputDevice::setTextInput(QWaylandTextInput *textInput)
{
    mTextInput = textInput;
}

QWaylandTextInput *QWaylandInputDevice::textInput() const
{
    return mTextInput;
}

void QWaylandInputDevice::removeMouseButtonFromState(Qt::MouseButton button)
{
    if (mPointer)
        mPointer->mButtons = mPointer->mButtons & !button;
}

QWaylandWindow *QWaylandInputDevice::pointerFocus() const
{
    return mPointer ? mPointer->mFocus : nullptr;
}

QWaylandWindow *QWaylandInputDevice::keyboardFocus() const
{
    return mKeyboard ? mKeyboard->mFocus : nullptr;
}

QWaylandWindow *QWaylandInputDevice::touchFocus() const
{
    return mTouch ? mTouch->mFocus : nullptr;
}

Qt::KeyboardModifiers QWaylandInputDevice::modifiers() const
{
    if (!mKeyboard)
        return Qt::NoModifier;

    return mKeyboard->modifiers();
}

Qt::KeyboardModifiers QWaylandInputDevice::Keyboard::modifiers() const
{
    Qt::KeyboardModifiers ret = Qt::NoModifier;

#if QT_CONFIG(xkbcommon)
    if (!mXkbState)
        return ret;

    ret = QWaylandXkb::modifiers(mXkbState);
#endif

    return ret;
}

#if QT_CONFIG(cursor)
uint32_t QWaylandInputDevice::cursorSerial() const
{
    if (mPointer)
        return mPointer->mCursorSerial;
    return 0;
}

void QWaylandInputDevice::setCursor(Qt::CursorShape newShape, QWaylandScreen *screen)
{
    struct wl_cursor_image *image = screen->waylandCursor()->cursorImage(newShape);
    if (!image) {
        return;
    }

    struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    setCursor(buffer, image, screen->devicePixelRatio());
}

void QWaylandInputDevice::setCursor(const QCursor &cursor, QWaylandScreen *screen)
{
    if (mPointer->mCursorSerial >= mPointer->mEnterSerial && (cursor.shape() != Qt::BitmapCursor && cursor.shape() == mPointer->mCursorShape))
        return;

    mPointer->mCursorShape = cursor.shape();
    if (cursor.shape() == Qt::BitmapCursor) {
        setCursor(screen->waylandCursor()->cursorBitmapImage(&cursor), cursor.hotSpot(), screen->devicePixelRatio());
        return;
    }
    setCursor(cursor.shape(), screen);
}

void QWaylandInputDevice::setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image, int bufferScale)
{
    if (image) {
        // Convert from pixel coordinates to surface coordinates
        QPoint hotspot = QPoint(image->hotspot_x, image->hotspot_y) / bufferScale;
        QSize size = QSize(image->width, image->height) / bufferScale;
        setCursor(buffer, hotspot, size, bufferScale);
    } else {
        setCursor(buffer, QPoint(), QSize(), bufferScale);
    }
}

// size and hotspot are in surface coordinates
void QWaylandInputDevice::setCursor(struct wl_buffer *buffer, const QPoint &hotSpot, const QSize &size, int bufferScale)
{
    if (mCaps & WL_SEAT_CAPABILITY_POINTER) {
        bool force = mPointer->mEnterSerial > mPointer->mCursorSerial;

        if (!force && mPointer->mCursorBuffer == buffer)
            return;

        mPixmapCursor.clear();
        mPointer->mCursorSerial = mPointer->mEnterSerial;

        mPointer->mCursorBuffer = buffer;

        /* Hide cursor */
        if (!buffer)
        {
            mPointer->set_cursor(mPointer->mEnterSerial, nullptr, 0, 0);
            return;
        }

        mPointer->set_cursor(mPointer->mEnterSerial, pointerSurface,
                             hotSpot.x(), hotSpot.y());
        wl_surface_attach(pointerSurface, buffer, 0, 0);
        if (mQDisplay->compositorVersion() >= 3)
            wl_surface_set_buffer_scale(pointerSurface, bufferScale);
        wl_surface_damage(pointerSurface, 0, 0, size.width(), size.height());
        wl_surface_commit(pointerSurface);
    }
}

void QWaylandInputDevice::setCursor(const QSharedPointer<QWaylandBuffer> &buffer, const QPoint &hotSpot, int bufferScale)
{
    setCursor(buffer->buffer(), hotSpot, buffer->size(), bufferScale);
    mPixmapCursor = buffer;
}
#endif

class EnterEvent : public QWaylandPointerEvent
{
public:
    EnterEvent(const QPointF &l, const QPointF &g)
        : QWaylandPointerEvent(QWaylandPointerEvent::Enter, 0, l, g, nullptr, Qt::NoModifier)
    {}
};

void QWaylandInputDevice::Pointer::pointer_enter(uint32_t serial, struct wl_surface *surface,
                                                 wl_fixed_t sx, wl_fixed_t sy)
{
    if (!surface)
        return;

    QWaylandWindow *window = QWaylandWindow::fromWlSurface(surface);
    if (!window)
        return; // Ignore foreign surfaces

    mFocus = window;
    mSurfacePos = QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
    mGlobalPos = window->window()->mapToGlobal(mSurfacePos.toPoint());

    mParent->mSerial = serial;
    mEnterSerial = serial;

#if QT_CONFIG(cursor)
    // Depends on mEnterSerial being updated
    window->window()->setCursor(window->window()->cursor());
#endif

    QWaylandWindow *grab = QWaylandWindow::mouseGrab();
    if (!grab) {
        EnterEvent evt(mSurfacePos, mGlobalPos);
        window->handleMouse(mParent, evt);
    }
}

void QWaylandInputDevice::Pointer::pointer_leave(uint32_t time, struct wl_surface *surface)
{
    // The event may arrive after destroying the window, indicated by
    // a null surface.
    if (!surface)
        return;

    auto *window = QWaylandWindow::fromWlSurface(surface);
    if (!window)
        return; // Ignore foreign surfaces

    if (!QWaylandWindow::mouseGrab()) {
        QWaylandWindow *window = QWaylandWindow::fromWlSurface(surface);
        window->handleMouseLeave(mParent);
    }
    mFocus = nullptr;
    mButtons = Qt::NoButton;

    mParent->mTime = time;
}

class MotionEvent : public QWaylandPointerEvent
{
public:
    MotionEvent(ulong t, const QPointF &l, const QPointF &g, Qt::MouseButtons b, Qt::KeyboardModifiers m)
        : QWaylandPointerEvent(QWaylandPointerEvent::Motion, t, l, g, b, m)
    {
    }
};

void QWaylandInputDevice::Pointer::pointer_motion(uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    QWaylandWindow *window = mFocus;
    if (!window) {
        // We destroyed the pointer focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first. In either case, ignore the event.
        return;
    }

    QPointF pos(wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
    QPointF delta = pos - pos.toPoint();
    QPointF global = window->window()->mapToGlobal(pos.toPoint());
    global += delta;

    mSurfacePos = pos;
    mGlobalPos = global;
    mParent->mTime = time;

    QWaylandWindow *grab = QWaylandWindow::mouseGrab();
    if (grab && grab != window) {
        // We can't know the true position since we're getting events for another surface,
        // so we just set it outside of the window boundaries.
        pos = QPointF(-1, -1);
        global = grab->window()->mapToGlobal(pos.toPoint());
        MotionEvent e(time, pos, global, mButtons, mParent->modifiers());
        grab->handleMouse(mParent, e);
    } else {
        MotionEvent e(time, mSurfacePos, mGlobalPos, mButtons, mParent->modifiers());
        window->handleMouse(mParent, e);
    }
}

void QWaylandInputDevice::Pointer::pointer_button(uint32_t serial, uint32_t time,
                                                  uint32_t button, uint32_t state)
{
    QWaylandWindow *window = mFocus;
    if (!window) {
        // We destroyed the pointer focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first. In either case, ignore the event.
        return;
    }

    Qt::MouseButton qt_button;

    // translate from kernel (input.h) 'button' to corresponding Qt:MouseButton.
    // The range of mouse values is 0x110 <= mouse_button < 0x120, the first Joystick button.
    switch (button) {
    case 0x110: qt_button = Qt::LeftButton; break;    // kernel BTN_LEFT
    case 0x111: qt_button = Qt::RightButton; break;
    case 0x112: qt_button = Qt::MiddleButton; break;
    case 0x113: qt_button = Qt::ExtraButton1; break;  // AKA Qt::BackButton
    case 0x114: qt_button = Qt::ExtraButton2; break;  // AKA Qt::ForwardButton
    case 0x115: qt_button = Qt::ExtraButton3; break;  // AKA Qt::TaskButton
    case 0x116: qt_button = Qt::ExtraButton4; break;
    case 0x117: qt_button = Qt::ExtraButton5; break;
    case 0x118: qt_button = Qt::ExtraButton6; break;
    case 0x119: qt_button = Qt::ExtraButton7; break;
    case 0x11a: qt_button = Qt::ExtraButton8; break;
    case 0x11b: qt_button = Qt::ExtraButton9; break;
    case 0x11c: qt_button = Qt::ExtraButton10; break;
    case 0x11d: qt_button = Qt::ExtraButton11; break;
    case 0x11e: qt_button = Qt::ExtraButton12; break;
    case 0x11f: qt_button = Qt::ExtraButton13; break;
    default: return; // invalid button number (as far as Qt is concerned)
    }

    if (state)
        mButtons |= qt_button;
    else
        mButtons &= ~qt_button;

    mParent->mTime = time;
    mParent->mSerial = serial;
    if (state)
        mParent->mQDisplay->setLastInputDevice(mParent, serial, window);

    QWaylandWindow *grab = QWaylandWindow::mouseGrab();
    if (grab && grab != mFocus) {
        QPointF pos = QPointF(-1, -1);
        QPointF global = grab->window()->mapToGlobal(pos.toPoint());
        MotionEvent e(time, pos, global, mButtons, mParent->modifiers());
        grab->handleMouse(mParent, e);
    } else if (window) {
        MotionEvent e(time, mSurfacePos, mGlobalPos, mButtons, mParent->modifiers());
        window->handleMouse(mParent, e);
    }
}

void QWaylandInputDevice::Pointer::releaseButtons()
{
    mButtons = Qt::NoButton;
    MotionEvent e(mParent->mTime, mSurfacePos, mGlobalPos, mButtons, mParent->modifiers());
    if (mFocus)
        mFocus->handleMouse(mParent, e);
}

class WheelEvent : public QWaylandPointerEvent
{
public:
    WheelEvent(ulong t, const QPointF &l, const QPointF &g, const QPoint &pd, const QPoint &ad, Qt::KeyboardModifiers m)
        : QWaylandPointerEvent(QWaylandPointerEvent::Wheel, t, l, g, pd, ad, m)
    {
    }
};

void QWaylandInputDevice::Pointer::pointer_axis(uint32_t time, uint32_t axis, int32_t value)
{
    QWaylandWindow *window = mFocus;
    if (!window) {
        // We destroyed the pointer focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first. In either case, ignore the event.
        return;
    }

    QPoint pixelDelta;
    QPoint angleDelta;

    //normalize value and inverse axis
    int valueDelta = wl_fixed_to_int(value) * -12;

    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        pixelDelta = QPoint();
        angleDelta.setX(valueDelta);
    } else {
        pixelDelta = QPoint();
        angleDelta.setY(valueDelta);
    }

    WheelEvent e(time, mSurfacePos, mGlobalPos, pixelDelta, angleDelta, mParent->modifiers());
    window->handleMouse(mParent, e);
}

void QWaylandInputDevice::Keyboard::keyboard_keymap(uint32_t format, int32_t fd, uint32_t size)
{
#if QT_CONFIG(xkbcommon)
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(fd);
        return;
    }

    char *map_str = static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
    if (map_str == MAP_FAILED) {
        close(fd);
        return;
    }

    // Release the old keymap resources in the case they were already created in
    // the key event or when the compositor issues a new map
    releaseComposeState();
    releaseKeyMap();

    mXkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    mXkbMap = xkb_map_new_from_string(mXkbContext, map_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map_str, size);
    close(fd);

    mXkbState = xkb_state_new(mXkbMap);
    createComposeState();

#else
    Q_UNUSED(format);
    Q_UNUSED(fd);
    Q_UNUSED(size);
#endif
}

void QWaylandInputDevice::Keyboard::keyboard_enter(uint32_t time, struct wl_surface *surface, struct wl_array *keys)
{
    Q_UNUSED(time);
    Q_UNUSED(keys);

    if (!surface)
        return;


    QWaylandWindow *window = QWaylandWindow::fromWlSurface(surface);
    mFocus = window;

    mParent->mQDisplay->handleKeyboardFocusChanged(mParent);
}

void QWaylandInputDevice::Keyboard::keyboard_leave(uint32_t time, struct wl_surface *surface)
{
    Q_UNUSED(time);
    Q_UNUSED(surface);

    if (surface) {
        QWaylandWindow *window = QWaylandWindow::fromWlSurface(surface);
        window->unfocus();
    }

    mFocus = nullptr;

    mParent->mQDisplay->handleKeyboardFocusChanged(mParent);

    mRepeatTimer.stop();
}

static void sendKey(QWindow *tlw, ulong timestamp, QEvent::Type type, int key, Qt::KeyboardModifiers modifiers,
                    quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers,
                    const QString& text = QString(), bool autorep = false, ushort count = 1)
{
    QPlatformInputContext *inputContext = QGuiApplicationPrivate::platformIntegration()->inputContext();
    bool filtered = false;

    if (inputContext) {
        QKeyEvent event(type, key, modifiers, nativeScanCode, nativeVirtualKey, nativeModifiers,
                        text, autorep, count);
        event.setTimestamp(timestamp);
        filtered = inputContext->filterEvent(&event);
    }

    if (!filtered) {
        QWindowSystemInterface::handleExtendedKeyEvent(tlw, timestamp, type, key, modifiers,
                nativeScanCode, nativeVirtualKey, nativeModifiers, text, autorep, count);
    }
}

void QWaylandInputDevice::Keyboard::keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    QWaylandWindow *window = mFocus;
    if (!window) {
        // We destroyed the keyboard focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first. In either case, ignore the event.
        return;
    }

    uint32_t code = key + 8;
    bool isDown = state != WL_KEYBOARD_KEY_STATE_RELEASED;
    QEvent::Type type = isDown ? QEvent::KeyPress : QEvent::KeyRelease;
    QString text;
    int qtkey = key + 8;  // qt-compositor substracts 8 for some reason
    mParent->mSerial = serial;

    if (isDown)
        mParent->mQDisplay->setLastInputDevice(mParent, serial, window);

#if QT_CONFIG(xkbcommon)
    if (!createDefaultKeyMap()) {
        return;
    }

    QString composedText;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(mXkbState, code);
    if (mXkbComposeState) {
        if (isDown)
            xkb_compose_state_feed(mXkbComposeState, sym);
        xkb_compose_status status = xkb_compose_state_get_status(mXkbComposeState);

        switch (status) {
            case XKB_COMPOSE_COMPOSED: {
                int size = xkb_compose_state_get_utf8(mXkbComposeState, nullptr, 0);
                QVarLengthArray<char, 32> buffer(size + 1);
                xkb_compose_state_get_utf8(mXkbComposeState, buffer.data(), buffer.size());
                composedText = QString::fromUtf8(buffer.constData());
                sym = xkb_compose_state_get_one_sym(mXkbComposeState);
                xkb_compose_state_reset(mXkbComposeState);
            } break;
            case XKB_COMPOSE_COMPOSING:
            case XKB_COMPOSE_CANCELLED:
                return;
            case XKB_COMPOSE_NOTHING:
                break;
        }
    }

    Qt::KeyboardModifiers modifiers = mParent->modifiers();

    std::tie(qtkey, text) = QWaylandXkb::keysymToQtKey(sym, modifiers);

    if (!composedText.isNull())
        text = composedText;

    sendKey(window->window(), time, type, qtkey, modifiers, code, sym, mNativeModifiers, text);
#else
    // Generic fallback for single hard keys: Assume 'key' is a Qt key code.
    sendKey(window->window(), time, type, qtkey, Qt::NoModifier, code, 0, 0);
#endif

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED
#if QT_CONFIG(xkbcommon)
        && xkb_keymap_key_repeats(mXkbMap, code)
#endif
        ) {
        mRepeatKey = qtkey;
        mRepeatCode = code;
        mRepeatTime = time;
        mRepeatText = text;
#if QT_CONFIG(xkbcommon)
        mRepeatSym = sym;
#endif
        mRepeatTimer.setInterval(400);
        mRepeatTimer.start();
    } else if (mRepeatCode == code) {
        mRepeatTimer.stop();
    }
}

void QWaylandInputDevice::Keyboard::repeatKey()
{
    if (!mFocus) {
        // We destroyed the keyboard focus surface, but the server didn't get the message yet...
        // or the server didn't send an enter event first.
        return;
    }

    mRepeatTimer.setInterval(25);
    sendKey(mFocus->window(), mRepeatTime, QEvent::KeyRelease, mRepeatKey, modifiers(), mRepeatCode,
#if QT_CONFIG(xkbcommon)
            mRepeatSym, mNativeModifiers,
#else
            0, 0,
#endif
            mRepeatText, true);

    sendKey(mFocus->window(), mRepeatTime, QEvent::KeyPress, mRepeatKey, modifiers(), mRepeatCode,
#if QT_CONFIG(xkbcommon)
            mRepeatSym, mNativeModifiers,
#else
            0, 0,
#endif
            mRepeatText, true);
}

void QWaylandInputDevice::Keyboard::keyboard_modifiers(uint32_t serial,
                                             uint32_t mods_depressed,
                                             uint32_t mods_latched,
                                             uint32_t mods_locked,
                                             uint32_t group)
{
    Q_UNUSED(serial);
#if QT_CONFIG(xkbcommon)
    if (mXkbState)
        xkb_state_update_mask(mXkbState,
                              mods_depressed, mods_latched, mods_locked,
                              0, 0, group);
    mNativeModifiers = mods_depressed | mods_latched | mods_locked;
#else
    Q_UNUSED(serial);
    Q_UNUSED(mods_depressed);
    Q_UNUSED(mods_latched);
    Q_UNUSED(mods_locked);
    Q_UNUSED(group);
#endif
}

void QWaylandInputDevice::Touch::touch_down(uint32_t serial,
                                     uint32_t time,
                                     struct wl_surface *surface,
                                     int32_t id,
                                     wl_fixed_t x,
                                     wl_fixed_t y)
{
    if (!surface)
        return;

    auto *window = QWaylandWindow::fromWlSurface(surface);
    if (!window)
        return; // Ignore foreign surfaces

    mParent->mTime = time;
    mParent->mSerial = serial;
    mFocus = window;
    mParent->mQDisplay->setLastInputDevice(mParent, serial, mFocus);
    QPointF position(wl_fixed_to_double(x), wl_fixed_to_double(y));
    mParent->handleTouchPoint(id, Qt::TouchPointPressed, position);
}

void QWaylandInputDevice::Touch::touch_up(uint32_t serial, uint32_t time, int32_t id)
{
    Q_UNUSED(serial);
    Q_UNUSED(time);
    mParent->handleTouchPoint(id, Qt::TouchPointReleased);

    if (allTouchPointsReleased()) {
        mFocus = nullptr;

        // As of Weston 7.0.0 there is no touch_frame after the last touch_up
        // (i.e. when the last finger is released). To accommodate for this, issue a
        // touch_frame. This cannot hurt since it is safe to call the touch_frame
        // handler multiple times when there are no points left.
        // See: https://gitlab.freedesktop.org/wayland/weston/issues/44
        // TODO: change logging category to lcQpaWaylandInput in newer versions.
        qCDebug(lcQpaWayland, "Generating fake frame event to work around Weston bug");
        touch_frame();
    }
}

void QWaylandInputDevice::Touch::touch_motion(uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(time);
    QPointF position(wl_fixed_to_double(x), wl_fixed_to_double(y));
    mParent->handleTouchPoint(id, Qt::TouchPointMoved, position);
}

void QWaylandInputDevice::Touch::touch_cancel()
{
    mPendingTouchPoints.clear();

    QWaylandTouchExtension *touchExt = mParent->mQDisplay->touchExtension();
    if (touchExt)
        touchExt->touchCanceled();

    QWindowSystemInterface::handleTouchCancelEvent(nullptr, mParent->mTouchDevice);
}

void QWaylandInputDevice::handleTouchPoint(int id, Qt::TouchPointState state, const QPointF &surfacePosition)
{
    auto end = mTouch->mPendingTouchPoints.end();
    auto it = std::find_if(mTouch->mPendingTouchPoints.begin(), end, [id](const QWindowSystemInterface::TouchPoint &tp){ return tp.id == id; });
    if (it == end) {
        it = mTouch->mPendingTouchPoints.insert(end, QWindowSystemInterface::TouchPoint());
        it->id = id;
    }
    QWindowSystemInterface::TouchPoint &tp = *it;

    // Only moved and pressed needs to update/set position
    if (state == Qt::TouchPointMoved || state == Qt::TouchPointPressed) {
        // We need a global (screen) position.
        QWaylandWindow *win = mTouch->mFocus;

        //is it possible that mTouchFocus is null;
        if (!win && mPointer)
            win = mPointer->mFocus;
        if (!win && mKeyboard)
            win = mKeyboard->mFocus;
        if (!win || !win->window())
            return;

        tp.area = QRectF(0, 0, 8, 8);
        QMargins margins = win->frameMargins();
        QPointF localPosition = surfacePosition - QPointF(margins.left(), margins.top());
        // TODO: This doesn't account for high dpi scaling for the delta, but at least it matches
        // what we have for mouse input.
        QPointF delta = localPosition - localPosition.toPoint();
        QPointF globalPosition = win->window()->mapToGlobal(localPosition.toPoint()) + delta;
        tp.area.moveCenter(globalPosition);
    }

    // If the touch point was pressed earlier this frame, we don't want to overwrite its state.
    if (tp.state != Qt::TouchPointPressed)
        tp.state = state;

    tp.pressure = tp.state == Qt::TouchPointReleased ? 0 : 1;
}

bool QWaylandInputDevice::Touch::allTouchPointsReleased()
{
    for (const auto &tp : qAsConst(mPendingTouchPoints)) {
        if (tp.state != Qt::TouchPointReleased)
            return false;
    }
    return true;
}

void QWaylandInputDevice::Touch::releasePoints()
{
    if (mPendingTouchPoints.empty())
        return;

    for (QWindowSystemInterface::TouchPoint &tp : mPendingTouchPoints)
        tp.state = Qt::TouchPointReleased;

    touch_frame();
}

void QWaylandInputDevice::Touch::touch_frame()
{
    // TODO: early return if no events?

    QWindow *window = mFocus ? mFocus->window() : nullptr;

    if (mFocus) {
        const QWindowSystemInterface::TouchPoint &tp = mPendingTouchPoints.last();
        // When the touch event is received, the global pos is calculated with the margins
        // in mind. Now we need to adjust again to get the correct local pos back.
        QMargins margins = window->frameMargins();
        QPoint p = tp.area.center().toPoint();
        QPointF localPos(window->mapFromGlobal(QPoint(p.x() + margins.left(), p.y() + margins.top())));
        if (mFocus->touchDragDecoration(mParent, localPos, tp.area.center(), tp.state, mParent->modifiers()))
            return;
    }

    QWindowSystemInterface::handleTouchEvent(window, mParent->mTouchDevice, mPendingTouchPoints);

    // Prepare state for next frame
    const auto prevTouchPoints = mPendingTouchPoints;
    mPendingTouchPoints.clear();
    for (const auto &prevPoint: prevTouchPoints) {
        // All non-released touch points should be part of the next touch event
        if (prevPoint.state != Qt::TouchPointReleased) {
            QWindowSystemInterface::TouchPoint tp = prevPoint;
            tp.state = Qt::TouchPointStationary; // ... as stationary (unless proven otherwise)
            mPendingTouchPoints.append(tp);
        }
    }

}

}

QT_END_NAMESPACE
