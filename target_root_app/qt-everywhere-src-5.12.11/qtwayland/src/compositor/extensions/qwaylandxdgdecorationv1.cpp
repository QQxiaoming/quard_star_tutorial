/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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

#include "qwaylandxdgdecorationv1_p.h"

#include <QtWaylandCompositor/QWaylandXdgToplevel>
#include <QtWaylandCompositor/private/qwaylandxdgshell_p.h>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

/*!
    \qmltype XdgDecorationManagerV1
    \inqmlmodule QtWayland.Compositor
    \since 5.12
    \brief Provides an extension for negotiation of server-side and client-side window decorations

    The XdgDecorationManagerV1 extension provides a way for a compositor to announce support for
    server-side window decorations, and for xdg-shell clients to communicate whether they prefer
    client-side or server-side decorations.

    XdgDecorationManagerV1 corresponds to the Wayland interface, \c zxdg_decoration_manager_v1.

    To provide the functionality of the extension in a compositor, create an instance of the
    XdgDecorationManagerV1 component and add it to the list of extensions supported by the compositor:

    \code
    import QtWayland.Compositor 1.3

    WaylandCompositor {
        // Xdg decoration manager assumes xdg-shell is being used
        XdgShell {
            onToplevelCreated: // ...
        }
        XdgDecorationManagerV1 {
            // Provide a hint to clients that support the extension they should use server-side
            // decorations.
            preferredMode: XdgToplevel.ServerSideDecoration
        }
    }
    \endcode

    \sa XdgToplevel::decorationMode
*/

/*!
    \class QWaylandXdgDecorationManagerV1
    \inmodule QtWaylandCompositor
    \since 5.12
    \brief Provides an extension for negotiation of server-side and client-side window decorations

    The QWaylandXdgDecorationManagerV1 extension provides a way for a compositor to announce support
    for server-side window decorations, and for xdg-shell clients to communicate whether they prefer
    client-side or server-side decorations.

    QWaylandXdgDecorationManagerV1 corresponds to the Wayland interface, \c zxdg_decoration_manager_v1.

    \sa QWaylandXdgToplevel::decorationMode
*/

/*!
    Constructs a QWaylandXdgDecorationManagerV1 object.
*/
QWaylandXdgDecorationManagerV1::QWaylandXdgDecorationManagerV1()
    : QWaylandCompositorExtensionTemplate<QWaylandXdgDecorationManagerV1>(*new QWaylandXdgDecorationManagerV1Private)
{
}

/*!
    Initializes the extension.
*/
void QWaylandXdgDecorationManagerV1::initialize()
{
    Q_D(QWaylandXdgDecorationManagerV1);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandXdgDecorationV1";
        return;
    }
    d->init(compositor->display(), 1);
}

/*!
    \qmlproperty string QtWaylandCompositor::XdgDecorationManagerV1::preferredMode

    This property holds the decoration mode the compositor prefers.

    This is the mode used for clients that don't indicate a preference for server-side or
    client-side decorations.
*/
/*!
    \property QWaylandXdgDecorationManagerV1::preferredMode

    This property holds the decoration mode the compositor prefers.

    This is the mode used for clients that don't indicate a preference for server-side or
    client-side decorations.
*/
QWaylandXdgToplevel::DecorationMode QWaylandXdgDecorationManagerV1::preferredMode() const
{
    Q_D(const QWaylandXdgDecorationManagerV1);
    return d->m_preferredMode;
}

void QWaylandXdgDecorationManagerV1::setPreferredMode(QWaylandXdgToplevel::DecorationMode preferredMode)
{
    Q_D(QWaylandXdgDecorationManagerV1);
    if (d->m_preferredMode == preferredMode)
        return;

    d->m_preferredMode = preferredMode;
    emit preferredModeChanged();
}

/*!
    Returns the Wayland interface for the QWaylandXdgDecorationManagerV1.
*/
const wl_interface *QWaylandXdgDecorationManagerV1::interface()
{
    return QWaylandXdgDecorationManagerV1Private::interface();
}

void QWaylandXdgDecorationManagerV1Private::zxdg_decoration_manager_v1_get_toplevel_decoration(
        Resource *resource, uint id, wl_resource *toplevelResource)
{
    Q_Q(QWaylandXdgDecorationManagerV1);

    auto *toplevel = QWaylandXdgToplevel::fromResource(toplevelResource);
    if (!toplevel) {
        qWarning() << "Couldn't find toplevel for decoration";
        return;
    }

    //TODO: verify that the xdg surface is unconfigured, and post protocol error/warning

    auto *toplevelPrivate = QWaylandXdgToplevelPrivate::get(toplevel);

    if (toplevelPrivate->m_decoration) {
        qWarning() << "zxdg_decoration_manager_v1.get_toplevel_decoration:"
                   << toplevel << "already has a decoration object, ignoring";
        //TODO: protocol error as well?
        return;
    }

    new QWaylandXdgToplevelDecorationV1(toplevel, q, resource->client(), id);
}

QWaylandXdgToplevelDecorationV1::QWaylandXdgToplevelDecorationV1(QWaylandXdgToplevel *toplevel,
                                                                 QWaylandXdgDecorationManagerV1 *manager,
                                                                 wl_client *client, int id)
    : QtWaylandServer::zxdg_toplevel_decoration_v1(client, id, /*version*/ 1)
    , m_toplevel(toplevel)
    , m_manager(manager)
{
    Q_ASSERT(toplevel);
    auto *toplevelPrivate = QWaylandXdgToplevelPrivate::get(toplevel);
    Q_ASSERT(!toplevelPrivate->m_decoration);
    toplevelPrivate->m_decoration = this;
    sendConfigure(manager->preferredMode());
}

QWaylandXdgToplevelDecorationV1::~QWaylandXdgToplevelDecorationV1()
{
    QWaylandXdgToplevelPrivate::get(m_toplevel)->m_decoration = nullptr;
}

void QWaylandXdgToplevelDecorationV1::sendConfigure(QWaylandXdgToplevelDecorationV1::DecorationMode mode)
{
    if (configuredMode() == mode)
        return;

    switch (mode) {
    case DecorationMode::ClientSideDecoration:
        send_configure(mode_client_side);
        break;
    case DecorationMode::ServerSideDecoration:
        send_configure(mode_server_side);
        break;
    default:
        qWarning() << "Illegal mode in QWaylandXdgToplevelDecorationV1::sendConfigure" << mode;
        break;
    }

    m_configuredMode = mode;
    emit m_toplevel->decorationModeChanged();
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode)
{
    Q_UNUSED(resource);
    m_clientPreferredMode = mode;
    handleClientPreferredModeChanged();
}

void QWaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_unset_mode(Resource *resource)
{
    Q_UNUSED(resource);
    m_clientPreferredMode = 0;
    handleClientPreferredModeChanged();
}

void QWaylandXdgToplevelDecorationV1::handleClientPreferredModeChanged()
{
    if (m_clientPreferredMode != m_configuredMode) {
        if (m_clientPreferredMode == 0)
            sendConfigure(m_manager->preferredMode());
        else
            sendConfigure(DecorationMode(m_clientPreferredMode));
    }
}

QT_END_NAMESPACE
