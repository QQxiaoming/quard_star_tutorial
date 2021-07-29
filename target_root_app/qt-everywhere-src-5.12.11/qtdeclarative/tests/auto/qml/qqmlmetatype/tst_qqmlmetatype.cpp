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

#include <qstandardpaths.h>
#include <qtest.h>
#include <qqml.h>
#include <qqmlprivate.h>
#include <qqmlengine.h>
#include <qqmlcomponent.h>

#include <private/qqmlmetatype_p.h>
#include <private/qqmlpropertyvalueinterceptor_p.h>
#include <private/qqmlengine_p.h>
#include <private/qhashedstring_p.h>
#include "../../shared/util.h"

class tst_qqmlmetatype : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_qqmlmetatype() {}

private slots:
    void initTestCase();

    void qmlParserStatusCast();
    void qmlPropertyValueSourceCast();
    void qmlPropertyValueInterceptorCast();
    void qmlType();
    void invalidQmlTypeName();
    void prettyTypeName();
    void registrationType();
    void compositeType();
    void externalEnums();

    void isList();

    void defaultObject();
    void unregisterCustomType();
    void unregisterCustomSingletonType();

    void normalizeUrls();
    void unregisterAttachedProperties();
    void revisionedGroupedProperties();

    void enumsInRecursiveImport_data();
    void enumsInRecursiveImport();
};

class TestType : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int foo READ foo)

    Q_CLASSINFO("DefaultProperty", "foo")
public:
    int foo() { return 0; }
};
QML_DECLARE_TYPE(TestType);

class TestType2 : public QObject
{
    Q_OBJECT
};

class TestType3 : public QObject
{
    Q_OBJECT
};

class ExternalEnums : public QObject
{
    Q_OBJECT
    Q_ENUMS(QStandardPaths::StandardLocation QStandardPaths::LocateOptions)
public:
    ExternalEnums(QObject *parent = nullptr) : QObject(parent) {}

    static QObject *create(QQmlEngine *engine, QJSEngine *scriptEngine) {
        Q_UNUSED(scriptEngine);
        return new ExternalEnums(engine);
    }
};
QML_DECLARE_TYPE(ExternalEnums);

QObject *testTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);
    return new TestType();
}

class ParserStatusTestType : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    void classBegin(){}
    void componentComplete(){}
    Q_CLASSINFO("DefaultProperty", "foo") // Missing default property
    Q_INTERFACES(QQmlParserStatus)
};
QML_DECLARE_TYPE(ParserStatusTestType);

class ValueSourceTestType : public QObject, public QQmlPropertyValueSource
{
    Q_OBJECT
    Q_INTERFACES(QQmlPropertyValueSource)
public:
    virtual void setTarget(const QQmlProperty &) {}
};
QML_DECLARE_TYPE(ValueSourceTestType);

class ValueInterceptorTestType : public QObject, public QQmlPropertyValueInterceptor
{
    Q_OBJECT
    Q_INTERFACES(QQmlPropertyValueInterceptor)
public:
    virtual void setTarget(const QQmlProperty &) {}
    virtual void write(const QVariant &) {}
};
QML_DECLARE_TYPE(ValueInterceptorTestType);

void tst_qqmlmetatype::initTestCase()
{
    QQmlDataTest::initTestCase();
    qmlRegisterType<TestType>("Test", 1, 0, "TestType");
    qmlRegisterSingletonType<TestType>("Test", 1, 0, "TestTypeSingleton", testTypeProvider);
    qmlRegisterType<ParserStatusTestType>("Test", 1, 0, "ParserStatusTestType");
    qmlRegisterType<ValueSourceTestType>("Test", 1, 0, "ValueSourceTestType");
    qmlRegisterType<ValueInterceptorTestType>("Test", 1, 0, "ValueInterceptorTestType");

    QUrl testTypeUrl(testFileUrl("CompositeType.qml"));
    qmlRegisterType(testTypeUrl, "Test", 1, 0, "TestTypeComposite");
}

