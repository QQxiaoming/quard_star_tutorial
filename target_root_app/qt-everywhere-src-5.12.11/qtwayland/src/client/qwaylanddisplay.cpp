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

#include "qwaylanddisplay_p.h"

#include "qwaylandintegration_p.h"
#include "qwaylandwindow_p.h"
#include "qwaylandabstractdecoration_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylandcursor_p.h"
#include "qwaylandinputdevice_p.h"
#if QT_CONFIG(clipboard)
#include "qwaylandclipboard_p.h"
#endif
#if QT_CONFIG(wayland_datadevice)
#include "qwaylanddatadevicemanager_p.h"
#endif
#if QT_CONFIG(cursor)
#include <wayland-cursor.h>
#endif
#include "qwaylandhardwareintegration_p.h"
#include "qwaylandinputcontext_p.h"

#include "qwaylandwindowmanagerintegration_p.h"
#include "qwaylandshellintegration_p.h"
#include "qwaylandclientbufferintegration_p.h"

#include "qwaylandextendedsurface_p.h"
#include "qwaylandsubsurface_p.h"
#include "qwaylandtouch_p.h"
#include "qwaylandqtkey_p.h"

#include <QtWaylandClient/private/qwayland-text-input-unstable-v2.h>

#include <QtCore/private/qcore_unix_p.h>

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/private/qguiapplication_p.h>

#include <QtCore/QDebug>

#include <errno.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

Q_LOGGING_CATEGORY(lcQpaWayland, "qt.qpa.wayland"); // for general (uncategorized) Wayland platform logging

struct wl_surface *QWaylandDisplay::createSurface(void *handle)
{
    struct wl_surface *surface = mCompositor.create_surface();
    wl_surface_set_user_data(surface, handle);
    return surface;
}

struct ::wl_region *QWaylandDisplay::createRegion(const QRegion &qregion)
{
    struct ::wl_region *region = mCompositor.create_region();

    for (const QRect &rect : qregion)
        wl_region_add(region, rect.x(), rect.y(), rect.width(), rect.height());

    return region;
}

::wl_subsurface *QWaylandDisplay::createSubSurface(QWaylandWindow *window, QWaylandWindow *parent)
{
    if (!mSubCompositor) {
        qCWarning(lcQpaWayland) << "Can't create subsurface, not supported by the compositor.";
        return nullptr;
    }

    // Make sure we don't pass NULL surfaces to libwayland (crashes)
    Q_ASSERT(parent->object());
    Q_ASSERT(window->object());

    return mSubCompositor->get_subsurface(window->object(), parent->object());
}

QWaylandShellIntegration *QWaylandDisplay::shellIntegration() const
{
    return mWaylandIntegration->shellIntegration();
}

QWaylandClientBufferIntegration * QWaylandDisplay::clientBufferIntegration() const
{
    return mWaylandIntegration->clientBufferIntegration();
}

QWaylandWindowManagerIntegration *QWaylandDisplay::windowManagerIntegration() const
{
    return mWindowManagerIntegration.data();
}

QWaylandDisplay::QWaylandDisplay(QWaylandIntegration *waylandIntegration)
    : mWaylandIntegration(waylandIntegration)
{
    qRegisterMetaType<uint32_t>("uint32_t");

    mDisplay = wl_display_connect(nullptr);
    if (!mDisplay) {
        qErrnoWarning(errno, "Failed to create wl_display");
        return;
    }

    struct ::wl_registry *registry = wl_display_get_registry(mDisplay);
    init(registry);

    mWindowManagerIntegration.reset(new QWaylandWindowManagerIntegration(this));

    forceRoundTrip();

    if (!mWaitingScreens.isEmpty()) {
        // Give wl_output.done and zxdg_output_v1.done events a chance to arrive
        forceRoundTrip();
    }
}

QWaylandDisplay::~QWaylandDisplay(void)
{
    if (mSyncCallback)
        wl_callback_destroy(mSyncCallback);

    qDeleteAll(mInputDevices);
    mInputDevices.clear();

    foreach (QWaylandScreen *screen, mScreens) {
        mWaylandIntegration->destroyScreen(screen);
    }
    mScreens.clear();
    qDeleteAll(mWaitingScreens);

#if QT_CONFIG(wayland_datadevice)
    delete mDndSelectionHandler.take();
#endif
#if QT_CONFIG(cursor)
    qDeleteAll(mCursorThemesBySize);
#endif
    if (mDisplay)
        wl_display_disconnect(mDisplay);
}

void QWaylandDisplay::checkError() const
{
    int ecode = wl_display_get_error(mDisplay);
    if ((ecode == EPIPE || ecode == ECONNRESET)) {
        // special case this to provide a nicer error
        qFatal("The Wayland connection broke. Did the Wayland compositor die?");
    } else {
        qFatal("The Wayland connection experienced a fatal error: %s", strerror(ecode));
    }
}

