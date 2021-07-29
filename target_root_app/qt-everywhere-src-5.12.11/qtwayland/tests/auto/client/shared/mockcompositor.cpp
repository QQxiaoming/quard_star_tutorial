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

#include "mockcompositor.h"
#include "mockinput.h"
#include "mockoutput.h"
#include "mocksurface.h"
#include "mockwlshell.h"
#include "mockxdgshellv6.h"
#include "mockiviapplication.h"

#include <wayland-xdg-shell-unstable-v6-server-protocol.h>

#include <stdio.h>
MockCompositor::MockCompositor()
{
    pthread_create(&m_thread, 0, run, this);

    m_mutex.lock();
    m_waitCondition.wait(&m_mutex);
    m_mutex.unlock();
}

MockCompositor::~MockCompositor()
{
    m_alive = false;
    m_waitCondition.wakeOne();
    pthread_join(m_thread, 0);
}

void MockCompositor::lock()
{
    m_mutex.lock();
}

void MockCompositor::unlock()
{
    m_mutex.unlock();
}

void MockCompositor::applicationInitialized()
{
    m_ready = true;
}

int MockCompositor::waylandFileDescriptor() const
{
    return m_compositor->fileDescriptor();
}

void MockCompositor::processWaylandEvents()
{
    m_waitCondition.wakeOne();
}

void MockCompositor::setOutputMode(const QSize &size)
{
    Command command = makeCommand(Impl::Compositor::setOutputMode, m_compositor);
    command.parameters << size;
    processCommand(command);
}