void tst_qqmlmetatype::qmlParserStatusCast()
{
    QVERIFY(!QQmlMetaType::qmlType(QVariant::Int).isValid());
    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<TestType *>()).isValid());
    QCOMPARE(QQmlMetaType::qmlType(qMetaTypeId<TestType *>()).parserStatusCast(), -1);
    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<ValueSourceTestType *>()).isValid());
    QCOMPARE(QQmlMetaType::qmlType(qMetaTypeId<ValueSourceTestType *>()).parserStatusCast(), -1);

    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<ParserStatusTestType *>()).isValid());
    int cast = QQmlMetaType::qmlType(qMetaTypeId<ParserStatusTestType *>()).parserStatusCast();
    QVERIFY(cast != -1);
    QVERIFY(cast != 0);

    ParserStatusTestType t;
    QVERIFY(reinterpret_cast<char *>((QObject *)&t) != reinterpret_cast<char *>((QQmlParserStatus *)&t));

    QQmlParserStatus *status = reinterpret_cast<QQmlParserStatus *>(reinterpret_cast<char *>((QObject *)&t) + cast);
    QCOMPARE(status, (QQmlParserStatus*)&t);
}

void tst_qqmlmetatype::qmlPropertyValueSourceCast()
{
    QVERIFY(!QQmlMetaType::qmlType(QVariant::Int).isValid());
    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<TestType *>()).isValid());
    QCOMPARE(QQmlMetaType::qmlType(qMetaTypeId<TestType *>()).propertyValueSourceCast(), -1);
    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<ParserStatusTestType *>()).isValid());
    QCOMPARE(QQmlMetaType::qmlType(qMetaTypeId<ParserStatusTestType *>()).propertyValueSourceCast(), -1);

    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<ValueSourceTestType *>()).isValid());
    int cast = QQmlMetaType::qmlType(qMetaTypeId<ValueSourceTestType *>()).propertyValueSourceCast();
    QVERIFY(cast != -1);
    QVERIFY(cast != 0);

    ValueSourceTestType t;
    QVERIFY(reinterpret_cast<char *>((QObject *)&t) != reinterpret_cast<char *>((QQmlPropertyValueSource *)&t));

    QQmlPropertyValueSource *source = reinterpret_cast<QQmlPropertyValueSource *>(reinterpret_cast<char *>((QObject *)&t) + cast);
    QCOMPARE(source, (QQmlPropertyValueSource*)&t);
}

void tst_qqmlmetatype::qmlPropertyValueInterceptorCast()
{
    QVERIFY(!QQmlMetaType::qmlType(QVariant::Int).isValid());
    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<TestType *>()).isValid());
    QCOMPARE(QQmlMetaType::qmlType(qMetaTypeId<TestType *>()).propertyValueInterceptorCast(), -1);
    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<ParserStatusTestType *>()).isValid());
    QCOMPARE(QQmlMetaType::qmlType(qMetaTypeId<ParserStatusTestType *>()).propertyValueInterceptorCast(), -1);

    QVERIFY(QQmlMetaType::qmlType(qMetaTypeId<ValueInterceptorTestType *>()).isValid());
    int cast = QQmlMetaType::qmlType(qMetaTypeId<ValueInterceptorTestType *>()).propertyValueInterceptorCast();
    QVERIFY(cast != -1);
    QVERIFY(cast != 0);

    ValueInterceptorTestType t;
    QVERIFY(reinterpret_cast<char *>((QObject *)&t) != reinterpret_cast<char *>((QQmlPropertyValueInterceptor *)&t));

    QQmlPropertyValueInterceptor *interceptor = reinterpret_cast<QQmlPropertyValueInterceptor *>(reinterpret_cast<char *>((QObject *)&t) + cast);
    QCOMPARE(interceptor, (QQmlPropertyValueInterceptor*)&t);
}

