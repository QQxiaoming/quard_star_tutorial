/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
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

#include <QtWaylandCompositor/QWaylandShellSurface>

/*!
 * \qmltype ShellSurface
 * \inqmlmodule QtWayland.Compositor
 * \since 5.8
 * \brief Provides a common interface for surface roles specified by shell extensions.
 *
 * This interface represents a Wayland surface role given by a Wayland protocol extension that
 * defines how the WaylandSurface should map onto the screen.
 *
 * Note: Even though this type contains a very limited API, the properties and signals of the
 * implementations are named consistently. For example, if you're only using desktop shell
 * extensions in your compositor, it's safe to access properties such as title, maximized, etc.
 * directly on the ShellSurface. See the various implementations for additional properties and
 * signals.
 *
 * \sa WaylandSurface, ShellSurfaceItem, WlShellSurface, XdgSurfaceV5, IviSurface
 */

/*!
 * \class QWaylandShellSurface
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandShellSurface class is a common interface for surface roles specified by shell extensions.
 *
 * This interface represents a Wayland surface role given by a Wayland protocol extension that
 * defines how the QWaylandSurface should map onto the screen.
 *
 * \sa QWaylandSurface, QWaylandWlShellSurface, QWaylandXdgSurfaceV5, QWaylandIviSurface
 */

#if QT_CONFIG(wayland_compositor_quick)
/*!
 * \fn QWaylandQuickShellIntegration *QWaylandShellSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
 *
 * Creates a QWaylandQuickShellIntegration for this QWaylandQuickShellSurface. It's called
 * automatically when QWaylandQuickShellSurfaceItem::shellSurface is assigned.
 *
 * \sa QWaylandQuickShellSurfaceItem
 */
#endif

/*!
 * \qmlproperty enum QtWaylandCompositor::ShellSurface::windowType
 *
 * This property holds the window type of the ShellSurface.
 */

/*!
 * \property QWaylandWlShellSurface::windowType
 *
 * This property holds the window type of the QWaylandShellSurface.
 */