void MockCompositor::setKeyboardFocus(const QSharedPointer<MockSurface> &surface)
{
    Command command = makeCommand(Impl::Compositor::setKeyboardFocus, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    processCommand(command);
}

void MockCompositor::sendMousePress(const QSharedPointer<MockSurface> &surface, const QPoint &pos)
{
    Command command = makeCommand(Impl::Compositor::sendMousePress, m_compositor);
    command.parameters << QVariant::fromValue(surface) << pos;
    processCommand(command);
}

void MockCompositor::sendMouseRelease(const QSharedPointer<MockSurface> &surface)
{
    Command command = makeCommand(Impl::Compositor::sendMouseRelease, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    processCommand(command);
}

void MockCompositor::sendKeyPress(const QSharedPointer<MockSurface> &surface, uint code)
{
    Command command = makeCommand(Impl::Compositor::sendKeyPress, m_compositor);
    command.parameters << QVariant::fromValue(surface) << code;
    processCommand(command);
}

void MockCompositor::sendKeyRelease(const QSharedPointer<MockSurface> &surface, uint code)
{
    Command command = makeCommand(Impl::Compositor::sendKeyRelease, m_compositor);
    command.parameters << QVariant::fromValue(surface) << code;
    processCommand(command);
}

void MockCompositor::sendTouchDown(const QSharedPointer<MockSurface> &surface, const QPoint &position, int id)
{
    Command command = makeCommand(Impl::Compositor::sendTouchDown, m_compositor);
    command.parameters << QVariant::fromValue(surface) << position << id;
    processCommand(command);
}

void MockCompositor::sendTouchMotion(const QSharedPointer<MockSurface> &surface, const QPoint &position, int id)
{
    Command command = makeCommand(Impl::Compositor::sendTouchMotion, m_compositor);
    command.parameters << QVariant::fromValue(surface) << position << id;
    processCommand(command);
}

void MockCompositor::sendTouchUp(const QSharedPointer<MockSurface> &surface, int id)
{
    Command command = makeCommand(Impl::Compositor::sendTouchUp, m_compositor);
    command.parameters << QVariant::fromValue(surface) << id;
    processCommand(command);
}

void MockCompositor::sendTouchFrame(const QSharedPointer<MockSurface> &surface)
{
    Command command = makeCommand(Impl::Compositor::sendTouchFrame, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    processCommand(command);
}

void MockCompositor::sendDataDeviceDataOffer(const QSharedPointer<MockSurface> &surface)
{
    Command command = makeCommand(Impl::Compositor::sendDataDeviceDataOffer, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    processCommand(command);
}

void MockCompositor::sendDataDeviceEnter(const QSharedPointer<MockSurface> &surface, const QPoint& position)
{
    Command command = makeCommand(Impl::Compositor::sendDataDeviceEnter, m_compositor);
    command.parameters << QVariant::fromValue(surface) << QVariant::fromValue(position);
    processCommand(command);
}

void MockCompositor::sendDataDeviceMotion(const QPoint &position)
{
    Command command = makeCommand(Impl::Compositor::sendDataDeviceMotion, m_compositor);
    command.parameters << QVariant::fromValue(position);
    processCommand(command);
}

void MockCompositor::sendDataDeviceDrop(const QSharedPointer<MockSurface> &surface)
{
    Command command = makeCommand(Impl::Compositor::sendDataDeviceDrop, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    processCommand(command);
}

void MockCompositor::sendDataDeviceLeave(const QSharedPointer<MockSurface> &surface)
{
    Command command = makeCommand(Impl::Compositor::sendDataDeviceLeave, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    processCommand(command);
}

void MockCompositor::sendAddOutput()
{
    Command command = makeCommand(Impl::Compositor::sendAddOutput, m_compositor);
    processCommand(command);
}

void MockCompositor::sendRemoveOutput(const QSharedPointer<MockOutput> &output)
{
    Command command = makeCommand(Impl::Compositor::sendRemoveOutput, m_compositor);
    command.parameters << QVariant::fromValue(output);
    processCommand(command);
}

void MockCompositor::sendOutputGeometry(const QSharedPointer<MockOutput> &output, const QRect &geometry)
{
    Command command = makeCommand(Impl::Compositor::sendOutputGeometry, m_compositor);
    command.parameters << QVariant::fromValue(output);
    command.parameters << QVariant::fromValue(geometry);
    processCommand(command);
}

void MockCompositor::sendSurfaceEnter(const QSharedPointer<MockSurface> &surface, QSharedPointer<MockOutput> &output)
{
    Command command = makeCommand(Impl::Compositor::sendSurfaceEnter, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    command.parameters << QVariant::fromValue(output);
    processCommand(command);
}

void MockCompositor::sendSurfaceLeave(const QSharedPointer<MockSurface> &surface, QSharedPointer<MockOutput> &output)
{
    Command command = makeCommand(Impl::Compositor::sendSurfaceLeave, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    command.parameters << QVariant::fromValue(output);
    processCommand(command);
}

void MockCompositor::sendShellSurfaceConfigure(const QSharedPointer<MockSurface> surface, const QSize &size)
{
    Command command = makeCommand(Impl::Compositor::sendShellSurfaceConfigure, m_compositor);
    command.parameters << QVariant::fromValue(surface);
    command.parameters << QVariant::fromValue(size);
    processCommand(command);
}

void MockCompositor::sendIviSurfaceConfigure(const QSharedPointer<MockIviSurface> iviSurface, const QSize &size)
{
    Command command = makeCommand(Impl::Compositor::sendIviSurfaceConfigure, m_compositor);
    command.parameters << QVariant::fromValue(iviSurface);
    command.parameters << QVariant::fromValue(size);
    processCommand(command);
}

void MockCompositor::sendXdgToplevelV6Configure(const QSharedPointer<MockXdgToplevelV6> toplevel, const QSize &size, const QVector<uint> &states)
{
    Command command = makeCommand(Impl::Compositor::sendXdgToplevelV6Configure, m_compositor);
    command.parameters << QVariant::fromValue(toplevel);
    command.parameters << QVariant::fromValue(size);
    QByteArray statesBytes(reinterpret_cast<const char *>(states.data()),
                           states.size() * static_cast<int>(sizeof(uint)));
    command.parameters << statesBytes;
    processCommand(command);
}

void MockCompositor::waitForStartDrag()
{
    Command command = makeCommand(Impl::Compositor::waitForStartDrag, m_compositor);
    processCommand(command);
}

QSharedPointer<MockSurface> MockCompositor::surface()
{
    QSharedPointer<MockSurface> result;
    lock();
    QVector<Impl::Surface *> surfaces = m_compositor->surfaces();
    foreach (Impl::Surface *surface, surfaces) {
        // we don't want to mistake the cursor surface for a window surface
        if (surface->isMapped()) {
            result = surface->mockSurface();
            break;
        }
    }
    unlock();
    return result;
}

QSharedPointer<MockOutput> MockCompositor::output(int index)
{
    QSharedPointer<MockOutput> result;
    lock();
    if (Impl::Output *output = m_compositor->outputs().value(index, nullptr))
        result = output->mockOutput();
    unlock();
    return result;
}

QSharedPointer<MockIviSurface> MockCompositor::iviSurface(int index)
{
    QSharedPointer<MockIviSurface> result;
    lock();
    if (Impl::IviSurface *toplevel = m_compositor->iviApplication()->iviSurfaces().value(index, nullptr))
        result = toplevel->mockIviSurface();
    unlock();
    return result;
}

QSharedPointer<MockXdgToplevelV6> MockCompositor::xdgToplevelV6(int index)
{
    QSharedPointer<MockXdgToplevelV6> result;
    lock();
    if (Impl::XdgToplevelV6 *toplevel = m_compositor->xdgShellV6()->toplevels().value(index, nullptr))
        result = toplevel->mockToplevel();
    unlock();
    return result;
}

MockCompositor::Command MockCompositor::makeCommand(Command::Callback callback, void *target)
{
    Command command;
    command.callback = callback;
    command.target = target;
    return command;
}

void MockCompositor::processCommand(const Command &command)
{
    lock();
    m_commandQueue << command;
    unlock();

    m_waitCondition.wakeOne();
}

void MockCompositor::dispatchCommands()
{
    lock();
    int count = m_commandQueue.length();
    unlock();

    for (int i = 0; i < count; ++i) {
        lock();
        const Command command = m_commandQueue.takeFirst();
        unlock();
        command.callback(command.target, command.parameters);
    }
}

void *MockCompositor::run(void *data)
{
    MockCompositor *controller = static_cast<MockCompositor *>(data);

    Impl::Compositor compositor;

    controller->m_compositor = &compositor;

    while (!controller->m_ready) {
        controller->m_waitCondition.wakeOne();
        controller->dispatchCommands();
        compositor.dispatchEvents(20);
    }

    while (controller->m_alive) {
        {
            QMutexLocker locker(&controller->m_mutex);
            if (controller->m_commandQueue.isEmpty())
                controller->m_waitCondition.wait(&controller->m_mutex);
        }
        controller->dispatchCommands();
        compositor.dispatchEvents(20);
    }

    return 0;
}

namespace Impl {

Compositor::Compositor()
    : m_display(wl_display_create())
{
    if (wl_display_add_socket(m_display, 0)) {
        fprintf(stderr, "Fatal: Failed to open server socket\n");
        exit(EXIT_FAILURE);
    }

    wl_global_create(m_display, &wl_compositor_interface, 1, this, bindCompositor);

    m_data_device_manager.reset(new DataDeviceManager(this, m_display));

    wl_display_init_shm(m_display);

    m_seat.reset(new Seat(this, m_display));
    m_pointer = m_seat->pointer();
    m_keyboard = m_seat->keyboard();
    m_touch = m_seat->touch();

    m_outputs.append(new Output(m_display, QSize(1920, 1080), QPoint(0, 0)));
    m_iviApplication.reset(new IviApplication(m_display));
    m_wlShell.reset(new WlShell(m_display));
    m_xdgShellV6.reset(new XdgShellV6(m_display));

    m_loop = wl_display_get_event_loop(m_display);
    m_fd = wl_event_loop_get_fd(m_loop);
}

Compositor::~Compositor()
{
    wl_display_destroy(m_display);
}

void Compositor::dispatchEvents(int timeout)
{
    wl_display_flush_clients(m_display);
    wl_event_loop_dispatch(m_loop, timeout);
}

static void compositor_create_surface(wl_client *client, wl_resource *compositorResource, uint32_t id)
{
    Compositor *compositor = static_cast<Compositor *>(wl_resource_get_user_data(compositorResource));
    compositor->addSurface(new Surface(client, id, wl_resource_get_version(compositorResource), compositor));
}

static void compositor_create_region(wl_client *client, wl_resource *compositorResource, uint32_t id)
{
    Q_UNUSED(client);
    Q_UNUSED(compositorResource);
    Q_UNUSED(id);
}

void Compositor::bindCompositor(wl_client *client, void *compositorData, uint32_t version, uint32_t id)
{
    static const struct wl_compositor_interface compositorInterface = {
        compositor_create_surface,
        compositor_create_region
    };

    wl_resource *resource = wl_resource_create(client, &wl_compositor_interface, static_cast<int>(version), id);
    wl_resource_set_implementation(resource, &compositorInterface, compositorData, nullptr);
}

static void unregisterResourceCallback(wl_listener *listener, void *data)
{
    struct wl_resource *resource = reinterpret_cast<struct wl_resource *>(data);
    wl_list_remove(wl_resource_get_link(resource));
    delete listener;
}

void registerResource(wl_list *list, wl_resource *resource)
{
    wl_list_insert(list, wl_resource_get_link(resource));

    wl_listener *listener = new wl_listener;
    listener->notify = unregisterResourceCallback;

    wl_resource_add_destroy_listener(resource, listener);
}

QVector<Surface *> Compositor::surfaces() const
{
    return m_surfaces;
}

QVector<Output *> Compositor::outputs() const
{
    return m_outputs;
}

IviApplication *Compositor::iviApplication() const
{
    return m_iviApplication.data();
}

XdgShellV6 *Compositor::xdgShellV6() const
{
    return m_xdgShellV6.data();
}

uint32_t Compositor::nextSerial()
{
    return wl_display_next_serial(m_display);
}

void Compositor::addSurface(Surface *surface)
{
    m_surfaces << surface;
}

void Compositor::removeSurface(Surface *surface)
{
    m_surfaces.removeOne(surface);
    m_keyboard->handleSurfaceDestroyed(surface);
    m_pointer->handleSurfaceDestroyed(surface);
}

Surface *Compositor::resolveSurface(const QVariant &v)
{
    QSharedPointer<MockSurface> mockSurface = v.value<QSharedPointer<MockSurface> >();
    return mockSurface ? mockSurface->handle() : nullptr;
}

Output *Compositor::resolveOutput(const QVariant &v)
{
    QSharedPointer<MockOutput> mockOutput = v.value<QSharedPointer<MockOutput> >();
    return mockOutput ? mockOutput->handle() : nullptr;
}

IviSurface *Compositor::resolveIviSurface(const QVariant &v)
{
    QSharedPointer<MockIviSurface> mockIviSurface = v.value<QSharedPointer<MockIviSurface>>();
    return mockIviSurface ? mockIviSurface->handle() : nullptr;
}

XdgToplevelV6 *Compositor::resolveToplevel(const QVariant &v)
{
    QSharedPointer<MockXdgToplevelV6> mockToplevel = v.value<QSharedPointer<MockXdgToplevelV6>>();
    return mockToplevel ? mockToplevel->handle() : nullptr;
}

}