void tst_qqmlmetatype::qmlType()
{
    QQmlType type = QQmlMetaType::qmlType(QString("ParserStatusTestType"), QString("Test"), 1, 0);
    QVERIFY(type.isValid());
    QVERIFY(type.module() == QLatin1String("Test"));
    QVERIFY(type.elementName() == QLatin1String("ParserStatusTestType"));
    QCOMPARE(type.qmlTypeName(), QLatin1String("Test/ParserStatusTestType"));

    type = QQmlMetaType::qmlType("Test/ParserStatusTestType", 1, 0);
    QVERIFY(type.isValid());
    QVERIFY(type.module() == QLatin1String("Test"));
    QVERIFY(type.elementName() == QLatin1String("ParserStatusTestType"));
    QCOMPARE(type.qmlTypeName(), QLatin1String("Test/ParserStatusTestType"));
}

void tst_qqmlmetatype::invalidQmlTypeName()
{
    QTest::ignoreMessage(QtWarningMsg, "Invalid QML element name \"testtype\"; type names must begin with an uppercase letter");
    QTest::ignoreMessage(QtWarningMsg, "Invalid QML element name \"Test$Type\"");
    QTest::ignoreMessage(QtWarningMsg, "Invalid QML element name \"EndingInSlash/\"");

    QCOMPARE(qmlRegisterType<TestType>("TestNamespace", 1, 0, "Test$Type"), -1); // should fail due to invalid QML type name.
    QCOMPARE(qmlRegisterType<TestType>("Test", 1, 0, "EndingInSlash/"), -1);
    QCOMPARE(qmlRegisterType<TestType>("Test", 1, 0, "testtype"), -1);
}

void tst_qqmlmetatype::prettyTypeName()
{
    TestType2 obj2;
    QCOMPARE(QQmlMetaType::prettyTypeName(&obj2), QString("TestType2"));
    QVERIFY(qmlRegisterType<TestType2>("Test", 1, 0, "") >= 0);
    QCOMPARE(QQmlMetaType::prettyTypeName(&obj2), QString("TestType2"));

    TestType3 obj3;
    QCOMPARE(QQmlMetaType::prettyTypeName(&obj3), QString("TestType3"));
    QVERIFY(qmlRegisterType<TestType3>("Test", 1, 0, "OtherName") >= 0);
    QCOMPARE(QQmlMetaType::prettyTypeName(&obj3), QString("OtherName"));
}

void tst_qqmlmetatype::isList()
{
    QCOMPARE(QQmlMetaType::isList(QVariant::Invalid), false);
    QCOMPARE(QQmlMetaType::isList(QVariant::Int), false);

    QQmlListProperty<TestType> list;

    QCOMPARE(QQmlMetaType::isList(qMetaTypeId<QQmlListProperty<TestType> >()), true);
}

void tst_qqmlmetatype::defaultObject()
{
    QVERIFY(!QQmlMetaType::defaultProperty(&QObject::staticMetaObject).name());
    QVERIFY(!QQmlMetaType::defaultProperty(&ParserStatusTestType::staticMetaObject).name());
    QCOMPARE(QString(QQmlMetaType::defaultProperty(&TestType::staticMetaObject).name()), QString("foo"));

    QObject o;
    TestType t;
    ParserStatusTestType p;

    QVERIFY(QQmlMetaType::defaultProperty((QObject *)nullptr).name() == nullptr);
    QVERIFY(!QQmlMetaType::defaultProperty(&o).name());
    QVERIFY(!QQmlMetaType::defaultProperty(&p).name());
    QCOMPARE(QString(QQmlMetaType::defaultProperty(&t).name()), QString("foo"));
}

