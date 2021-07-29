/****************************************************************************
**
** Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
** Contact: http://www.qt-project.org/legal
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

#include "qwaylandivishellintegration.h"

#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>

#include "qwaylandivisurface_p.h"

#include <unistd.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandIviShellIntegration::QWaylandIviShellIntegration()
{
}

bool QWaylandIviShellIntegration::initialize(QWaylandDisplay *display)
{
    for (QWaylandDisplay::RegistryGlobal global : display->globals()) {
        if (global.interface == QLatin1String("ivi_application") && !m_iviApplication)
            m_iviApplication.reset(new QtWayland::ivi_application(display->wl_registry(), global.id, global.version));
        if (global.interface == QLatin1String("ivi_controller") && !m_iviController)
            m_iviController.reset(new QtWayland::ivi_controller(display->wl_registry(), global.id, global.version));
    }

    if (!m_iviApplication) {
        qCDebug(lcQpaWayland) << "Couldn't find global ivi_application for ivi-shell";
        return false;
    }

    return QWaylandShellIntegration::initialize(display);
}

/* get unique id
 * pattern1:
 *   When set QT_IVI_SURFACE_ID, We use it as ID.
 *   Next ID is increment.
 * pattern2:
 *   When not set QT_IVI_SURFACE_ID, We use process ID and unused bit.
 *   process ID maximum is 2^22. Unused bit is 23 to 32 bit.
 *   Therefor, We use 23 to 32 bit. This do not overlap with other clients.
 *   Next ID is increment of 23 to 32 bit.
 *   +------------+---------------------------+
 *   |31        23|22                        0|
 *   +------------+---------------------------+
 *   |0000 0000 00|00 0000 0000 0000 0000 0000|
 *   |<- ID     ->|<- process ID            ->|
 *   +------------+---------------------------+
 */
uint32_t QWaylandIviShellIntegration::getNextUniqueSurfaceId()
{
    const uint32_t PID_MAX_EXPONENTIATION = 22; // 22 bit shift operation
    const uint32_t ID_LIMIT = 1 << (32 - PID_MAX_EXPONENTIATION); // 10 bit is unique id
    QMutexLocker locker(&m_mutex);

    if (m_lastSurfaceId == 0) {
        QByteArray env = qgetenv("QT_IVI_SURFACE_ID");
        bool ok;
        m_lastSurfaceId = env.toUInt(&ok, 10);
        if (ok)
            m_useEnvSurfaceId = true;
        else
            m_lastSurfaceId = getpid();

        return m_lastSurfaceId;
    }

    if (m_useEnvSurfaceId) {
        m_lastSurfaceId++;
    } else {
        m_surfaceNumber++;
        if (m_surfaceNumber >= ID_LIMIT) {
            qWarning("IVI surface id counter overflow\n");
            return 0;
        }
        m_lastSurfaceId += (m_surfaceNumber << PID_MAX_EXPONENTIATION);
    }

    return m_lastSurfaceId;
}

QWaylandShellSurface *QWaylandIviShellIntegration::createShellSurface(QWaylandWindow *window)
{
    if (!m_iviApplication)
        return nullptr;

    uint32_t surfaceId = getNextUniqueSurfaceId();
    if (surfaceId == 0)
        return nullptr;

    struct ivi_surface *surface = m_iviApplication->surface_create(surfaceId, window->object());
    if (!m_iviController)
        return new QWaylandIviSurface(surface, window);

    struct ::ivi_controller_surface *controller = m_iviController->ivi_controller::surface_create(surfaceId);
    QWaylandIviSurface *iviSurface = new QWaylandIviSurface(surface, window, controller);

    if (window->window()->type() == Qt::Popup) {
        QPoint transientPos = window->geometry().topLeft(); // this is absolute
        QWaylandWindow *parent = window->transientParent();
        if (parent && parent->decoration()) {
            transientPos -= parent->geometry().topLeft();
            transientPos.setX(transientPos.x() + parent->decoration()->margins().left());
            transientPos.setY(transientPos.y() + parent->decoration()->margins().top());
        }
        QSize size = window->window()->geometry().size();
        iviSurface->ivi_controller_surface::set_destination_rectangle(transientPos.x(),
                                                                      transientPos.y(),
                                                                      size.width(),
                                                                      size.height());
    }

    return iviSurface;
}

}

QT_END_NAMESPACE
