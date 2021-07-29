/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the manual tests of the Qt Toolkit.
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
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include "inputinspector.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    qmlRegisterType<InputInspector>("org.qtproject.Test", 1, 0, "InputInspector");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;
    if (!app.arguments().isEmpty()) {
        QQuickWindow * win = static_cast<QQuickWindow *>(engine.rootObjects().first());
        auto lastArg = app.arguments().last();
        if (lastArg.endsWith(QLatin1String(".qml"))) {
            auto root = win->findChild<QQuickItem *>("LauncherList");
            int showExampleIdx = -1;
            for (int i = root->metaObject()->methodCount(); showExampleIdx < 0 && i >= 0; --i)
                if (root->metaObject()->method(i).name() == QByteArray("showExample"))
                    showExampleIdx = i;
            QMetaMethod showExampleFn = root->metaObject()->method(showExampleIdx);
            showExampleFn.invoke(root, Q_ARG(QVariant, QVariant(QLatin1String("../../") + lastArg)));
        }
    }

    return app.exec();
}