void tst_qqmlmetatype::registrationType()
{
    QQmlType type = QQmlMetaType::qmlType(QString("TestType"), QString("Test"), 1, 0);
    QVERIFY(type.isValid());
    QVERIFY(!type.isInterface());
    QVERIFY(!type.isSingleton());
    QVERIFY(!type.isComposite());

    type = QQmlMetaType::qmlType(QString("TestTypeSingleton"), QString("Test"), 1, 0);
    QVERIFY(type.isValid());
    QVERIFY(!type.isInterface());
    QVERIFY(type.isSingleton());
    QVERIFY(!type.isComposite());

    type = QQmlMetaType::qmlType(QString("TestTypeComposite"), QString("Test"), 1, 0);
    QVERIFY(type.isValid());
    QVERIFY(!type.isInterface());
    QVERIFY(!type.isSingleton());
    QVERIFY(type.isComposite());
}

void tst_qqmlmetatype::compositeType()
{
    QQmlEngine engine;

    //Loading the test file also loads all composite types it imports
    QQmlComponent c(&engine, testFileUrl("testImplicitComposite.qml"));
    QObject* obj = c.create();
    QVERIFY(obj);

    QQmlType type = QQmlMetaType::qmlType(QString("ImplicitType"), QString(""), 1, 0);
    QVERIFY(type.isValid());
    QVERIFY(type.module().isEmpty());
    QCOMPARE(type.elementName(), QLatin1String("ImplicitType"));
    QCOMPARE(type.qmlTypeName(), QLatin1String("ImplicitType"));
    QCOMPARE(type.sourceUrl(), testFileUrl("ImplicitType.qml"));
}

void tst_qqmlmetatype::externalEnums()
{
    QQmlEngine engine;
    qmlRegisterSingletonType<ExternalEnums>("x.y.z", 1, 0, "ExternalEnums", ExternalEnums::create);

    QQmlComponent c(&engine, testFileUrl("testExternalEnums.qml"));
    QObject *obj = c.create();
    QVERIFY(obj);
    QVariant a = obj->property("a");
    QCOMPARE(a.type(), QVariant::Int);
    QCOMPARE(a.toInt(), int(QStandardPaths::DocumentsLocation));
    QVariant b = obj->property("b");
    QCOMPARE(b.type(), QVariant::Int);
    QCOMPARE(b.toInt(), int(QStandardPaths::DocumentsLocation));

}

class Controller1 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString string MEMBER m_string)
    Q_PROPERTY(Controller1Enum enumVal MEMBER m_enumVal)
public:
    enum Controller1Enum {
        ENUM_VALUE_1 = 1,
        ENUM_VALUE_2 = 2
    };
    Q_ENUMS(Controller1Enum)

    Controller1(QObject *parent = nullptr) : QObject(parent), m_string("Controller #1"),
        m_enumVal(ENUM_VALUE_1)
    {}
private:
    QString m_string;
    Controller1Enum m_enumVal;
};

class Controller2 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString string MEMBER m_string)
    Q_PROPERTY(Controller2Enum enumVal MEMBER m_enumVal)
public:
    enum Controller2Enum {
        ENUM_VALUE_1 = 111,
        ENUM_VALUE_2 = 222
    };
    Q_ENUMS(Controller2Enum)

    Controller2(QObject *parent = nullptr) : QObject(parent), m_string("Controller #2"),
        m_enumVal(ENUM_VALUE_1)
    {}
private:
    QString m_string;
    Controller2Enum m_enumVal;
};

