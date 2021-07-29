/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#ifndef QTEST_QUICKCONTROLS_H
#define QTEST_QUICKCONTROLS_H

#include <QtTest/qtest.h>
#include <QtTest/private/qtestresult_p.h>
#include <QtGui/qguiapplication.h>
#include <QtQml/qqml.h>
#include <QtQuickControls2/qquickstyle.h>

static QStringList testStyles()
{
    if (QQuickStyle::name().isEmpty())
        return QQuickStyle::availableStyles();
    return QStringList(QQuickStyle::name());
}

static int runTests(QObject *testObject, int argc, char *argv[])
{
    int res = 0;
    QTest::qInit(testObject, argc, argv);
    const QByteArray testObjectName = QTestResult::currentTestObjectName();
    // setCurrentTestObject() takes a C string, which means we must ensure
    // that the string we pass in lives long enough (i.e until the next call
    // to setCurrentTestObject()), so store the name outside of the loop.
    QByteArray testName;
    const QStringList styles = testStyles();
    for (const QString &style : styles) {
        qmlClearTypeRegistrations();
        QQuickStyle::setStyle(style);
        testName = testObjectName + "::" + style.toLocal8Bit();
        QTestResult::setCurrentTestObject(testName);
        res += QTest::qRun();
    }
    QTestResult::setCurrentTestObject(testObjectName);
    QTest::qCleanup();
    return res;
}

#define QTEST_QUICKCONTROLS_MAIN(TestCase) \
QT_BEGIN_NAMESPACE \
QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
QT_END_NAMESPACE \
int main(int argc, char *argv[]) \
{ \
    qputenv("QML_NO_TOUCH_COMPRESSION", "1"); \
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); \
    QGuiApplication app(argc, argv); \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT \
    TestCase tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return runTests(&tc, argc, argv); \
}

#endif // QTEST_QUICKCONTROLS_H
