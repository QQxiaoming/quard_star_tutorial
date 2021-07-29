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

#include <qwayland-server-wayland.h>

#ifndef MOCKWLSHELL_H
#define MOCKWLSHELL_H

namespace Impl {

class Surface;

class WlShellSurface : public QtWaylandServer::wl_shell_surface
{
public:
    explicit WlShellSurface(::wl_client *client, int id, Surface *surface);
    ~WlShellSurface() override;
    void shell_surface_destroy_resource(Resource *) override { delete this; }

private:
    Surface *m_surface = nullptr;
};

class WlShell : public QtWaylandServer::wl_shell
{
public:
    explicit WlShell(::wl_display *display) : wl_shell(display, 1) {}
    void shell_get_shell_surface(Resource *resource, uint32_t id, ::wl_resource *surface) override;
};

} // namespace Impl

#endif // MOCKWLSHELL_H
