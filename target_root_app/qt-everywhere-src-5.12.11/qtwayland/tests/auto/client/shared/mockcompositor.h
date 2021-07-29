/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MOCKCOMPOSITOR_H
#define MOCKCOMPOSITOR_H

#include "mockxdgshellv6.h"
#include "mockiviapplication.h"

#include <pthread.h>
#include <qglobal.h>
#include <wayland-server.h>

#include <QImage>
#include <QMutex>
#include <QRect>
#include <QSharedPointer>
#include <QVariant>
#include <QVector>
#include <QWaitCondition>

namespace Impl {

typedef void (**Implementation)(void);

class Keyboard;
class Pointer;
class Touch;
class Seat;
class DataDeviceManager;
class Surface;
class Output;
class IviApplication;
class WlShell;
class XdgShellV6;

class Compositor
{
public:
    Compositor();
    ~Compositor();

    int fileDescriptor() const { return m_fd; }
    void dispatchEvents(int timeout = 0);

    uint32_t nextSerial();
    uint32_t time() { return ++m_time; }

    QVector<Surface *> surfaces() const;
    QVector<Output *> outputs() const;

    IviApplication *iviApplication() const;
    XdgShellV6 *xdgShellV6() const;

    void addSurface(Surface *surface);
    void removeSurface(Surface *surface);

    static void setKeyboardFocus(void *data, const QList<QVariant> &parameters);
    static void sendMousePress(void *data, const QList<QVariant> &parameters);
    static void sendMouseRelease(void *data, const QList<QVariant> &parameters);
    static void sendKeyPress(void *data, const QList<QVariant> &parameters);
    static void sendKeyRelease(void *data, const QList<QVariant> &parameters);
    static void sendTouchDown(void *data, const QList<QVariant> &parameters);
    static void sendTouchUp(void *data, const QList<QVariant> &parameters);
    static void sendTouchMotion(void *data, const QList<QVariant> &parameters);
    static void sendTouchFrame(void *data, const QList<QVariant> &parameters);
    static void sendDataDeviceDataOffer(void *data, const QList<QVariant> &parameters);
    static void sendDataDeviceEnter(void *data, const QList<QVariant> &parameters);
    static void sendDataDeviceMotion(void *data, const QList<QVariant> &parameters);
    static void sendDataDeviceDrop(void *data, const QList<QVariant> &parameters);
    static void sendDataDeviceLeave(void *data, const QList<QVariant> &parameters);
    static void waitForStartDrag(void *data, const QList<QVariant> &parameters);
    static void setOutputMode(void *compositor, const QList<QVariant> &parameters);
    static void sendAddOutput(void *data, const QList<QVariant> &parameters);
    static void sendRemoveOutput(void *data, const QList<QVariant> &parameters);
    static void sendOutputGeometry(void *data, const QList<QVariant> &parameters);
    static void sendSurfaceEnter(void *data, const QList<QVariant> &parameters);
    static void sendSurfaceLeave(void *data, const QList<QVariant> &parameters);
    static void sendShellSurfaceConfigure(void *data, const QList<QVariant> &parameters);
    static void sendIviSurfaceConfigure(void *data, const QList<QVariant> &parameters);
    static void sendXdgToplevelV6Configure(void *data, const QList<QVariant> &parameters);

public:
    bool m_startDragSeen = false;

private:
    static void bindCompositor(wl_client *client, void *data, uint32_t version, uint32_t id);
    static Surface *resolveSurface(const QVariant &v);
    static Output *resolveOutput(const QVariant &v);
    static IviSurface *resolveIviSurface(const QVariant &v);
    static XdgToplevelV6 *resolveToplevel(const QVariant &v);

    void initShm();

    QRect m_outputGeometry;

    wl_display *m_display = nullptr;
    wl_event_loop *m_loop = nullptr;
    wl_shm *m_shm = nullptr;
    int m_fd = -1;

    uint32_t m_time = 0;

    QScopedPointer<Seat> m_seat;
    Pointer *m_pointer = nullptr;
    Keyboard *m_keyboard = nullptr;
    Touch *m_touch = nullptr;
    QScopedPointer<DataDeviceManager> m_data_device_manager;
    QVector<Surface *> m_surfaces;
    QVector<Output *> m_outputs;
    QScopedPointer<IviApplication> m_iviApplication;
    QScopedPointer<WlShell> m_wlShell;
    QScopedPointer<XdgShellV6> m_xdgShellV6;
};

void registerResource(wl_list *list, wl_resource *resource);

}

class MockSurface
{
public:
    Impl::Surface *handle() const { return m_surface; }

    QImage image;

private:
    MockSurface(Impl::Surface *surface);
    friend class Impl::Compositor;
    friend class Impl::Surface;

    Impl::Surface *m_surface = nullptr;
};

Q_DECLARE_METATYPE(QSharedPointer<MockSurface>)

