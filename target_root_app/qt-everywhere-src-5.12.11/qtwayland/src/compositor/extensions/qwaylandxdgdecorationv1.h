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

#ifndef QWAYLANDXDGDECORATIONV1_H
#define QWAYLANDXDGDECORATIONV1_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandXdgToplevel>

QT_BEGIN_NAMESPACE

class QWaylandXdgDecorationManagerV1Private;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgDecorationManagerV1 : public QWaylandCompositorExtensionTemplate<QWaylandXdgDecorationManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgDecorationManagerV1)
    Q_PROPERTY(QWaylandXdgToplevel::DecorationMode preferredMode READ preferredMode WRITE setPreferredMode NOTIFY preferredModeChanged)

public:
    explicit QWaylandXdgDecorationManagerV1();

    void initialize() override;

    QWaylandXdgToplevel::DecorationMode preferredMode() const;
    void setPreferredMode(QWaylandXdgToplevel::DecorationMode preferredMode);

    static const struct wl_interface *interface();

Q_SIGNALS:
    void preferredModeChanged();
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGDECORATIONV1_H
