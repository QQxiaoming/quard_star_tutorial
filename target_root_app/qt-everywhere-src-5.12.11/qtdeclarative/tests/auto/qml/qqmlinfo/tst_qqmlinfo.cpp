/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include <qtest.h>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QTimer>
#include <QQmlContext>
#include <qqmlinfo.h>
#include "../../shared/util.h"

class tst_qqmlinfo : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_qqmlinfo() {}

private slots:
    void qmlObject();
    void nestedQmlObject();
    void nestedComponent();
    void nonQmlObject();
    void nullObject();
    void nonQmlContextedObject();
    void types();
    void chaining();
    void messageTypes();
    void component();

private:
    QQmlEngine engine;
};

void tst_qqmlinfo::qmlObject()
{
    QQmlComponent component(&engine, testFileUrl("qmlObject.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QString message = component.url().toString() + ":3:1: QML QtObject: Test Message";
    QTest::ignoreMessage(QtInfoMsg, qPrintable(message));
    qmlInfo(object) << "Test Message";

    QObject *nested = qvariant_cast<QObject *>(object->property("nested"));
    QVERIFY(nested != nullptr);

    message = component.url().toString() + ":6:13: QML QtObject: Second Test Message";
    QTest::ignoreMessage(QtInfoMsg, qPrintable(message));
    qmlInfo(nested) << "Second Test Message";
}

void tst_qqmlinfo::nestedQmlObject()
{
    QQmlComponent component(&engine, testFileUrl("nestedQmlObject.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QObject *nested = qvariant_cast<QObject *>(object->property("nested"));
    QVERIFY(nested != nullptr);
    QObject *nested2 = qvariant_cast<QObject *>(object->property("nested2"));
    QVERIFY(nested2 != nullptr);

    QString message = component.url().toString() + ":5:13: QML NestedObject: Outer Object";
    QTest::ignoreMessage(QtInfoMsg, qPrintable(message));
    qmlInfo(nested) << "Outer Object";

    message = testFileUrl("NestedObject.qml").toString() + ":6:14: QML QtObject: Inner Object";
    QTest::ignoreMessage(QtInfoMsg, qPrintable(message));
    qmlInfo(nested2) << "Inner Object";
}

void tst_qqmlinfo::nestedComponent()
{
    QQmlComponent component(&engine, testFileUrl("NestedComponent.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QObject *nested = qvariant_cast<QObject *>(object->property("nested"));
    QVERIFY(nested != nullptr);
    QObject *nested2 = qvariant_cast<QObject *>(object->property("nested2"));
    QVERIFY(nested2 != nullptr);

    QString message = component.url().toString() + ":10:9: QML NestedObject: Complex Object";
    QTest::ignoreMessage(QtInfoMsg, qPrintable(message));
    qmlInfo(nested) << "Complex Object";

    message = component.url().toString() + ":16:9: QML Image: Simple Object";
    QTest::ignoreMessage(QtInfoMsg, qPrintable(message));
    qmlInfo(nested2) << "Simple Object";
}

void tst_qqmlinfo::nonQmlObject()
{
    QObject object;
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: QML QtObject: Test Message");
    qmlInfo(&object) << "Test Message";

    QTimer nonQmlObject;
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: QML QTimer: Test Message");
    qmlInfo(&nonQmlObject) << "Test Message";
}

void tst_qqmlinfo::nullObject()
{
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: Null Object Test Message");
    qmlInfo(nullptr) << "Null Object Test Message";
}

void tst_qqmlinfo::nonQmlContextedObject()
{
    QObject object;
    QQmlContext context(&engine);
    QQmlEngine::setContextForObject(&object, &context);
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: QML QtObject: Test Message");
    qmlInfo(&object) << "Test Message";
}

void tst_qqmlinfo::types()
{
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: false");
    qmlInfo(nullptr) << false;

    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: 1.1");
    qmlInfo(nullptr) << 1.1;

    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: 1.2");
    qmlInfo(nullptr) << 1.2f;

    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: 15");
    qmlInfo(nullptr) << 15;

    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: 'b'");
    qmlInfo(nullptr) << QChar('b');

    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: \"Qt\"");
    qmlInfo(nullptr) << QByteArray("Qt");

    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: true");
    qmlInfo(nullptr) << bool(true);

    //### do we actually want QUrl to show up in the output?
    //### why the extra space at the end?
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: QUrl(\"http://www.qt-project.org\") ");
    qmlInfo(nullptr) << QUrl("http://www.qt-project.org");

    //### should this be quoted?
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: hello");
    qmlInfo(nullptr) << QLatin1String("hello");

    //### should this be quoted?
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: World");
    QString str("Hello World");
    QStringRef ref(&str, 6, 5);
    qmlInfo(nullptr) << ref;

    //### should this be quoted?
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: Quick");
    qmlInfo(nullptr) << QString ("Quick");
}

void tst_qqmlinfo::chaining()
{
    QString str("Hello World");
    QStringRef ref(&str, 6, 5);
    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: false 1.1 1.2 15 hello 'b' World \"Qt\" true Quick QUrl(\"http://www.qt-project.org\") ");
    qmlInfo(nullptr) << false << ' '
               << 1.1 << ' '
               << 1.2f << ' '
               << 15 << ' '
               << QLatin1String("hello") << ' '
               << QChar('b') << ' '
               << ref << ' '
               << QByteArray("Qt") << ' '
               << bool(true) << ' '
               << QString ("Quick") << ' '
               << QUrl("http://www.qt-project.org");
}

// Ensure that messages of different types are sent with the correct QtMsgType.
void tst_qqmlinfo::messageTypes()
{
    QTest::ignoreMessage(QtDebugMsg, "<Unknown File>: debug");
    qmlDebug(nullptr) << QLatin1String("debug");

    QTest::ignoreMessage(QtInfoMsg, "<Unknown File>: info");
    qmlInfo(nullptr) << QLatin1String("info");

    QTest::ignoreMessage(QtWarningMsg, "<Unknown File>: warning");
    qmlWarning(nullptr) << QLatin1String("warning");
}

void tst_qqmlinfo::component()
{
    QQmlComponent component(&engine, testFileUrl("Component.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);
    QQmlComponent *delegate = qobject_cast<QQmlComponent*>(object->property("delegate").value<QObject*>());
    QVERIFY(delegate);

    QString message = component.url().toString() + ":4:34: QML Component: Delegate error";
    QTest::ignoreMessage(QtInfoMsg, qPrintable(message));
    qmlInfo(delegate) << "Delegate error";
}

QTEST_MAIN(tst_qqmlinfo)

#include "tst_qqmlinfo.moc"
