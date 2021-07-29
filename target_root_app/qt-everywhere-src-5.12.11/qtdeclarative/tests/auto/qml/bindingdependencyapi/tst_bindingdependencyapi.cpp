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
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <private/qqmldata_p.h>
#include <private/qqmlbinding_p.h>
#include <QtQuick/private/qquicktext_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include "../../shared/util.h"
#include <qqmlcontext.h>

class tst_bindingdependencyapi : public QObject
{
    Q_OBJECT
public:
    tst_bindingdependencyapi();

private slots:
    void testSingleDep_data();
    void testSingleDep();
    void testManyDeps_data();
    void testManyDeps();
    void testConditionalDependencies_data();
    void testConditionalDependencies();
    void testBindingLoop();

private:
    bool findProperties(const QVector<QQmlProperty> &properties, QObject *obj, const QString &propertyName, const QVariant &value);
};

tst_bindingdependencyapi::tst_bindingdependencyapi()
{
}


void tst_bindingdependencyapi::testSingleDep_data()
{
    QTest::addColumn<QByteArray>("code");
    QTest::addColumn<QString>("referencedObjectName");

    QTest::addRow("context property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "objectName: \"rect\"\n"
                          "property string labelText: \"Hello world!\"\n"
                          "Text { text: labelText }\n"
                      "}") << "rect";

    QTest::addRow("scope property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "property string labelText: \"I am wrong!\"\n"
                          "Text {\n"
                              "objectName: \"text\"\n"
                              "property string labelText: \"Hello world!\"\n"
                              "text: labelText\n"
                          "}\n"
                      "}") << "text";

    QTest::addRow("id object property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "id: rect\n"
                          "objectName: \"rect\"\n"
                          "property string labelText: \"Hello world!\"\n"
                          "Text { text: rect.labelText }\n"
                      "}") << "rect";

    QTest::addRow("dynamic context property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "objectName: \"rect\"\n"
                          "property string labelText: \"Hello world!\"\n"
                          "Text { Component.onCompleted: text = Qt.binding(function() { return labelText; }); }\n"
                      "}") << "rect";

    QTest::addRow("dynamic scope property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "property string labelText: \"I am wrong!\"\n"
                          "Text {\n"
                              "objectName: \"text\"\n"
                              "property string labelText: \"Hello world!\"\n"
                              "Component.onCompleted: text = Qt.binding(function() { return labelText; });\n"
                          "}\n"
                      "}") << "text";

    QTest::addRow("dynamic id object property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "id: rect\n"
                          "objectName: \"rect\"\n"
                          "property string labelText: \"Hello world!\"\n"
                          "Text { Component.onCompleted: text = Qt.binding(function() { return rect.labelText; }); }\n"
                      "}") << "rect";
}

void tst_bindingdependencyapi::testSingleDep()
{
    QFETCH(QByteArray, code);
    QFETCH(QString, referencedObjectName);

    QQmlEngine engine;
    QQmlComponent c(&engine);
    c.setData(code, QUrl());
    QObject *rect = c.create();
    QTest::qWait(10);
    QVERIFY(rect != nullptr);
    QObject *text = rect->findChildren<QQuickText *>().front();

    QObject *referencedObject = rect->objectName() == referencedObjectName ? rect : rect->findChild<QObject *>(referencedObjectName);

    auto data = QQmlData::get(text);
    QVERIFY(data);
    auto b = data->bindings;
    QVERIFY(b);
    auto binding = dynamic_cast<QQmlBinding*>(b);
    QVERIFY(binding);
    auto dependencies = binding->dependencies();
    QCOMPARE(dependencies.size(), 1);
    auto dependency = dependencies.front();
    QVERIFY(dependency.isValid());
    QCOMPARE(quintptr(dependency.object()), quintptr(referencedObject));
    QCOMPARE(dependency.property().name(), "labelText");
    QCOMPARE(dependency.read().toString(), QStringLiteral("Hello world!"));
    QCOMPARE(dependency, QQmlProperty(referencedObject, "labelText"));

    delete rect;
}

bool tst_bindingdependencyapi::findProperties(const QVector<QQmlProperty> &properties, QObject *obj, const QString &propertyName, const QVariant &value)
{
    auto dep = std::find_if(properties.cbegin(), properties.cend(), [&](const QQmlProperty &dep) {
        return dep.object() == obj
            && dep.property().name() == propertyName
            && dep.read() == value;
    });
    if (dep == properties.cend()) {
        qWarning() << "Searched for property with:" << "{ object:" << obj << ", propertyName:" << propertyName << ", value:" << value << "}" << "but only found:";
        for (auto dep : properties) {
            qWarning() << "{ object:" << dep.object() << ", propertyName:" << dep.property().name() << ", value:" << dep.read() << "}";
        }
        return false;
    }
    return true;
}

void tst_bindingdependencyapi::testManyDeps_data()
{
    QTest::addColumn<QByteArray>("code");

    QTest::addRow("permanent binding")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "id: rect\n"
                          "objectName: 'rect'\n"
                          "property string name: 'world'\n"
                          "Text {\n"
                            "text: config.helloWorldTemplate.arg(greeting).arg(rect.name) \n"
                            "property string greeting: 'Hello'\n"
                          "}\n"
                          "QtObject { id: config; objectName: 'config'; property string helloWorldTemplate: '%1 %2!' }\n"
                      "}");

    QTest::addRow("dynamic binding")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "id: rect\n"
                          "objectName: 'rect'\n"
                          "property string name: 'world'\n"
                          "Text {\n"
                            "Component.onCompleted: text = Qt.binding(function() { return config.helloWorldTemplate.arg(greeting).arg(rect.name); }); \n"
                            "property string greeting: 'Hello'\n"
                          "}\n"
                          "QtObject { id: config; objectName: 'config'; property string helloWorldTemplate: '%1 %2!' }\n"
                      "}");
}