void QWaylandDisplay::flushRequests()
{
    if (wl_display_prepare_read(mDisplay) == 0) {
        wl_display_read_events(mDisplay);
    }

    if (wl_display_dispatch_pending(mDisplay) < 0)
        checkError();

    wl_display_flush(mDisplay);
}

void QWaylandDisplay::blockingReadEvents()
{
    if (wl_display_dispatch(mDisplay) < 0)
        checkError();
}

wl_event_queue *QWaylandDisplay::createEventQueue()
{
    return wl_display_create_queue(mDisplay);
}

void QWaylandDisplay::dispatchQueueWhile(wl_event_queue *queue, std::function<bool ()> condition, int timeout)
{
    if (!condition())
        return;

    QElapsedTimer timer;
    timer.start();
    struct pollfd pFd = qt_make_pollfd(wl_display_get_fd(mDisplay), POLLIN);
    while (timeout == -1 || timer.elapsed() < timeout) {
        while (wl_display_prepare_read_queue(mDisplay, queue) != 0)
            wl_display_dispatch_queue_pending(mDisplay, queue);

        wl_display_flush(mDisplay);

        const int remaining = qMax(timeout - timer.elapsed(), 0ll);
        const int pollTimeout = timeout == -1 ? -1 : remaining;
        if (qt_poll_msecs(&pFd, 1, pollTimeout) > 0)
            wl_display_read_events(mDisplay);
        else
            wl_display_cancel_read(mDisplay);

        if (wl_display_dispatch_queue_pending(mDisplay, queue) < 0)
            checkError();

        if (!condition())
            break;
    }
}

QWaylandScreen *QWaylandDisplay::screenForOutput(struct wl_output *output) const
{
    for (int i = 0; i < mScreens.size(); ++i) {
        QWaylandScreen *screen = static_cast<QWaylandScreen *>(mScreens.at(i));
        if (screen->output() == output)
            return screen;
    }
    return nullptr;
}

void QWaylandDisplay::handleScreenInitialized(QWaylandScreen *screen)
{
    if (!mWaitingScreens.removeOne(screen))
        return;
    mScreens.append(screen);
    QWindowSystemInterface::handleScreenAdded(screen);
}

void QWaylandDisplay::waitForScreens()
{
    flushRequests();

    while (true) {
        bool screensReady = !mScreens.isEmpty();

        for (int ii = 0; screensReady && ii < mScreens.count(); ++ii) {
            if (mScreens.at(ii)->geometry() == QRect(0, 0, 0, 0))
                screensReady = false;
        }

        if (!screensReady)
            blockingReadEvents();
        else
            return;
    }
}

void QWaylandDisplay::registry_global(uint32_t id, const QString &interface, uint32_t version)
{
    Q_UNUSED(version);

    struct ::wl_registry *registry = object();

    if (interface == QStringLiteral("wl_output")) {
        mWaitingScreens << new QWaylandScreen(this, version, id);
    } else if (interface == QStringLiteral("wl_compositor")) {
        mCompositorVersion = qMin((int)version, 3);
        mCompositor.init(registry, id, mCompositorVersion);
    } else if (interface == QStringLiteral("wl_shm")) {
        mShm.reset(new QWaylandShm(this, version, id));
    } else if (interface == QStringLiteral("wl_seat")) {
        QWaylandInputDevice *inputDevice = mWaylandIntegration->createInputDevice(this, version, id);
        mInputDevices.append(inputDevice);
#if QT_CONFIG(wayland_datadevice)
    } else if (interface == QStringLiteral("wl_data_device_manager")) {
        mDndSelectionHandler.reset(new QWaylandDataDeviceManager(this, id));
#endif
    } else if (interface == QStringLiteral("qt_surface_extension")) {
        mWindowExtension.reset(new QtWayland::qt_surface_extension(registry, id, 1));
    } else if (interface == QStringLiteral("wl_subcompositor")) {
        mSubCompositor.reset(new QtWayland::wl_subcompositor(registry, id, 1));
    } else if (interface == QStringLiteral("qt_touch_extension")) {
        mTouchExtension.reset(new QWaylandTouchExtension(this, id));
    } else if (interface == QStringLiteral("zqt_key_v1")) {
        mQtKeyExtension.reset(new QWaylandQtKeyExtension(this, id));
    } else if (interface == QStringLiteral("zwp_text_input_manager_v2")) {
        mTextInputManager.reset(new QtWayland::zwp_text_input_manager_v2(registry, id, 1));
        foreach (QWaylandInputDevice *inputDevice, mInputDevices) {
            inputDevice->setTextInput(new QWaylandTextInput(this, mTextInputManager->get_text_input(inputDevice->wl_seat())));
        }
    } else if (interface == QStringLiteral("qt_hardware_integration")) {
        mHardwareIntegration.reset(new QWaylandHardwareIntegration(registry, id));
        // make a roundtrip here since we need to receive the events sent by
        // qt_hardware_integration before creating windows
        forceRoundTrip();
    } else if (interface == QLatin1String("zxdg_output_manager_v1")) {
        mXdgOutputManager.reset(new QtWayland::zxdg_output_manager_v1(registry, id, 1));
        for (auto *screen : qAsConst(mWaitingScreens))
            screen->initXdgOutput(xdgOutputManager());
        forceRoundTrip();
    }

    mGlobals.append(RegistryGlobal(id, interface, version, registry));

    foreach (Listener l, mRegistryListeners)
        (*l.listener)(l.data, registry, id, interface, version);
}

