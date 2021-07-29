/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "mockwlshell.h"
#include "mocksurface.h"

namespace Impl {

WlShellSurface::WlShellSurface(wl_client *client, int id, Surface *surface)
    : QtWaylandServer::wl_shell_surface(client, id, 1)
    , m_surface(surface)
{
    surface->m_wlShellSurface = this;
    surface->map();
}

WlShellSurface::~WlShellSurface()
{
    m_surface->m_wlShellSurface = nullptr;
}

void WlShell::shell_get_shell_surface(QtWaylandServer::wl_shell::Resource *resource, uint32_t id, wl_resource *surface)
{
    new WlShellSurface(resource->client(), id, Surface::fromResource(surface));
}

} // namespace Impl