class MockIviSurface
{
public:
    Impl::IviSurface *handle() const { return m_iviSurface; }
    const uint iviId;

private:
    MockIviSurface(Impl::IviSurface *iviSurface) : iviId(iviSurface->iviId()), m_iviSurface(iviSurface) {}
    friend class Impl::Compositor;
    friend class Impl::IviSurface;

    Impl::IviSurface *m_iviSurface;
};

Q_DECLARE_METATYPE(QSharedPointer<MockIviSurface>)

class MockXdgToplevelV6 : public QObject
{
    Q_OBJECT
public:
    Impl::XdgToplevelV6 *handle() const { return m_toplevel; }

    void sendConfigure(const QSharedPointer<MockXdgToplevelV6> toplevel);

signals:
    uint setMinimizedRequested();
    uint setMaximizedRequested();
    uint unsetMaximizedRequested();
    uint setFullscreenRequested();
    uint unsetFullscreenRequested();
    void windowGeometryRequested(QRect geometry); // NOTE: This is really an xdg surface event

private:
    MockXdgToplevelV6(Impl::XdgToplevelV6 *toplevel) : m_toplevel(toplevel) {}
    friend class Impl::Compositor;
    friend class Impl::XdgToplevelV6;

    Impl::XdgToplevelV6 *m_toplevel;
};

Q_DECLARE_METATYPE(QSharedPointer<MockXdgToplevelV6>)

class MockOutput {
public:
    Impl::Output *handle() const { return m_output; }
    MockOutput(Impl::Output *output);
private:
    Impl::Output *m_output = nullptr;
};

Q_DECLARE_METATYPE(QSharedPointer<MockOutput>)

class MockCompositor
{
public:
    MockCompositor();
    ~MockCompositor();

    void applicationInitialized();

    int waylandFileDescriptor() const;
    void processWaylandEvents();

    void setOutputMode(const QSize &size);
    void setKeyboardFocus(const QSharedPointer<MockSurface> &surface);
    void sendMousePress(const QSharedPointer<MockSurface> &surface, const QPoint &pos);
    void sendMouseRelease(const QSharedPointer<MockSurface> &surface);
    void sendKeyPress(const QSharedPointer<MockSurface> &surface, uint code);
    void sendKeyRelease(const QSharedPointer<MockSurface> &surface, uint code);
    void sendTouchDown(const QSharedPointer<MockSurface> &surface, const QPoint &position, int id);
    void sendTouchMotion(const QSharedPointer<MockSurface> &surface, const QPoint &position, int id);
    void sendTouchUp(const QSharedPointer<MockSurface> &surface, int id);
    void sendTouchFrame(const QSharedPointer<MockSurface> &surface);
    void sendDataDeviceDataOffer(const QSharedPointer<MockSurface> &surface);
    void sendDataDeviceEnter(const QSharedPointer<MockSurface> &surface, const QPoint &position);
    void sendDataDeviceMotion(const QPoint &position);
    void sendDataDeviceDrop(const QSharedPointer<MockSurface> &surface);
    void sendDataDeviceLeave(const QSharedPointer<MockSurface> &surface);
    void sendAddOutput();
    void sendRemoveOutput(const QSharedPointer<MockOutput> &output);
    void sendOutputGeometry(const QSharedPointer<MockOutput> &output, const QRect &geometry);
    void sendSurfaceEnter(const QSharedPointer<MockSurface> &surface, QSharedPointer<MockOutput> &output);
    void sendSurfaceLeave(const QSharedPointer<MockSurface> &surface, QSharedPointer<MockOutput> &output);
    void sendShellSurfaceConfigure(const QSharedPointer<MockSurface> surface, const QSize &size = QSize(0, 0));
    void sendIviSurfaceConfigure(const QSharedPointer<MockIviSurface> iviSurface, const QSize &size);
    void sendXdgToplevelV6Configure(const QSharedPointer<MockXdgToplevelV6> toplevel, const QSize &size = QSize(0, 0),
                                    const QVector<uint> &states = { ZXDG_TOPLEVEL_V6_STATE_ACTIVATED });
    void waitForStartDrag();

    QSharedPointer<MockSurface> surface();
    QSharedPointer<MockOutput> output(int index = 0);
    QSharedPointer<MockIviSurface> iviSurface(int index = 0);
    QSharedPointer<MockXdgToplevelV6> xdgToplevelV6(int index = 0);

    void lock();
    void unlock();

private:
    struct Command
    {
        typedef void (*Callback)(void *target, const QList<QVariant> &parameters);

        Callback callback;
        void *target = nullptr;
        QList<QVariant> parameters;
    };

    static Command makeCommand(Command::Callback callback, void *target);

    void processCommand(const Command &command);
    void dispatchCommands();

    static void *run(void *data);

    bool m_alive = true;
    bool m_ready = false;
    pthread_t m_thread;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;

    Impl::Compositor *m_compositor = nullptr;

    QList<Command> m_commandQueue;
};

#endif