void tst_bindingdependencyapi::testManyDeps()
{
    QFETCH(QByteArray, code);
    QQmlEngine engine;
    QQmlComponent c(&engine);
    c.setData(code, QUrl());
    QObject *rect = c.create();
    if (c.isError()) {
        qWarning() << c.errorString();
    }
    QTest::qWait(100);
    QVERIFY(rect != nullptr);
    QObject *text = rect->findChildren<QQuickText *>().front();
    QObject *configObj = rect->findChild<QObject *>("config");

    auto data = QQmlData::get(text);
    QVERIFY(data);
    auto b = data->bindings;
    QVERIFY(b);
    auto binding = dynamic_cast<QQmlBinding*>(b);
    QVERIFY(binding);
    auto dependencies = binding->dependencies();
    QCOMPARE(dependencies.size(), 3);

    QVERIFY(findProperties(dependencies, rect, "name", "world"));
    QVERIFY(findProperties(dependencies, text, "greeting", "Hello"));
    QVERIFY(findProperties(dependencies, configObj, "helloWorldTemplate", "%1 %2!"));

    delete rect;
}

void tst_bindingdependencyapi::testConditionalDependencies_data()
{
    QTest::addColumn<QByteArray>("code");
    QTest::addColumn<QString>("referencedObjectName");

    QTest::addRow("id object property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "id: rect\n"
                          "objectName: \"rect\"\n"
                          "property bool haveDep: false\n"
                          "property string labelText: \"Hello world!\"\n"
                          "Text { text: rect.haveDep ? rect.labelText : '' }\n"
                      "}") << "rect";

    QTest::addRow("dynamic context property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "objectName: \"rect\"\n"
                          "property bool haveDep: false\n"
                          "property string labelText: \"Hello world!\"\n"
                          "Text { Component.onCompleted: text = Qt.binding(function() { return haveDep ? labelText : ''; }); }\n"
                      "}") << "rect";

    QTest::addRow("dynamic scope property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "property string labelText: \"I am wrong!\"\n"
                          "Text {\n"
                              "objectName: \"text\"\n"
                              "property bool haveDep: false\n"
                              "property string labelText: \"Hello world!\"\n"
                              "Component.onCompleted: text = Qt.binding(function() { return haveDep ? labelText : ''; });\n"
                          "}\n"
                      "}") << "text";

    QTest::addRow("dynamic id object property")
        << QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "id: rect\n"
                          "objectName: \"rect\"\n"
                          "property bool haveDep: false\n"
                          "property string labelText: \"Hello world!\"\n"
                          "Text { Component.onCompleted: text = Qt.binding(function() { return rect.haveDep ? rect.labelText : ''; }); }\n"
                      "}") << "rect";
}

void tst_bindingdependencyapi::testConditionalDependencies()
{
    QFETCH(QByteArray, code);
    QFETCH(QString, referencedObjectName);

    QQmlEngine engine;
    QQmlComponent c(&engine);
    c.setData(code, QUrl());
    QObject *rect = c.create();
    QTest::qWait(10);
    QVERIFY(rect != nullptr);
    QObject *text = rect->findChildren<QQuickText *>().front();

    QObject *referencedObject = rect->objectName() == referencedObjectName ? rect : rect->findChild<QObject *>(referencedObjectName);

    auto data = QQmlData::get(text);
    QVERIFY(data);
    auto b = data->bindings;
    QVERIFY(b);
    auto binding = dynamic_cast<QQmlBinding*>(b);
    QVERIFY(binding);
    auto dependencies = binding->dependencies();
    QCOMPARE(dependencies.size(), 1);
    QVERIFY(findProperties(dependencies, referencedObject, "haveDep", false));

    referencedObject->setProperty("haveDep", true);
    dependencies = binding->dependencies();
    QCOMPARE(dependencies.size(), 2);
    QVERIFY(findProperties(dependencies, referencedObject, "haveDep", true));
    QVERIFY(findProperties(dependencies, referencedObject, "labelText", "Hello world!"));

    referencedObject->setProperty("haveDep", false);
    dependencies = binding->dependencies();
    QCOMPARE(dependencies.size(), 1);
    QVERIFY(findProperties(dependencies, referencedObject, "haveDep", false));

    delete rect;
}

void tst_bindingdependencyapi::testBindingLoop()
{
    QQmlEngine engine;
    QQmlComponent c(&engine);
    c.setData(QByteArray("import QtQuick 2.0\n"
                      "Rectangle {\n"
                          "property string labelText: label.text\n"
                          "Text {\n"
                              "id: label\n"
                              "text: labelText\n"
                          "}\n"
                      "}"), QUrl());
    QObject *rect = c.create();
    if (c.isError()) {
        qWarning() << c.errorString();
    }
    QTest::qWait(100);
    QVERIFY(rect != nullptr);
    QObject *text = rect->findChildren<QQuickText *>().front();

    auto data = QQmlData::get(text);
    QVERIFY(data);
    auto b = data->bindings;
    QVERIFY(b);
    auto binding = dynamic_cast<QQmlBinding*>(b);
    QVERIFY(binding);
    auto dependencies = binding->dependencies();
    QCOMPARE(dependencies.size(), 1);
    auto dependency = dependencies.front();
    QVERIFY(dependency.isValid());
    QCOMPARE(quintptr(dependency.object()), quintptr(rect));
    QCOMPARE(dependency.property().name(), "labelText");

    delete rect;
}

QTEST_MAIN(tst_bindingdependencyapi)

#include "tst_bindingdependencyapi.moc"
