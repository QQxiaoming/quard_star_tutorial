/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef QWAYLANDXDGDECORATIONV1_P_H
#define QWAYLANDXDGDECORATIONV1_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qwayland-xdg-decoration-unstable-v1.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandXdgToplevel;
class QWaylandXdgToplevelDecorationV1;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgDecorationManagerV1 : public QtWayland::zxdg_decoration_manager_v1
{
public:
    QWaylandXdgDecorationManagerV1(struct ::wl_registry *registry, uint32_t id, uint32_t availableVersion);
    ~QWaylandXdgDecorationManagerV1() override;
    QWaylandXdgToplevelDecorationV1 *createToplevelDecoration(::xdg_toplevel *toplevel);
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgToplevelDecorationV1 : public QtWayland::zxdg_toplevel_decoration_v1
{
public:
    QWaylandXdgToplevelDecorationV1(::zxdg_toplevel_decoration_v1 *decoration);
    ~QWaylandXdgToplevelDecorationV1() override;
    void requestMode(mode mode);
    void unsetMode();
    mode pending() const;

protected:
    void zxdg_toplevel_decoration_v1_configure(uint32_t mode) override;

private:
    mode m_pending = mode_client_side;
    mode m_requested = mode_client_side;
    bool m_modeSet = false;
};

QT_END_NAMESPACE

}

#endif // QWAYLANDXDGDECORATIONV1_P_H
