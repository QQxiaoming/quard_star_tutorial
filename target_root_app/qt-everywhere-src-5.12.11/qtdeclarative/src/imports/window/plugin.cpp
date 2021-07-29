/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
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

#include <QtQml/qqmlextensionplugin.h>

#include <private/qquickwindowmodule_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmlmodule QtQuick.Window 2.\QtMinorVersion
    \title Qt Quick Window QML Types
    \ingroup qmlmodules
    \brief Provides QML types for window management

    This QML module contains types for creating top-level windows and accessing screen information.

    To use the types in this module, import the module with the following line:

    \qml \QtMinorVersion
    import QtQuick.Window 2.\1
    \endqml
*/


//![class decl]
class QtQuick2WindowPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    QtQuick2WindowPlugin(QObject *parent = nullptr) : QQmlExtensionPlugin(parent) { }
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtQuick.Window"));
        QQuickWindowModule::defineModule();

        // Auto-increment the import to stay in sync with ALL future QtQuick minor versions from 5.11 onward
        qmlRegisterModule(uri, 2, QT_VERSION_MINOR);
    }
};
//![class decl]

QT_END_NAMESPACE

#include "plugin.moc"
