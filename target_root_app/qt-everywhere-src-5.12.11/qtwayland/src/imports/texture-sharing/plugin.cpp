/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandClient module of the Qt Toolkit.
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

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqmlengine.h>

#include "sharedtextureprovider.h"

/*!
    \qmlmodule QtWayland.Client.TextureSharing 1
    \title Qt Wayland Shared Texture Provider
    \ingroup qmlmodules
    \brief Adds an image provider which utilizes shared GPU memory

    \section2 Summary

    This module allows Qt Wayland clients to use graphical resources exported
    by the compositor, without allocating any graphics memory in the client.
    \section2 Usage

    To use this module, import it like this:
    \code
    import QtWayland.Client.TextureSharing 1.0
    \endcode

    The sharing functionality is provided through a QQuickImageProvider. Use
    the "image:" scheme for the URL source of the image, followed by the
    identifier \e wlshared, followed by the image file path. For example:

    \code
    Image { source: "image://wlshared/wallpapers/mybackground.jpg" }
    \endcode

    The shared texture module does not provide any directly usable QML types.
*/

QT_BEGIN_NAMESPACE

class QWaylandTextureSharingPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    QWaylandTextureSharingPlugin(QObject *parent = nullptr) : QQmlExtensionPlugin(parent) {}

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(uri == QStringLiteral("QtWayland.Client.TextureSharing"));
        qmlRegisterModule(uri, 1, 0);
    }

    void initializeEngine(QQmlEngine *engine, const char *uri) override
    {
        Q_UNUSED(uri);
        engine->addImageProvider("wlshared", new SharedTextureProvider);
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