void QWaylandDisplay::registry_global_remove(uint32_t id)
{
    for (int i = 0, ie = mGlobals.count(); i != ie; ++i) {
        RegistryGlobal &global = mGlobals[i];
        if (global.id == id) {
            if (global.interface == QStringLiteral("wl_output")) {
                for (auto *screen : mWaitingScreens) {
                    if (screen->outputId() == id) {
                        mWaitingScreens.removeOne(screen);
                        delete screen;
                        break;
                    }
                }

                foreach (QWaylandScreen *screen, mScreens) {
                    if (screen->outputId() == id) {
                        mScreens.removeOne(screen);
                        mWaylandIntegration->destroyScreen(screen);
                        break;
                    }
                }
            }
            mGlobals.removeAt(i);
            break;
        }
    }
}

bool QWaylandDisplay::hasRegistryGlobal(const QString &interfaceName)
{
    Q_FOREACH (const RegistryGlobal &global, mGlobals)
        if (global.interface == interfaceName)
            return true;

    return false;
}

void QWaylandDisplay::addRegistryListener(RegistryListener listener, void *data)
{
    Listener l = { listener, data };
    mRegistryListeners.append(l);
    for (int i = 0, ie = mGlobals.count(); i != ie; ++i)
        (*l.listener)(l.data, mGlobals[i].registry, mGlobals[i].id, mGlobals[i].interface, mGlobals[i].version);
}

void QWaylandDisplay::removeListener(RegistryListener listener, void *data)
{
    std::remove_if(mRegistryListeners.begin(), mRegistryListeners.end(), [=](Listener l){
        return (l.listener == listener && l.data == data);
    });
}

uint32_t QWaylandDisplay::currentTimeMillisec()
{
    //### we throw away the time information
    struct timeval tv;
    int ret = gettimeofday(&tv, nullptr);
    if (ret == 0)
        return tv.tv_sec*1000 + tv.tv_usec/1000;
    return 0;
}

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
    Q_UNUSED(serial)
    bool *done = static_cast<bool *>(data);

    *done = true;

    // If the wl_callback done event is received after the condition check in the while loop in
    // forceRoundTrip(), but before the call to processEvents, the call to processEvents may block
    // forever if no more events are posted (eventhough the callback is handled in response to the
    // aboutToBlock signal). Hence, we wake up the event dispatcher so forceRoundTrip may return.
    // (QTBUG-64696)
    if (auto *dispatcher = QThread::currentThread()->eventDispatcher())
        dispatcher->wakeUp();

    wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
    sync_callback
};

void QWaylandDisplay::forceRoundTrip()
{
    // wl_display_roundtrip() works on the main queue only,
    // but we use a separate one, so basically reimplement it here
    int ret = 0;
    bool done = false;
    wl_callback *callback = wl_display_sync(mDisplay);
    wl_callback_add_listener(callback, &sync_listener, &done);
    flushRequests();
    if (QThread::currentThread()->eventDispatcher()) {
        while (!done && ret >= 0) {
            QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::WaitForMoreEvents);
            ret = wl_display_dispatch_pending(mDisplay);
        }
    } else {
        while (!done && ret >= 0)
            ret = wl_display_dispatch(mDisplay);
    }

    if (ret == -1 && !done)
        wl_callback_destroy(callback);
}

bool QWaylandDisplay::supportsWindowDecoration() const
{
    static bool disabled = qgetenv("QT_WAYLAND_DISABLE_WINDOWDECORATION").toInt();
    // Stop early when disabled via the environment. Do not try to load the integration in
    // order to play nice with SHM-only, buffer integration-less systems.
    if (disabled)
        return false;

    static bool integrationSupport = clientBufferIntegration() && clientBufferIntegration()->supportsWindowDecoration();
    return integrationSupport;
}

QWaylandWindow *QWaylandDisplay::lastInputWindow() const
{
    return mLastInputWindow.data();
}

void QWaylandDisplay::setLastInputDevice(QWaylandInputDevice *device, uint32_t serial, QWaylandWindow *win)
{
    mLastInputDevice = device;
    mLastInputSerial = serial;
    mLastInputWindow = win;
}