void tst_qqmlmetatype::unregisterCustomType()
{
    int controllerId = 0;
    {
        QQmlEngine engine;
        QQmlType type = QQmlMetaType::qmlType(QString("Controller"), QString("mytypes"), 1, 0);
        QVERIFY(!type.isValid());
        controllerId = qmlRegisterType<Controller1>("mytypes", 1, 0, "Controller");
        type = QQmlMetaType::qmlType(QString("Controller"), QString("mytypes"), 1, 0);
        QVERIFY(type.isValid());
        QVERIFY(!type.isInterface());
        QVERIFY(!type.isSingleton());
        QVERIFY(!type.isComposite());
        QQmlComponent c(&engine, testFileUrl("testUnregisterCustomType.qml"));
        QScopedPointer<QObject> obj(c.create());
        QVERIFY(obj);
        QObject *controller = obj->findChild<QObject *>("controller");
        QVERIFY(qobject_cast<Controller1 *>(controller));
        QVariant stringVal = controller->property("string");
        QCOMPARE(stringVal.type(), QVariant::String);
        QCOMPARE(stringVal.toString(), QStringLiteral("Controller #1"));
        QVariant enumVal = controller->property("enumVal");
        QCOMPARE(enumVal.type(), QVariant::Int);
        QCOMPARE(enumVal.toInt(), 1);
    }
    qmlUnregisterType(controllerId);
    {
        QQmlEngine engine;
        QQmlType type = QQmlMetaType::qmlType(QString("Controller"), QString("mytypes"), 1, 0);
        QVERIFY(!type.isValid());
        controllerId = qmlRegisterType<Controller2>("mytypes", 1, 0, "Controller");
        type = QQmlMetaType::qmlType(QString("Controller"), QString("mytypes"), 1, 0);
        QVERIFY(type.isValid());
        QVERIFY(!type.isInterface());
        QVERIFY(!type.isSingleton());
        QVERIFY(!type.isComposite());
        QQmlComponent c(&engine, testFileUrl("testUnregisterCustomType.qml"));
        QScopedPointer<QObject> obj(c.create());
        QVERIFY(obj);
        QObject *controller = obj->findChild<QObject *>("controller");
        QVERIFY(qobject_cast<Controller2 *>(controller));
        QVariant stringVal = controller->property("string");
        QCOMPARE(stringVal.type(), QVariant::String);
        QCOMPARE(stringVal.toString(), QStringLiteral("Controller #2"));
        QVariant enumVal = controller->property("enumVal");
        QCOMPARE(enumVal.type(), QVariant::Int);
        QCOMPARE(enumVal.toInt(), 111);
    }
    qmlUnregisterType(controllerId);
    {
        QQmlEngine engine;
        QQmlType type = QQmlMetaType::qmlType(QString("Controller"), QString("mytypes"), 1, 0);
        QVERIFY(!type.isValid());
        controllerId = qmlRegisterType<Controller1>("mytypes", 1, 0, "Controller");
        type = QQmlMetaType::qmlType(QString("Controller"), QString("mytypes"), 1, 0);
        QVERIFY(type.isValid());
        QVERIFY(!type.isInterface());
        QVERIFY(!type.isSingleton());
        QVERIFY(!type.isComposite());
        QQmlComponent c(&engine, testFileUrl("testUnregisterCustomType.qml"));
        QScopedPointer<QObject> obj(c.create());
        QVERIFY(obj);
        QObject *controller = obj->findChild<QObject *>("controller");
        QVERIFY(qobject_cast<Controller1 *>(controller));
        QVariant stringVal = controller->property("string");
        QCOMPARE(stringVal.type(), QVariant::String);
        QCOMPARE(stringVal.toString(), QStringLiteral("Controller #1"));
        QVariant enumVal = controller->property("enumVal");
        QCOMPARE(enumVal.type(), QVariant::Int);
        QCOMPARE(enumVal.toInt(), 1);
    }
}

class StaticProvider1 : public QObject
{
    Q_OBJECT
public:
    StaticProvider1(QObject *parent = nullptr) : QObject(parent) {}
    Q_INVOKABLE QString singletonGetString() { return "StaticProvider #1"; }
};

static QObject* createStaticProvider1(QQmlEngine *, QJSEngine *)
{
    return new StaticProvider1;
}

class StaticProvider2 : public QObject
{
    Q_OBJECT
public:
    StaticProvider2(QObject *parent = nullptr) : QObject(parent) {}
    Q_INVOKABLE QString singletonGetString() { return "StaticProvider #2"; }
};

static QObject* createStaticProvider2(QQmlEngine *, QJSEngine *)
{
    return new StaticProvider2;
}

