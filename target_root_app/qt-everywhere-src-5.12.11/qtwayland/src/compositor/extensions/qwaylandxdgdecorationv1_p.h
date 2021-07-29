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

#ifndef QWAYLANDXDGDECORATIONV1_P_H
#define QWAYLANDXDGDECORATIONV1_P_H

#include "qwaylandxdgdecorationv1.h"

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-xdg-decoration-unstable-v1.h>

#include <QtWaylandCompositor/QWaylandXdgToplevel>

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

QT_BEGIN_NAMESPACE

class QWaylandXdgToplevel;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgDecorationManagerV1Private
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::zxdg_decoration_manager_v1
{
    Q_DECLARE_PUBLIC(QWaylandXdgDecorationManagerV1)
public:
    using DecorationMode = QWaylandXdgToplevel::DecorationMode;
    explicit QWaylandXdgDecorationManagerV1Private() {}

protected:
    void zxdg_decoration_manager_v1_get_toplevel_decoration(Resource *resource, uint id, ::wl_resource *toplevelResource) override;

private:
    DecorationMode m_preferredMode = DecorationMode::ClientSideDecoration;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgToplevelDecorationV1
        : public QtWaylandServer::zxdg_toplevel_decoration_v1
{
public:
    using DecorationMode = QWaylandXdgToplevel::DecorationMode;
    explicit QWaylandXdgToplevelDecorationV1(QWaylandXdgToplevel *toplevel,
                                             QWaylandXdgDecorationManagerV1 *manager,
                                             wl_client *client, int id);
    ~QWaylandXdgToplevelDecorationV1() override;

    DecorationMode configuredMode() const { return DecorationMode(m_configuredMode); }
    void sendConfigure(DecorationMode mode);

protected:
    void zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_destroy(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode) override;
    void zxdg_toplevel_decoration_v1_unset_mode(Resource *resource) override;

private:
    void handleClientPreferredModeChanged();

    QWaylandXdgToplevel *m_toplevel = nullptr;
    QWaylandXdgDecorationManagerV1 *m_manager = nullptr;
    uint m_configuredMode = 0;
    uint m_clientPreferredMode = 0;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGDECORATIONV1_P_H
