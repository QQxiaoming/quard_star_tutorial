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

#include <QtTest/qtest.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcontext.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>
#include <QtQuickTest/quicktest.h>

#include "../../shared/util.h"

class QmlRegisterTypeCppType : public QObject
{
    Q_OBJECT

public:
    QmlRegisterTypeCppType() {}
};

class CustomTestSetup : public QObject
{
    Q_OBJECT

public:
    CustomTestSetup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine *qmlEngine)
    {
        // Test that modules are successfully imported by the TestCaseCollector that
        // parses the QML files (but doesn't run them). For that to happen, qmlEngineAvailable()
        // must be called before TestCaseCollector does its thing.
        qmlRegisterType<QmlRegisterTypeCppType>("QmlRegisterTypeCppModule", 1, 0, "QmlRegisterTypeCppType");
        qmlEngine->addImportPath(QString::fromUtf8(QT_QMLTEST_DATADIR) + "/../imports");

        qmlEngine->rootContext()->setContextProperty("qmlEngineAvailableCalled", true);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(qquicktestsetup, CustomTestSetup)

#include "tst_quicktestmainwithsetup.moc"