void tst_qqmlmetatype::unregisterCustomSingletonType()
{
    int staticProviderId = 0;
    {
        QQmlEngine engine;
        staticProviderId = qmlRegisterSingletonType<StaticProvider1>("mytypes", 1, 0, "StaticProvider", createStaticProvider1);
        QQmlType type = QQmlMetaType::qmlType(QString("StaticProvider"), QString("mytypes"), 1, 0);
        QVERIFY(type.isValid());
        QVERIFY(!type.isInterface());
        QVERIFY(type.isSingleton());
        QVERIFY(!type.isComposite());
        QQmlComponent c(&engine, testFileUrl("testUnregisterCustomSingletonType.qml"));
        QScopedPointer<QObject> obj(c.create());
        QVERIFY(obj.data());
        QVariant stringVal = obj->property("text");
        QCOMPARE(stringVal.type(), QVariant::String);
        QCOMPARE(stringVal.toString(), QStringLiteral("StaticProvider #1"));
    }
    qmlUnregisterType(staticProviderId);
    {
        QQmlEngine engine;
        staticProviderId = qmlRegisterSingletonType<StaticProvider2>("mytypes", 1, 0, "StaticProvider", createStaticProvider2);
        QQmlType type = QQmlMetaType::qmlType(QString("StaticProvider"), QString("mytypes"), 1, 0);
        QVERIFY(type.isValid());
        QVERIFY(!type.isInterface());
        QVERIFY(type.isSingleton());
        QVERIFY(!type.isComposite());
        QQmlComponent c(&engine, testFileUrl("testUnregisterCustomSingletonType.qml"));
        QScopedPointer<QObject> obj(c.create());
        QVERIFY(obj.data());
        QVariant stringVal = obj->property("text");
        QCOMPARE(stringVal.type(), QVariant::String);
        QCOMPARE(stringVal.toString(), QStringLiteral("StaticProvider #2"));
    }
    qmlUnregisterType(staticProviderId);
    {
        QQmlEngine engine;
        staticProviderId = qmlRegisterSingletonType<StaticProvider1>("mytypes", 1, 0, "StaticProvider", createStaticProvider1);
        QQmlType type = QQmlMetaType::qmlType(QString("StaticProvider"), QString("mytypes"), 1, 0);
        QVERIFY(type.isValid());
        QVERIFY(!type.isInterface());
        QVERIFY(type.isSingleton());
        QVERIFY(!type.isComposite());
        QQmlComponent c(&engine, testFileUrl("testUnregisterCustomSingletonType.qml"));
        QScopedPointer<QObject> obj(c.create());
        QVERIFY(obj.data());
        QVariant stringVal = obj->property("text");
        QCOMPARE(stringVal.type(), QVariant::String);
        QCOMPARE(stringVal.toString(), QStringLiteral("StaticProvider #1"));
    }
}

void tst_qqmlmetatype::normalizeUrls()
{
    const QUrl url("qrc:///tstqqmlmetatype/data/CompositeType.qml");
    QVERIFY(!QQmlMetaType::qmlType(url).isValid());
    const auto registrationId = qmlRegisterType(url, "Test", 1, 0, "ResourceCompositeType");
    QVERIFY(QQmlMetaType::qmlType(url, /*includeNonFileImports=*/true).isValid());
    QUrl normalizedURL("qrc:/tstqqmlmetatype/data/CompositeType.qml");
    QVERIFY(QQmlMetaType::qmlType(normalizedURL, /*includeNonFileImports=*/true).isValid());
    qmlUnregisterType(registrationId);
    QVERIFY(!QQmlMetaType::qmlType(url, /*includeNonFileImports=*/true).isValid());
}