bool QWaylandDisplay::isWindowActivated(const QWaylandWindow *window)
{
    return mActiveWindows.contains(const_cast<QWaylandWindow *>(window));
}

void QWaylandDisplay::handleWindowActivated(QWaylandWindow *window)
{
    if (mActiveWindows.contains(window))
        return;

    mActiveWindows.append(window);
    requestWaylandSync();

    if (auto *decoration = window->decoration())
        decoration->update();
}

void QWaylandDisplay::handleWindowDeactivated(QWaylandWindow *window)
{
    Q_ASSERT(!mActiveWindows.empty());

    if (mActiveWindows.last() == window)
        requestWaylandSync();

    mActiveWindows.removeOne(window);

    if (auto *decoration = window->decoration())
        decoration->update();
}

void QWaylandDisplay::handleKeyboardFocusChanged(QWaylandInputDevice *inputDevice)
{
    QWaylandWindow *keyboardFocus = inputDevice->keyboardFocus();

    if (mLastKeyboardFocus == keyboardFocus)
        return;

    if (mWaylandIntegration->mShellIntegration) {
        mWaylandIntegration->mShellIntegration->handleKeyboardFocusChanged(keyboardFocus, mLastKeyboardFocus);
    } else {
        if (keyboardFocus)
            handleWindowActivated(keyboardFocus);
        if (mLastKeyboardFocus)
            handleWindowDeactivated(mLastKeyboardFocus);
    }

    mLastKeyboardFocus = keyboardFocus;
}

void QWaylandDisplay::handleWindowDestroyed(QWaylandWindow *window)
{
    if (mActiveWindows.contains(window))
        handleWindowDeactivated(window);
}

void QWaylandDisplay::handleWaylandSync()
{
    // This callback is used to set the window activation because we may get an activate/deactivate
    // pair, and the latter one would be lost in the QWindowSystemInterface queue, if we issue the
    // handleWindowActivated() calls immediately.
    QWindow *activeWindow = mActiveWindows.empty() ? nullptr : mActiveWindows.last()->window();
    if (activeWindow != QGuiApplication::focusWindow())
        QWindowSystemInterface::handleWindowActivated(activeWindow);
}

const wl_callback_listener QWaylandDisplay::syncCallbackListener = {
    [](void *data, struct wl_callback *callback, uint32_t time){
        Q_UNUSED(time);
        wl_callback_destroy(callback);
        QWaylandDisplay *display = static_cast<QWaylandDisplay *>(data);
        display->mSyncCallback = nullptr;
        display->handleWaylandSync();
    }
};

void QWaylandDisplay::requestWaylandSync()
{
    if (mSyncCallback)
        return;

    mSyncCallback = wl_display_sync(mDisplay);
    wl_callback_add_listener(mSyncCallback, &syncCallbackListener, this);
}

QWaylandInputDevice *QWaylandDisplay::defaultInputDevice() const
{
    return mInputDevices.isEmpty() ? 0 : mInputDevices.first();
}

#if QT_CONFIG(cursor)

void QWaylandDisplay::setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image, qreal dpr)
{
    /* Qt doesn't tell us which input device we should set the cursor
     * for, so set it for all devices. */
    for (int i = 0; i < mInputDevices.count(); i++) {
        QWaylandInputDevice *inputDevice = mInputDevices.at(i);
        inputDevice->setCursor(buffer, image, dpr);
    }
}

void QWaylandDisplay::setCursor(const QSharedPointer<QWaylandBuffer> &buffer, const QPoint &hotSpot, qreal dpr)
{
    /* Qt doesn't tell us which input device we should set the cursor
     * for, so set it for all devices. */
    for (int i = 0; i < mInputDevices.count(); i++) {
        QWaylandInputDevice *inputDevice = mInputDevices.at(i);
        inputDevice->setCursor(buffer, hotSpot, dpr);
    }
}

QWaylandCursorTheme *QWaylandDisplay::loadCursorTheme(qreal devicePixelRatio)
{
    constexpr int defaultCursorSize = 32;
    static const int xCursorSize = qEnvironmentVariableIntValue("XCURSOR_SIZE");
    int cursorSize = xCursorSize > 0 ? xCursorSize : defaultCursorSize;

    if (compositorVersion() >= 3) // set_buffer_scale is not supported on earlier versions
        cursorSize *= devicePixelRatio;

    if (auto *theme = mCursorThemesBySize.value(cursorSize, nullptr))
        return theme;

    if (auto *theme = QWaylandCursorTheme::create(shm(), cursorSize)) {
        mCursorThemesBySize[cursorSize] = theme;
        return theme;
    }

    return nullptr;
}

#endif // QT_CONFIG(cursor)

}

QT_END_NAMESPACE
