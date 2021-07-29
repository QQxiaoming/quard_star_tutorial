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

#include <QtTest/QtTest>
#include <QGuiApplication>
#include <qpa/qplatformintegrationfactory_p.h>

class tst_WlConnect : public QObject
{
    Q_OBJECT
private slots:
    void failsGracefully()
    {
        // This tests whether the Wayland platform integration will fail gracefully when it's
        // unable to connect to a compositor

        // Make sure the connection actually fails
        setenv("XDG_RUNTIME_DIR", "/dev/null", 1); // a place where there are no Wayland sockets
        setenv("WAYLAND_DISPLAY", "qt_invalid_socket", 1); // just to be sure

        QStringList arguments;
        QString platformPluginPath;
        int argc = 0;
        char **argv = nullptr; //It's not currently used by the wayland plugin
        auto *platformIntegration = QPlatformIntegrationFactory::create("wayland", arguments, argc, argv, platformPluginPath);

        // The factory method should return nullptr to signify it failed gracefully
        Q_ASSERT(!platformIntegration);
    }
};

QTEST_APPLESS_MAIN(tst_WlConnect)
#include <tst_wlconnect.moc>