void tst_qqmlmetatype::unregisterAttachedProperties()
{
    qmlClearTypeRegistrations();

    const QUrl dummy("qrc:///doesnotexist.qml");
    {
        QQmlEngine e;
        QQmlComponent c(&e);
        c.setData("import QtQuick 2.2\n Item { }", dummy);

        const QQmlType attachedType = QQmlMetaType::qmlType("QtQuick/KeyNavigation", 2, 2);
        QCOMPARE(attachedType.attachedPropertiesType(QQmlEnginePrivate::get(&e)),
                 attachedType.metaObject());

        QVERIFY(c.create());
    }

    qmlClearTypeRegistrations();
    {
        QQmlEngine e;
        QQmlComponent c(&e);

        // The extra import shuffles the type IDs around, so that we
        // get a different ID for the attached properties. If the attached
        // properties aren't properly cleared, this will crash.
        c.setData("import QtQml.StateMachine 1.0 \n"
                  "import QtQuick 2.2 \n"
                  "Item { KeyNavigation.up: null }", dummy);

        const QQmlType attachedType = QQmlMetaType::qmlType("QtQuick/KeyNavigation", 2, 2);
        QCOMPARE(attachedType.attachedPropertiesType(QQmlEnginePrivate::get(&e)),
                 attachedType.metaObject());

        QVERIFY(c.create());
    }
}

class Grouped : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int prop READ prop WRITE setProp NOTIFY propChanged REVISION 1)
public:
    int prop() const { return m_prop; }
    void setProp(int prop)
    {
        if (prop != m_prop) {
            m_prop = prop;
            emit propChanged(prop);
        }
    }

signals:
    Q_REVISION(1) void propChanged(int prop);

private:
    int m_prop = 0;
};

class MyItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Grouped *grouped READ grouped CONSTANT)
public:
    MyItem() : m_grouped(new Grouped) {}
    Grouped *grouped() const { return m_grouped.data(); }

private:
    QScopedPointer<Grouped> m_grouped;
};

void tst_qqmlmetatype::revisionedGroupedProperties()
{
    qmlClearTypeRegistrations();
    qmlRegisterType<MyItem>("GroupedTest", 1, 0, "MyItem");
    qmlRegisterType<MyItem, 1>("GroupedTest", 1, 1, "MyItem");
    qmlRegisterUncreatableType<Grouped>("GroupedTest", 1, 0, "Grouped", "Grouped");
    qmlRegisterUncreatableType<Grouped, 1>("GroupedTest", 1, 1, "Grouped", "Grouped");

    {
        QQmlEngine engine;
        QQmlComponent valid(&engine, testFileUrl("revisionedGroupedPropertiesValid.qml"));
        QVERIFY(valid.isReady());
        QScopedPointer<QObject> obj(valid.create());
        QVERIFY(!obj.isNull());
    }

    {
        QQmlEngine engine;
        QQmlComponent invalid(&engine, testFileUrl("revisionedGroupedPropertiesInvalid.qml"));
        QVERIFY(invalid.isError());
    }
}

void tst_qqmlmetatype::enumsInRecursiveImport_data()
{
    QTest::addColumn<QString>("importPath");
    QTest::addColumn<QUrl>("componentUrl");

    QTest::addRow("data directory") << dataDirectory()
                                    << testFileUrl("enumsInRecursiveImport.qml");

    // The qrc case behaves differently because we failed to detect the recursion in type loading
    // due to varying numbers of slashes after the "qrc:" in the URLs.
    QTest::addRow("resources") << QStringLiteral("qrc:/data")
                               << QUrl("qrc:/data/enumsInRecursiveImport.qml");
}

void tst_qqmlmetatype::enumsInRecursiveImport()
{
    QFETCH(QString, importPath);
    QFETCH(QUrl, componentUrl);

    qmlClearTypeRegistrations();
    QQmlEngine engine;
    engine.addImportPath(importPath);
    QQmlComponent c(&engine, componentUrl);
    QVERIFY(c.isReady());
    QScopedPointer<QObject> obj(c.create());
    QVERIFY(!obj.isNull());
    QTRY_COMPARE(obj->property("color").toString(), QString("green"));
}

QTEST_MAIN(tst_qqmlmetatype)

#include "tst_qqmlmetatype.moc"
