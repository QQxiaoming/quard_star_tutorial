/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebView module of the Qt Toolkit.
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

#include "qdarwinwebview_p.h"
#include <private/qwebviewplugin_p.h>
#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class QDarwinWebViewPlugin : public QWebViewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWebViewPluginInterface_iid FILE "darwin.json")

public:
    QAbstractWebView *create(const QString &key) const override
    {
        return (key == QLatin1String("webview")) ? new QDarwinWebViewPrivate() : nullptr;
    }

    void prepare() const override
    {
#ifdef Q_OS_MACOS
        // On macOS, correct WebView / QtQuick compositing and stacking requires running
        // Qt in layer-backed mode, which again resuires rendering on the Gui thread.
        qWarning("Setting QT_MAC_WANTS_LAYER=1 and QSG_RENDER_LOOP=basic");
        qputenv("QT_MAC_WANTS_LAYER", "1");
        qputenv("QSG_RENDER_LOOP", "basic");
#endif // Q_OS_MACOS
    }
};

QT_END_NAMESPACE

#include "qdarwinwebviewplugin.moc"
