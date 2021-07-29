/****************************************************************************
**
** Copyright (C) 2017 Crimson AS <info@crimson.no>
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
#include <QtTest/QtTest>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlexpression.h>
#include <QtQml/qqmlcontext.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qnumeric.h>
#include <private/qqmlengine_p.h>
#include <private/qqmlvmemetaobject_p.h>
#include <private/qv4qmlcontext_p.h>
#include "testtypes.h"
#include "testhttpserver.h"
#include "../../shared/util.h"
#include <private/qv4functionobject_p.h>
#include <private/qv4scopedvalue_p.h>
#include <private/qv4jscall_p.h>
#include <private/qv4alloca_p.h>
#include <private/qv4runtime_p.h>
#include <private/qv4object_p.h>
#include <private/qqmlcomponentattached_p.h>
#include <private/qv4objectiterator_p.h>
#include <private/qqmlabstractbinding_p.h>
#include <private/qqmlvaluetypeproxybinding_p.h>

#ifdef Q_CC_MSVC
#define NO_INLINE __declspec(noinline)
#else
#define NO_INLINE __attribute__((noinline))
#endif

/*
This test covers evaluation of ECMAScript expressions and bindings from within
QML.  This does not include static QML language issues.

Static QML language issues are covered in qmllanguage
*/

class tst_qqmlecmascript : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_qqmlecmascript() {}

private slots:
    void initTestCase();
    void assignBasicTypes();
    void assignDate_data();
    void assignDate();
    void exportDate_data();
    void exportDate();
    void idShortcutInvalidates();
    void boolPropertiesEvaluateAsBool();
    void methods();
    void signalAssignment();
    void signalArguments();
    void bindingLoop();
    void basicExpressions();
    void basicExpressions_data();
    void arrayExpressions();
    void contextPropertiesTriggerReeval();
    void objectPropertiesTriggerReeval();
    void dependenciesWithFunctions();
    void deferredProperties();
    void deferredPropertiesErrors();
    void deferredPropertiesInComponents();
    void deferredPropertiesInDestruction();
    void extensionObjects();
    void overrideExtensionProperties();
    void attachedProperties();
    void enums();
    void valueTypeFunctions();
    void constantsOverrideBindings();
    void outerBindingOverridesInnerBinding();
    void aliasPropertyAndBinding();
    void aliasPropertyReset();
    void nonExistentAttachedObject();
    void scope();
    void importScope();
    void signalParameterTypes();
    void objectsCompareAsEqual();
    void componentCreation_data();
    void componentCreation();
    void dynamicCreation_data();
    void dynamicCreation();
    void dynamicDestruction();
    void objectToString();
    void objectHasOwnProperty();
    void selfDeletingBinding();
    void extendedObjectPropertyLookup();
    void extendedObjectPropertyLookup2();
    void uncreatableExtendedObjectFailureCheck();
    void extendedObjectPropertyLookup3();
    void scriptErrors();
    void functionErrors();
    void propertyAssignmentErrors();
    void signalTriggeredBindings();
    void listProperties();
    void exceptionClearsOnReeval();
    void exceptionSlotProducesWarning();
    void exceptionBindingProducesWarning();
    void compileInvalidBinding();
    void transientErrors();
    void shutdownErrors();
    void compositePropertyType();
    void jsObject();
    void undefinedResetsProperty();
    void listToVariant();
    void listAssignment();
    void multiEngineObject();
    void deletedObject();
    void attachedPropertyScope();
    void scriptConnect();
    void scriptDisconnect();
    void ownership();
    void cppOwnershipReturnValue();
    void ownershipCustomReturnValue();
    void ownershipRootObject();
    void ownershipConsistency();
    void ownershipQmlIncubated();
    void qlistqobjectMethods();
    void strictlyEquals();
    void compiled();
    void numberAssignment();
    void propertySplicing();
    void signalWithUnknownTypes();
    void signalWithJSValueInVariant_data();
    void signalWithJSValueInVariant();
    void signalWithJSValueInVariant_twoEngines_data();
    void signalWithJSValueInVariant_twoEngines();
    void signalWithQJSValue_data();
    void signalWithQJSValue();
    void singletonType_data();
    void singletonType();
    void singletonTypeCaching_data();
    void singletonTypeCaching();
    void singletonTypeImportOrder();
    void singletonTypeResolution();
    void importScripts_data();
    void importScripts();
    void importCreationContext();
    void scarceResources();
    void scarceResources_data();
    void scarceResources_other();
    void propertyChangeSlots();
    void propertyVar_data();
    void propertyVar();
    void propertyQJSValue_data();
    void propertyQJSValue();
    void propertyVarCpp();
    void propertyVarOwnership();
    void propertyVarImplicitOwnership();
    void propertyVarReparent();
    void propertyVarReparentNullContext();
    void propertyVarCircular();
    void propertyVarCircular2();
    void propertyVarInheritance();
    void propertyVarInheritance2();
    void elementAssign();
    void objectPassThroughSignals();
    void objectConversion();
    void booleanConversion();
    void handleReferenceManagement();
    void stringArg();
    void readonlyDeclaration();
    void sequenceConversionRead();
    void sequenceConversionWrite();
    void sequenceConversionArray();
    void sequenceConversionIndexes();
    void sequenceConversionThreads();
    void sequenceConversionBindings();
    void sequenceConversionCopy();
    void assignSequenceTypes();
    void sequenceSort_data();
    void sequenceSort();
    void dateParse();
    void utcDate();
    void negativeYear();
    void qtbug_22464();
    void qtbug_21580();
    void singleV8BindingDestroyedDuringEvaluation();
    void bug1();
#ifndef QT_NO_WIDGETS
    void bug2();
#endif
    void dynamicCreationCrash();
    void dynamicCreationOwnership();
    void regExpBug();
    void nullObjectBinding();
    void nullObjectInitializer();
    void deletedEngine();
    void libraryScriptAssert();
    void variantsAssignedUndefined();
    void variants();
    void qtbug_9792();
    void qtcreatorbug_1289();
    void noSpuriousWarningsAtShutdown();
    void canAssignNullToQObject();
    void functionAssignment_fromBinding();
    void functionAssignment_fromJS();
    void functionAssignment_fromJS_data();
    void functionAssignmentfromJS_invalid();
    void functionAssignment_afterBinding();
    void eval();
    void function();
    void qtbug_10696();
    void qtbug_11606();
    void qtbug_11600();
    void qtbug_21864();
    void qobjectConnectionListExceptionHandling();
    void nonscriptable();
    void deleteLater();
    void objectNameChangedSignal();
    void destroyedSignal();
    void in();
    void typeOf();
    void qtbug_24448();
    void sharedAttachedObject();
    void objectName();
    void writeRemovesBinding();
    void aliasBindingsAssignCorrectly();
    void aliasBindingsOverrideTarget();
    void aliasWritesOverrideBindings();
    void aliasToCompositeElement();
    void realToInt();
    void urlProperty();
    void urlPropertyWithEncoding();
    void urlListPropertyWithEncoding();
    void dynamicString();
    void include();
    void includeRemoteSuccess();
    void signalHandlers();
    void qtbug_37351();
    void doubleEvaluate();
    void forInLoop();
    void nonNotifyable();
    void nonNotifyableConstant();
    void deleteWhileBindingRunning();
    void callQtInvokables();
    void resolveClashingProperties();
    void invokableObjectArg();
    void invokableObjectRet();
    void invokableEnumRet();
    void qtbug_20344();
    void qtbug_22679();
    void qtbug_22843_data();
    void qtbug_22843();
    void rewriteMultiLineStrings();
    void revisionErrors();
    void revision();
    void invokableWithQObjectDerived();
    void realTypePrecision();
    void registeredFlagMethod();
    void deleteLaterObjectMethodCall();
    void automaticSemicolon();
    void compatibilitySemicolon();
    void incrDecrSemicolon1();
    void incrDecrSemicolon2();
    void incrDecrSemicolon_error1();
    void unaryExpression();
    void switchStatement();
    void withStatement();
    void tryStatement();
    void replaceBinding();
    void bindingBoundFunctions();
    void deleteRootObjectInCreation();
    void onDestruction();
    void onDestructionViaGC();
    void bindingSuppression();
    void signalEmitted();
    void threadSignal();
    void qqmldataDestroyed();
    void secondAlias();
    void varAlias();
    void overrideDataAssert();
    void fallbackBindings_data();
    void fallbackBindings();
    void propertyOverride();
    void concatenatedStringPropertyAccess();
    void jsOwnedObjectsDeletedOnEngineDestroy();
    void updateCall();
    void numberParsing();
    void stringParsing();
    void push_and_shift();
    void qtbug_32801();
    void thisObject();
    void qtbug_33754();
    void qtbug_34493();
    void singletonFromQMLToCpp();
    void singletonFromQMLAndBackAndCompare();
    void setPropertyOnInvalid();
    void miscTypeTest();
    void stackLimits();
    void idsAsLValues();
    void qtbug_34792();
    void noCaptureWhenWritingProperty();
    void singletonWithEnum();
    void lazyBindingEvaluation();
    void varPropertyAccessOnObjectWithInvalidContext();
    void importedScriptsAccessOnObjectWithInvalidContext();
    void importedScriptsWithoutQmlMode();
    void contextObjectOnLazyBindings();
    void garbageCollectionDuringCreation();
    void qtbug_39520();
    void readUnregisteredQObjectProperty();
    void writeUnregisteredQObjectProperty();
    void switchExpression();
    void qtbug_46022();
    void qtbug_52340();
    void qtbug_54589();
    void qtbug_54687();
    void stringify_qtbug_50592();
    void instanceof_data();
    void instanceof();
    void constkw_data();
    void constkw();
    void redefineGlobalProp();
    void freeze_empty_object();
    void singleBlockLoops();
    void qtbug_60547();
    void delayLoadingArgs();
    void manyArguments();
    void forInIterator();
    void localForInIterator();
    void shadowedFunctionName();
    void anotherNaN();
    void callPropertyOnUndefined();
    void jumpStrictNotEqualUndefined();
    void removeBindingsWithNoDependencies();
    void preserveBindingWithUnresolvedNames();
    void temporaryDeadZone();
    void importLexicalVariables_data();
    void importLexicalVariables();
    void hugeObject();
    void templateStringTerminator();
    void arrayAndException();
    void numberToStringWithRadix();
    void tailCallWithArguments();
    void deleteSparseInIteration();
    void saveAccumulatorBeforeToInt32();
    void intMinDividedByMinusOne();
    void undefinedPropertiesInObjectWrapper();
    void hugeRegexpQuantifiers();
    void singletonTypeWrapperLookup();
    void getThisObject();
    void semicolonAfterProperty();

    void gcCrashRegressionTest();

private:
//    static void propertyVarWeakRefCallback(v8::Persistent<v8::Value> object, void* parameter);
    static void verifyContextLifetime(QQmlContextData *ctxt);

    // When calling into JavaScript, the specific type of the return value can differ if that return
    // value is a number. This is not only the case for non-integral numbers, or numbers that do not
    // fit into the (signed) integer range, but it also depends on which optimizations are run. So,
    // to check if the return value is of a number type, use this method instead of checking against
    // a specific userType.
    static bool isJSNumberType(int userType)
    {
        return userType == (int) QVariant::Int
                || userType == (int) QVariant::UInt
                || userType == (int) QVariant::Double;
    }
};

static void gc(QQmlEngine &engine)
{
    engine.collectGarbage();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}


void tst_qqmlecmascript::initTestCase()
{
    QQmlDataTest::initTestCase();
    registerTypes();
}

void tst_qqmlecmascript::assignBasicTypes()
{
    QQmlEngine engine;
    {
    QQmlComponent component(&engine, testFileUrl("assignBasicTypes.qml"));
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->flagProperty(), MyTypeObject::FlagVal1 | MyTypeObject::FlagVal3);
    QCOMPARE(object->enumProperty(), MyTypeObject::EnumVal2);
    QCOMPARE(object->relatedEnumProperty(), MyEnumContainer::RelatedValue);
    QCOMPARE(object->stringProperty(), QString("Hello World!"));
    QCOMPARE(object->uintProperty(), uint(10));
    QCOMPARE(object->intProperty(), -19);
    QCOMPARE((float)object->realProperty(), float(23.2));
    QCOMPARE((float)object->doubleProperty(), float(-19.75));
    QCOMPARE((float)object->floatProperty(), float(8.5));
    QCOMPARE(object->colorProperty(), QColor("red"));
    QCOMPARE(object->dateProperty(), QDate(1982, 11, 25));
    QCOMPARE(object->timeProperty(), QTime(11, 11, 32));
    QCOMPARE(object->dateTimeProperty(), QDateTime(QDate(2009, 5, 12), QTime(13, 22, 1), Qt::UTC));
    QCOMPARE(object->pointProperty(), QPoint(99,13));
    QCOMPARE(object->pointFProperty(), QPointF(-10.1, 12.3));
    QCOMPARE(object->sizeProperty(), QSize(99, 13));
    QCOMPARE(object->sizeFProperty(), QSizeF(0.1, 0.2));
    QCOMPARE(object->rectProperty(), QRect(9, 7, 100, 200));
    QCOMPARE(object->rectFProperty(), QRectF(1000.1, -10.9, 400, 90.99));
    QCOMPARE(object->boolProperty(), true);
    QCOMPARE(object->variantProperty(), QVariant("Hello World!"));
    QCOMPARE(object->vectorProperty(), QVector3D(10, 1, 2.2f));
    QCOMPARE(object->urlProperty(), component.url().resolved(QUrl("main.qml")));
    delete object;
    }
    {
    QQmlComponent component(&engine, testFileUrl("assignBasicTypes.2.qml"));
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->flagProperty(), MyTypeObject::FlagVal1 | MyTypeObject::FlagVal3);
    QCOMPARE(object->enumProperty(), MyTypeObject::EnumVal2);
    QCOMPARE(object->relatedEnumProperty(), MyEnumContainer::RelatedValue);
    QCOMPARE(object->stringProperty(), QString("Hello World!"));
    QCOMPARE(object->uintProperty(), uint(10));
    QCOMPARE(object->intProperty(), -19);
    QCOMPARE((float)object->realProperty(), float(23.2));
    QCOMPARE((float)object->doubleProperty(), float(-19.75));
    QCOMPARE((float)object->floatProperty(), float(8.5));
    QCOMPARE(object->colorProperty(), QColor("red"));
    QCOMPARE(object->dateProperty(), QDate(1982, 11, 25));
    QCOMPARE(object->timeProperty(), QTime(11, 11, 32));
    QCOMPARE(object->dateTimeProperty(), QDateTime(QDate(2009, 5, 12), QTime(13, 22, 1), Qt::UTC));
    QCOMPARE(object->pointProperty(), QPoint(99,13));
    QCOMPARE(object->pointFProperty(), QPointF(-10.1, 12.3));
    QCOMPARE(object->sizeProperty(), QSize(99, 13));
    QCOMPARE(object->sizeFProperty(), QSizeF(0.1, 0.2));
    QCOMPARE(object->rectProperty(), QRect(9, 7, 100, 200));
    QCOMPARE(object->rectFProperty(), QRectF(1000.1, -10.9, 400, 90.99));
    QCOMPARE(object->boolProperty(), true);
    QCOMPARE(object->variantProperty(), QVariant("Hello World!"));
    QCOMPARE(object->vectorProperty(), QVector3D(10, 1, 2.2f));
    QCOMPARE(object->urlProperty(), component.url().resolved(QUrl("main.qml")));
    delete object;
    }
}

void tst_qqmlecmascript::assignDate_data()
{
    QTest::addColumn<QUrl>("source");
    QTest::addColumn<int>("timeOffset"); // -1 for local-time, else minutes from UTC

    QTest::newRow("Component.onComplete JS Parse") << testFileUrl("assignDate.qml") << -1;
    QTest::newRow("Component.onComplete JS") << testFileUrl("assignDate.1.qml") << 0;
    QTest::newRow("Binding JS") << testFileUrl("assignDate.2.qml") << -1;
    QTest::newRow("Binding UTC") << testFileUrl("assignDate.3.qml") << 0;
    QTest::newRow("Binding JS UTC") << testFileUrl("assignDate.4.qml") << 0;
    QTest::newRow("Binding UTC+2") << testFileUrl("assignDate.5.qml") << 120;
    QTest::newRow("Binding JS UTC+2 ") << testFileUrl("assignDate.6.qml") << 120;
}

void tst_qqmlecmascript::assignDate()
{
    QFETCH(QUrl, source);
    QFETCH(int, timeOffset);

    QQmlEngine engine;
    QQmlComponent component(&engine, source);
    QScopedPointer<QObject> obj(component.create());
    MyTypeObject *object = qobject_cast<MyTypeObject *>(obj.data());
    QVERIFY(object != nullptr);

    QDate expectedDate(2009, 5, 12);
    QDateTime expectedDateTime;
    QDateTime expectedDateTime2;
    if (timeOffset == -1) {
        expectedDateTime = QDateTime(QDate(2009, 5, 12), QTime(0, 0, 1), Qt::LocalTime);
        expectedDateTime2 = QDateTime(QDate(2009, 5, 12), QTime(23, 59, 59), Qt::LocalTime);
    } else {
        expectedDateTime = QDateTime(QDate(2009, 5, 12), QTime(0, 0, 1), Qt::OffsetFromUTC, timeOffset * 60);
        expectedDateTime2 = QDateTime(QDate(2009, 5, 12), QTime(23, 59, 59), Qt::OffsetFromUTC, timeOffset * 60);
    }

    QCOMPARE(object->dateProperty(), expectedDate);
    QCOMPARE(object->dateTimeProperty(), expectedDateTime);
    QCOMPARE(object->dateTimeProperty2(), expectedDateTime2);
    QCOMPARE(object->boolProperty(), true);
}

void tst_qqmlecmascript::exportDate_data()
{
    QTest::addColumn<QUrl>("source");
    QTest::addColumn<QDateTime>("datetime");

    // Verify that we can export datetime information to QML and that consumers can access
    // the data correctly provided they know the TZ info associated with the value

    const QDate date(2009, 5, 12);
    const QTime early(0, 0, 1);
    const QTime late(23, 59, 59);
    const int offset(((11 * 60) + 30) * 60);

    QTest::newRow("Localtime early") << testFileUrl("exportDate.qml") << QDateTime(date, early, Qt::LocalTime);
    QTest::newRow("Localtime late") << testFileUrl("exportDate.2.qml") << QDateTime(date, late, Qt::LocalTime);
    QTest::newRow("UTC early") << testFileUrl("exportDate.3.qml") << QDateTime(date, early, Qt::UTC);
    QTest::newRow("UTC late") << testFileUrl("exportDate.4.qml") << QDateTime(date, late, Qt::UTC);
    {
        QDateTime dt(date, early, Qt::OffsetFromUTC);
        dt.setUtcOffset(offset);
        QTest::newRow("+11:30 early") << testFileUrl("exportDate.5.qml") << dt;
    }
    {
        QDateTime dt(date, late, Qt::OffsetFromUTC);
        dt.setUtcOffset(offset);
        QTest::newRow("+11:30 late") << testFileUrl("exportDate.6.qml") << dt;
    }
    {
        QDateTime dt(date, early, Qt::OffsetFromUTC);
        dt.setUtcOffset(-offset);
        QTest::newRow("-11:30 early") << testFileUrl("exportDate.7.qml") << dt;
    }
    {
        QDateTime dt(date, late, Qt::OffsetFromUTC);
        dt.setUtcOffset(-offset);
        QTest::newRow("-11:30 late") << testFileUrl("exportDate.8.qml") << dt;
    }
}

void tst_qqmlecmascript::exportDate()
{
    QFETCH(QUrl, source);
    QFETCH(QDateTime, datetime);

    DateTimeExporter exporter(datetime);

    QQmlEngine e;
    e.rootContext()->setContextProperty("datetimeExporter", &exporter);

    QQmlComponent component(&e, source);
    QScopedPointer<QObject> obj(component.create());
    MyTypeObject *object = qobject_cast<MyTypeObject *>(obj.data());
    QVERIFY(object != nullptr);
    QCOMPARE(object->boolProperty(), true);
}

void tst_qqmlecmascript::idShortcutInvalidates()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("idShortcutInvalidates.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QVERIFY(object->objectProperty() != nullptr);
        delete object->objectProperty();
        QVERIFY(!object->objectProperty());
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("idShortcutInvalidates.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QVERIFY(object->objectProperty() != nullptr);
        delete object->objectProperty();
        QVERIFY(!object->objectProperty());
        delete object;
    }
}

void tst_qqmlecmascript::boolPropertiesEvaluateAsBool()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("boolPropertiesEvaluateAsBool.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->stringProperty(), QLatin1String("pass"));
        delete object;
    }
    {
        QQmlComponent component(&engine, testFileUrl("boolPropertiesEvaluateAsBool.2.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->stringProperty(), QLatin1String("pass"));
        delete object;
    }
}

void tst_qqmlecmascript::signalAssignment()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("signalAssignment.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->string(), QString());
        emit object->basicSignal();
        QCOMPARE(object->string(), QString("pass"));
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("signalAssignment.2.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->string(), QString());
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->string(), QString("pass 19 Hello world! 10.25 3 2"));
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("signalAssignment.3.qml"));
        QVERIFY(component.isError());
        QString expectedErrorString = component.url().toString() + QLatin1String(":4 Signal uses unnamed parameter followed by named parameter.\n");
        QCOMPARE(component.errorString(), expectedErrorString);
    }

    {
        QQmlComponent component(&engine, testFileUrl("signalAssignment.4.qml"));
        QVERIFY(component.isError());
        QString expectedErrorString = component.url().toString() + QLatin1String(":5 Signal parameter \"parseInt\" hides global variable.\n");
        QCOMPARE(component.errorString(), expectedErrorString);
    }
}

void tst_qqmlecmascript::signalArguments()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("signalArguments.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->string(), QString());
        emit object->basicSignal();
        QCOMPARE(object->string(), QString("pass"));
        QCOMPARE(object->property("argumentCount").toInt(), 0);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("signalArguments.2.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->string(), QString());
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->string(), QString("pass 19 Hello world! 10.25 3 2"));
        QCOMPARE(object->property("argumentCount").toInt(), 5);
        delete object;
    }
}

void tst_qqmlecmascript::methods()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("methods.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->methodCalled(), false);
        QCOMPARE(object->methodIntCalled(), false);
        emit object->basicSignal();
        QCOMPARE(object->methodCalled(), true);
        QCOMPARE(object->methodIntCalled(), false);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("methods.2.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->methodCalled(), false);
        QCOMPARE(object->methodIntCalled(), false);
        emit object->basicSignal();
        QCOMPARE(object->methodCalled(), false);
        QCOMPARE(object->methodIntCalled(), true);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("methods.3.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("test").toInt(), 19);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("methods.4.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("test").toInt(), 19);
        QCOMPARE(object->property("test2").toInt(), 17);
        QCOMPARE(object->property("test3").toInt(), 16);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("methods.5.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("test").toInt(), 9);
        delete object;
    }
}

void tst_qqmlecmascript::bindingLoop()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("bindingLoop.qml"));
    QString warning = component.url().toString() + ":9:9: QML MyQmlObject: Binding loop detected for property \"stringProperty\"";
    QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    delete object;
}

void tst_qqmlecmascript::basicExpressions_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<QVariant>("result");
    QTest::addColumn<bool>("nest");

    QTest::newRow("Syntax error (self test)") << "{console.log({'a':1'}.a)}" << QVariant() << false;
    QTest::newRow("Context property") << "a" << QVariant(1944) << false;
    QTest::newRow("Context property") << "a" << QVariant(1944) << true;
    QTest::newRow("Context property expression") << "a * 2" << QVariant(3888) << false;
    QTest::newRow("Context property expression") << "a * 2" << QVariant(3888) << true;
    QTest::newRow("Overridden context property") << "b" << QVariant("Milk") << false;
    QTest::newRow("Overridden context property") << "b" << QVariant("Cow") << true;
    QTest::newRow("Object property") << "object.stringProperty" << QVariant("Object1") << false;
    QTest::newRow("Object property") << "object.stringProperty" << QVariant("Object1") << true;
    QTest::newRow("Overridden object property") << "objectOverride.stringProperty" << QVariant("Object2") << false;
    QTest::newRow("Overridden object property") << "objectOverride.stringProperty" << QVariant("Object3") << true;
    QTest::newRow("Default object property") << "horseLegs" << QVariant(4) << false;
    QTest::newRow("Default object property") << "antLegs" << QVariant(6) << false;
    QTest::newRow("Default object property") << "emuLegs" << QVariant(2) << false;
    QTest::newRow("Nested default object property") << "horseLegs" << QVariant(4) << true;
    QTest::newRow("Nested default object property") << "antLegs" << QVariant(7) << true;
    QTest::newRow("Nested default object property") << "emuLegs" << QVariant(2) << true;
    QTest::newRow("Nested default object property") << "humanLegs" << QVariant(2) << true;
    QTest::newRow("Context property override default object property") << "millipedeLegs" << QVariant(100) << true;
}

void tst_qqmlecmascript::basicExpressions()
{
    QFETCH(QString, expression);
    QFETCH(QVariant, result);
    QFETCH(bool, nest);

    QQmlEngine engine;

    MyQmlObject object1;
    MyQmlObject object2;
    MyQmlObject object3;
    MyDefaultObject1 default1;
    MyDefaultObject3 default3;
    object1.setStringProperty("Object1");
    object2.setStringProperty("Object2");
    object3.setStringProperty("Object3");

    QQmlContext context(engine.rootContext());
    QQmlContext nestedContext(&context);

    context.setContextObject(&default1);
    context.setContextProperty("a", QVariant(1944));
    context.setContextProperty("b", QVariant("Milk"));
    context.setContextProperty("object", &object1);
    context.setContextProperty("objectOverride", &object2);
    nestedContext.setContextObject(&default3);
    nestedContext.setContextProperty("b", QVariant("Cow"));
    nestedContext.setContextProperty("objectOverride", &object3);
    nestedContext.setContextProperty("millipedeLegs", QVariant(100));

    MyExpression expr(nest?&nestedContext:&context, expression);
    QCOMPARE(expr.evaluate(), result);
}

void tst_qqmlecmascript::arrayExpressions()
{
    QObject obj1;
    QObject obj2;
    QObject obj3;

    QQmlEngine engine;
    QQmlContext context(engine.rootContext());
    context.setContextProperty("a", &obj1);
    context.setContextProperty("b", &obj2);
    context.setContextProperty("c", &obj3);

    MyExpression expr(&context, "[a, b, c, 10]");
    QVariant result = expr.evaluate();
    QCOMPARE(result.userType(), qMetaTypeId<QJSValue>());
    QJSValue list = qvariant_cast<QJSValue>(result);
    QCOMPARE(list.property("length").toInt(), 4);
    QCOMPARE(list.property(0).toQObject(), &obj1);
    QCOMPARE(list.property(1).toQObject(), &obj2);
    QCOMPARE(list.property(2).toQObject(), &obj3);
    QCOMPARE(list.property(3).toInt(), 10);
}

// Tests that modifying a context property will reevaluate expressions
void tst_qqmlecmascript::contextPropertiesTriggerReeval()
{
    QQmlEngine engine;
    QQmlContext context(engine.rootContext());
    MyQmlObject object1;
    MyQmlObject object2;
    MyQmlObject *object3 = new MyQmlObject;

    object1.setStringProperty("Hello");
    object2.setStringProperty("World");

    context.setContextProperty("testProp", QVariant(1));
    context.setContextProperty("testObj", &object1);
    context.setContextProperty("testObj2", object3);

    {
        MyExpression expr(&context, "testProp + 1");
        QCOMPARE(expr.changed, false);
        QCOMPARE(expr.evaluate(), QVariant(2));

        context.setContextProperty("testProp", QVariant(2));
        QCOMPARE(expr.changed, true);
        QCOMPARE(expr.evaluate(), QVariant(3));
    }

    {
        MyExpression expr(&context, "testProp + testProp + testProp");
        QCOMPARE(expr.changed, false);
        QCOMPARE(expr.evaluate(), QVariant(6));

        context.setContextProperty("testProp", QVariant(4));
        QCOMPARE(expr.changed, true);
        QCOMPARE(expr.evaluate(), QVariant(12));
    }

    {
        MyExpression expr(&context, "testObj.stringProperty");
        QCOMPARE(expr.changed, false);
        QCOMPARE(expr.evaluate(), QVariant("Hello"));

        context.setContextProperty("testObj", &object2);
        QCOMPARE(expr.changed, true);
        QCOMPARE(expr.evaluate(), QVariant("World"));
    }

    {
        MyExpression expr(&context, "testObj.stringProperty /**/");
        QCOMPARE(expr.changed, false);
        QCOMPARE(expr.evaluate(), QVariant("World"));

        context.setContextProperty("testObj", &object1);
        QCOMPARE(expr.changed, true);
        QCOMPARE(expr.evaluate(), QVariant("Hello"));
    }

    {
        MyExpression expr(&context, "testObj2");
        QCOMPARE(expr.changed, false);
        QCOMPARE(expr.evaluate(), QVariant::fromValue((QObject *)object3));
    }

    delete object3;
}

void tst_qqmlecmascript::objectPropertiesTriggerReeval()
{
    QQmlEngine engine;
    QQmlContext context(engine.rootContext());
    MyQmlObject object1;
    MyQmlObject object2;
    MyQmlObject object3;
    context.setContextProperty("testObj", &object1);

    object1.setStringProperty(QLatin1String("Hello"));
    object2.setStringProperty(QLatin1String("Dog"));
    object3.setStringProperty(QLatin1String("Cat"));

    {
        MyExpression expr(&context, "testObj.stringProperty");
        QCOMPARE(expr.changed, false);
        QCOMPARE(expr.evaluate(), QVariant("Hello"));

        object1.setStringProperty(QLatin1String("World"));
        QCOMPARE(expr.changed, true);
        QCOMPARE(expr.evaluate(), QVariant("World"));
    }

    {
        MyExpression expr(&context, "testObj.objectProperty.stringProperty");
        QCOMPARE(expr.changed, false);
        QCOMPARE(expr.evaluate(), QVariant());

        object1.setObjectProperty(&object2);
        QCOMPARE(expr.changed, true);
        expr.changed = false;
        QCOMPARE(expr.evaluate(), QVariant("Dog"));

        object1.setObjectProperty(&object3);
        QCOMPARE(expr.changed, true);
        expr.changed = false;
        QCOMPARE(expr.evaluate(), QVariant("Cat"));

        object1.setObjectProperty(nullptr);
        QCOMPARE(expr.changed, true);
        expr.changed = false;
        QCOMPARE(expr.evaluate(), QVariant());

        object1.setObjectProperty(&object3);
        QCOMPARE(expr.changed, true);
        expr.changed = false;
        QCOMPARE(expr.evaluate(), QVariant("Cat"));

        object3.setStringProperty("Donkey");
        QCOMPARE(expr.changed, true);
        expr.changed = false;
        QCOMPARE(expr.evaluate(), QVariant("Donkey"));
    }
}

void tst_qqmlecmascript::dependenciesWithFunctions()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("dependenciesWithFunctions.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(object, qPrintable(component.errorString()));
    QVERIFY(!object->property("success").toBool());
    object->setProperty("value", 42);
    QVERIFY(object->property("success").toBool());
}

void tst_qqmlecmascript::deferredProperties()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deferredProperties.qml"));
    MyDeferredObject *object =
        qobject_cast<MyDeferredObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->value(), 0);
    QVERIFY(!object->objectProperty());
    QVERIFY(object->objectProperty2() != nullptr);
    qmlExecuteDeferred(object);
    QCOMPARE(object->value(), 10);
    QVERIFY(object->objectProperty() != nullptr);
    MyQmlObject *qmlObject =
        qobject_cast<MyQmlObject *>(object->objectProperty());
    QVERIFY(qmlObject != nullptr);
    QCOMPARE(qmlObject->value(), 10);
    object->setValue(19);
    QCOMPARE(qmlObject->value(), 19);

    delete object;
}

// Check errors on deferred properties are correctly emitted
void tst_qqmlecmascript::deferredPropertiesErrors()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deferredPropertiesErrors.qml"));
    MyDeferredObject *object =
        qobject_cast<MyDeferredObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->value(), 0);
    QVERIFY(!object->objectProperty());
    QVERIFY(!object->objectProperty2());

    QString warning = component.url().toString() + ":6:5: Unable to assign [undefined] to QObject*";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));

    qmlExecuteDeferred(object);

    delete object;
}

void tst_qqmlecmascript::deferredPropertiesInComponents()
{
    // Test that it works when the property is set inside and outside component
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deferredPropertiesInComponents.qml"));
    QObject *object = component.create();
    if (!object)
        qDebug() << component.errorString();
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("value").value<int>(), 10);

    MyDeferredObject *defObjectA =
        qobject_cast<MyDeferredObject *>(object->property("deferredInside").value<QObject*>());
    QVERIFY(defObjectA != nullptr);
    QVERIFY(!defObjectA->objectProperty());

    qmlExecuteDeferred(defObjectA);
    QVERIFY(defObjectA->objectProperty() != nullptr);
    QCOMPARE(defObjectA->objectProperty()->property("value").value<int>(), 10);

    MyDeferredObject *defObjectB =
        qobject_cast<MyDeferredObject *>(object->property("deferredOutside").value<QObject*>());
    QVERIFY(defObjectB != nullptr);
    QVERIFY(!defObjectB->objectProperty());

    qmlExecuteDeferred(defObjectB);
    QVERIFY(defObjectB->objectProperty() != nullptr);
    QCOMPARE(defObjectB->objectProperty()->property("value").value<int>(), 10);

    delete object;
}

void tst_qqmlecmascript::deferredPropertiesInDestruction()
{
    //Test that the component does not get created at all if creation is deferred until the containing context is destroyed
    //Very specific operation ordering is needed for this to occur, currently accessing object from object destructor.
    //
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deferredPropertiesInDestruction.qml"));
    QObject *object = component.create();
    if (!object)
        qDebug() << component.errorString();
    QVERIFY(object != nullptr);
    delete object; //QTBUG-33112 was that this used to cause a crash
}

void tst_qqmlecmascript::extensionObjects()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("extensionObjects.qml"));
    MyExtendedObject *object =
        qobject_cast<MyExtendedObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->baseProperty(), 13);
    QCOMPARE(object->coreProperty(), 9);
    object->setProperty("extendedProperty", QVariant(11));
    object->setProperty("baseExtendedProperty", QVariant(92));
    QCOMPARE(object->coreProperty(), 11);
    QCOMPARE(object->baseProperty(), 92);

    MyExtendedObject *nested = qobject_cast<MyExtendedObject*>(qvariant_cast<QObject *>(object->property("nested")));
    QVERIFY(nested);
    QCOMPARE(nested->baseProperty(), 13);
    QCOMPARE(nested->coreProperty(), 9);
    nested->setProperty("extendedProperty", QVariant(11));
    nested->setProperty("baseExtendedProperty", QVariant(92));
    QCOMPARE(nested->coreProperty(), 11);
    QCOMPARE(nested->baseProperty(), 92);

    delete object;
}

void tst_qqmlecmascript::overrideExtensionProperties()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("extensionObjectsPropertyOverride.qml"));
    OverrideDefaultPropertyObject *object =
        qobject_cast<OverrideDefaultPropertyObject *>(component.create());
    QVERIFY(object != nullptr);
    QVERIFY(object->secondProperty() != nullptr);
    QVERIFY(!object->firstProperty());

    delete object;
}

void tst_qqmlecmascript::attachedProperties()
{
    QQmlEngine engine;

    {
        QQmlComponent component(&engine, testFileUrl("attachedProperty.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("a").toInt(), 19);
        QCOMPARE(object->property("b").toInt(), 19);
        QCOMPARE(object->property("c").toInt(), 19);
        QCOMPARE(object->property("d").toInt(), 19);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("attachedProperty.2.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("a").toInt(), 26);
        QCOMPARE(object->property("b").toInt(), 26);
        QCOMPARE(object->property("c").toInt(), 26);
        QCOMPARE(object->property("d").toInt(), 26);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("writeAttachedProperty.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);

        QMetaObject::invokeMethod(object, "writeValue2");

        MyQmlAttachedObject *attached =
            qobject_cast<MyQmlAttachedObject *>(qmlAttachedPropertiesObject<MyQmlObject>(object));
        QVERIFY(attached != nullptr);

        QCOMPARE(attached->value2(), 9);
        delete object;
    }
}

void tst_qqmlecmascript::enums()
{
    QQmlEngine engine;

    // Existent enums
    {
    QQmlComponent component(&engine, testFileUrl("enums.1.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("enumProperty").toInt(), (int)MyQmlObject::EnumValue2);
    QCOMPARE(object->property("relatedEnumProperty").toInt(), (int)MyEnumContainer::RelatedValue);
    QCOMPARE(object->property("unrelatedEnumProperty").toInt(), (int)MyEnumContainer::RelatedValue);
    QCOMPARE(object->property("qtEnumProperty").toInt(), (int)Qt::CaseInsensitive);
    QCOMPARE(object->property("a").toInt(), 0);
    QCOMPARE(object->property("b").toInt(), 1);
    QCOMPARE(object->property("c").toInt(), 2);
    QCOMPARE(object->property("d").toInt(), 3);
    QCOMPARE(object->property("e").toInt(), 0);
    QCOMPARE(object->property("f").toInt(), 1);
    QCOMPARE(object->property("g").toInt(), 2);
    QCOMPARE(object->property("h").toInt(), 3);
    QCOMPARE(object->property("i").toInt(), 19);
    QCOMPARE(object->property("j").toInt(), 19);
    QCOMPARE(object->property("k").toInt(), 42);
    QCOMPARE(object->property("l").toInt(), 333);

    delete object;
    }
    // Non-existent enums
    {
    QUrl file = testFileUrl("enums.2.qml");
    QString w1 = QLatin1String("QMetaProperty::read: Unable to handle unregistered datatype 'MyEnum' for property 'MyUnregisteredEnumTypeObject::enumProperty'");
    QString w2 = QLatin1String("QQmlExpression: Expression ") + testFileUrl("enums.2.qml").toString() + QLatin1String(":9:5 depends on non-NOTIFYable properties:");
    QString w3 = QLatin1String("    MyUnregisteredEnumTypeObject::enumProperty");
    QString w4 = file.toString() + ":7:5: Unable to assign [undefined] to int";
    QString w5 = file.toString() + ":8:5: Unable to assign [undefined] to int";
    QString w6 = file.toString() + ":9:5: Unable to assign [undefined] to int";
    QString w7 = file.toString() + ":13:9: Unable to assign [undefined] to [unknown property type]";
    QString w8 = file.toString() + ":31:9: Unable to assign int to [unknown property type]";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w1));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w2));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w3));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w4));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w5));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w6));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w7));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w8));

    QQmlComponent component(&engine, testFileUrl("enums.2.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("a").toInt(), 0);
    QCOMPARE(object->property("b").toInt(), 0);
    QCOMPARE(object->property("c").toInt(), 0);

    QString w9 = file.toString() + ":18: Error: Cannot assign JavaScript function to [unknown property type]";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w9));
    QMetaObject::invokeMethod(object, "testAssignmentOne");

    QString w10 = file.toString() + ":21: Error: Cannot assign [undefined] to [unknown property type]";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w10));
    QMetaObject::invokeMethod(object, "testAssignmentTwo");

    QString w11 = file.toString() + ":24: Error: Cannot assign [undefined] to [unknown property type]";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w11));
    QMetaObject::invokeMethod(object, "testAssignmentThree");

    QString w12 = file.toString() + ":34: Error: Cannot assign int to an unregistered type";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w12));
    QMetaObject::invokeMethod(object, "testAssignmentFour");

    delete object;
    }
    // Enums as literals
    {
    QQmlComponent component(&engine, testFileUrl("enums.3.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    // check the values are what we expect
    QCOMPARE(object->property("a").toInt(), 4);
    QCOMPARE(object->property("b").toInt(), 5);
    QCOMPARE(object->property("c").toInt(), 9);
    QCOMPARE(object->property("d").toInt(), 13);
    QCOMPARE(object->property("e").toInt(), 2);
    QCOMPARE(object->property("f").toInt(), 3);
    QCOMPARE(object->property("h").toInt(), 2);
    QCOMPARE(object->property("i").toInt(), 3);
    QCOMPARE(object->property("j").toInt(), -1);
    QCOMPARE(object->property("k").toInt(), 42);

    // count of change signals
    QCOMPARE(object->property("ac").toInt(), 0);
    QCOMPARE(object->property("bc").toInt(), 0);
    QCOMPARE(object->property("cc").toInt(), 0);
    QCOMPARE(object->property("dc").toInt(), 0);
    QCOMPARE(object->property("ec").toInt(), 0);
    QCOMPARE(object->property("fc").toInt(), 0);
    QCOMPARE(object->property("hc").toInt(), 1); // namespace -> binding
    QCOMPARE(object->property("ic").toInt(), 1); // namespace -> binding
    QCOMPARE(object->property("jc").toInt(), 0);
    QCOMPARE(object->property("kc").toInt(), 0);

    delete object;
    }
}

void tst_qqmlecmascript::valueTypeFunctions()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("valueTypeFunctions.qml"));
    MyTypeObject *obj = qobject_cast<MyTypeObject*>(component.create());
    QVERIFY(obj != nullptr);
    QCOMPARE(obj->rectProperty(), QRect(0,0,100,100));
    QCOMPARE(obj->rectFProperty(), QRectF(0,0.5,100,99.5));

    delete obj;
}

/*
Tests that writing a constant to a property with a binding on it disables the
binding.
*/
void tst_qqmlecmascript::constantsOverrideBindings()
{
    QQmlEngine engine;

    // From ECMAScript
    {
        QQmlComponent component(&engine, testFileUrl("constantsOverrideBindings.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("c2").toInt(), 0);
        object->setProperty("c1", QVariant(9));
        QCOMPARE(object->property("c2").toInt(), 9);

        emit object->basicSignal();

        QCOMPARE(object->property("c2").toInt(), 13);
        object->setProperty("c1", QVariant(8));
        QCOMPARE(object->property("c2").toInt(), 13);

        delete object;
    }

    // During construction
    {
        QQmlComponent component(&engine, testFileUrl("constantsOverrideBindings.2.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("c1").toInt(), 0);
        QCOMPARE(object->property("c2").toInt(), 10);
        object->setProperty("c1", QVariant(9));
        QCOMPARE(object->property("c1").toInt(), 9);
        QCOMPARE(object->property("c2").toInt(), 10);

        delete object;
    }

#if 0
    // From C++
    {
        QQmlComponent component(&engine, testFileUrl("constantsOverrideBindings.3.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != 0);

        QCOMPARE(object->property("c2").toInt(), 0);
        object->setProperty("c1", QVariant(9));
        QCOMPARE(object->property("c2").toInt(), 9);

        object->setProperty("c2", QVariant(13));
        QCOMPARE(object->property("c2").toInt(), 13);
        object->setProperty("c1", QVariant(7));
        QCOMPARE(object->property("c1").toInt(), 7);
        QCOMPARE(object->property("c2").toInt(), 13);

        delete object;
    }
#endif

    // Using an alias
    {
        QQmlComponent component(&engine, testFileUrl("constantsOverrideBindings.4.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("c1").toInt(), 0);
        QCOMPARE(object->property("c3").toInt(), 10);
        object->setProperty("c1", QVariant(9));
        QCOMPARE(object->property("c1").toInt(), 9);
        QCOMPARE(object->property("c3").toInt(), 10);

        delete object;
    }
}

/*
Tests that assigning a binding to a property that already has a binding causes
the original binding to be disabled.
*/
void tst_qqmlecmascript::outerBindingOverridesInnerBinding()
{
    QQmlEngine engine;
    QQmlComponent component(&engine,
                           testFileUrl("outerBindingOverridesInnerBinding.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("c1").toInt(), 0);
    QCOMPARE(object->property("c2").toInt(), 0);
    QCOMPARE(object->property("c3").toInt(), 0);

    object->setProperty("c1", QVariant(9));
    QCOMPARE(object->property("c1").toInt(), 9);
    QCOMPARE(object->property("c2").toInt(), 0);
    QCOMPARE(object->property("c3").toInt(), 0);

    object->setProperty("c3", QVariant(8));
    QCOMPARE(object->property("c1").toInt(), 9);
    QCOMPARE(object->property("c2").toInt(), 8);
    QCOMPARE(object->property("c3").toInt(), 8);

    delete object;
}

/*
Access a non-existent attached object.

Tests for a regression where this used to crash.
*/
void tst_qqmlecmascript::nonExistentAttachedObject()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("nonExistentAttachedObject.qml"));

    QString warning = component.url().toString() + ":4:5: Unable to assign [undefined] to QString";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    delete object;
}

void tst_qqmlecmascript::scope()
{
    QQmlEngine engine;

    {
        QQmlComponent component(&engine, testFileUrl("scope.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test1").toInt(), 1);
        QCOMPARE(object->property("test2").toInt(), 2);
        QCOMPARE(object->property("test3").toString(), QString("1Test"));
        QCOMPARE(object->property("test4").toString(), QString("2Test"));
        QCOMPARE(object->property("test5").toInt(), 1);
        QCOMPARE(object->property("test6").toInt(), 1);
        QCOMPARE(object->property("test7").toInt(), 2);
        QCOMPARE(object->property("test8").toInt(), 2);
        QCOMPARE(object->property("test9").toInt(), 1);
        QCOMPARE(object->property("test10").toInt(), 3);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scope.2.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test1").toInt(), 19);
        QCOMPARE(object->property("test2").toInt(), 19);
        QCOMPARE(object->property("test3").toInt(), 14);
        QCOMPARE(object->property("test4").toInt(), 14);
        QCOMPARE(object->property("test5").toInt(), 24);
        QCOMPARE(object->property("test6").toInt(), 24);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scope.3.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test1").toBool(), true);
        QEXPECT_FAIL("", "Properties resolvable at compile time come before the global object, which is not 100% compatible with older QML versions", Continue);
        QCOMPARE(object->property("test2").toBool(), true);
        QCOMPARE(object->property("test3").toBool(), true);

        delete object;
    }

    // Signal argument scope
    {
        QQmlComponent component(&engine, testFileUrl("scope.4.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toInt(), 0);
        QCOMPARE(object->property("test2").toString(), QString());

        emit object->argumentSignal(13, "Argument Scope", 9, MyQmlObject::EnumValue4, Qt::RightButton);

        QCOMPARE(object->property("test").toInt(), 13);
        QCOMPARE(object->property("test2").toString(), QString("Argument Scope"));

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scope.5.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test1").toBool(), true);
        QCOMPARE(object->property("test2").toBool(), true);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scope.6.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toBool(), true);

        delete object;
    }
}

// In 4.7, non-library javascript files that had no imports shared the imports of their
// importing context
void tst_qqmlecmascript::importScope()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("importScope.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toInt(), 240);

    delete o;
}

/*
Tests that "any" type passes through a synthesized signal parameter.  This
is essentially a test of QQmlMetaType::copy()
*/
void tst_qqmlecmascript::signalParameterTypes()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("signalParameterTypes.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != nullptr);

    emit object->basicSignal();

    QCOMPARE(object->property("intProperty").toInt(), 10);
    QCOMPARE(object->property("realProperty").toReal(), 19.2);
    QVERIFY(object->property("colorProperty").value<QColor>() == QColor(255, 255, 0, 255));
    QVERIFY(object->property("variantProperty") == QVariant::fromValue(QColor(255, 0, 255, 255)));
    QVERIFY(object->property("enumProperty") == MyQmlObject::EnumValue3);
    QVERIFY(object->property("qtEnumProperty") == Qt::LeftButton);

    emit object->qjsValueEmittingSignal(QJSValue());
    QVERIFY(object->property("emittedQjsValueWasUndefined").toBool());
    emit object->qjsValueEmittingSignal(QJSValue(42));
    QVERIFY(!object->property("emittedQjsValueWasUndefined").toBool());
    QCOMPARE(object->property("emittedQjsValueAsInt").value<int>(), 42);

    delete object;
}

/*
Test that two JS objects for the same QObject compare as equal.
*/
void tst_qqmlecmascript::objectsCompareAsEqual()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("objectsCompareAsEqual.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test1").toBool(), true);
    QCOMPARE(object->property("test2").toBool(), true);
    QCOMPARE(object->property("test3").toBool(), true);
    QCOMPARE(object->property("test4").toBool(), true);
    QCOMPARE(object->property("test5").toBool(), true);

    delete object;
}

/*
Confirm bindings and alias properties can coexist.

Tests for a regression where the binding would not reevaluate.
*/
void tst_qqmlecmascript::aliasPropertyAndBinding()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("aliasPropertyAndBinding.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("c2").toInt(), 3);
    QCOMPARE(object->property("c3").toInt(), 3);

    object->setProperty("c2", QVariant(19));

    QCOMPARE(object->property("c2").toInt(), 19);
    QCOMPARE(object->property("c3").toInt(), 19);

    delete object;
}

/*
Ensure that we can write undefined value to an alias property,
and that the aliased property is reset correctly if possible.
*/
void tst_qqmlecmascript::aliasPropertyReset()
{
    QQmlEngine engine;
    QObject *object = nullptr;

    // test that a manual write (of undefined) to a resettable aliased property succeeds
    QQmlComponent c1(&engine, testFileUrl("aliasreset/aliasPropertyReset.1.qml"));
    object = c1.create();
    QVERIFY(object != nullptr);
    QVERIFY(object->property("sourceComponentAlias").value<QQmlComponent*>() != 0);
    QCOMPARE(object->property("aliasIsUndefined"), QVariant(false));
    QMetaObject::invokeMethod(object, "resetAliased");
    QVERIFY(!object->property("sourceComponentAlias").value<QQmlComponent*>());
    QCOMPARE(object->property("aliasIsUndefined"), QVariant(true));
    delete object;

    // test that a manual write (of undefined) to a resettable alias property succeeds
    QQmlComponent c2(&engine, testFileUrl("aliasreset/aliasPropertyReset.2.qml"));
    object = c2.create();
    QVERIFY(object != nullptr);
    QVERIFY(object->property("sourceComponentAlias").value<QQmlComponent*>() != 0);
    QCOMPARE(object->property("loaderSourceComponentIsUndefined"), QVariant(false));
    QMetaObject::invokeMethod(object, "resetAlias");
    QVERIFY(!object->property("sourceComponentAlias").value<QQmlComponent*>());
    QCOMPARE(object->property("loaderSourceComponentIsUndefined"), QVariant(true));
    delete object;

    // test that an alias to a bound property works correctly
    QQmlComponent c3(&engine, testFileUrl("aliasreset/aliasPropertyReset.3.qml"));
    object = c3.create();
    QVERIFY(object != nullptr);
    QVERIFY(object->property("sourceComponentAlias").value<QQmlComponent*>() != 0);
    QCOMPARE(object->property("loaderOneSourceComponentIsUndefined"), QVariant(false));
    QCOMPARE(object->property("loaderTwoSourceComponentIsUndefined"), QVariant(false));
    QMetaObject::invokeMethod(object, "resetAlias");
    QVERIFY(!object->property("sourceComponentAlias").value<QQmlComponent*>());
    QCOMPARE(object->property("loaderOneSourceComponentIsUndefined"), QVariant(true));
    QCOMPARE(object->property("loaderTwoSourceComponentIsUndefined"), QVariant(false));
    delete object;

    // test that a manual write (of undefined) to a resettable alias property
    // whose aliased property's object has been deleted, does not crash.
    QQmlComponent c4(&engine, testFileUrl("aliasreset/aliasPropertyReset.4.qml"));
    object = c4.create();
    QVERIFY(object != nullptr);
    QVERIFY(object->property("sourceComponentAlias").value<QQmlComponent*>() != 0);
    QObject *loader = object->findChild<QObject*>("loader");
    QVERIFY(loader != nullptr);
    delete loader;
    QVERIFY(object->property("sourceComponentAlias").value<QQmlComponent*>() == 0); // deletion should have caused value unset.
    QMetaObject::invokeMethod(object, "resetAlias"); // shouldn't crash.
    QVERIFY(!object->property("sourceComponentAlias").value<QQmlComponent*>());
    QMetaObject::invokeMethod(object, "setAlias");   // shouldn't crash, and shouldn't change value (since it's no longer referencing anything).
    QVERIFY(!object->property("sourceComponentAlias").value<QQmlComponent*>());
    delete object;

    // test that binding an alias property to an undefined value works correctly
    QQmlComponent c5(&engine, testFileUrl("aliasreset/aliasPropertyReset.5.qml"));
    object = c5.create();
    QVERIFY(object != nullptr);
    QVERIFY(object->property("sourceComponentAlias").value<QQmlComponent*>() == 0); // bound to undefined value.
    delete object;

    // test that a manual write (of undefined) to a non-resettable property fails properly
    QUrl url = testFileUrl("aliasreset/aliasPropertyReset.error.1.qml");
    QString warning1 = url.toString() + QLatin1String(":15: Error: Cannot assign [undefined] to int");
    QQmlComponent e1(&engine, url);
    object = e1.create();
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("intAlias").value<int>(), 12);
    QCOMPARE(object->property("aliasedIntIsUndefined"), QVariant(false));
    QTest::ignoreMessage(QtWarningMsg, warning1.toLatin1().constData());
    QMetaObject::invokeMethod(object, "resetAlias");
    QCOMPARE(object->property("intAlias").value<int>(), 12);
    QCOMPARE(object->property("aliasedIntIsUndefined"), QVariant(false));
    delete object;
}

void tst_qqmlecmascript::componentCreation_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("creationError");
    QTest::addColumn<QString>("createdParent");

    QTest::newRow("url")
        << "url"
        << ""
        << "";
    QTest::newRow("urlMode")
        << "urlMode"
        << ""
        << "";
    QTest::newRow("urlParent")
        << "urlParent"
        << ""
        << "obj";
    QTest::newRow("urlNullParent")
        << "urlNullParent"
        << ""
        << "null";
    QTest::newRow("urlModeParent")
        << "urlModeParent"
        << ""
        << "obj";
    QTest::newRow("urlModeNullParent")
        << "urlModeNullParent"
        << ""
        << "null";
    QTest::newRow("invalidSecondArg")
        << "invalidSecondArg"
        << ":40: Error: Qt.createComponent(): Invalid arguments"
        << "";
    QTest::newRow("invalidThirdArg")
        << "invalidThirdArg"
        << ":45: Error: Qt.createComponent(): Invalid parent object"
        << "";
    QTest::newRow("invalidMode")
        << "invalidMode"
        << ":50: Error: Qt.createComponent(): Invalid arguments"
        << "";
}

/*
Test using createComponent to dynamically generate a component.
*/
void tst_qqmlecmascript::componentCreation()
{
    QFETCH(QString, method);
    QFETCH(QString, creationError);
    QFETCH(QString, createdParent);

    QQmlEngine engine;
    QUrl testUrl(testFileUrl("componentCreation.qml"));

    if (!creationError.isEmpty()) {
        QString warning = testUrl.toString() + creationError;
        QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());
    }

    QQmlComponent component(&engine, testUrl);
    MyTypeObject *object = qobject_cast<MyTypeObject*>(component.create());
    QVERIFY(object != nullptr);

    QMetaObject::invokeMethod(object, method.toUtf8());
    QQmlComponent *created = object->componentProperty();

    if (creationError.isEmpty()) {
        QVERIFY(created);

        QObject *expectedParent = reinterpret_cast<QObject *>(quintptr(-1));
        if (createdParent == QLatin1String("obj")) {
            expectedParent = object;
        } else if ((createdParent == QLatin1String("null")) || createdParent.isEmpty()) {
            expectedParent = nullptr;
        }
        QCOMPARE(created->parent(), expectedParent);
    }
}

void tst_qqmlecmascript::dynamicCreation_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("createdName");

    QTest::newRow("One") << "createOne" << "objectOne";
    QTest::newRow("Two") << "createTwo" << "objectTwo";
    QTest::newRow("Three") << "createThree" << "objectThree";
}

/*
Test using createQmlObject to dynamically generate an item
Also using createComponent is tested.
*/
void tst_qqmlecmascript::dynamicCreation()
{
    QFETCH(QString, method);
    QFETCH(QString, createdName);

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("dynamicCreation.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);

    QMetaObject::invokeMethod(object, method.toUtf8());
    QObject *created = object->objectProperty();
    QVERIFY(created);
    QCOMPARE(created->objectName(), createdName);

    delete object;
}

/*
   Tests the destroy function
*/
void tst_qqmlecmascript::dynamicDestruction()
{
    QQmlEngine engine;

    {
    QQmlComponent component(&engine, testFileUrl("dynamicDeletion.qml"));
    QPointer<MyQmlObject> object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);
    QPointer<QObject> createdQmlObject = nullptr;

    QMetaObject::invokeMethod(object, "create");
    createdQmlObject = object->objectProperty();
    QVERIFY(createdQmlObject);
    QCOMPARE(createdQmlObject->objectName(), QString("emptyObject"));

    QMetaObject::invokeMethod(object, "killOther");
    QVERIFY(createdQmlObject);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    QVERIFY(createdQmlObject);
    for (int ii = 0; createdQmlObject && ii < 50; ++ii) { // After 5 seconds we should give up
        if (createdQmlObject) {
            QTest::qWait(100);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            QCoreApplication::processEvents();
        }
    }
    QVERIFY(!createdQmlObject);

    QQmlEngine::setObjectOwnership(object, QQmlEngine::JavaScriptOwnership);
    QMetaObject::invokeMethod(object, "killMe");
    QVERIFY(object);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    QVERIFY(!object);
    }

    {
    QQmlComponent component(&engine, testFileUrl("dynamicDeletion.2.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QVERIFY(!qvariant_cast<QObject*>(o->property("objectProperty")));

    QMetaObject::invokeMethod(o, "create");

    QVERIFY(qvariant_cast<QObject*>(o->property("objectProperty")) != 0);

    QMetaObject::invokeMethod(o, "destroy");

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    QVERIFY(!qvariant_cast<QObject*>(o->property("objectProperty")));

    delete o;
    }

    {
    // QTBUG-23451
    QPointer<QObject> createdQmlObject = nullptr;
    QQmlComponent component(&engine, testFileUrl("dynamicDeletion.3.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QVERIFY(!qvariant_cast<QObject*>(o->property("objectProperty")));
    QMetaObject::invokeMethod(o, "create");
    createdQmlObject = qvariant_cast<QObject*>(o->property("objectProperty"));
    QVERIFY(createdQmlObject);
    QMetaObject::invokeMethod(o, "destroy");
    QCOMPARE(qvariant_cast<bool>(o->property("test")), false);
    for (int ii = 0; createdQmlObject && ii < 50; ++ii) { // After 5 seconds we should give up
        QTest::qWait(100);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    }
    QVERIFY(!qvariant_cast<QObject*>(o->property("objectProperty")));
    QCOMPARE(qvariant_cast<bool>(o->property("test")), true);
    delete o;
    }
}

/*
   tests that id.toString() works
*/
void tst_qqmlecmascript::objectToString()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qmlToString.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "testToString");
    QVERIFY(object->stringProperty().startsWith("MyQmlObject_QML_"));
    QVERIFY(object->stringProperty().endsWith(", \"objName\")"));

    delete object;
}

/*
  tests that id.hasOwnProperty() works
*/
void tst_qqmlecmascript::objectHasOwnProperty()
{
    QUrl url = testFileUrl("qmlHasOwnProperty.qml");
    QString warning1 = url.toString() + ":59: TypeError: Cannot call method 'hasOwnProperty' of undefined";
    QString warning2 = url.toString() + ":64: TypeError: Cannot call method 'hasOwnProperty' of undefined";
    QString warning3 = url.toString() + ":69: TypeError: Cannot call method 'hasOwnProperty' of undefined";

    QQmlEngine engine;
    QQmlComponent component(&engine, url);
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    // test QObjects in QML
    QMetaObject::invokeMethod(object, "testHasOwnPropertySuccess");
    QVERIFY(object->property("result").value<bool>());
    QMetaObject::invokeMethod(object, "testHasOwnPropertyFailure");
    QVERIFY(!object->property("result").value<bool>());

    // now test other types in QML
    QObject *child = object->findChild<QObject*>("typeObj");
    QVERIFY(child != nullptr);
    QMetaObject::invokeMethod(child, "testHasOwnPropertySuccess");
    QCOMPARE(child->property("valueTypeHasOwnProperty").toBool(), true);
    QCOMPARE(child->property("valueTypeHasOwnProperty2").toBool(), true);
    QCOMPARE(child->property("variantTypeHasOwnProperty").toBool(), true);
    QCOMPARE(child->property("stringTypeHasOwnProperty").toBool(), true);
    QCOMPARE(child->property("listTypeHasOwnProperty").toBool(), true);
    QCOMPARE(child->property("emptyListTypeHasOwnProperty").toBool(), true);
    QCOMPARE(child->property("enumTypeHasOwnProperty").toBool(), true);
    QCOMPARE(child->property("typenameHasOwnProperty").toBool(), true);
    QCOMPARE(child->property("typenameHasOwnProperty2").toBool(), true);
    QCOMPARE(child->property("singletonTypeTypeHasOwnProperty").toBool(), true);
    QCOMPARE(child->property("singletonTypePropertyTypeHasOwnProperty").toBool(), true);

    QTest::ignoreMessage(QtWarningMsg, warning1.toLatin1().constData());
    QMetaObject::invokeMethod(child, "testHasOwnPropertyFailureOne");
    QCOMPARE(child->property("enumNonValueHasOwnProperty").toBool(), false);
    QTest::ignoreMessage(QtWarningMsg, warning2.toLatin1().constData());
    QMetaObject::invokeMethod(child, "testHasOwnPropertyFailureTwo");
    QCOMPARE(child->property("singletonTypeNonPropertyHasOwnProperty").toBool(), false);
    QTest::ignoreMessage(QtWarningMsg, warning3.toLatin1().constData());
    QMetaObject::invokeMethod(child, "testHasOwnPropertyFailureThree");
    QCOMPARE(child->property("listAtInvalidHasOwnProperty").toBool(), false);

    delete object;
}

/*
Tests bindings that indirectly cause their own deletion work.

This test is best run under valgrind to ensure no invalid memory access occur.
*/
void tst_qqmlecmascript::selfDeletingBinding()
{
    QQmlEngine engine;

    {
        QQmlComponent component(&engine, testFileUrl("selfDeletingBinding.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        object->setProperty("triggerDelete", true);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("selfDeletingBinding.2.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        object->setProperty("triggerDelete", true);
        delete object;
    }
}

/*
Test that extended object properties can be accessed.

This test a regression where this used to crash.  The issue was specificially
for extended objects that did not include a synthesized meta object (so non-root
and no synthesiszed properties).
*/
void tst_qqmlecmascript::extendedObjectPropertyLookup()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("extendedObjectPropertyLookup.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    delete object;
}

/*
Test that extended object properties can be accessed correctly.
*/
void tst_qqmlecmascript::extendedObjectPropertyLookup2()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("extendedObjectPropertyLookup2.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QVariant returnValue;
    QVERIFY(QMetaObject::invokeMethod(object, "getValue", Q_RETURN_ARG(QVariant, returnValue)));
    QCOMPARE(returnValue.toInt(), 42);

    delete object;
}

/*
Test failure when trying to create and uncreatable extended type object.
 */
void tst_qqmlecmascript::uncreatableExtendedObjectFailureCheck()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("uncreatableExtendedObjectFailureCheck.qml"));

    QObject *object = component.create();
    QVERIFY(!object);
}

/*
Test that an subclass of an uncreatable extended object contains all the required properties.
 */
void tst_qqmlecmascript::extendedObjectPropertyLookup3()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("extendedObjectPropertyLookup3.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QVariant returnValue;
    QVERIFY(QMetaObject::invokeMethod(object, "getAbstractProperty", Q_RETURN_ARG(QVariant, returnValue)));
    QCOMPARE(returnValue.toInt(), -1);
    QVERIFY(QMetaObject::invokeMethod(object, "getImplementedProperty", Q_RETURN_ARG(QVariant, returnValue)));
    QCOMPARE(returnValue.toInt(), 883);
    QVERIFY(QMetaObject::invokeMethod(object, "getExtendedProperty", Q_RETURN_ARG(QVariant, returnValue)));
    QCOMPARE(returnValue.toInt(), 42);

    delete object;
}
/*
Test file/lineNumbers for binding/Script errors.
*/
void tst_qqmlecmascript::scriptErrors()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("scriptErrors.qml"));
    QString url = component.url().toString();

    QString warning1 = url.left(url.length() - 3) + "js:2: Error: Invalid write to global property \"a\"";
    QString warning2 = url + ":5: ReferenceError: a is not defined";
    QString warning3 = url.left(url.length() - 3) + "js:4: Error: Invalid write to global property \"a\"";
    QString warning4 = url + ":13: ReferenceError: a is not defined";
    QString warning5 = url + ":11: ReferenceError: a is not defined";
    QString warning6 = url + ":10:5: Unable to assign [undefined] to int";
    QString warning7 = url + ":15: TypeError: Cannot assign to read-only property \"trueProperty\"";
    QString warning8 = url + ":16: Error: Cannot assign to non-existent property \"fakeProperty\"";

    QTest::ignoreMessage(QtWarningMsg, warning1.toLatin1().constData());
    QTest::ignoreMessage(QtWarningMsg, warning2.toLatin1().constData());
    QTest::ignoreMessage(QtWarningMsg, warning3.toLatin1().constData());
    QTest::ignoreMessage(QtWarningMsg, warning5.toLatin1().constData());
    QTest::ignoreMessage(QtWarningMsg, warning6.toLatin1().constData());
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != nullptr);

    QTest::ignoreMessage(QtWarningMsg, warning4.toLatin1().constData());
    emit object->basicSignal();

    QTest::ignoreMessage(QtWarningMsg, warning7.toLatin1().constData());
    emit object->anotherBasicSignal();

    QTest::ignoreMessage(QtWarningMsg, warning8.toLatin1().constData());
    emit object->thirdBasicSignal();

    delete object;
}

/*
Test file/lineNumbers for inline functions.
*/
void tst_qqmlecmascript::functionErrors()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("functionErrors.qml"));
    QString url = component.url().toString();

    QString warning = url + ":5: Error: Invalid write to global property \"a\"";

    QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());

    QObject *object = component.create();
    QVERIFY(object != nullptr);
    delete object;

    // test that if an exception occurs while invoking js function from cpp, it is reported as expected.
    QQmlComponent componentTwo(&engine, testFileUrl("scarceResourceFunctionFail.var.qml"));
    url = componentTwo.url().toString();
    object = componentTwo.create();
    QVERIFY(object != nullptr);

    QObject *resource = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    warning = url + QLatin1String(":16: TypeError: Property 'scarceResource' of object ScarceResourceObject(0x%1) is not a function");
    warning = warning.arg(QString::number((qintptr)resource, 16));
    QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData()); // we expect a meaningful warning to be printed.
    QMetaObject::invokeMethod(object, "retrieveScarceResource");
    delete object;
}

/*
Test various errors that can occur when assigning a property from script
*/
void tst_qqmlecmascript::propertyAssignmentErrors()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyAssignmentErrors.qml"));

    QString url = component.url().toString();

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test1").toBool(), true);
    QCOMPARE(object->property("test2").toBool(), true);

    delete object;
}

/*
Test bindings still work when the reeval is triggered from within
a signal script.
*/
void tst_qqmlecmascript::signalTriggeredBindings()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("signalTriggeredBindings.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("base").toReal(), 50.);
    QCOMPARE(object->property("test1").toReal(), 50.);
    QCOMPARE(object->property("test2").toReal(), 50.);

    object->basicSignal();

    QCOMPARE(object->property("base").toReal(), 200.);
    QCOMPARE(object->property("test1").toReal(), 200.);
    QCOMPARE(object->property("test2").toReal(), 200.);

    object->argumentSignal(10, QString(), 10, MyQmlObject::EnumValue4, Qt::RightButton);

    QCOMPARE(object->property("base").toReal(), 400.);
    QCOMPARE(object->property("test1").toReal(), 400.);
    QCOMPARE(object->property("test2").toReal(), 400.);

    delete object;
}

/*
Test that list properties can be iterated from ECMAScript
*/
void tst_qqmlecmascript::listProperties()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("listProperties.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test1").toInt(), 21);
    QCOMPARE(object->property("test2").toInt(), 2);
    QCOMPARE(object->property("test3").toBool(), true);
    QCOMPARE(object->property("test4").toBool(), true);

    delete object;
}

void tst_qqmlecmascript::exceptionClearsOnReeval()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("exceptionClearsOnReeval.qml"));
    QString url = component.url().toString();

    QString warning = url + ":4: TypeError: Cannot read property 'objectProperty' of null";

    QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), false);

    MyQmlObject object2;
    MyQmlObject object3;
    object2.setObjectProperty(&object3);
    object->setObjectProperty(&object2);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

void tst_qqmlecmascript::exceptionSlotProducesWarning()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("exceptionProducesWarning.qml"));
    QString url = component.url().toString();

    QString warning = component.url().toString() + ":6: Error: JS exception";

    QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);
    delete object;
}

void tst_qqmlecmascript::exceptionBindingProducesWarning()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("exceptionProducesWarning2.qml"));
    QString url = component.url().toString();

    QString warning = component.url().toString() + ":5: Error: JS exception";

    QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);
    delete object;
}

void tst_qqmlecmascript::compileInvalidBinding()
{
    // QTBUG-23387: ensure that invalid bindings don't cause a crash.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("v8bindingException.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    delete object;
}

// Check that transient binding errors are not displayed
void tst_qqmlecmascript::transientErrors()
{
    QQmlEngine engine;

    {
    QQmlComponent component(&engine, testFileUrl("transientErrors.qml"));

    QQmlTestMessageHandler messageHandler;

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QVERIFY2(messageHandler.messages().isEmpty(), qPrintable(messageHandler.messageString()));

    delete object;
    }

    // One binding erroring multiple times, but then resolving
    {
    QQmlComponent component(&engine, testFileUrl("transientErrors.2.qml"));

    QQmlTestMessageHandler messageHandler;

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QVERIFY2(messageHandler.messages().isEmpty(), qPrintable(messageHandler.messageString()));

    delete object;
    }
}

// Check that errors during shutdown are minimized
void tst_qqmlecmascript::shutdownErrors()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("shutdownErrors.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QQmlTestMessageHandler messageHandler;

    delete object;

    QVERIFY2(messageHandler.messages().isEmpty(), qPrintable(messageHandler.messageString()));
}

void tst_qqmlecmascript::compositePropertyType()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("compositePropertyType.qml"));

    QTest::ignoreMessage(QtDebugMsg, "hello world");
    QObject *object = qobject_cast<QObject *>(component.create());
    delete object;
}

// QTBUG-5759
void tst_qqmlecmascript::jsObject()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("jsObject.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toInt(), 92);

    delete object;
}

void tst_qqmlecmascript::undefinedResetsProperty()
{
    QQmlEngine engine;

    {
    QQmlComponent component(&engine, testFileUrl("undefinedResetsProperty.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("resettableProperty").toInt(), 92);

    object->setProperty("setUndefined", true);

    QCOMPARE(object->property("resettableProperty").toInt(), 13);

    object->setProperty("setUndefined", false);

    QCOMPARE(object->property("resettableProperty").toInt(), 92);

    delete object;
    }
    {
    QQmlComponent component(&engine, testFileUrl("undefinedResetsProperty.2.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("resettableProperty").toInt(), 19);

    QMetaObject::invokeMethod(object, "doReset");

    QCOMPARE(object->property("resettableProperty").toInt(), 13);

    delete object;
    }
}

// Aliases to variant properties should work
void tst_qqmlecmascript::qtbug_22464()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_22464.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

void tst_qqmlecmascript::qtbug_21580()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_21580.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

// Causes a v8 binding, but not all v8 bindings to be destroyed during evaluation
void tst_qqmlecmascript::singleV8BindingDestroyedDuringEvaluation()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("singleV8BindingDestroyedDuringEvaluation.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);
    delete object;
}

// QTBUG-6781
void tst_qqmlecmascript::bug1()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("bug.1.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toInt(), 14);

    object->setProperty("a", 11);

    QCOMPARE(object->property("test").toInt(), 3);

    object->setProperty("b", true);

    QCOMPARE(object->property("test").toInt(), 9);

    delete object;
}

#ifndef QT_NO_WIDGETS
void tst_qqmlecmascript::bug2()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Qt.test 1.0;\nQPlainTextEdit { width: 100 }", QUrl());

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    delete object;
}
#endif

// Don't crash in createObject when the component has errors.
void tst_qqmlecmascript::dynamicCreationCrash()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("dynamicCreation.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);

    QTest::ignoreMessage(QtWarningMsg, "QQmlComponent: Component is not ready");
    QMetaObject::invokeMethod(object, "dontCrash");
    QObject *created = object->objectProperty();
    QVERIFY(!created);

    delete object;
}

// ownership transferred to JS, ensure that GC runs the dtor
void tst_qqmlecmascript::dynamicCreationOwnership()
{
    int dtorCount = 0;
    int expectedDtorCount = 1; // start at 1 since we expect mdcdo to dtor too.

    // allow the engine to go out of scope too.
    {
        QQmlEngine dcoEngine;
        QQmlComponent component(&dcoEngine, testFileUrl("dynamicCreationOwnership.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        MyDynamicCreationDestructionObject *mdcdo = object->findChild<MyDynamicCreationDestructionObject*>("mdcdo");
        QVERIFY(mdcdo != nullptr);
        mdcdo->setDtorCount(&dtorCount);

        for (int i = 1; i < 105; ++i, ++expectedDtorCount) {
            QMetaObject::invokeMethod(object, "dynamicallyCreateJsOwnedObject");
            if (i % 90 == 0) {
                // we do this once manually, but it should be done automatically
                // when the engine goes out of scope (since it should gc in dtor)
                QMetaObject::invokeMethod(object, "performGc");
            }
            if (i % 10 == 0) {
                QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
                QCoreApplication::processEvents();
            }
        }

        delete object;
    }
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    QCOMPARE(dtorCount, expectedDtorCount);
}

void tst_qqmlecmascript::regExpBug()
{
    QQmlEngine engine;

    //QTBUG-9367
    {
        QQmlComponent component(&engine, testFileUrl("regExp.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->regExp().pattern(), QLatin1String("[a-zA-z]"));
        delete object;
    }

    //QTBUG-23068
    {
        QString err = QString(QLatin1String("%1:6 Invalid property assignment: regular expression expected; use /pattern/ syntax\n")).arg(testFileUrl("regExp.2.qml").toString());
        QQmlComponent component(&engine, testFileUrl("regExp.2.qml"));
        QTest::ignoreMessage(QtWarningMsg, "QQmlComponent: Component is not ready");
        MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
        QVERIFY(!object);
        QCOMPARE(component.errorString(), err);
    }
}

static inline bool evaluate_error(QV4::ExecutionEngine *v4, const QV4::Value &o, const char *source)
{
    QString functionSource = QLatin1String("(function(object) { return ") +
                             QLatin1String(source) + QLatin1String(" })");

    QV4::Scope scope(v4);
    QV4::Script program(QV4::ScopedContext(scope, scope.engine->rootContext()), QV4::Compiler::ContextType::Eval, functionSource);
    program.inheritContext = true;

    QV4::ScopedFunctionObject function(scope, program.run());
    if (scope.engine->hasException) {
        scope.engine->catchException();
        return true;
    }
    QV4::JSCallData jsCallData(scope, 1);
    jsCallData->args[0] = o;
    *jsCallData->thisObject = v4->global();
    function->call(jsCallData);
    if (scope.engine->hasException) {
        scope.engine->catchException();
        return true;
    }
    return false;
}

static inline bool evaluate_value(QV4::ExecutionEngine *v4, const QV4::Value &o,
                                  const char *source, const QV4::Value &result)
{
    QString functionSource = QLatin1String("(function(object) { return ") +
                             QLatin1String(source) + QLatin1String(" })");

    QV4::Scope scope(v4);
    QV4::Script program(QV4::ScopedContext(scope, scope.engine->rootContext()), QV4::Compiler::ContextType::Eval, functionSource);
    program.inheritContext = true;

    QV4::ScopedFunctionObject function(scope, program.run());
    if (scope.engine->hasException) {
        scope.engine->catchException();
        return false;
    }
    if (!function)
        return false;

    QV4::ScopedValue value(scope);
    QV4::JSCallData jsCallData(scope, 1);
    jsCallData->args[0] = o;
    *jsCallData->thisObject = v4->global();
    value = function->call(jsCallData);
    if (scope.engine->hasException) {
        scope.engine->catchException();
        return false;
    }
    return QV4::Runtime::method_strictEqual(value, result);
}

static inline QV4::ReturnedValue evaluate(QV4::ExecutionEngine *v4, const QV4::Value &o,
                                          const char *source)
{
    QString functionSource = QLatin1String("(function(object) { return ") +
                             QLatin1String(source) + QLatin1String(" })");

    QV4::Scope scope(v4);

    QV4::Script program(QV4::ScopedContext(scope, scope.engine->rootContext()), QV4::Compiler::ContextType::Eval, functionSource);
    program.inheritContext = true;

    QV4::ScopedFunctionObject function(scope, program.run());
    if (scope.engine->hasException) {
        scope.engine->catchException();
        return QV4::Encode::undefined();
    }
    if (!function)
        return QV4::Encode::undefined();
    QV4::JSCallData jsCallData(scope, 1);
    jsCallData->args[0] = o;
    *jsCallData->thisObject = v4->global();
    QV4::ScopedValue result(scope, function->call(jsCallData));
    if (scope.engine->hasException) {
        scope.engine->catchException();
        return QV4::Encode::undefined();
    }
    return result->asReturnedValue();
}

#define EVALUATE_ERROR(source) evaluate_error(engine, object, source)
#define EVALUATE_VALUE(source, result) evaluate_value(engine, object, source, result)
#define EVALUATE(source) evaluate(engine, object, source)

void tst_qqmlecmascript::callQtInvokables()
{
    // This object has JS ownership, as the call to method_NoArgs_QObject() in this test will return
    // it, which will set the indestructible flag to false.
    MyInvokableObject *o = new MyInvokableObject();

    QQmlEngine qmlengine;

    QV4::ExecutionEngine *engine = qmlengine.handle();
    QV4::Scope scope(engine);

    QV4::ScopedValue object(scope, QV4::QObjectWrapper::wrap(engine, o));

    // Non-existent methods
    o->reset();
    QVERIFY(EVALUATE_ERROR("object.method_nonexistent()"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), -1);
    QCOMPARE(o->actuals().count(), 0);

    o->reset();
    QVERIFY(EVALUATE_ERROR("object.method_nonexistent(10, 11)"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), -1);
    QCOMPARE(o->actuals().count(), 0);

    // Insufficient arguments
    o->reset();
    QVERIFY(EVALUATE_ERROR("object.method_int()"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), -1);
    QCOMPARE(o->actuals().count(), 0);

    o->reset();
    QVERIFY(EVALUATE_ERROR("object.method_intint(10)"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), -1);
    QCOMPARE(o->actuals().count(), 0);

    // Excessive arguments
    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_int(10, 11)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 8);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(10));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_intint(10, 11, 12)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 9);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(10));
    QCOMPARE(o->actuals().at(1), QVariant(11));

    // Test return types
    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_NoArgs()", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 0);
    QCOMPARE(o->actuals().count(), 0);

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_NoArgs_int()", QV4::Primitive::fromInt32(6)));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 1);
    QCOMPARE(o->actuals().count(), 0);

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_NoArgs_real()", QV4::Primitive::fromDouble(19.75)));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 2);
    QCOMPARE(o->actuals().count(), 0);

    o->reset();
    {
    QV4::ScopedValue ret(scope, EVALUATE("object.method_NoArgs_QPointF()"));
    QVERIFY(!ret->isUndefined());
    QCOMPARE(scope.engine->toVariant(ret, -1), QVariant(QPointF(123, 4.5)));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 3);
    QCOMPARE(o->actuals().count(), 0);
    }

    o->reset();
    {
    QV4::Scoped<QV4::QObjectWrapper> qobjectWrapper(scope, EVALUATE("object.method_NoArgs_QObject()"));
    QVERIFY(qobjectWrapper);
    QCOMPARE(qobjectWrapper->object(), (QObject *)o);
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 4);
    QCOMPARE(o->actuals().count(), 0);
    }

    o->reset();
    QVERIFY(EVALUATE_ERROR("object.method_NoArgs_unknown()"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), -1);
    QCOMPARE(o->actuals().count(), 0);

    o->reset();
    {
    QV4::ScopedValue ret(scope, EVALUATE("object.method_NoArgs_QScriptValue()"));
    QVERIFY(ret->isString());
    QCOMPARE(ret->toQStringNoThrow(), QString("Hello world"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 6);
    QCOMPARE(o->actuals().count(), 0);
    }

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_NoArgs_QVariant()", QV4::ScopedValue(scope, scope.engine->newString("QML rocks"))));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 7);
    QCOMPARE(o->actuals().count(), 0);

    // Test arg types
    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_int(94)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 8);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(94));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_int(\"94\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 8);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(94));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_int(\"not a number\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 8);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(0));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_int(null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 8);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(0));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_int(undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 8);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(0));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_int(object)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 8);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(0));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_intint(122, 9)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 9);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(122));
    QCOMPARE(o->actuals().at(1), QVariant(9));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_real(94.3)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 10);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(94.3));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_real(\"94.3\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 10);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(94.3));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_real(\"not a number\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 10);
    QCOMPARE(o->actuals().count(), 1);
    QVERIFY(qIsNaN(o->actuals().at(0).toDouble()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_real(null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 10);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(0));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_real(undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 10);
    QCOMPARE(o->actuals().count(), 1);
    QVERIFY(qIsNaN(o->actuals().at(0).toDouble()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_real(object)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 10);
    QCOMPARE(o->actuals().count(), 1);
    QVERIFY(qIsNaN(o->actuals().at(0).toDouble()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QString(\"Hello world\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 11);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant("Hello world"));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QString(19)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 11);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant("19"));

    o->reset();
    {
    QString expected = "MyInvokableObject(0x" + QString::number((quintptr)o, 16) + ")";
    QVERIFY(EVALUATE_VALUE("object.method_QString(object)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 11);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(expected));
    }

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QString(null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 11);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QString()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QString(undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 11);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QString()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QPointF(0)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 12);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QPointF()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QPointF(null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 12);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QPointF()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QPointF(undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 12);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QPointF()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QPointF(object)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 12);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QPointF()));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QPointF(object.method_get_QPointF())", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 12);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QPointF(99.3, -10.2)));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QPointF(object.method_get_QPoint())", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 12);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QPointF(9, 12)));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QObject(0)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 13);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), qVariantFromValue((QObject *)nullptr));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QObject(\"Hello world\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 13);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), qVariantFromValue((QObject *)nullptr));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QObject(null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 13);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), qVariantFromValue((QObject *)nullptr));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QObject(undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 13);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), qVariantFromValue((QObject *)nullptr));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QObject(object)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 13);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), qVariantFromValue((QObject *)o));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QScriptValue(null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 14);
    QCOMPARE(o->actuals().count(), 1);
    QVERIFY(qvariant_cast<QJSValue>(o->actuals().at(0)).isNull());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QScriptValue(undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 14);
    QCOMPARE(o->actuals().count(), 1);
    QVERIFY(qvariant_cast<QJSValue>(o->actuals().at(0)).isUndefined());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QScriptValue(19)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 14);
    QCOMPARE(o->actuals().count(), 1);
    QVERIFY(qvariant_cast<QJSValue>(o->actuals().at(0)).strictlyEquals(QJSValue(19)));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QScriptValue([19, 20])", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 14);
    QCOMPARE(o->actuals().count(), 1);
    QVERIFY(qvariant_cast<QJSValue>(o->actuals().at(0)).isArray());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_intQScriptValue(4, null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 15);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(4));
    QVERIFY(qvariant_cast<QJSValue>(o->actuals().at(1)).isNull());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_intQScriptValue(8, undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 15);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(8));
    QVERIFY(qvariant_cast<QJSValue>(o->actuals().at(1)).isUndefined());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_intQScriptValue(3, 19)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 15);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(3));
    QVERIFY(qvariant_cast<QJSValue>(o->actuals().at(1)).strictlyEquals(QJSValue(19)));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_intQScriptValue(44, [19, 20])", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 15);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(44));
    QVERIFY(qvariant_cast<QJSValue>(o->actuals().at(1)).isArray());

    o->reset();
    QVERIFY(EVALUATE_ERROR("object.method_overload()"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), -1);
    QCOMPARE(o->actuals().count(), 0);

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_overload(10)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 16);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(10));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_overload(10, 11)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 17);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(10));
    QCOMPARE(o->actuals().at(1), QVariant(11));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_overload(\"Hello\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 18);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(QString("Hello")));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_with_enum(9)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 19);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(9));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_default(10)", QV4::Primitive::fromInt32(19)));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 20);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(10));
    QCOMPARE(o->actuals().at(1), QVariant(19));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_default(10, 13)", QV4::Primitive::fromInt32(13)));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 20);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(10));
    QCOMPARE(o->actuals().at(1), QVariant(13));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_inherited(9)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), -3);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(o->actuals().at(0), QVariant(9));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QVariant(9)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 21);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(9));
    QCOMPARE(o->actuals().at(1), QVariant());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QVariant(\"Hello\", \"World\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 21);
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(QString("Hello")));
    QCOMPARE(o->actuals().at(1), QVariant(QString("World")));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonObject({foo:123})", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 22);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonObject>(o->actuals().at(0)), QJsonDocument::fromJson("{\"foo\":123}").object());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonArray([123])", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 23);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonArray>(o->actuals().at(0)), QJsonDocument::fromJson("[123]").array());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonValue(123)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 24);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(123));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonValue(42.35)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 24);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(42.35));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonValue('ciao')", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 24);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(QStringLiteral("ciao")));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonValue(true)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 24);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(true));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonValue(false)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 24);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(false));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonValue(null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 24);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(QJsonValue::Null));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QJsonValue(undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 24);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(QJsonValue::Undefined));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_overload({foo:123})", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 25);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonObject>(o->actuals().at(0)), QJsonDocument::fromJson("{\"foo\":123}").object());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_overload([123])", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 26);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonArray>(o->actuals().at(0)), QJsonDocument::fromJson("[123]").array());

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_overload(null)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 27);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(QJsonValue::Null));

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_overload(undefined)", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 27);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QJsonValue>(o->actuals().at(0)), QJsonValue(QJsonValue::Undefined));

    o->reset();
    QVERIFY(EVALUATE_ERROR("object.method_unknown(null)"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), -1);
    QCOMPARE(o->actuals().count(), 0);

    o->reset();
    QVERIFY(EVALUATE_VALUE("object.method_QByteArray(\"Hello\")", QV4::Primitive::undefinedValue()));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 29);
    QCOMPARE(o->actuals().count(), 1);
    QCOMPARE(qvariant_cast<QByteArray>(o->actuals().at(0)), QByteArray("Hello"));

    o->reset();
    QV4::ScopedValue ret(scope, EVALUATE("object.method_intQJSValue(123, function() { return \"Hello world!\";})"));
    QCOMPARE(o->error(), false);
    QCOMPARE(o->invoked(), 30);
    QVERIFY(ret->isString());
    QCOMPARE(ret->toQStringNoThrow(), QString("Hello world!"));
    QCOMPARE(o->actuals().count(), 2);
    QCOMPARE(o->actuals().at(0), QVariant(123));
    QJSValue callback = qvariant_cast<QJSValue>(o->actuals().at(1));
    QVERIFY(!callback.isNull());
    QVERIFY(callback.isCallable());
}

void tst_qqmlecmascript::resolveClashingProperties()
{
    ClashingNames *o = new ClashingNames();
    QQmlEngine qmlengine;

    QV4::ExecutionEngine *engine = qmlengine.handle();
    QV4::Scope scope(engine);

    QV4::ScopedValue object(scope, QV4::QObjectWrapper::wrap(engine, o));
    QV4::ObjectIterator it(scope, object->as<QV4::Object>(), QV4::ObjectIterator::EnumerableOnly);
    QV4::ScopedValue name(scope);
    QV4::ScopedValue value(scope);

    bool seenProperty = false;
    bool seenMethod = false;
    while (true) {
        QV4::Value v;
        name = it.nextPropertyNameAsString(&v);
        if (name->isNull())
            break;
        QString key = name->toQStringNoThrow();
        if (key == QLatin1String("clashes")) {
            value = v;
            QV4::ScopedValue typeString(scope, QV4::Runtime::method_typeofValue(engine, value));
            QString type = typeString->toQStringNoThrow();
            if (type == QLatin1String("boolean")) {
                QVERIFY(!seenProperty);
                seenProperty = true;
            } else if (type == QLatin1String("function")) {
                QVERIFY(!seenMethod);
                seenMethod = true;
            } else {
                QFAIL(qPrintable(QString::fromLatin1("found 'clashes' property of type %1")
                                 .arg(type)));
            }
        }
    }
    QVERIFY(seenProperty);
    QVERIFY(seenMethod);
}

// QTBUG-13047 (check that you can pass registered object types as args)
void tst_qqmlecmascript::invokableObjectArg()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("invokableObjectArg.qml"));

    QObject *o = component.create();
    QVERIFY(o);
    MyQmlObject *qmlobject = qobject_cast<MyQmlObject *>(o);
    QVERIFY(qmlobject);
    QCOMPARE(qmlobject->myinvokableObject, qmlobject);

    delete o;
}

// QTBUG-13047 (check that you can return registered object types from methods)
void tst_qqmlecmascript::invokableObjectRet()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("invokableObjectRet.qml"));

    QObject *o = component.create();
    QVERIFY(o);
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

void tst_qqmlecmascript::invokableEnumRet()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("invokableEnumRet.qml"));

    QObject *o = component.create();
    QVERIFY(o);
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

// QTBUG-5675
void tst_qqmlecmascript::listToVariant()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("listToVariant.qml"));

    MyQmlContainer container;

    QQmlContext context(engine.rootContext());
    context.setContextObject(&container);

    QObject *object = component.create(&context);
    QVERIFY(object != nullptr);

    QVariant v = object->property("test");
    QCOMPARE(v.userType(), qMetaTypeId<QQmlListReference>());
    QCOMPARE(qvariant_cast<QQmlListReference>(v).object(), &container);

    delete object;
}

// QTBUG-16316
void tst_qqmlecmascript::listAssignment()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("listAssignment.qml"));
    QObject *obj = component.create();
    QCOMPARE(obj->property("list1length").toInt(), 2);
    QQmlListProperty<MyQmlObject> list1 = obj->property("list1").value<QQmlListProperty<MyQmlObject> >();
    QQmlListProperty<MyQmlObject> list2 = obj->property("list2").value<QQmlListProperty<MyQmlObject> >();
    QCOMPARE(list1.count(&list1), list2.count(&list2));
    QCOMPARE(list1.at(&list1, 0), list2.at(&list2, 0));
    QCOMPARE(list1.at(&list1, 1), list2.at(&list2, 1));
    delete obj;
}

// QTBUG-7957
void tst_qqmlecmascript::multiEngineObject()
{
    MyQmlObject obj;
    obj.setStringProperty("Howdy planet");

    QQmlEngine e1;
    e1.rootContext()->setContextProperty("thing", &obj);
    QQmlComponent c1(&e1, testFileUrl("multiEngineObject.qml"));

    QQmlEngine e2;
    e2.rootContext()->setContextProperty("thing", &obj);
    QQmlComponent c2(&e2, testFileUrl("multiEngineObject.qml"));

    QObject *o1 = c1.create();
    QObject *o2 = c2.create();

    QCOMPARE(o1->property("test").toString(), QString("Howdy planet"));
    QCOMPARE(o2->property("test").toString(), QString("Howdy planet"));

    delete o2;
    delete o1;
}

// Test that references to QObjects are cleanup when the object is destroyed
void tst_qqmlecmascript::deletedObject()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deletedObject.qml"));

    QObject *object = component.create();

    QCOMPARE(object->property("test1").toBool(), true);
    QCOMPARE(object->property("test2").toBool(), true);
    QCOMPARE(object->property("test3").toBool(), true);
    QCOMPARE(object->property("test4").toBool(), true);

    delete object;
}

void tst_qqmlecmascript::attachedPropertyScope()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("attachedPropertyScope.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    MyQmlAttachedObject *attached =
        qobject_cast<MyQmlAttachedObject *>(qmlAttachedPropertiesObject<MyQmlObject>(object));
    QVERIFY(attached != nullptr);

    QCOMPARE(object->property("value2").toInt(), 0);

    attached->emitMySignal();

    QCOMPARE(object->property("value2").toInt(), 9);

    delete object;
}

void tst_qqmlecmascript::scriptConnect()
{
    QQmlEngine engine;

    {
        QQmlComponent component(&engine, testFileUrl("scriptConnect.1.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toBool(), false);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toBool(), true);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scriptConnect.2.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toBool(), false);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toBool(), true);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scriptConnect.3.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toBool(), false);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toBool(), true);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scriptConnect.4.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->methodCalled(), false);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->methodCalled(), true);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scriptConnect.5.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->methodCalled(), false);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->methodCalled(), true);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scriptConnect.6.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toInt(), 0);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 2);

        delete object;
    }
}

void tst_qqmlecmascript::scriptDisconnect()
{
    QQmlEngine engine;

    {
        QQmlComponent component(&engine, testFileUrl("scriptDisconnect.1.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toInt(), 0);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 1);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 2);
        emit object->basicSignal();
        QCOMPARE(object->property("test").toInt(), 2);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 2);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scriptDisconnect.2.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toInt(), 0);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 1);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 2);
        emit object->basicSignal();
        QCOMPARE(object->property("test").toInt(), 2);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 2);

        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("scriptDisconnect.3.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toInt(), 0);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 1);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 2);
        emit object->basicSignal();
        QCOMPARE(object->property("test").toInt(), 2);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 3);

        delete object;
    }
    {
        QQmlComponent component(&engine, testFileUrl("scriptDisconnect.4.qml"));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->property("test").toInt(), 0);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 1);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 2);
        emit object->basicSignal();
        QCOMPARE(object->property("test").toInt(), 2);
        emit object->argumentSignal(19, "Hello world!", 10.25, MyQmlObject::EnumValue4, Qt::RightButton);
        QCOMPARE(object->property("test").toInt(), 3);

        delete object;
    }
}

class OwnershipObject : public QObject
{
    Q_OBJECT
public:
    OwnershipObject() { object = new QObject; }

    QPointer<QObject> object;

public slots:
    QObject *getObject() { return object; }
};

void tst_qqmlecmascript::ownership()
{
    QQmlEngine engine;
    OwnershipObject own;
    QQmlContext *context = new QQmlContext(engine.rootContext());
    context->setContextObject(&own);

    {
        QQmlComponent component(&engine, testFileUrl("ownership.qml"));

        QVERIFY(own.object != nullptr);

        QObject *object = component.create(context);

        engine.collectGarbage();

        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();

        QVERIFY(own.object.isNull());

        delete object;
    }

    own.object = new QObject(&own);

    {
        QQmlComponent component(&engine, testFileUrl("ownership.qml"));

        QVERIFY(own.object != nullptr);

        QObject *object = component.create(context);

        engine.collectGarbage();

        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();

        QVERIFY(own.object != nullptr);

        delete object;
    }

    delete context;
}

class CppOwnershipReturnValue : public QObject
{
    Q_OBJECT
public:
    CppOwnershipReturnValue() : value(nullptr) {}
    ~CppOwnershipReturnValue() { delete value; }

    Q_INVOKABLE QObject *create() {
        value = new QObject;
        QQmlEngine::setObjectOwnership(value, QQmlEngine::CppOwnership);
        return value;
    }

    Q_INVOKABLE MyQmlObject *createQmlObject() {
        MyQmlObject *rv = new MyQmlObject;
        value = rv;
        return rv;
    }

    QPointer<QObject> value;
};

// QTBUG-15695.
// Test setObjectOwnership(CppOwnership) works even when there is no QQmlData
void tst_qqmlecmascript::cppOwnershipReturnValue()
{
    CppOwnershipReturnValue source;

    {
    QQmlEngine engine;
    engine.rootContext()->setContextProperty("source", &source);

    QVERIFY(source.value.isNull());

    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.0\nQtObject {\nComponent.onCompleted: { var a = source.create(); }\n}\n", QUrl());

    QObject *object = component.create();

    QVERIFY(object != nullptr);
    QVERIFY(source.value != nullptr);

    delete object;
    }

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    QVERIFY(source.value != nullptr);
}

// QTBUG-15697
void tst_qqmlecmascript::ownershipCustomReturnValue()
{
    QQmlEngine engine;
    CppOwnershipReturnValue source;

    {
    QQmlEngine engine;
    engine.rootContext()->setContextProperty("source", &source);

    QVERIFY(source.value.isNull());

    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.0\nQtObject {\nComponent.onCompleted: { var a = source.createQmlObject(); }\n}\n", QUrl());

    QObject *object = component.create();

    QVERIFY(object != nullptr);
    QVERIFY(source.value != nullptr);

    delete object;
    }

    engine.collectGarbage();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    QVERIFY(source.value.isNull());
}

//the return value from getObject will be JS ownership,
//unless strong Cpp ownership has been set
class OwnershipChangingObject : public QObject
{
    Q_OBJECT
public:
    OwnershipChangingObject(): object(nullptr) { }

    QPointer<QObject> object;

public slots:
    QObject *getObject() { return object; }
    void setObject(QObject *obj) { object = obj; }
};

void tst_qqmlecmascript::ownershipRootObject()
{
    QQmlEngine engine;
    OwnershipChangingObject own;
    QQmlContext *context = new QQmlContext(engine.rootContext());
    context->setContextObject(&own);

    QQmlComponent component(&engine, testFileUrl("ownershipRootObject.qml"));
    QPointer<QObject> object = component.create(context);
    QVERIFY(object);

    engine.collectGarbage();

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    QVERIFY(own.object != nullptr);

    delete context;
    delete object;
}

void tst_qqmlecmascript::ownershipConsistency()
{
    QQmlEngine engine;
    OwnershipChangingObject own;
    QQmlContext *context = new QQmlContext(engine.rootContext());
    context->setContextObject(&own);

    QString expectedWarning = testFileUrl("ownershipConsistency.qml").toString() + QLatin1String(":19: Error: Invalid attempt to destroy() an indestructible object");
    QTest::ignoreMessage(QtWarningMsg, qPrintable(expectedWarning)); // we expect a meaningful warning to be printed.
    expectedWarning = testFileUrl("ownershipConsistency.qml").toString() + QLatin1String(":15: Error: Invalid attempt to destroy() an indestructible object");
    QTest::ignoreMessage(QtWarningMsg, qPrintable(expectedWarning)); // we expect a meaningful warning to be printed.
    expectedWarning = testFileUrl("ownershipConsistency.qml").toString() + QLatin1String(":6: Error: Invalid attempt to destroy() an indestructible object");
    QTest::ignoreMessage(QtWarningMsg, qPrintable(expectedWarning)); // we expect a meaningful warning to be printed.
    expectedWarning = testFileUrl("ownershipConsistency.qml").toString() + QLatin1String(":10: Error: Invalid attempt to destroy() an indestructible object");
    QTest::ignoreMessage(QtWarningMsg, qPrintable(expectedWarning)); // we expect a meaningful warning to be printed.

    QQmlComponent component(&engine, testFileUrl("ownershipConsistency.qml"));
    QPointer<QObject> object = component.create(context);
    QVERIFY(object);

    engine.collectGarbage();

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    QVERIFY(own.object != nullptr);

    delete context;
    delete object;
}

void tst_qqmlecmascript::ownershipQmlIncubated()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("ownershipQmlIncubated.qml"));
    QObject *object = component.create();
    QVERIFY(object);

    QTRY_VERIFY(object->property("incubatedItem").value<QObject*>() != 0);

    QMetaObject::invokeMethod(object, "deleteIncubatedItem");

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();

    QVERIFY(!object->property("incubatedItem").value<QObject*>());

    delete object;
}

class QListQObjectMethodsObject : public QObject
{
    Q_OBJECT
public:
    QListQObjectMethodsObject() {
        m_objects.append(new MyQmlObject());
        m_objects.append(new MyQmlObject());
    }

    ~QListQObjectMethodsObject() {
        qDeleteAll(m_objects);
    }

public slots:
    QList<QObject *> getObjects() { return m_objects; }

private:
    QList<QObject *> m_objects;
};

// Tests that returning a QList<QObject*> from a method works
void tst_qqmlecmascript::qlistqobjectMethods()
{
    QQmlEngine engine;
    QListQObjectMethodsObject obj;
    QQmlContext *context = new QQmlContext(engine.rootContext());
    context->setContextObject(&obj);

    QQmlComponent component(&engine, testFileUrl("qlistqobjectMethods.qml"));

    QObject *object = component.create(context);

    QCOMPARE(object->property("test").toInt(), 2);
    QCOMPARE(object->property("test2").toBool(), true);

    delete object;
    delete context;
}

// QTBUG-9205
void tst_qqmlecmascript::strictlyEquals()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("strictlyEquals.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test1").toBool(), true);
    QCOMPARE(object->property("test2").toBool(), true);
    QCOMPARE(object->property("test3").toBool(), true);
    QCOMPARE(object->property("test4").toBool(), true);
    QCOMPARE(object->property("test5").toBool(), true);
    QCOMPARE(object->property("test6").toBool(), true);
    QCOMPARE(object->property("test7").toBool(), true);
    QCOMPARE(object->property("test8").toBool(), true);

    delete object;
}

void tst_qqmlecmascript::compiled()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("compiled.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test1").toReal(), qreal(15.7));
    QCOMPARE(object->property("test2").toReal(), qreal(-6.7));
    QCOMPARE(object->property("test3").toBool(), true);
    QCOMPARE(object->property("test4").toBool(), false);
    QCOMPARE(object->property("test5").toBool(), false);
    QCOMPARE(object->property("test6").toBool(), true);

    QCOMPARE(object->property("test7").toInt(), 185);
    QCOMPARE(object->property("test8").toInt(), 167);
    QCOMPARE(object->property("test9").toBool(), true);
    QCOMPARE(object->property("test10").toBool(), false);
    QCOMPARE(object->property("test11").toBool(), false);
    QCOMPARE(object->property("test12").toBool(), true);

    QCOMPARE(object->property("test13").toString(), QLatin1String("HelloWorld"));
    QCOMPARE(object->property("test14").toString(), QLatin1String("Hello World"));
    QCOMPARE(object->property("test15").toBool(), false);
    QCOMPARE(object->property("test16").toBool(), true);

    QCOMPARE(object->property("test17").toInt(), 4);
    QCOMPARE(object->property("test18").toReal(), qreal(176));
    QCOMPARE(object->property("test19").toInt(), 6);
    QCOMPARE(object->property("test20").toReal(), qreal(6.5));
    QCOMPARE(object->property("test21").toString(), QLatin1String("6.5"));
    QCOMPARE(object->property("test22").toString(), QLatin1String("!"));
    QCOMPARE(object->property("test23").toBool(), true);
    QCOMPARE(qvariant_cast<QColor>(object->property("test24")), QColor(0x11,0x22,0x33));
    QCOMPARE(qvariant_cast<QColor>(object->property("test25")), QColor(0x11,0x22,0x33,0xAA));

    delete object;
}

// Test that numbers assigned in bindings as strings work consistently
void tst_qqmlecmascript::numberAssignment()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("numberAssignment.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test1"), QVariant((qreal)6.7));
    QCOMPARE(object->property("test2"), QVariant((qreal)6.7));
    QCOMPARE(object->property("test2"), QVariant((qreal)6.7));
    QCOMPARE(object->property("test3"), QVariant((qreal)6));
    QCOMPARE(object->property("test4"), QVariant((qreal)6));

    QCOMPARE(object->property("test5"), QVariant((int)6));
    QCOMPARE(object->property("test6"), QVariant((int)7));
    QCOMPARE(object->property("test7"), QVariant((int)6));
    QCOMPARE(object->property("test8"), QVariant((int)6));

    QCOMPARE(object->property("test9"), QVariant((unsigned int)7));
    QCOMPARE(object->property("test10"), QVariant((unsigned int)7));
    QCOMPARE(object->property("test11"), QVariant((unsigned int)6));
    QCOMPARE(object->property("test12"), QVariant((unsigned int)6));

    delete object;
}

void tst_qqmlecmascript::propertySplicing()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertySplicing.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

// QTBUG-16683
void tst_qqmlecmascript::signalWithUnknownTypes()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("signalWithUnknownTypes.qml"));

    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != nullptr);

    MyQmlObject::MyType type;
    type.value = 0x8971123;
    emit object->signalWithUnknownType(type);

    MyQmlObject::MyType result = qvariant_cast<MyQmlObject::MyType>(object->variant());

    QCOMPARE(result.value, type.value);

    MyQmlObject::MyOtherType othertype;
    othertype.value = 17;
    emit object->signalWithCompletelyUnknownType(othertype);

    QVERIFY(!object->variant().isValid());

    delete object;
}

void tst_qqmlecmascript::signalWithJSValueInVariant_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<QString>("compare");

    QString compareStrict("(function(a, b) { return a === b; })");
    QTest::newRow("true") << "true" << compareStrict;
    QTest::newRow("undefined") << "undefined" << compareStrict;
    QTest::newRow("null") << "null" << compareStrict;
    QTest::newRow("123") << "123" << compareStrict;
    QTest::newRow("'ciao'") << "'ciao'" << compareStrict;

    QString comparePropertiesStrict(
        "(function compareMe(a, b) {"
        "  if (typeof b != 'object')"
        "    return a === b;"
        "  var props = Object.getOwnPropertyNames(b);"
        "  for (var i = 0; i < props.length; ++i) {"
        "    var p = props[i];"
        "    return compareMe(a[p], b[p]);"
        "  }"
        "})");
    QTest::newRow("{ foo: 'bar' }") << "({ foo: 'bar' })"  << comparePropertiesStrict;
    QTest::newRow("[10,20,30]") << "[10,20,30]"  << comparePropertiesStrict;
}

void tst_qqmlecmascript::signalWithJSValueInVariant()
{
    QFETCH(QString, expression);
    QFETCH(QString, compare);

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("signalWithJSValueInVariant.qml"));
    QScopedPointer<MyQmlObject> object(qobject_cast<MyQmlObject *>(component.create()));
    QVERIFY(object != nullptr);

    QJSValue value = engine.evaluate(expression);
    QVERIFY(!value.isError());
    object->setProperty("expression", expression);
    object->setProperty("compare", compare);
    object->setProperty("pass", false);

    emit object->signalWithVariant(QVariant::fromValue(value));
    QVERIFY(object->property("pass").toBool());
}

void tst_qqmlecmascript::signalWithJSValueInVariant_twoEngines_data()
{
    signalWithJSValueInVariant_data();
}

void tst_qqmlecmascript::signalWithJSValueInVariant_twoEngines()
{
    QFETCH(QString, expression);
    QFETCH(QString, compare);

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("signalWithJSValueInVariant.qml"));
    QScopedPointer<MyQmlObject> object(qobject_cast<MyQmlObject *>(component.create()));
    QVERIFY(object != nullptr);

    QJSEngine engine2;
    QJSValue value = engine2.evaluate(expression);
    QVERIFY(!value.isError());
    object->setProperty("expression", expression);
    object->setProperty("compare", compare);
    object->setProperty("pass", false);

    QTest::ignoreMessage(QtWarningMsg, "JSValue can't be reassigned to another engine.");
    emit object->signalWithVariant(QVariant::fromValue(value));
    if (expression == "undefined")
        // if the engine is wrong, we return undefined to the other engine,
        // making this one case pass
        return;
    QVERIFY(!object->property("pass").toBool());
}

void tst_qqmlecmascript::signalWithQJSValue_data()
{
    signalWithJSValueInVariant_data();
}

void tst_qqmlecmascript::signalWithQJSValue()
{
    QFETCH(QString, expression);
    QFETCH(QString, compare);

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("signalWithQJSValue.qml"));
    QScopedPointer<MyQmlObject> object(qobject_cast<MyQmlObject *>(component.create()));
    QVERIFY(object != nullptr);

    QJSValue value = engine.evaluate(expression);
    QVERIFY(!value.isError());
    object->setProperty("expression", expression);
    object->setProperty("compare", compare);
    object->setProperty("pass", false);

    emit object->signalWithQJSValue(value);

    QVERIFY(object->property("pass").toBool());
    QVERIFY(object->qjsvalue().strictlyEquals(value));
}

void tst_qqmlecmascript::singletonType_data()
{
    QTest::addColumn<QUrl>("testfile");
    QTest::addColumn<QString>("errorMessage");
    QTest::addColumn<QStringList>("warningMessages");
    QTest::addColumn<QStringList>("readProperties");
    QTest::addColumn<QVariantList>("readExpectedValues");
    QTest::addColumn<QStringList>("writeProperties");
    QTest::addColumn<QVariantList>("writeValues");
    QTest::addColumn<QStringList>("readBackProperties");
    QTest::addColumn<QVariantList>("readBackExpectedValues");

    QTest::newRow("qobject, register + read + method [no qualifier]")
            << testFileUrl("singletontype/qobjectSingletonTypeNoQualifier.qml")
            << QString()
            << QStringList()
            << (QStringList() << "qobjectPropertyTest" << "qobjectMethodTest")
            << (QVariantList() << 20 << 1)
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList();

    QTest::newRow("script, register + read [no qualifier]")
            << testFileUrl("singletontype/scriptSingletonTypeNoQualifier.qml")
            << QString()
            << QStringList()
            << (QStringList() << "scriptTest")
            << (QVariantList() << 13)
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList();

    QTest::newRow("qobject, register + read + method")
            << testFileUrl("singletontype/qobjectSingletonType.qml")
            << QString()
            << QStringList()
            << (QStringList() << "existingUriTest" << "qobjectTest" << "qobjectMethodTest"
                   << "qobjectMinorVersionMethodTest" << "qobjectMinorVersionTest"
                   << "qobjectMajorVersionTest" << "qobjectParentedTest")
            << (QVariantList() << 20 << 20 << 2 << 1 << 20 << 20 << 26)
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList();

    QTest::newRow("script, register + read")
            << testFileUrl("singletontype/scriptSingletonType.qml")
            << QString()
            << QStringList()
            << (QStringList() << "scriptTest")
            << (QVariantList() << 14) // will have incremented, since we create a new engine each row in this test.
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList();

    QTest::newRow("qobject, writing + readonly constraints")
            << testFileUrl("singletontype/qobjectSingletonTypeWriting.qml")
            << QString()
            << (QStringList() <<
                    QString(testFileUrl("singletontype/qobjectSingletonTypeWriting.qml").toString() + QLatin1String(":15: TypeError: Cannot assign to read-only property \"qobjectTestProperty\"")))
            << (QStringList() << "readOnlyProperty" << "writableProperty" << "writableFinalProperty")
            << (QVariantList() << 20 << 50 << 10)
            << (QStringList() << "firstProperty" << "secondProperty")
            << (QVariantList() << 30 << 30)
            << (QStringList() << "readOnlyProperty" << "writableProperty" << "writableFinalProperty")
            << (QVariantList() << 20 << 30 << 30);

    QTest::newRow("script, writing + readonly constraints")
            << testFileUrl("singletontype/scriptSingletonTypeWriting.qml")
            << QString()
            << (QStringList())
            << (QStringList() << "readBack" << "unchanged")
            << (QVariantList() << 15 << 42)
            << (QStringList() << "firstProperty" << "secondProperty")
            << (QVariantList() << 30 << 30)
            << (QStringList() << "readBack" << "unchanged")
            << (QVariantList() << 30 << 42);

    QTest::newRow("qobject singleton Type enum values in JS")
            << testFileUrl("singletontype/qobjectSingletonTypeEnums.qml")
            << QString()
            << QStringList()
            << (QStringList() << "enumValue" << "enumMethod")
            << (QVariantList() << 42 << 30)
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList();

    QTest::newRow("qobject, invalid major version fail")
            << testFileUrl("singletontype/singletonTypeMajorVersionFail.qml")
            << QString("QQmlComponent: Component is not ready")
            << QStringList()
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList();

    QTest::newRow("qobject, invalid minor version fail")
            << testFileUrl("singletontype/singletonTypeMinorVersionFail.qml")
            << QString("QQmlComponent: Component is not ready")
            << QStringList()
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList();

    QTest::newRow("qobject, multiple in namespace")
            << testFileUrl("singletontype/singletonTypeMultiple.qml")
            << QString()
            << QStringList()
            << (QStringList() << "first" << "second")
            << (QVariantList() << 35 << 42)
            << QStringList()
            << QVariantList()
            << QStringList()
            << QVariantList();
}

void tst_qqmlecmascript::singletonType()
{
    QFETCH(QUrl, testfile);
    QFETCH(QString, errorMessage);
    QFETCH(QStringList, warningMessages);
    QFETCH(QStringList, readProperties);
    QFETCH(QVariantList, readExpectedValues);
    QFETCH(QStringList, writeProperties);
    QFETCH(QVariantList, writeValues);
    QFETCH(QStringList, readBackProperties);
    QFETCH(QVariantList, readBackExpectedValues);

    QQmlEngine cleanEngine; // so tests don't interfere which each other, as singleton types are engine-singletons only.
    QQmlComponent component(&cleanEngine, testfile);

    if (!errorMessage.isEmpty())
        QTest::ignoreMessage(QtWarningMsg, errorMessage.toLatin1().constData());

    if (warningMessages.size())
        foreach (const QString &warning, warningMessages)
            QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());

    QObject *object = component.create();
    if (!errorMessage.isEmpty()) {
        QVERIFY(!object);
    } else {
        QVERIFY(object != nullptr);
        for (int i = 0; i < readProperties.size(); ++i)
            QCOMPARE(object->property(readProperties.at(i).toLatin1().constData()), readExpectedValues.at(i));
        for (int i = 0; i < writeProperties.size(); ++i)
            QVERIFY(object->setProperty(writeProperties.at(i).toLatin1().constData(), writeValues.at(i)));
        for (int i = 0; i < readBackProperties.size(); ++i)
            QCOMPARE(object->property(readBackProperties.at(i).toLatin1().constData()), readBackExpectedValues.at(i));
        delete object;
    }
}

void tst_qqmlecmascript::singletonTypeCaching_data()
{
    QTest::addColumn<QUrl>("testfile");
    QTest::addColumn<QStringList>("readProperties");

    QTest::newRow("qobject, caching + read")
            << testFileUrl("singletontype/qobjectSingletonTypeCaching.qml")
            << (QStringList() << "existingUriTest" << "qobjectParentedTest");

    QTest::newRow("script, caching + read")
            << testFileUrl("singletontype/scriptSingletonTypeCaching.qml")
            << (QStringList() << "scriptTest");
}

void tst_qqmlecmascript::singletonTypeCaching()
{
    QFETCH(QUrl, testfile);
    QFETCH(QStringList, readProperties);

    // ensure that the singleton type instances are cached per-engine.

    QQmlEngine cleanEngine;
    QQmlComponent component(&cleanEngine, testfile);
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QList<QVariant> firstValues;
    QMetaObject::invokeMethod(object, "modifyValues");
    for (int i = 0; i < readProperties.size(); ++i)
        firstValues << object->property(readProperties.at(i).toLatin1().constData());
    delete object;

    QQmlComponent component2(&cleanEngine, testfile);
    QObject *object2 = component2.create();
    QVERIFY(object2 != nullptr);
    for (int i = 0; i < readProperties.size(); ++i)
        QCOMPARE(object2->property(readProperties.at(i).toLatin1().constData()), firstValues.at(i)); // cached, shouldn't have changed.
    delete object2;
}

void tst_qqmlecmascript::singletonTypeImportOrder()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("singletontype/singletonTypeImportOrder.qml"));
    QObject *object = component.create();
    QVERIFY(object);
    QCOMPARE(object->property("v").toInt(), 1);
    delete object;
}

void tst_qqmlecmascript::singletonTypeResolution()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("singletontype/singletonTypeResolution.qml"));
    QObject *object = component.create();
    QVERIFY(object);
    QVERIFY(object->property("success").toBool());
    delete object;
}

void tst_qqmlecmascript::verifyContextLifetime(QQmlContextData *ctxt) {
    QQmlContextData *childCtxt = ctxt->childContexts;

    if (!ctxt->importedScripts.isNullOrUndefined()) {
        QV4::ExecutionEngine *v4 = ctxt->engine->handle();
        QV4::Scope scope(v4);
        QV4::ScopedArrayObject scripts(scope, ctxt->importedScripts.value());
        QV4::Scoped<QV4::QQmlContextWrapper> qml(scope);
        for (quint32 i = 0; i < scripts->getLength(); ++i) {
            QQmlContextData *scriptContext, *newContext;
            qml = scripts->get(i);

            scriptContext = qml ? qml->getContext() : nullptr;
            qml = QV4::Encode::undefined();

            {
                QV4::Scope scope(v4);
                QV4::ScopedContext temporaryScope(scope, QV4::QmlContext::create(scope.engine->rootContext(), scriptContext, nullptr));
                Q_UNUSED(temporaryScope)
            }

            ctxt->engine->collectGarbage();
            qml = scripts->get(i);
            newContext = qml ? qml->getContext() : nullptr;
            QCOMPARE(scriptContext, newContext);
        }
    }

    while (childCtxt) {
        verifyContextLifetime(childCtxt);

        childCtxt = childCtxt->nextChild;
    }
}

void tst_qqmlecmascript::importScripts_data()
{
    QTest::addColumn<QUrl>("testfile");
    QTest::addColumn<bool>("compilationShouldSucceed");
    QTest::addColumn<QString>("errorMessage");
    QTest::addColumn<QStringList>("warningMessages");
    QTest::addColumn<QStringList>("propertyNames");
    QTest::addColumn<QVariantList>("propertyValues");

    QTest::newRow("basic functionality")
            << testFileUrl("jsimport/testImport.qml")
            << true /* compilation should succeed */
            << QString()
            << QStringList()
            << (QStringList() << QLatin1String("importedScriptStringValue")
                              << QLatin1String("importedScriptFunctionValue")
                              << QLatin1String("importedModuleAttachedPropertyValue")
                              << QLatin1String("importedModuleEnumValue"))
            << (QVariantList() << QVariant(QLatin1String("Hello, World!"))
                               << QVariant(20)
                               << QVariant(19)
                               << QVariant(2));

    QTest::newRow("import scoping")
            << testFileUrl("jsimport/testImportScoping.qml")
            << true /* compilation should succeed */
            << QString()
            << QStringList()
            << (QStringList() << QLatin1String("componentError"))
            << (QVariantList() << QVariant(5));

    QTest::newRow("parent scope shouldn't be inherited by import with imports")
            << testFileUrl("jsimportfail/failOne.qml")
            << true /* compilation should succeed */
            << QString()
            << (QStringList() << QString(testFileUrl("jsimportfail/failOne.qml").toString() + QLatin1String(":6: TypeError: Cannot call method 'greetingString' of undefined")))
            << (QStringList() << QLatin1String("importScriptFunctionValue"))
            << (QVariantList() << QVariant(QString()));

    QTest::newRow("javascript imports in an import should be private to the import scope")
            << testFileUrl("jsimportfail/failTwo.qml")
            << true /* compilation should succeed */
            << QString()
            << (QStringList() << QString(testFileUrl("jsimportfail/failTwo.qml").toString() + QLatin1String(":6: ReferenceError: ImportOneJs is not defined")))
            << (QStringList() << QLatin1String("importScriptFunctionValue"))
            << (QVariantList() << QVariant(QString()));

    QTest::newRow("module imports in an import should be private to the import scope")
            << testFileUrl("jsimportfail/failThree.qml")
            << true /* compilation should succeed */
            << QString()
            << (QStringList() << QString(testFileUrl("jsimportfail/failThree.qml").toString() + QLatin1String(":7: TypeError: Cannot read property 'JsQtTest' of undefined")))
            << (QStringList() << QLatin1String("importedModuleAttachedPropertyValue"))
            << (QVariantList() << QVariant(false));

    QTest::newRow("typenames in an import should be private to the import scope")
            << testFileUrl("jsimportfail/failFour.qml")
            << true /* compilation should succeed */
            << QString()
            << (QStringList() << QString(testFileUrl("jsimportfail/failFour.qml").toString() + QLatin1String(":6: ReferenceError: JsQtTest is not defined")))
            << (QStringList() << QLatin1String("importedModuleEnumValue"))
            << (QVariantList() << QVariant(0));

    QTest::newRow("import with imports has it's own activation scope")
            << testFileUrl("jsimportfail/failFive.qml")
            << true /* compilation should succeed */
            << QString()
            << (QStringList() << QString(testFileUrl("jsimportfail/importWithImports.js").toString() + QLatin1String(":8: ReferenceError: Component is not defined")))
            << (QStringList() << QLatin1String("componentError"))
            << (QVariantList() << QVariant(0));

    QTest::newRow("import pragma library script")
            << testFileUrl("jsimport/testImportPragmaLibrary.qml")
            << true /* compilation should succeed */
            << QString()
            << QStringList()
            << (QStringList() << QLatin1String("testValue"))
            << (QVariantList() << QVariant(31));

    QTest::newRow("pragma library imports shouldn't inherit parent imports or scope")
            << testFileUrl("jsimportfail/testImportPragmaLibrary.qml")
            << true /* compilation should succeed */
            << QString()
            << (QStringList() << QString(testFileUrl("jsimportfail/importPragmaLibrary.js").toString() + QLatin1String(":6: ReferenceError: Component is not defined")))
            << (QStringList() << QLatin1String("testValue"))
            << (QVariantList() << QVariant(0));

    QTest::newRow("import pragma library script which has an import")
            << testFileUrl("jsimport/testImportPragmaLibraryWithImports.qml")
            << true /* compilation should succeed */
            << QString()
            << QStringList()
            << (QStringList() << QLatin1String("testValue"))
            << (QVariantList() << QVariant(55));

    QTest::newRow("import pragma library script which has a pragma library import")
            << testFileUrl("jsimport/testImportPragmaLibraryWithPragmaLibraryImports.qml")
            << true /* compilation should succeed */
            << QString()
            << QStringList()
            << (QStringList() << QLatin1String("testValue"))
            << (QVariantList() << QVariant(16));

    QTest::newRow("import singleton type into js import")
            << testFileUrl("jsimport/testImportSingletonType.qml")
            << true /* compilation should succeed */
            << QString()
            << QStringList()
            << (QStringList() << QLatin1String("testValue"))
            << (QVariantList() << QVariant(20));

    QTest::newRow("import module which exports a script")
            << testFileUrl("jsimport/testJsImport.qml")
            << true /* compilation should succeed */
            << QString()
            << QStringList()
            << (QStringList() << QLatin1String("importedScriptStringValue")
                              << QLatin1String("renamedScriptStringValue")
                              << QLatin1String("reimportedScriptStringValue"))
            << (QVariantList() << QVariant(QString("Hello"))
                               << QVariant(QString("Hello"))
                               << QVariant(QString("Hello")));

    QTest::newRow("import module which exports a script which imports a remote module")
            << testFileUrl("jsimport/testJsRemoteImport.qml")
            << true /* compilation should succeed */
            << QString()
            << QStringList()
            << (QStringList() << QLatin1String("importedScriptStringValue")
                              << QLatin1String("renamedScriptStringValue")
                              << QLatin1String("reimportedScriptStringValue"))
            << (QVariantList() << QVariant(QString("Hello"))
                               << QVariant(QString("Hello"))
                               << QVariant(QString("Hello")));

    QTest::newRow("malformed import statement")
            << testFileUrl("jsimportfail/malformedImport.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/malformedImport.js").toString() + QLatin1String(":1:2: Syntax error"))
            << QStringList()
            << QVariantList();

    QTest::newRow("malformed file name")
            << testFileUrl("jsimportfail/malformedFile.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/malformedFile.js").toString() + QLatin1String(":1:9: Imported file must be a script"))
            << QStringList()
            << QVariantList();

    QTest::newRow("missing file qualifier")
            << testFileUrl("jsimportfail/missingFileQualifier.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/missingFileQualifier.js").toString() + QLatin1String(":1:1: File import requires a qualifier"))
            << QStringList()
            << QVariantList();

    QTest::newRow("malformed file qualifier")
            << testFileUrl("jsimportfail/malformedFileQualifier.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/malformedFileQualifier.js").toString() + QLatin1String(":1:20: File import requires a qualifier"))
            << QStringList()
            << QVariantList();

    QTest::newRow("malformed module qualifier 2")
            << testFileUrl("jsimportfail/malformedFileQualifier.2.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/malformedFileQualifier.2.js").toString() + QLatin1String(":1:23: Invalid import qualifier"))
            << QStringList()
            << QVariantList();

    QTest::newRow("malformed module uri")
            << testFileUrl("jsimportfail/malformedModule.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/malformedModule.js").toString() + QLatin1String(":1:17: Invalid module URI"))
            << QStringList()
            << QVariantList();

    QTest::newRow("missing module version")
            << testFileUrl("jsimportfail/missingModuleVersion.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/missingModuleVersion.js").toString() + QLatin1String(":1:17: Module import requires a version"))
            << QStringList()
            << QVariantList();

    QTest::newRow("malformed module version")
            << testFileUrl("jsimportfail/malformedModuleVersion.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/malformedModuleVersion.js").toString() + QLatin1String(":1:17: Module import requires a version"))
            << QStringList()
            << QVariantList();

    QTest::newRow("missing module qualifier")
            << testFileUrl("jsimportfail/missingModuleQualifier.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/missingModuleQualifier.js").toString() + QLatin1String(":1:1: Module import requires a qualifier"))
            << QStringList()
            << QVariantList();

    QTest::newRow("malformed module qualifier")
            << testFileUrl("jsimportfail/malformedModuleQualifier.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/malformedModuleQualifier.js").toString() + QLatin1String(":1:21: Module import requires a qualifier"))
            << QStringList()
            << QVariantList();

    QTest::newRow("malformed module qualifier 2")
            << testFileUrl("jsimportfail/malformedModuleQualifier.2.qml")
            << false /* compilation should succeed */
            << QString()
            << (QStringList() << testFileUrl("jsimportfail/malformedModuleQualifier.2.js").toString() + QLatin1String(":1:24: Invalid import qualifier"))
            << QStringList()
            << QVariantList();
}

void tst_qqmlecmascript::importScripts()
{
    QFETCH(QUrl, testfile);
    QFETCH(bool, compilationShouldSucceed);
    QFETCH(QString, errorMessage);
    QFETCH(QStringList, warningMessages); // error messages if !compilationShouldSucceed
    QFETCH(QStringList, propertyNames);
    QFETCH(QVariantList, propertyValues);

    ThreadedTestHTTPServer server(dataDirectory() + "/remote");

    QQmlEngine engine;
    QString dataDir(dataDirectory() + QLatin1Char('/') + QLatin1String("lib"));
    engine.addImportPath(dataDir);

    QStringList importPathList = engine.importPathList();

    QString remotePath(server.urlString("/"));
    engine.addImportPath(remotePath);

    QQmlComponent component(&engine, testfile);

    if (!errorMessage.isEmpty())
        QTest::ignoreMessage(QtWarningMsg, errorMessage.toLatin1().constData());

    if (compilationShouldSucceed && warningMessages.size())
        foreach (const QString &warning, warningMessages)
            QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());

    if (compilationShouldSucceed)
        QTRY_VERIFY(component.isReady());
    else {
        QVERIFY(component.isError());
        QCOMPARE(warningMessages.size(), 1);
        QCOMPARE(component.errors().count(), 2);
        QCOMPARE(component.errors().at(1).toString(), warningMessages.first());
        return;
    }

    QObject *object = component.create();
    if (!errorMessage.isEmpty()) {
        QVERIFY(!object);
    } else {
        QVERIFY(object != nullptr);

        QQmlContextData *ctxt = QQmlContextData::get(engine.rootContext());
        tst_qqmlecmascript::verifyContextLifetime(ctxt);

        for (int i = 0; i < propertyNames.size(); ++i)
            QCOMPARE(object->property(propertyNames.at(i).toLatin1().constData()), propertyValues.at(i));
        delete object;
    }

    engine.setImportPathList(importPathList);
}

void tst_qqmlecmascript::importCreationContext()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("jsimport/creationContext.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());
    bool success = object->property("success").toBool();
    if (!success) {
        QSignalSpy readySpy(object.data(), SIGNAL(loaded()));
        readySpy.wait();
    }
    success = object->property("success").toBool();
    QVERIFY(success);
}

void tst_qqmlecmascript::scarceResources_other()
{
    /* These tests require knowledge of state, since we test values after
       performing signal or function invocation. */

    QPixmap origPixmap(100, 100);
    origPixmap.fill(Qt::blue);
    QString srp_name, expectedWarning;
    QQmlEngine engine;
    QV4::ExecutionEngine *v4 = engine.handle();
    ScarceResourceObject *eo = nullptr;
    QObject *srsc = nullptr;
    QObject *object = nullptr;

    /* property var semantics */

    // test that scarce resources are handled properly in signal invocation
    QQmlComponent varComponentTen(&engine, testFileUrl("scarceResourceSignal.var.qml"));
    object = varComponentTen.create();
    srsc = object->findChild<QObject*>("srsc");
    QVERIFY(srsc);
    QVERIFY(!srsc->property("scarceResourceCopy").isValid()); // hasn't been instantiated yet.
    QCOMPARE(srsc->property("width"), QVariant(5)); // default value is 5.
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    QMetaObject::invokeMethod(srsc, "testSignal");
    QVERIFY(!srsc->property("scarceResourceCopy").isValid()); // still hasn't been instantiated
    QCOMPARE(srsc->property("width"), QVariant(10)); // but width was assigned to 10.
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should still be no other copies of it at this stage.
    QMetaObject::invokeMethod(srsc, "testSignal2"); // assigns scarceResourceCopy to the scarce pixmap.
    QVERIFY(srsc->property("scarceResourceCopy").isValid());
    QCOMPARE(srsc->property("scarceResourceCopy").value<QPixmap>(), origPixmap);
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(!(eo->scarceResourceIsDetached())); // should be another copy of the resource now.
    QVERIFY(v4->scarceResources.isEmpty()); // should have been released by this point.
    delete object;

    // test that scarce resources are handled properly from js functions in qml files
    QQmlComponent varComponentEleven(&engine, testFileUrl("scarceResourceFunction.var.qml"));
    object = varComponentEleven.create();
    QVERIFY(object != nullptr);
    QVERIFY(!object->property("scarceResourceCopy").isValid()); // not yet assigned, so should not be valid
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    QMetaObject::invokeMethod(object, "retrieveScarceResource");
    QVERIFY(object->property("scarceResourceCopy").isValid()); // assigned, so should be valid.
    QCOMPARE(object->property("scarceResourceCopy").value<QPixmap>(), origPixmap);
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(!eo->scarceResourceIsDetached()); // should be a copy of the resource at this stage.
    QMetaObject::invokeMethod(object, "releaseScarceResource");
    QVERIFY(!object->property("scarceResourceCopy").isValid()); // just released, so should not be valid
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    QVERIFY(v4->scarceResources.isEmpty()); // should have been released by this point.
    delete object;

    // test that if an exception occurs while invoking js function from cpp, that the resources are released.
    QQmlComponent varComponentTwelve(&engine, testFileUrl("scarceResourceFunctionFail.var.qml"));
    object = varComponentTwelve.create();
    QVERIFY(object != nullptr);
    QVERIFY(!object->property("scarceResourceCopy").isValid()); // not yet assigned, so should not be valid
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    expectedWarning = varComponentTwelve.url().toString() + QLatin1String(":16: TypeError: Property 'scarceResource' of object ScarceResourceObject(0x%1) is not a function");
    expectedWarning = expectedWarning.arg(QString::number((qintptr)eo, 16));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(expectedWarning)); // we expect a meaningful warning to be printed.
    QMetaObject::invokeMethod(object, "retrieveScarceResource");
    QVERIFY(!object->property("scarceResourceCopy").isValid()); // due to exception, assignment will NOT have occurred.
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    QVERIFY(v4->scarceResources.isEmpty()); // should have been released by this point.
    delete object;

    // test that if an Item which has JS ownership but has a scarce resource property is garbage collected,
    // that the scarce resource is removed from the engine's list of scarce resources to clean up.
    QQmlComponent varComponentThirteen(&engine, testFileUrl("scarceResourceObjectGc.var.qml"));
    object = varComponentThirteen.create();
    QVERIFY(object != nullptr);
    QVERIFY(!object->property("varProperty").isValid()); // not assigned yet
    QMetaObject::invokeMethod(object, "assignVarProperty");
    QVERIFY(v4->scarceResources.isEmpty());             // the scarce resource is a VME property.
    QMetaObject::invokeMethod(object, "deassignVarProperty");
    gc(engine);
    QVERIFY(v4->scarceResources.isEmpty());             // should still be empty; the resource should have been released on gc.
    delete object;

    /* property variant semantics */

    // test that scarce resources are handled properly in signal invocation
    QQmlComponent variantComponentTen(&engine, testFileUrl("scarceResourceSignal.variant.qml"));
    object = variantComponentTen.create();
    QVERIFY(object != nullptr);
    srsc = object->findChild<QObject*>("srsc");
    QVERIFY(srsc);
    QVERIFY(!srsc->property("scarceResourceCopy").isValid()); // hasn't been instantiated yet.
    QCOMPARE(srsc->property("width"), QVariant(5)); // default value is 5.
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    QMetaObject::invokeMethod(srsc, "testSignal");
    QVERIFY(!srsc->property("scarceResourceCopy").isValid()); // still hasn't been instantiated
    QCOMPARE(srsc->property("width"), QVariant(10)); // but width was assigned to 10.
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should still be no other copies of it at this stage.
    QMetaObject::invokeMethod(srsc, "testSignal2"); // assigns scarceResourceCopy to the scarce pixmap.
    QVERIFY(srsc->property("scarceResourceCopy").isValid());
    QCOMPARE(srsc->property("scarceResourceCopy").value<QPixmap>(), origPixmap);
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(!(eo->scarceResourceIsDetached())); // should be another copy of the resource now.
    QVERIFY(v4->scarceResources.isEmpty()); // should have been released by this point.
    delete object;

    // test that scarce resources are handled properly from js functions in qml files
    QQmlComponent variantComponentEleven(&engine, testFileUrl("scarceResourceFunction.variant.qml"));
    object = variantComponentEleven.create();
    QVERIFY(object != nullptr);
    QVERIFY(!object->property("scarceResourceCopy").isValid()); // not yet assigned, so should not be valid
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    QMetaObject::invokeMethod(object, "retrieveScarceResource");
    QVERIFY(object->property("scarceResourceCopy").isValid()); // assigned, so should be valid.
    QCOMPARE(object->property("scarceResourceCopy").value<QPixmap>(), origPixmap);
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(!eo->scarceResourceIsDetached()); // should be a copy of the resource at this stage.
    QMetaObject::invokeMethod(object, "releaseScarceResource");
    QVERIFY(!object->property("scarceResourceCopy").isValid()); // just released, so should not be valid
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    QVERIFY(v4->scarceResources.isEmpty()); // should have been released by this point.
    delete object;

    // test that if an exception occurs while invoking js function from cpp, that the resources are released.
    QQmlComponent variantComponentTwelve(&engine, testFileUrl("scarceResourceFunctionFail.variant.qml"));
    object = variantComponentTwelve.create();
    QVERIFY(object != nullptr);
    QVERIFY(!object->property("scarceResourceCopy").isValid()); // not yet assigned, so should not be valid
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    expectedWarning = variantComponentTwelve.url().toString() + QLatin1String(":16: TypeError: Property 'scarceResource' of object ScarceResourceObject(0x%1) is not a function");
    expectedWarning = expectedWarning.arg(QString::number((qintptr)eo, 16));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(expectedWarning)); // we expect a meaningful warning to be printed.
    QMetaObject::invokeMethod(object, "retrieveScarceResource");
    QVERIFY(!object->property("scarceResourceCopy").isValid()); // due to exception, assignment will NOT have occurred.
    eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
    QVERIFY(eo->scarceResourceIsDetached()); // should be no other copies of it at this stage.
    QVERIFY(v4->scarceResources.isEmpty()); // should have been released by this point.
    delete object;
}

void tst_qqmlecmascript::scarceResources_data()
{
    QTest::addColumn<QUrl>("qmlFile");
    QTest::addColumn<bool>("readDetachStatus");
    QTest::addColumn<bool>("expectedDetachStatus");
    QTest::addColumn<QStringList>("propertyNames");
    QTest::addColumn<QVariantList>("expectedValidity");
    QTest::addColumn<QVariantList>("expectedValues");
    QTest::addColumn<QStringList>("expectedErrors");

    QPixmap origPixmap(100, 100);
    origPixmap.fill(Qt::blue);

    /* property var semantics */

    // in the following three cases, the instance created from the component
    // has a property which is a copy of the scarce resource; hence, the
    // resource should NOT be detached prior to deletion of the object instance,
    // unless the resource is destroyed explicitly.
    QTest::newRow("var: import scarce resource copy directly")
        << testFileUrl("scarceResourceCopy.var.qml")
        << true
        << false // won't be detached, because assigned to property and not explicitly released
        << (QStringList() << QLatin1String("scarceResourceCopy"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << origPixmap)
        << QStringList();

    QTest::newRow("var: import scarce resource copy from JS")
        << testFileUrl("scarceResourceCopyFromJs.var.qml")
        << true
        << false // won't be detached, because assigned to property and not explicitly released
        << (QStringList() << QLatin1String("scarceResourceCopy"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << origPixmap)
        << QStringList();

    QTest::newRow("var: import released scarce resource copy from JS")
        << testFileUrl("scarceResourceDestroyedCopy.var.qml")
        << true
        << true // explicitly released, so it will be detached
        << (QStringList() << QLatin1String("scarceResourceCopy"))
        << (QList<QVariant>() << false)
        << (QList<QVariant>() << QVariant())
        << QStringList();

    // in the following three cases, no other copy should exist in memory,
    // and so it should be detached (unless explicitly preserved).
    QTest::newRow("var: import auto-release SR from JS in binding side-effect")
        << testFileUrl("scarceResourceTest.var.qml")
        << true
        << true // auto released, so it will be detached
        << (QStringList() << QLatin1String("scarceResourceTest"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << QVariant(100))
        << QStringList();
    QTest::newRow("var: import explicit-preserve SR from JS in binding side-effect")
        << testFileUrl("scarceResourceTestPreserve.var.qml")
        << true
        << false // won't be detached because we explicitly preserve it
        << (QStringList() << QLatin1String("scarceResourceTest"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << QVariant(100))
        << QStringList();
    QTest::newRow("var: import explicit-preserve SR from JS in binding side-effect")
        << testFileUrl("scarceResourceTestMultiple.var.qml")
        << true
        << true // will be detached because all resources were released manually or automatically.
        << (QStringList() << QLatin1String("scarceResourceTest"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << QVariant(100))
        << QStringList();

    // In the following three cases, test that scarce resources are handled
    // correctly for imports.
    QTest::newRow("var: import with no binding")
        << testFileUrl("scarceResourceCopyImportNoBinding.var.qml")
        << false // cannot check detach status.
        << false
        << QStringList()
        << QList<QVariant>()
        << QList<QVariant>()
        << QStringList();
    QTest::newRow("var: import with binding without explicit preserve")
        << testFileUrl("scarceResourceCopyImportNoBinding.var.qml")
        << false
        << false
        << (QStringList() << QLatin1String("scarceResourceCopy"))
        << (QList<QVariant>() << false) // will have been released prior to evaluation of binding.
        << (QList<QVariant>() << QVariant())
        << QStringList();
    QTest::newRow("var: import with explicit release after binding evaluation")
        << testFileUrl("scarceResourceCopyImport.var.qml")
        << false
        << false
        << (QStringList() << QLatin1String("scarceResourceImportedCopy") << QLatin1String("scarceResourceAssignedCopyOne") << QLatin1String("scarceResourceAssignedCopyTwo") << QLatin1String("arePropertiesEqual"))
        << (QList<QVariant>() << false << false << false << true) // since property var = JS object reference, by releasing the provider's resource, all handles are invalidated.
        << (QList<QVariant>() << QVariant() << QVariant() << QVariant() << QVariant(true))
        << QStringList();
    QTest::newRow("var: import with different js objects")
        << testFileUrl("scarceResourceCopyImportDifferent.var.qml")
        << false
        << false
        << (QStringList() << QLatin1String("scarceResourceAssignedCopyOne") << QLatin1String("scarceResourceAssignedCopyTwo") << QLatin1String("arePropertiesEqual"))
        << (QList<QVariant>() << false << true << true) // invalidating one shouldn't invalidate the other, because they're not references to the same JS object.
        << (QList<QVariant>() << QVariant() << QVariant(origPixmap) << QVariant(false))
        << QStringList();
    QTest::newRow("var: import with different js objects and explicit release")
        << testFileUrl("scarceResourceMultipleDifferentNoBinding.var.qml")
        << false
        << false
        << (QStringList() << QLatin1String("resourceOne") << QLatin1String("resourceTwo"))
        << (QList<QVariant>() << true << false) // invalidating one shouldn't invalidate the other, because they're not references to the same JS object.
        << (QList<QVariant>() << QVariant(origPixmap) << QVariant())
        << QStringList();
    QTest::newRow("var: import with same js objects and explicit release")
        << testFileUrl("scarceResourceMultipleSameNoBinding.var.qml")
        << false
        << false
        << (QStringList() << QLatin1String("resourceOne") << QLatin1String("resourceTwo"))
        << (QList<QVariant>() << false << false) // invalidating one should invalidate the other, because they're references to the same JS object.
        << (QList<QVariant>() << QVariant() << QVariant())
        << QStringList();
    QTest::newRow("var: binding with same js objects and explicit release")
        << testFileUrl("scarceResourceMultipleSameWithBinding.var.qml")
        << false
        << false
        << (QStringList() << QLatin1String("resourceOne") << QLatin1String("resourceTwo"))
        << (QList<QVariant>() << false << false) // invalidating one should invalidate the other, because they're references to the same JS object.
        << (QList<QVariant>() << QVariant() << QVariant())
        << QStringList();


    /* property variant semantics */

    // in the following three cases, the instance created from the component
    // has a property which is a copy of the scarce resource; hence, the
    // resource should NOT be detached prior to deletion of the object instance,
    // unless the resource is destroyed explicitly.
    QTest::newRow("variant: import scarce resource copy directly")
        << testFileUrl("scarceResourceCopy.variant.qml")
        << true
        << false // won't be detached, because assigned to property and not explicitly released
        << (QStringList() << QLatin1String("scarceResourceCopy"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << origPixmap)
        << QStringList();

    QTest::newRow("variant: import scarce resource copy from JS")
        << testFileUrl("scarceResourceCopyFromJs.variant.qml")
        << true
        << false // won't be detached, because assigned to property and not explicitly released
        << (QStringList() << QLatin1String("scarceResourceCopy"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << origPixmap)
        << QStringList();

    QTest::newRow("variant: import released scarce resource copy from JS")
        << testFileUrl("scarceResourceDestroyedCopy.variant.qml")
        << true
        << true // explicitly released, so it will be detached
        << (QStringList() << QLatin1String("scarceResourceCopy"))
        << (QList<QVariant>() << false)
        << (QList<QVariant>() << QVariant())
        << QStringList();

    // in the following three cases, no other copy should exist in memory,
    // and so it should be detached (unless explicitly preserved).
    QTest::newRow("variant: import auto-release SR from JS in binding side-effect")
        << testFileUrl("scarceResourceTest.variant.qml")
        << true
        << true // auto released, so it will be detached
        << (QStringList() << QLatin1String("scarceResourceTest"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << QVariant(100))
        << QStringList();
    QTest::newRow("variant: import explicit-preserve SR from JS in binding side-effect")
        << testFileUrl("scarceResourceTestPreserve.variant.qml")
        << true
        << false // won't be detached because we explicitly preserve it
        << (QStringList() << QLatin1String("scarceResourceTest"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << QVariant(100))
        << QStringList();
    QTest::newRow("variant: import multiple scarce resources")
        << testFileUrl("scarceResourceTestMultiple.variant.qml")
        << true
        << true // will be detached because all resources were released manually or automatically.
        << (QStringList() << QLatin1String("scarceResourceTest"))
        << (QList<QVariant>() << true)
        << (QList<QVariant>() << QVariant(100))
        << QStringList();

    // In the following three cases, test that scarce resources are handled
    // correctly for imports.
    QTest::newRow("variant: import with no binding")
        << testFileUrl("scarceResourceCopyImportNoBinding.variant.qml")
        << false // cannot check detach status.
        << false
        << QStringList()
        << QList<QVariant>()
        << QList<QVariant>()
        << QStringList();
    QTest::newRow("variant: import with binding without explicit preserve")
        << testFileUrl("scarceResourceCopyImportNoBinding.variant.qml")
        << false
        << false
        << (QStringList() << QLatin1String("scarceResourceCopy"))
        << (QList<QVariant>() << false) // will have been released prior to evaluation of binding.
        << (QList<QVariant>() << QVariant())
        << QStringList();
    QTest::newRow("variant: import with explicit release after binding evaluation")
        << testFileUrl("scarceResourceCopyImport.variant.qml")
        << false
        << false
        << (QStringList() << QLatin1String("scarceResourceImportedCopy") << QLatin1String("scarceResourceAssignedCopyOne") << QLatin1String("scarceResourceAssignedCopyTwo"))
        << (QList<QVariant>() << true << true << false) // since property variant = variant copy, releasing the provider's resource does not invalidate previously assigned copies.
        << (QList<QVariant>() << origPixmap << origPixmap << QVariant())
        << QStringList();
}

void tst_qqmlecmascript::scarceResources()
{
    QFETCH(QUrl, qmlFile);
    QFETCH(bool, readDetachStatus);
    QFETCH(bool, expectedDetachStatus);
    QFETCH(QStringList, propertyNames);
    QFETCH(QVariantList, expectedValidity);
    QFETCH(QVariantList, expectedValues);
    QFETCH(QStringList, expectedErrors);

    QQmlEngine engine;
    QV4::ExecutionEngine *v4 = engine.handle();
    ScarceResourceObject *eo = nullptr;
    QObject *object = nullptr;

    QQmlComponent c(&engine, qmlFile);
    object = c.create();
    QVERIFY(object != nullptr);
    for (int i = 0; i < propertyNames.size(); ++i) {
        QString prop = propertyNames.at(i);
        bool validity = expectedValidity.at(i).toBool();
        QVariant value = expectedValues.at(i);

        QCOMPARE(object->property(prop.toLatin1().constData()).isValid(), validity);
        if (value.type() == QVariant::Int) {
            QCOMPARE(object->property(prop.toLatin1().constData()).toInt(), value.toInt());
        } else if (value.type() == QVariant::Pixmap) {
            QCOMPARE(object->property(prop.toLatin1().constData()).value<QPixmap>(), value.value<QPixmap>());
        }
    }

    if (readDetachStatus) {
        eo = qobject_cast<ScarceResourceObject*>(QQmlProperty::read(object, "a").value<QObject*>());
        QCOMPARE(eo->scarceResourceIsDetached(), expectedDetachStatus);
    }

    QVERIFY(v4->scarceResources.isEmpty());
    delete object;
}

void tst_qqmlecmascript::propertyChangeSlots()
{
    // ensure that allowable property names are allowed and onPropertyNameChanged slots are generated correctly.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("changeslots/propertyChangeSlots.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    delete object;

    // ensure that invalid property names fail properly.
    QTest::ignoreMessage(QtWarningMsg, "QQmlComponent: Component is not ready");
    QQmlComponent e1(&engine, testFileUrl("changeslots/propertyChangeSlotErrors.1.qml"));
    QString expectedErrorString = e1.url().toString() + QLatin1String(":9:5: Cannot assign to non-existent property \"on_nameWithUnderscoreChanged\"");
    QCOMPARE(e1.errors().at(0).toString(), expectedErrorString);
    object = e1.create();
    QVERIFY(!object);
    delete object;

    QTest::ignoreMessage(QtWarningMsg, "QQmlComponent: Component is not ready");
    QQmlComponent e2(&engine, testFileUrl("changeslots/propertyChangeSlotErrors.2.qml"));
    expectedErrorString = e2.url().toString() + QLatin1String(":9:5: Cannot assign to non-existent property \"on____nameWithUnderscoresChanged\"");
    QCOMPARE(e2.errors().at(0).toString(), expectedErrorString);
    object = e2.create();
    QVERIFY(!object);
    delete object;

    QTest::ignoreMessage(QtWarningMsg, "QQmlComponent: Component is not ready");
    QQmlComponent e3(&engine, testFileUrl("changeslots/propertyChangeSlotErrors.3.qml"));
    expectedErrorString = e3.url().toString() + QLatin1String(":9:5: Cannot assign to non-existent property \"on$NameWithDollarsignChanged\"");
    QCOMPARE(e3.errors().at(0).toString(), expectedErrorString);
    object = e3.create();
    QVERIFY(!object);
    delete object;

    QTest::ignoreMessage(QtWarningMsg, "QQmlComponent: Component is not ready");
    QQmlComponent e4(&engine, testFileUrl("changeslots/propertyChangeSlotErrors.4.qml"));
    expectedErrorString = e4.url().toString() + QLatin1String(":9:5: Cannot assign to non-existent property \"on_6NameWithUnderscoreNumberChanged\"");
    QCOMPARE(e4.errors().at(0).toString(), expectedErrorString);
    object = e4.create();
    QVERIFY(!object);
    delete object;
}

void tst_qqmlecmascript::propertyVar_data()
{
    QTest::addColumn<QUrl>("qmlFile");

    // valid
    QTest::newRow("non-bindable object subproperty changed") << testFileUrl("propertyVar.1.qml");
    QTest::newRow("non-bindable object changed") << testFileUrl("propertyVar.2.qml");
    QTest::newRow("primitive changed") << testFileUrl("propertyVar.3.qml");
    QTest::newRow("javascript array modification") << testFileUrl("propertyVar.4.qml");
    QTest::newRow("javascript map modification") << testFileUrl("propertyVar.5.qml");
    QTest::newRow("javascript array assignment") << testFileUrl("propertyVar.6.qml");
    QTest::newRow("javascript map assignment") << testFileUrl("propertyVar.7.qml");
    QTest::newRow("literal property assignment") << testFileUrl("propertyVar.8.qml");
    QTest::newRow("qobject property assignment") << testFileUrl("propertyVar.9.qml");
    QTest::newRow("base class var property assignment") << testFileUrl("propertyVar.10.qml");
    QTest::newRow("javascript function assignment") << testFileUrl("propertyVar.11.qml");
    QTest::newRow("javascript special assignment") << testFileUrl("propertyVar.12.qml");
    QTest::newRow("declarative binding assignment") << testFileUrl("propertyVar.13.qml");
    QTest::newRow("imperative binding assignment") << testFileUrl("propertyVar.14.qml");
    QTest::newRow("stored binding assignment") << testFileUrl("propertyVar.15.qml");
    QTest::newRow("function expression binding assignment") << testFileUrl("propertyVar.16.qml");
}

void tst_qqmlecmascript::propertyVar()
{
    QFETCH(QUrl, qmlFile);

    QQmlEngine engine;
    QQmlComponent component(&engine, qmlFile);
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

void tst_qqmlecmascript::propertyQJSValue_data()
{
    QTest::addColumn<QUrl>("qmlFile");

    // valid
    QTest::newRow("non-bindable object subproperty changed") << testFileUrl("propertyQJSValue.1.qml");
    QTest::newRow("non-bindable object changed") << testFileUrl("propertyQJSValue.2.qml");
    QTest::newRow("primitive changed") << testFileUrl("propertyQJSValue.3.qml");
    QTest::newRow("javascript array modification") << testFileUrl("propertyQJSValue.4.qml");
    QTest::newRow("javascript map modification") << testFileUrl("propertyQJSValue.5.qml");
    QTest::newRow("javascript array assignment") << testFileUrl("propertyQJSValue.6.qml");
    QTest::newRow("javascript map assignment") << testFileUrl("propertyQJSValue.7.qml");
    QTest::newRow("literal property assignment") << testFileUrl("propertyQJSValue.8.qml");
    QTest::newRow("qobject property assignment") << testFileUrl("propertyQJSValue.9.qml");
    QTest::newRow("base class var property assignment") << testFileUrl("propertyQJSValue.10.qml");
    QTest::newRow("javascript function assignment") << testFileUrl("propertyQJSValue.11.qml");
    QTest::newRow("javascript special assignment") << testFileUrl("propertyQJSValue.12.qml");
    QTest::newRow("declarative binding assignment") << testFileUrl("propertyQJSValue.13.qml");
    QTest::newRow("imperative binding assignment") << testFileUrl("propertyQJSValue.14.qml");
    QTest::newRow("stored binding assignment") << testFileUrl("propertyQJSValue.15.qml");
    QTest::newRow("javascript function binding") << testFileUrl("propertyQJSValue.16.qml");

    QTest::newRow("reset property") << testFileUrl("propertyQJSValue.reset.qml");
    QTest::newRow("reset property in binding") << testFileUrl("propertyQJSValue.bindingreset.qml");
}

void tst_qqmlecmascript::propertyQJSValue()
{
    QFETCH(QUrl, qmlFile);

    QQmlEngine engine;
    QQmlComponent component(&engine, qmlFile);
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

// Tests that we can write QVariant values to var properties from C++
void tst_qqmlecmascript::propertyVarCpp()
{
    QObject *object = nullptr;

    // ensure that writing to and reading from a var property from cpp works as required.
    // Literal values stored in var properties can be read and written as QVariants
    // of a specific type, whereas object values are read as QVariantMaps.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyVarCpp.qml"));
    object = component.create();
    QVERIFY(object != nullptr);
    // assign int to property var that currently has int assigned
    QVERIFY(object->setProperty("varProperty", QVariant::fromValue(10)));
    QCOMPARE(object->property("varBound"), QVariant(15));
    QCOMPARE(object->property("intBound"), QVariant(15));
    QVERIFY(isJSNumberType(object->property("varProperty").userType()));
    QVERIFY(isJSNumberType(object->property("varBound").userType()));
    // assign string to property var that current has bool assigned
    QCOMPARE(object->property("varProperty2").userType(), (int)QVariant::Bool);
    QVERIFY(object->setProperty("varProperty2", QVariant(QLatin1String("randomString"))));
    QCOMPARE(object->property("varProperty2"), QVariant(QLatin1String("randomString")));
    QCOMPARE(object->property("varProperty2").userType(), (int)QVariant::String);
    // now enforce behaviour when accessing JavaScript objects from cpp.
    QCOMPARE(object->property("jsobject").userType(), qMetaTypeId<QJSValue>());
    delete object;
}

void tst_qqmlecmascript::propertyVarOwnership()
{
    QQmlEngine engine;

    // Referenced JS objects are not collected
    {
    QQmlComponent component(&engine, testFileUrl("propertyVarOwnership.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("test").toBool(), false);
    QMetaObject::invokeMethod(object, "runTest");
    QCOMPARE(object->property("test").toBool(), true);
    delete object;
    }
    // Referenced JS objects are not collected
    {
    QQmlComponent component(&engine, testFileUrl("propertyVarOwnership.2.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("test").toBool(), false);
    QMetaObject::invokeMethod(object, "runTest");
    QCOMPARE(object->property("test").toBool(), true);
    delete object;
    }
    // Qt objects are not collected until they've been dereferenced
    {
    QQmlComponent component(&engine, testFileUrl("propertyVarOwnership.3.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test2").toBool(), false);
    QCOMPARE(object->property("test2").toBool(), false);

    QMetaObject::invokeMethod(object, "runTest");
    QCOMPARE(object->property("test1").toBool(), true);

    QPointer<QObject> referencedObject = object->property("object").value<QObject*>();
    QVERIFY(!referencedObject.isNull());
    gc(engine);
    QVERIFY(!referencedObject.isNull());

    QMetaObject::invokeMethod(object, "runTest2");
    QCOMPARE(object->property("test2").toBool(), true);
    gc(engine);
    QVERIFY(referencedObject.isNull());

    delete object;
    }
    // Self reference does not prevent Qt object collection
    {
    QQmlComponent component(&engine, testFileUrl("propertyVarOwnership.4.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    QPointer<QObject> referencedObject = object->property("object").value<QObject*>();
    QVERIFY(!referencedObject.isNull());
    gc(engine);
    QVERIFY(!referencedObject.isNull());

    QMetaObject::invokeMethod(object, "runTest");
    gc(engine);
    QVERIFY(referencedObject.isNull());

    delete object;
    }
    // Garbage collection cannot result in attempted dereference of empty handle
    {
    QQmlComponent component(&engine, testFileUrl("propertyVarOwnership.5.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "createComponent");
    engine.collectGarbage();
    QMetaObject::invokeMethod(object, "runTest");
    QCOMPARE(object->property("test").toBool(), true);
    delete object;
    }
}

void tst_qqmlecmascript::propertyVarImplicitOwnership()
{
    // The childObject has a reference to a different QObject.  We want to ensure
    // that the different item will not be cleaned up until required.  IE, the childObject
    // has implicit ownership of the constructed QObject.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyVarImplicitOwnership.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "assignCircular");
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); // process deleteLater() events from QV8QObjectWrapper.
    QCoreApplication::processEvents();
    QObject *rootObject = object->property("vp").value<QObject*>();
    QVERIFY(rootObject != nullptr);
    QObject *childObject = rootObject->findChild<QObject*>("text");
    QVERIFY(childObject != nullptr);
    QCOMPARE(rootObject->property("rectCanary").toInt(), 5);
    QCOMPARE(childObject->property("textCanary").toInt(), 10);
    QMetaObject::invokeMethod(childObject, "constructQObject");    // creates a reference to a constructed QObject.
    QPointer<QObject> qobjectGuard(childObject->property("vp").value<QObject*>()); // get the pointer prior to processing deleteLater events.
    QVERIFY(!qobjectGuard.isNull());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); // process deleteLater() events from QV8QObjectWrapper.
    QCoreApplication::processEvents();
    QVERIFY(!qobjectGuard.isNull());
    QMetaObject::invokeMethod(object, "deassignCircular");
    gc(engine);
    QVERIFY(qobjectGuard.isNull());                                // should have been collected now.
    delete object;
}

void tst_qqmlecmascript::propertyVarReparent()
{
    // ensure that nothing breaks if we re-parent objects
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyVar.reparent.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "assignVarProp");
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); // process deleteLater() events from QV8QObjectWrapper.
    QCoreApplication::processEvents();
    QObject *rect = object->property("vp").value<QObject*>();
    QObject *text = rect->findChild<QObject*>("textOne");
    QObject *text2 = rect->findChild<QObject*>("textTwo");
    QPointer<QObject> rectGuard(rect);
    QPointer<QObject> textGuard(text);
    QPointer<QObject> text2Guard(text2);
    QVERIFY(!rectGuard.isNull());
    QVERIFY(!textGuard.isNull());
    QVERIFY(!text2Guard.isNull());
    QCOMPARE(text->property("textCanary").toInt(), 11);
    QCOMPARE(text2->property("textCanary").toInt(), 12);
    // now construct an image which we will reparent.
    QMetaObject::invokeMethod(text2, "constructQObject");
    QObject *image = text2->property("vp").value<QObject*>();
    QPointer<QObject> imageGuard(image);
    QVERIFY(!imageGuard.isNull());
    QCOMPARE(image->property("imageCanary").toInt(), 13);
    // now reparent the "Image" object (currently, it has JS ownership)
    image->setParent(text);                                        // shouldn't be collected after deassignVp now, since has a parent.
    QMetaObject::invokeMethod(text2, "deassignVp");
    gc(engine);
    QCOMPARE(text->property("textCanary").toInt(), 11);
    QCOMPARE(text2->property("textCanary").toInt(), 22);
    QVERIFY(!imageGuard.isNull());                                 // should still be alive.
    QCOMPARE(image->property("imageCanary").toInt(), 13);          // still able to access var properties
    QMetaObject::invokeMethod(object, "deassignVarProp");          // now deassign the root-object's vp, causing gc of rect+text+text2
    gc(engine);
    QVERIFY(imageGuard.isNull());                                  // should now have been deleted, due to parent being deleted.
    delete object;
}

void tst_qqmlecmascript::propertyVarReparentNullContext()
{
    // sometimes reparenting can cause problems
    // (eg, if the ctxt is collected, varproperties are no longer available)
    // this test ensures that no crash occurs in that situation.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyVar.reparent.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "assignVarProp");
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); // process deleteLater() events from QV8QObjectWrapper.
    QCoreApplication::processEvents();
    QObject *rect = object->property("vp").value<QObject*>();
    QObject *text = rect->findChild<QObject*>("textOne");
    QObject *text2 = rect->findChild<QObject*>("textTwo");
    QPointer<QObject> rectGuard(rect);
    QPointer<QObject> textGuard(text);
    QPointer<QObject> text2Guard(text2);
    QVERIFY(!rectGuard.isNull());
    QVERIFY(!textGuard.isNull());
    QVERIFY(!text2Guard.isNull());
    QCOMPARE(text->property("textCanary").toInt(), 11);
    QCOMPARE(text2->property("textCanary").toInt(), 12);
    // now construct an image which we will reparent.
    QMetaObject::invokeMethod(text2, "constructQObject");
    QObject *image = text2->property("vp").value<QObject*>();
    QPointer<QObject> imageGuard(image);
    QVERIFY(!imageGuard.isNull());
    QCOMPARE(image->property("imageCanary").toInt(), 13);
    // now reparent the "Image" object (currently, it has JS ownership)
    image->setParent(object);                                      // reparented to base object.  after deassignVarProp, the ctxt will be invalid.
    QMetaObject::invokeMethod(object, "deassignVarProp");          // now deassign the root-object's vp, causing gc of rect+text+text2
    gc(engine);
    QVERIFY(!imageGuard.isNull());                                 // should still be alive.
    QVERIFY(!image->property("imageCanary").isValid());            // but varProperties won't be available (null context).
    delete object;
    QVERIFY(imageGuard.isNull());                                  // should now be dead.
}

void tst_qqmlecmascript::propertyVarCircular()
{
    // enforce behaviour regarding circular references - ensure qdvmemo deletion.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyVar.circular.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "assignCircular");           // cause assignment and gc
    {
        QCOMPARE(object->property("canaryInt"), QVariant(5));
        QVariant canaryResourceVariant = object->property("canaryResource");
        QVERIFY(canaryResourceVariant.isValid());
    }

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); // process deleteLater() events from QV8QObjectWrapper.
    QCoreApplication::processEvents();
    QCOMPARE(object->property("canaryInt"), QVariant(5));
    QVariant canaryResourceVariant = object->property("canaryResource");
    QVERIFY(canaryResourceVariant.isValid());
    QPixmap canaryResourcePixmap = canaryResourceVariant.value<QPixmap>();
    canaryResourceVariant = QVariant();                            // invalidate it to remove one copy of the pixmap from memory.
    QMetaObject::invokeMethod(object, "deassignCanaryResource");   // remove one copy of the pixmap from memory
    gc(engine);
    QVERIFY(!canaryResourcePixmap.isDetached());                   // two copies extant - this and the propertyVar.vp.vp.vp.vp.memoryHog.
    QMetaObject::invokeMethod(object, "deassignCircular");         // cause deassignment and gc
    gc(engine);
    QCOMPARE(object->property("canaryInt"), QVariant(2));
    QCOMPARE(object->property("canaryResource"), QVariant(1));
    QVERIFY(canaryResourcePixmap.isDetached());                    // now detached, since orig copy was member of qdvmemo which was deleted.
    delete object;
}

void tst_qqmlecmascript::propertyVarCircular2()
{
    // track deletion of JS-owned parent item with Cpp-owned child
    // where the child has a var property referencing its parent.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyVar.circular.2.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "assignCircular");
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); // process deleteLater() events from QV8QObjectWrapper.
    QCoreApplication::processEvents();
    QObject *rootObject = object->property("vp").value<QObject*>();
    QVERIFY(rootObject != nullptr);
    QObject *childObject = rootObject->findChild<QObject*>("text");
    QVERIFY(childObject != nullptr);
    QPointer<QObject> rootObjectTracker(rootObject);
    QVERIFY(!rootObjectTracker.isNull());
    QPointer<QObject> childObjectTracker(childObject);
    QVERIFY(!childObjectTracker.isNull());
    gc(engine);
    QCOMPARE(rootObject->property("rectCanary").toInt(), 5);
    QCOMPARE(childObject->property("textCanary").toInt(), 10);
    QMetaObject::invokeMethod(object, "deassignCircular");
    gc(engine);
    QVERIFY(rootObjectTracker.isNull());                           // should have been collected
    QVERIFY(childObjectTracker.isNull());                          // should have been collected
    delete object;
}

void tst_qqmlecmascript::propertyVarInheritance()
{
    // enforce behaviour regarding element inheritance - ensure handle disposal.
    // The particular component under test here has a chain of references.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyVar.inherit.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "assignCircular");           // cause assignment and gc
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); // process deleteLater() events from QV8QObjectWrapper.
    QCoreApplication::processEvents();
    // we want to be able to track when the varProperties array of the last metaobject is disposed
    QObject *cco5 = object->property("varProperty").value<QObject*>()->property("vp").value<QObject*>()->property("vp").value<QObject*>()->property("vp").value<QObject*>()->property("vp").value<QObject*>();
    QObject *ico5 = object->property("varProperty").value<QObject*>()->property("inheritanceVarProperty").value<QObject*>()->property("vp").value<QObject*>()->property("vp").value<QObject*>()->property("vp").value<QObject*>()->property("vp").value<QObject*>();
    QVERIFY(cco5);
    QVERIFY(ico5);
    QQmlVMEMetaObject *icovmemo = QQmlVMEMetaObject::get(ico5);
    QQmlVMEMetaObject *ccovmemo = QQmlVMEMetaObject::get(cco5);
    QV4::WeakValue icoCanaryHandle;
    QV4::WeakValue ccoCanaryHandle;
    {
        // XXX NOTE: this is very implementation dependent.  QDVMEMO->vmeProperty() is the only
        // public function which can return us a handle to something in the varProperties array.
        QV4::ReturnedValue tmp = icovmemo->vmeProperty(ico5->metaObject()->indexOfProperty("circ"));
        icoCanaryHandle.set(engine.handle(), tmp);
        tmp = ccovmemo->vmeProperty(cco5->metaObject()->indexOfProperty("circ"));
        ccoCanaryHandle.set(engine.handle(), tmp);
        tmp = QV4::Encode::null();
        QVERIFY(!icoCanaryHandle.isUndefined());
        QVERIFY(!ccoCanaryHandle.isUndefined());
        gc(engine);
        QVERIFY(!icoCanaryHandle.isUndefined());
        QVERIFY(!ccoCanaryHandle.isUndefined());
    }
    // now we deassign the var prop, which should trigger collection of item subtrees.
    QMetaObject::invokeMethod(object, "deassignCircular");         // cause deassignment and gc
    // ensure that there are only weak handles to the underlying varProperties array remaining.
    gc(engine);
    // an equivalent for pragma GCC optimize is still work-in-progress for CLang, so this test will fail.
    QVERIFY(icoCanaryHandle.isUndefined());
    QVERIFY(ccoCanaryHandle.isUndefined());
    delete object;
    // since there are no parent vmemo's to keep implicit references alive, and the only handles
    // to what remains are weak, all varProperties arrays must have been collected.
}

void tst_qqmlecmascript::propertyVarInheritance2()
{
    // The particular component under test here does NOT have a chain of references; the
    // only link between rootObject and childObject is that rootObject is the parent of childObject.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyVar.circular.2.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "assignCircular");
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); // process deleteLater() events from QV8QObjectWrapper.
    QCoreApplication::processEvents();
    QObject *rootObject = object->property("vp").value<QObject*>();
    QVERIFY(rootObject != nullptr);
    QObject *childObject = rootObject->findChild<QObject*>("text");
    QVERIFY(childObject != nullptr);
    QCOMPARE(rootObject->property("rectCanary").toInt(), 5);
    QCOMPARE(childObject->property("textCanary").toInt(), 10);
    QV4::WeakValue childObjectVarArrayValueHandle;
    {
        childObjectVarArrayValueHandle.set(engine.handle(),
                                           QQmlVMEMetaObject::get(childObject)->vmeProperty(childObject->metaObject()->indexOfProperty("vp")));
        QVERIFY(!childObjectVarArrayValueHandle.isUndefined());
        gc(engine);
        QVERIFY(!childObjectVarArrayValueHandle.isUndefined()); // should not have been collected yet.
        QCOMPARE(childObject->property("vp").value<QObject*>(), rootObject);
        QCOMPARE(childObject->property("textCanary").toInt(), 10);
    }
    QMetaObject::invokeMethod(object, "deassignCircular");
    gc(engine);
    // an equivalent for pragma GCC optimize is still work-in-progress for CLang, so this test will fail.
    QVERIFY(childObjectVarArrayValueHandle.isUndefined()); // should have been collected now.
    delete object;
}


// Ensure that QObject type conversion works on binding assignment
void tst_qqmlecmascript::elementAssign()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("elementAssign.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

// QTBUG-12457
void tst_qqmlecmascript::objectPassThroughSignals()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("objectsPassThroughSignals.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

// QTBUG-21626
void tst_qqmlecmascript::objectConversion()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("objectConversion.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QVariant retn;
    QMetaObject::invokeMethod(object, "circularObject", Q_RETURN_ARG(QVariant, retn));
    QCOMPARE(retn.value<QJSValue>().property("test").toInt(), int(100));

    delete object;
}


// QTBUG-20242
void tst_qqmlecmascript::booleanConversion()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("booleanConversion.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test_true1").toBool(), true);
    QCOMPARE(object->property("test_true2").toBool(), true);
    QCOMPARE(object->property("test_true3").toBool(), true);
    QCOMPARE(object->property("test_true4").toBool(), true);
    QCOMPARE(object->property("test_true5").toBool(), true);

    QCOMPARE(object->property("test_false1").toBool(), false);
    QCOMPARE(object->property("test_false2").toBool(), false);
    QCOMPARE(object->property("test_false3").toBool(), false);

    delete object;
}

void tst_qqmlecmascript::handleReferenceManagement()
{
    int dtorCount = 0;
    {
        // Linear QObject reference
        QQmlEngine hrmEngine;
        QQmlComponent component(&hrmEngine, testFileUrl("handleReferenceManagement.object.1.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        CircularReferenceObject *cro = object->findChild<CircularReferenceObject*>("cro");
        cro->setDtorCount(&dtorCount);
        QMetaObject::invokeMethod(object, "createReference");
        gc(hrmEngine);
        QCOMPARE(dtorCount, 0); // second has JS ownership, kept alive by first's reference
        delete object;
        hrmEngine.collectGarbage();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
        QCOMPARE(dtorCount, 3);
    }

    dtorCount = 0;
    {
        // Circular QObject reference
        QQmlEngine hrmEngine;
        QQmlComponent component(&hrmEngine, testFileUrl("handleReferenceManagement.object.2.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        CircularReferenceObject *cro = object->findChild<CircularReferenceObject*>("cro");
        cro->setDtorCount(&dtorCount);
        QMetaObject::invokeMethod(object, "circularReference");
        gc(hrmEngine);
        QCOMPARE(dtorCount, 2); // both should be cleaned up, since circular references shouldn't keep alive.
        delete object;
        hrmEngine.collectGarbage();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
        QCOMPARE(dtorCount, 3);
    }

    {
        // Dynamic variant property reference keeps target alive
        QQmlEngine hrmEngine;
        QQmlComponent component(&hrmEngine, testFileUrl("handleReferenceManagement.dynprop.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QMetaObject::invokeMethod(object, "createReference");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "ensureReference");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "removeReference");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "ensureDeletion");
        QCOMPARE(object->property("success").toBool(), true);
        delete object;
    }

    {
        // Dynamic Item property reference keeps target alive
        QQmlEngine hrmEngine;
        QQmlComponent component(&hrmEngine, testFileUrl("handleReferenceManagement.dynprop.2.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QMetaObject::invokeMethod(object, "createReference");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "ensureReference");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "removeReference");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "ensureDeletion");
        QCOMPARE(object->property("success").toBool(), true);
        delete object;
    }

    {
        // Item property reference to deleted item doesn't crash
        QQmlEngine hrmEngine;
        QQmlComponent component(&hrmEngine, testFileUrl("handleReferenceManagement.dynprop.3.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QMetaObject::invokeMethod(object, "createReference");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "ensureReference");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "manuallyDelete");
        gc(hrmEngine);
        QMetaObject::invokeMethod(object, "ensureDeleted");
        QCOMPARE(object->property("success").toBool(), true);
        delete object;
    }
}

void tst_qqmlecmascript::stringArg()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("stringArg.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "success");
    QVERIFY(object->property("returnValue").toBool());

    QString w1 = testFileUrl("stringArg.qml").toString() + QLatin1String(":45: Error: String.arg(): Invalid arguments");
    QTest::ignoreMessage(QtWarningMsg, w1.toLatin1().constData());
    QMetaObject::invokeMethod(object, "failure");
    QVERIFY(object->property("returnValue").toBool());

    delete object;
}

void tst_qqmlecmascript::readonlyDeclaration()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("readonlyDeclaration.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test").toBool(), true);

    delete object;
}

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<qreal>)
Q_DECLARE_METATYPE(QList<bool>)
Q_DECLARE_METATYPE(QList<QString>)
Q_DECLARE_METATYPE(QList<QUrl>)
void tst_qqmlecmascript::sequenceConversionRead()
{
    QQmlEngine engine;

    {
        QUrl qmlFile = testFileUrl("sequenceConversion.read.qml");
        QQmlComponent component(&engine, qmlFile);
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        MySequenceConversionObject *seq = object->findChild<MySequenceConversionObject*>("msco");
        QVERIFY(seq != nullptr);

        QMetaObject::invokeMethod(object, "readSequences");
        QList<int> intList; intList << 1 << 2 << 3 << 4;
        QCOMPARE(object->property("intListLength").toInt(), intList.length());
        QCOMPARE(object->property("intList").value<QList<int> >(), intList);
        QList<qreal> qrealList; qrealList << 1.1 << 2.2 << 3.3 << 4.4;
        QCOMPARE(object->property("qrealListLength").toInt(), qrealList.length());
        QCOMPARE(object->property("qrealList").value<QList<qreal> >(), qrealList);
        QList<bool> boolList; boolList << true << false << true << false;
        QCOMPARE(object->property("boolListLength").toInt(), boolList.length());
        QCOMPARE(object->property("boolList").value<QList<bool> >(), boolList);
        QList<QString> stringList; stringList << QLatin1String("first") << QLatin1String("second") << QLatin1String("third") << QLatin1String("fourth");
        QCOMPARE(object->property("stringListLength").toInt(), stringList.length());
        QCOMPARE(object->property("stringList").value<QList<QString> >(), stringList);
        QList<QUrl> urlList; urlList << QUrl("http://www.example1.com") << QUrl("http://www.example2.com") << QUrl("http://www.example3.com");
        QCOMPARE(object->property("urlListLength").toInt(), urlList.length());
        QCOMPARE(object->property("urlList").value<QList<QUrl> >(), urlList);
        QStringList qstringList; qstringList << QLatin1String("first") << QLatin1String("second") << QLatin1String("third") << QLatin1String("fourth");
        QCOMPARE(object->property("qstringListLength").toInt(), qstringList.length());
        QCOMPARE(object->property("qstringList").value<QStringList>(), qstringList);

        QMetaObject::invokeMethod(object, "readSequenceElements");
        QCOMPARE(object->property("intVal").toInt(), 2);
        QCOMPARE(object->property("qrealVal").toReal(), 2.2);
        QCOMPARE(object->property("boolVal").toBool(), false);
        QCOMPARE(object->property("stringVal").toString(), QString(QLatin1String("second")));
        QCOMPARE(object->property("urlVal").toUrl(), QUrl("http://www.example2.com"));
        QCOMPARE(object->property("qstringVal").toString(), QString(QLatin1String("second")));

        QMetaObject::invokeMethod(object, "enumerateSequenceElements");
        QCOMPARE(object->property("enumerationMatches").toBool(), true);

        intList.clear(); intList << 1 << 2 << 3 << 4 << 5; // set by the enumerateSequenceElements test.
        QQmlProperty seqProp(seq, "intListProperty");
        QCOMPARE(seqProp.read().value<QList<int> >(), intList);
        QQmlProperty seqProp2(seq, "intListProperty", &engine);
        QCOMPARE(seqProp2.read().value<QList<int> >(), intList);

        QMetaObject::invokeMethod(object, "testReferenceDeletion");
        QCOMPARE(object->property("referenceDeletion").toBool(), true);

        delete object;
    }

    {
        QUrl qmlFile = testFileUrl("sequenceConversion.read.error.qml");
        QQmlComponent component(&engine, qmlFile);
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        MySequenceConversionObject *seq = object->findChild<MySequenceConversionObject*>("msco");
        QVERIFY(seq != nullptr);

        // we haven't registered QList<NonRegisteredType> as a sequence type.
        QString warningOne = QLatin1String("QMetaProperty::read: Unable to handle unregistered datatype 'QList<NonRegisteredType>' for property 'MySequenceConversionObject::typeListProperty'");
        QString warningTwo = qmlFile.toString() + QLatin1String(":18: TypeError: Cannot read property 'length' of undefined");
        QTest::ignoreMessage(QtWarningMsg, warningOne.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warningTwo.toLatin1().constData());

        QMetaObject::invokeMethod(object, "performTest");

        // QList<NonRegisteredType> has not been registered as a sequence type.
        QCOMPARE(object->property("pointListLength").toInt(), 0);
        QVERIFY(!object->property("pointList").isValid());
        QTest::ignoreMessage(QtWarningMsg, "QMetaProperty::read: Unable to handle unregistered datatype 'QList<NonRegisteredType>' for property 'MySequenceConversionObject::typeListProperty'");
        QQmlProperty seqProp(seq, "typeListProperty", &engine);
        QVERIFY(!seqProp.read().isValid()); // not a valid/known sequence type

        delete object;
    }
}

void tst_qqmlecmascript::sequenceConversionWrite()
{
    QQmlEngine engine;
    {
        QUrl qmlFile = testFileUrl("sequenceConversion.write.qml");
        QQmlComponent component(&engine, qmlFile);
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        MySequenceConversionObject *seq = object->findChild<MySequenceConversionObject*>("msco");
        QVERIFY(seq != nullptr);

        QMetaObject::invokeMethod(object, "writeSequences");
        QCOMPARE(object->property("success").toBool(), true);

        QMetaObject::invokeMethod(object, "writeSequenceElements");
        QCOMPARE(object->property("success").toBool(), true);

        QMetaObject::invokeMethod(object, "writeOtherElements");
        QCOMPARE(object->property("success").toBool(), true);

        QMetaObject::invokeMethod(object, "testReferenceDeletion");
        QCOMPARE(object->property("referenceDeletion").toBool(), true);

        delete object;
    }

    {
        QUrl qmlFile = testFileUrl("sequenceConversion.write.error.qml");
        QQmlComponent component(&engine, qmlFile);
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        MySequenceConversionObject *seq = object->findChild<MySequenceConversionObject*>("msco");
        QVERIFY(seq != nullptr);

        // we haven't registered QList<QPoint> as a sequence type, so writing shouldn't work.
        QString warningOne = qmlFile.toString() + QLatin1String(":16: Error: Cannot assign QJSValue to QList<QPoint>");
        QTest::ignoreMessage(QtWarningMsg, warningOne.toLatin1().constData());

        QMetaObject::invokeMethod(object, "performTest");

        QList<QPoint> pointList; pointList << QPoint(1, 2) << QPoint(3, 4) << QPoint(5, 6); // original values, shouldn't have changed
        QCOMPARE(seq->pointListProperty(), pointList);

        delete object;
    }
}

void tst_qqmlecmascript::sequenceConversionArray()
{
    // ensure that in JS the returned sequences act just like normal JS Arrays.
    QUrl qmlFile = testFileUrl("sequenceConversion.array.qml");
    QQmlEngine engine;
    QQmlComponent component(&engine, qmlFile);
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object.data(), "indexedAccess");
    QVERIFY(object->property("success").toBool());
    QMetaObject::invokeMethod(object.data(), "arrayOperations");
    QVERIFY(object->property("success").toBool());
    QMetaObject::invokeMethod(object.data(), "testEqualitySemantics");
    QVERIFY(object->property("success").toBool());
    QMetaObject::invokeMethod(object.data(), "testReferenceDeletion");
    QCOMPARE(object->property("referenceDeletion").toBool(), true);
    QMetaObject::invokeMethod(object.data(), "jsonConversion");
    QVERIFY(object->property("success").toBool());
}


void tst_qqmlecmascript::sequenceConversionIndexes()
{
    // ensure that we gracefully fail if unsupported index values are specified.
    // Qt container classes only support non-negative, signed integer index values.
    QUrl qmlFile = testFileUrl("sequenceConversion.indexes.qml");
    QQmlEngine engine;
    QQmlComponent component(&engine, qmlFile);
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QString w1 = qmlFile.toString() + QLatin1String(":34: Index out of range during length set");
    QString w2 = qmlFile.toString() + QLatin1String(":41: Index out of range during indexed set");
    QString w3 = qmlFile.toString() + QLatin1String(":48: Index out of range during indexed get");
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w1));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w2));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(w3));
    QMetaObject::invokeMethod(object, "indexedAccess");
    QVERIFY(object->property("success").toBool());
    QMetaObject::invokeMethod(object, "indexOf");
    QVERIFY(object->property("success").toBool());
    delete object;
}

void tst_qqmlecmascript::sequenceConversionThreads()
{
    // ensure that sequence conversion operations work correctly in a worker thread
    // and that serialisation between the main and worker thread succeeds.
    QUrl qmlFile = testFileUrl("sequenceConversion.threads.qml");
    QQmlEngine engine;
    QQmlComponent component(&engine, qmlFile);
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QMetaObject::invokeMethod(object, "testIntSequence");
    QTRY_VERIFY(object->property("finished").toBool());
    QVERIFY(object->property("success").toBool());

    QMetaObject::invokeMethod(object, "testQrealSequence");
    QTRY_VERIFY(object->property("finished").toBool());
    QVERIFY(object->property("success").toBool());

    QMetaObject::invokeMethod(object, "testBoolSequence");
    QTRY_VERIFY(object->property("finished").toBool());
    QVERIFY(object->property("success").toBool());

    QMetaObject::invokeMethod(object, "testStringSequence");
    QTRY_VERIFY(object->property("finished").toBool());
    QVERIFY(object->property("success").toBool());

    QMetaObject::invokeMethod(object, "testQStringSequence");
    QTRY_VERIFY(object->property("finished").toBool());
    QVERIFY(object->property("success").toBool());

    QMetaObject::invokeMethod(object, "testUrlSequence");
    QTRY_VERIFY(object->property("finished").toBool());
    QVERIFY(object->property("success").toBool());

    QMetaObject::invokeMethod(object, "testVariantSequence");
    QTRY_VERIFY(object->property("finished").toBool());
    QVERIFY(object->property("success").toBool());

    delete object;
}

void tst_qqmlecmascript::sequenceConversionBindings()
{
    QQmlEngine engine;
    {
        QUrl qmlFile = testFileUrl("sequenceConversion.bindings.qml");
        QQmlComponent component(&engine, qmlFile);
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QList<int> intList; intList << 1 << 2 << 3 << 12 << 7;
        QCOMPARE(object->property("boundSequence").value<QList<int> >(), intList);
        QCOMPARE(object->property("boundElement").toInt(), intList.at(3));
        QList<int> intListTwo; intListTwo << 1 << 2 << 3 << 12 << 14;
        QCOMPARE(object->property("boundSequenceTwo").value<QList<int> >(), intListTwo);
        delete object;
    }

    {
        QUrl qmlFile = testFileUrl("sequenceConversion.bindings.error.qml");
        QString warning = QString(QLatin1String("%1:17:9: Unable to assign QList<int> to QList<bool>")).arg(qmlFile.toString());
        QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());
        QQmlComponent component(&engine, qmlFile);
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        delete object;
    }
}

void tst_qqmlecmascript::sequenceConversionCopy()
{
    QUrl qmlFile = testFileUrl("sequenceConversion.copy.qml");
    QQmlEngine engine;
    QQmlComponent component(&engine, qmlFile);
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QMetaObject::invokeMethod(object, "testCopySequences");
    QCOMPARE(object->property("success").toBool(), true);
    QMetaObject::invokeMethod(object, "readSequenceCopyElements");
    QCOMPARE(object->property("success").toBool(), true);
    QMetaObject::invokeMethod(object, "testEqualitySemantics");
    QCOMPARE(object->property("success").toBool(), true);
    QMetaObject::invokeMethod(object, "testCopyParameters");
    QCOMPARE(object->property("success").toBool(), true);
    QMetaObject::invokeMethod(object, "testReferenceParameters");
    QCOMPARE(object->property("success").toBool(), true);
    delete object;
}

void tst_qqmlecmascript::assignSequenceTypes()
{
    QQmlEngine engine;

    // test binding array to sequence type property
    {
    QQmlComponent component(&engine, testFileUrl("assignSequenceTypes.1.qml"));
    MySequenceConversionObject *object = qobject_cast<MySequenceConversionObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->intListProperty(), (QList<int>() << 1 << 2));
    QCOMPARE(object->qrealListProperty(), (QList<qreal>() << 1.1 << 2.2 << 3));
    QCOMPARE(object->boolListProperty(), (QList<bool>() << false << true));
    QCOMPARE(object->urlListProperty(), (QList<QUrl>() << QUrl("http://www.example1.com") << QUrl("http://www.example2.com")));
    QCOMPARE(object->stringListProperty(), (QList<QString>() << QLatin1String("one") << QLatin1String("two")));
    QCOMPARE(object->qstringListProperty(), (QStringList() << QLatin1String("one") << QLatin1String("two")));
    delete object;
    }

    // test binding literal to sequence type property
    {
    QQmlComponent component(&engine, testFileUrl("assignSequenceTypes.2.qml"));
    MySequenceConversionObject *object = qobject_cast<MySequenceConversionObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->intListProperty(), (QList<int>() << 1));
    QCOMPARE(object->qrealListProperty(), (QList<qreal>() << 1.1));
    QCOMPARE(object->boolListProperty(), (QList<bool>() << false));
    QCOMPARE(object->urlListProperty(), (QList<QUrl>() << QUrl("http://www.example1.com")));
    QCOMPARE(object->stringListProperty(), (QList<QString>() << QLatin1String("one")));
    QCOMPARE(object->qstringListProperty(), (QStringList() << QLatin1String("two")));
    delete object;
    }

    // test binding single value to sequence type property
    {
    QQmlComponent component(&engine, testFileUrl("assignSequenceTypes.3.qml"));
    MySequenceConversionObject *object = qobject_cast<MySequenceConversionObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->intListProperty(), (QList<int>() << 1));
    QCOMPARE(object->qrealListProperty(), (QList<qreal>() << 1));
    QCOMPARE(object->boolListProperty(), (QList<bool>() << false));
    QCOMPARE(object->urlListProperty(), (QList<QUrl>() << QUrl(testFileUrl("example.html"))));
    delete object;
    }

    // test assigning array to sequence type property in js function
    {
    QQmlComponent component(&engine, testFileUrl("assignSequenceTypes.4.qml"));
    MySequenceConversionObject *object = qobject_cast<MySequenceConversionObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->intListProperty(), (QList<int>() << 1 << 2));
    QCOMPARE(object->qrealListProperty(), (QList<qreal>() << 1.1 << 2.2 << 3));
    QCOMPARE(object->boolListProperty(), (QList<bool>() << false << true));
    QCOMPARE(object->urlListProperty(), (QList<QUrl>() << QUrl("http://www.example1.com") << QUrl("http://www.example2.com")));
    QCOMPARE(object->stringListProperty(), (QList<QString>() << QLatin1String("one") << QLatin1String("two")));
    QCOMPARE(object->qstringListProperty(), (QStringList() << QLatin1String("one") << QLatin1String("two")));
    delete object;
    }

    // test assigning literal to sequence type property in js function
    {
    QQmlComponent component(&engine, testFileUrl("assignSequenceTypes.5.qml"));
    MySequenceConversionObject *object = qobject_cast<MySequenceConversionObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->intListProperty(), (QList<int>() << 1));
    QCOMPARE(object->qrealListProperty(), (QList<qreal>() << 1.1));
    QCOMPARE(object->boolListProperty(), (QList<bool>() << false));
    QCOMPARE(object->urlListProperty(), (QList<QUrl>() << QUrl("http://www.example1.com")));
    QCOMPARE(object->stringListProperty(), (QList<QString>() << QLatin1String("one")));
    QCOMPARE(object->qstringListProperty(), (QStringList() << QLatin1String("two")));
    delete object;
    }

    // test assigning single value to sequence type property in js function
    {
    QQmlComponent component(&engine, testFileUrl("assignSequenceTypes.6.qml"));
    MySequenceConversionObject *object = qobject_cast<MySequenceConversionObject *>(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->intListProperty(), (QList<int>() << 1));
    QCOMPARE(object->qrealListProperty(), (QList<qreal>() << 1));
    QCOMPARE(object->boolListProperty(), (QList<bool>() << false));
    QCOMPARE(object->urlListProperty(), (QList<QUrl>() << QUrl(testFileUrl("example.html"))));
    delete object;
    }

    // test QList<QUrl> literal assignment and binding assignment causes url resolution when required
    {
    QQmlComponent component(&engine, testFileUrl("assignSequenceTypes.7.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    MySequenceConversionObject *msco1 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco1"));
    MySequenceConversionObject *msco2 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco2"));
    MySequenceConversionObject *msco3 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco3"));
    MySequenceConversionObject *msco4 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco4"));
    MySequenceConversionObject *msco5 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco5"));
    QVERIFY(msco1 != nullptr && msco2 != nullptr && msco3 != nullptr && msco4 != nullptr && msco5 != nullptr);
    QCOMPARE(msco1->urlListProperty(), (QList<QUrl>() << QUrl(testFileUrl("example.html"))));
    QCOMPARE(msco2->urlListProperty(), (QList<QUrl>() << QUrl(testFileUrl("example.html"))));
    QCOMPARE(msco3->urlListProperty(), (QList<QUrl>() << QUrl(testFileUrl("example.html")) << QUrl(testFileUrl("example2.html"))));
    QCOMPARE(msco4->urlListProperty(), (QList<QUrl>() << QUrl(testFileUrl("example.html")) << QUrl(testFileUrl("example2.html"))));
    QCOMPARE(msco5->urlListProperty(), (QList<QUrl>() << QUrl(testFileUrl("example.html")) << QUrl(testFileUrl("example2.html"))));
    delete object;
    }

    {
    QQmlComponent component(&engine, testFileUrl("assignSequenceTypes.8.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);
    QVariant result;
    QMetaObject::invokeMethod(object.data(), "tryWritingReadOnlySequence", Q_RETURN_ARG(QVariant, result));
    QVERIFY(result.type() == QVariant::Bool);
    QVERIFY(result.toBool());
    }
}

// Test that assigning a null object works
// Regressed with: df1788b4dbbb2826ae63f26bdf166342595343f4
void tst_qqmlecmascript::nullObjectBinding()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("nullObjectBinding.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QVERIFY(object->property("test") == QVariant::fromValue((QObject *)nullptr));

    delete object;
}

void tst_qqmlecmascript::nullObjectInitializer()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("nullObjectInitializer.qml"));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());

        QQmlData *ddata = QQmlData::get(obj.data(), /*create*/false);
        QVERIFY(ddata);

        {
            const int propertyIndex = obj->metaObject()->indexOfProperty("testProperty");
            QVERIFY(propertyIndex > 0);
            QVERIFY(!ddata->hasBindingBit(propertyIndex));
        }

        QVariant value = obj->property("testProperty");
        QVERIFY(value.userType() == qMetaTypeId<QObject*>());
        QVERIFY(!value.value<QObject*>());
    }

    {
        QQmlComponent component(&engine, testFileUrl("nullObjectInitializer.2.qml"));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());

        QQmlData *ddata = QQmlData::get(obj.data(), /*create*/false);
        QVERIFY(ddata);

        {
            const int propertyIndex = obj->metaObject()->indexOfProperty("testProperty");
            QVERIFY(propertyIndex > 0);
            QVERIFY(!ddata->hasBindingBit(propertyIndex));
        }

        QVERIFY(obj->property("success").toBool());

        QVariant value = obj->property("testProperty");
        QVERIFY(value.userType() == qMetaTypeId<QObject*>());
        QVERIFY(!value.value<QObject*>());
    }
}

// Test that bindings don't evaluate once the engine has been destroyed
void tst_qqmlecmascript::deletedEngine()
{
    QQmlEngine *engine = new QQmlEngine;
    QQmlComponent component(engine, testFileUrl("deletedEngine.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("a").toInt(), 39);
    object->setProperty("b", QVariant(9));
    QCOMPARE(object->property("a").toInt(), 117);

    delete engine;

    QCOMPARE(object->property("a").toInt(), 0);
    object->setProperty("b", QVariant(10));
    object->setProperty("b", QVariant());
    QCOMPARE(object->property("a").toInt(), 0);

    delete object;
}

// Test the crashing part of QTBUG-9705
void tst_qqmlecmascript::libraryScriptAssert()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("libraryScriptAssert.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    delete object;
}

void tst_qqmlecmascript::variantsAssignedUndefined()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("variantsAssignedUndefined.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("test1").toInt(), 10);
    QCOMPARE(object->property("test2").toInt(), 11);

    object->setProperty("runTest", true);

    QCOMPARE(object->property("test1"), QVariant());
    QCOMPARE(object->property("test2"), QVariant());


    delete object;
}

void tst_qqmlecmascript::variants()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("variants.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("undefinedVariant").type(), QVariant::Invalid);
    QCOMPARE(int(object->property("nullVariant").type()), int(QMetaType::Nullptr));
    QCOMPARE(object->property("intVariant").type(), QVariant::Int);
    QCOMPARE(object->property("doubleVariant").type(), QVariant::Double);

    QVariant result;
    QMetaObject::invokeMethod(object, "checkNull", Q_RETURN_ARG(QVariant, result));
    QCOMPARE(result.toBool(), true);

    QMetaObject::invokeMethod(object, "checkUndefined", Q_RETURN_ARG(QVariant, result));
    QCOMPARE(result.toBool(), true);

    QMetaObject::invokeMethod(object, "checkNumber", Q_RETURN_ARG(QVariant, result));
    QCOMPARE(result.toBool(), true);
}

void tst_qqmlecmascript::qtbug_9792()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_9792.qml"));

    QQmlContext *context = new QQmlContext(engine.rootContext());

    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create(context));
    QVERIFY(object != nullptr);

    QTest::ignoreMessage(QtDebugMsg, "Hello world!");
    object->basicSignal();

    delete context;

    QQmlTestMessageHandler messageHandler;

    object->basicSignal();

    QVERIFY2(messageHandler.messages().isEmpty(), qPrintable(messageHandler.messageString()));

    delete object;
}

// Verifies that QPointer<>s used in the vmemetaobject are cleaned correctly
void tst_qqmlecmascript::qtcreatorbug_1289()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtcreatorbug_1289.qml"));

    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QObject *nested = qvariant_cast<QObject *>(o->property("object"));
    QVERIFY(nested != nullptr);

    QVERIFY(qvariant_cast<QObject *>(nested->property("nestedObject")) == o);

    delete nested;
    nested = qvariant_cast<QObject *>(o->property("object"));
    QVERIFY(!nested);

    // If the bug is present, the next line will crash
    delete o;
}

// Test that we shut down without stupid warnings
void tst_qqmlecmascript::noSpuriousWarningsAtShutdown()
{
    QQmlEngine engine;
    {
    QQmlComponent component(&engine, testFileUrl("noSpuriousWarningsAtShutdown.qml"));

    QObject *o = component.create();

    QQmlTestMessageHandler messageHandler;

    delete o;

    QVERIFY2(messageHandler.messages().isEmpty(), qPrintable(messageHandler.messageString()));
    }


    {
    QQmlComponent component(&engine, testFileUrl("noSpuriousWarningsAtShutdown.2.qml"));

    QObject *o = component.create();

    QQmlTestMessageHandler messageHandler;

    delete o;

    QVERIFY2(messageHandler.messages().isEmpty(), qPrintable(messageHandler.messageString()));
    }
}

void tst_qqmlecmascript::canAssignNullToQObject()
{
    QQmlEngine engine;
    {
    QQmlComponent component(&engine, testFileUrl("canAssignNullToQObject.1.qml"));

    MyQmlObject *o = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(o != nullptr);

    QVERIFY(o->objectProperty() != nullptr);

    o->setProperty("runTest", true);

    QVERIFY(!o->objectProperty());

    delete o;
    }

    {
    QQmlComponent component(&engine, testFileUrl("canAssignNullToQObject.2.qml"));

    MyQmlObject *o = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(o != nullptr);

    QVERIFY(!o->objectProperty());

    delete o;
    }
}

void tst_qqmlecmascript::functionAssignment_fromBinding()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("functionAssignment.1.qml"));

    QString url = component.url().toString();
    QString w1 = url + ":4:5: Unable to assign a function to a property of any type other than var.";
    QString w2 = url + ":5:5: Invalid use of Qt.binding() in a binding declaration.";
    QString w3 = url + ":6:5: Invalid use of Qt.binding() in a binding declaration.";
    QString w4 = url + ":7:5: Invalid use of Qt.binding() in a binding declaration.";
    QTest::ignoreMessage(QtWarningMsg, w1.toLatin1().constData());
    QTest::ignoreMessage(QtWarningMsg, w2.toLatin1().constData());
    QTest::ignoreMessage(QtWarningMsg, w3.toLatin1().constData());
    QTest::ignoreMessage(QtWarningMsg, w4.toLatin1().constData());

    MyQmlObject *o = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(o != nullptr);

    QVERIFY(!o->property("a").isValid());

    delete o;
}

void tst_qqmlecmascript::functionAssignment_fromJS()
{
    QFETCH(QString, triggerProperty);

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("functionAssignment.2.qml"));
    QVERIFY2(component.errorString().isEmpty(), qPrintable(component.errorString()));

    MyQmlObject *o = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(o != nullptr);
    QVERIFY(!o->property("a").isValid());

    o->setProperty("aNumber", QVariant(5));
    o->setProperty(triggerProperty.toUtf8().constData(), true);
    QCOMPARE(o->property("a"), QVariant(50));

    o->setProperty("aNumber", QVariant(10));
    QCOMPARE(o->property("a"), QVariant(100));

    delete o;
}

void tst_qqmlecmascript::functionAssignment_fromJS_data()
{
    QTest::addColumn<QString>("triggerProperty");

    QTest::newRow("assign to property") << "assignToProperty";
    QTest::newRow("assign to property, from JS file") << "assignToPropertyFromJsFile";

    QTest::newRow("assign to value type") << "assignToValueType";

    QTest::newRow("use 'this'") << "assignWithThis";
    QTest::newRow("use 'this' from JS file") << "assignWithThisFromJsFile";
}

void tst_qqmlecmascript::functionAssignmentfromJS_invalid()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("functionAssignment.2.qml"));
    QVERIFY2(component.errorString().isEmpty(), qPrintable(component.errorString()));

    MyQmlObject *o = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(o != nullptr);
    QVERIFY(!o->property("a").isValid());

    o->setProperty("assignFuncWithoutReturn", true);
    QVERIFY(!o->property("a").isValid());

    QString url = component.url().toString();
    QString warning = url + ":67: Unable to assign QString to int";
    QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());
    o->setProperty("assignWrongType", true);

    warning = url + ":71: Unable to assign QString to int";
    QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());
    o->setProperty("assignWrongTypeToValueType", true);

    delete o;
}

void tst_qqmlecmascript::functionAssignment_afterBinding()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("functionAssignment.3.qml"));

    QString url = component.url().toString();
    QString w1 = url + ":16: Error: Cannot assign JavaScript function to int";
    QTest::ignoreMessage(QtWarningMsg, w1.toLatin1().constData());

    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("t1"), QVariant::fromValue<int>(4)); // should have bound
    QCOMPARE(o->property("t2"), QVariant::fromValue<int>(2)); // should not have changed

    delete o;
}

void tst_qqmlecmascript::eval()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("eval.qml"));

    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    QCOMPARE(o->property("test3").toBool(), true);
    QCOMPARE(o->property("test4").toBool(), true);
    QCOMPARE(o->property("test5").toBool(), true);

    delete o;
}

void tst_qqmlecmascript::function()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("function.qml"));

    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    QCOMPARE(o->property("test3").toBool(), true);

    delete o;
}

// Test the "Qt.include" method
void tst_qqmlecmascript::include()
{
    QQmlEngine engine;
    // Non-library relative include
    {
    QQmlComponent component(&engine, testFileUrl("include.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test0").toInt(), 99);
    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    QCOMPARE(o->property("test2_1").toBool(), true);
    QCOMPARE(o->property("test3").toBool(), true);
    QCOMPARE(o->property("test3_1").toBool(), true);

    delete o;
    }

    // Library relative include
    {
    QQmlComponent component(&engine, testFileUrl("include_shared.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test0").toInt(), 99);
    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    QCOMPARE(o->property("test2_1").toBool(), true);
    QCOMPARE(o->property("test3").toBool(), true);
    QCOMPARE(o->property("test3_1").toBool(), true);

    delete o;
    }

    // Callback
    {
    QQmlComponent component(&engine, testFileUrl("include_callback.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    QCOMPARE(o->property("test3").toBool(), true);
    QCOMPARE(o->property("test4").toBool(), true);
    QCOMPARE(o->property("test5").toBool(), true);
    QCOMPARE(o->property("test6").toBool(), true);

    delete o;
    }

    // Including file with ".pragma library"
    {
    QQmlComponent component(&engine, testFileUrl("include_pragma.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test1").toInt(), 100);

    delete o;
    }

    // Including file with ".pragma library", shadowing a global var
    {
    QQmlComponent component(&engine, testFileUrl("include_pragma_shadow.qml"));
    QScopedPointer<QObject> o(component.create());
    QVERIFY(!o.isNull());
    QCOMPARE(o->property("result").toBool(), true);
    }

    // Remote - error
    {
    TestHTTPServer server;
    QVERIFY2(server.listen(), qPrintable(server.errorString()));
    server.serveDirectory(dataDirectory());

    QQmlComponent component(&engine, testFileUrl("include_remote_missing.qml"));
    QObject *o = component.beginCreate(engine.rootContext());
    QVERIFY(o != nullptr);
    o->setProperty("serverBaseUrl", server.baseUrl().toString());
    component.completeCreate();

    QTRY_VERIFY(o->property("done").toBool());

    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    QCOMPARE(o->property("test3").toBool(), true);

    delete o;
    }

    // include from resources
    {
    QQmlComponent component(&engine, QUrl("qrc:///data/include.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test0").toInt(), 99);
    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    QCOMPARE(o->property("test2_1").toBool(), true);
    QCOMPARE(o->property("test3").toBool(), true);
    QCOMPARE(o->property("test3_1").toBool(), true);

    delete o;
    }

}

void tst_qqmlecmascript::includeRemoteSuccess()
{
    // Remote - success
    TestHTTPServer server;
    QVERIFY2(server.listen(), qPrintable(server.errorString()));
    server.serveDirectory(dataDirectory());

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("include_remote.qml"));
    QObject *o = component.beginCreate(engine.rootContext());
    QVERIFY(o != nullptr);
    o->setProperty("serverBaseUrl", server.baseUrl().toString());
    component.completeCreate();

    QTRY_VERIFY(o->property("done").toBool());
    QTRY_VERIFY(o->property("done2").toBool());

    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    QCOMPARE(o->property("test3").toBool(), true);
    QCOMPARE(o->property("test4").toBool(), true);
    QCOMPARE(o->property("test5").toBool(), true);

    QCOMPARE(o->property("test6").toBool(), true);
    QCOMPARE(o->property("test7").toBool(), true);
    QCOMPARE(o->property("test8").toBool(), true);
    QCOMPARE(o->property("test9").toBool(), true);
    QCOMPARE(o->property("test10").toBool(), true);

    delete o;
}

void tst_qqmlecmascript::signalHandlers()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("signalHandlers.qml"));
    QScopedPointer<QObject> o(component.create());
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("count").toInt(), 0);
    QMetaObject::invokeMethod(o.data(), "testSignalCall");
    QCOMPARE(o->property("count").toInt(), 1);

    QMetaObject::invokeMethod(o.data(), "testSignalHandlerCall");
    QCOMPARE(o->property("count").toInt(), 1);
    QString scopeObjectAsString = o->property("scopeObjectAsString").toString();
    QCOMPARE(o->property("errorString").toString(), QString("TypeError: Property 'onTestSignal' of object %1 is not a function").arg(scopeObjectAsString));

    QCOMPARE(o->property("funcCount").toInt(), 0);
    QMetaObject::invokeMethod(o.data(), "testSignalConnection");
    QCOMPARE(o->property("funcCount").toInt(), 1);

    QMetaObject::invokeMethod(o.data(), "testSignalHandlerConnection");
    QCOMPARE(o->property("funcCount").toInt(), 2);

    QMetaObject::invokeMethod(o.data(), "testSignalDefined");
    QCOMPARE(o->property("definedResult").toBool(), true);

    QMetaObject::invokeMethod(o.data(), "testSignalHandlerDefined");
    QCOMPARE(o->property("definedHandlerResult").toBool(), true);

    QVariant result;
    QMetaObject::invokeMethod(o.data(), "testConnectionOnAlias", Q_RETURN_ARG(QVariant, result));
    QCOMPARE(result.toBool(), true);

    QMetaObject::invokeMethod(o.data(), "testAliasSignalHandler", Q_RETURN_ARG(QVariant, result));
    QCOMPARE(result.toBool(), true);

    QMetaObject::invokeMethod(o.data(), "testSignalWithClosureArgument", Q_RETURN_ARG(QVariant, result));
    QCOMPARE(result.toBool(), true);
    QMetaObject::invokeMethod(o.data(), "testThisInSignalHandler", Q_RETURN_ARG(QVariant, result));
    QCOMPARE(result.toBool(), true);
}

void tst_qqmlecmascript::qtbug_37351()
{
    MyTypeObject signalEmitter;
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import Qt.test 1.0; import QtQml 2.0;\nQtObject {\n"
            "    Component.onCompleted: {\n"
            "        testObject.action.connect(function() { print('dont crash'); });"
            "    }\n"
            "}", QUrl());

        engine.rootContext()->setContextProperty("testObject", &signalEmitter);

        QScopedPointer<QObject> object(component.create());
        QVERIFY(!object.isNull());
    }
    signalEmitter.doAction();
    // Don't crash
}

void tst_qqmlecmascript::qtbug_10696()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_10696.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    delete o;
}

void tst_qqmlecmascript::qtbug_11606()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_11606.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

void tst_qqmlecmascript::qtbug_11600()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_11600.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

void tst_qqmlecmascript::qtbug_21864()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_21864.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

void tst_qqmlecmascript::rewriteMultiLineStrings()
{
    QQmlEngine engine;
    {
        // QTBUG-23387
        QQmlComponent component(&engine, testFileUrl("rewriteMultiLineStrings.qml"));
        QObject *o = component.create();
        QVERIFY(o != nullptr);
        QTRY_COMPARE(o->property("test").toBool(), true);
        delete o;
    }

    {
        QQmlComponent component(&engine, testFileUrl("rewriteMultiLineStrings_crlf.1.qml"));
        QObject *o = component.create();
        QVERIFY(o != nullptr);
        delete o;
    }
}

void tst_qqmlecmascript::qobjectConnectionListExceptionHandling()
{
    // QTBUG-23375
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qobjectConnectionListExceptionHandling.qml"));
    QString warning = component.url().toString() + QLatin1String(":13: TypeError: Cannot read property 'undefined' of undefined");
    QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

// Reading and writing non-scriptable properties should fail
void tst_qqmlecmascript::nonscriptable()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("nonscriptable.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("readOk").toBool(), true);
    QCOMPARE(o->property("writeOk").toBool(), true);
    delete o;
}

// deleteLater() should not be callable from QML
void tst_qqmlecmascript::deleteLater()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deleteLater.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

// objectNameChanged() should be usable from QML
void tst_qqmlecmascript::objectNameChangedSignal()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("objectNameChangedSignal.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test").toBool(), false);
    o->setObjectName("obj");
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

// destroyed() should not be usable from QML
void tst_qqmlecmascript::destroyedSignal()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("destroyedSignal.qml"));
    QVERIFY(component.isError());

    QString expectedErrorString = component.url().toString() + QLatin1String(":5:5: Cannot assign to non-existent property \"onDestroyed\"");
    QCOMPARE(component.errors().at(0).toString(), expectedErrorString);
}

void tst_qqmlecmascript::in()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("in.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    delete o;
}

void tst_qqmlecmascript::typeOf()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("typeOf.qml"));

    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test1").toString(), QLatin1String("undefined"));
    QCOMPARE(o->property("test2").toString(), QLatin1String("object"));
    QCOMPARE(o->property("test3").toString(), QLatin1String("number"));
    QCOMPARE(o->property("test4").toString(), QLatin1String("string"));
    QCOMPARE(o->property("test5").toString(), QLatin1String("function"));
    QCOMPARE(o->property("test6").toString(), QLatin1String("object"));
    QCOMPARE(o->property("test7").toString(), QLatin1String("undefined"));
    QCOMPARE(o->property("test8").toString(), QLatin1String("boolean"));
    QCOMPARE(o->property("test9").toString(), QLatin1String("object"));

    delete o;
}

void tst_qqmlecmascript::qtbug_24448()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_24448.qml"));
    QScopedPointer<QObject> o(component.create());
    QVERIFY(o != nullptr);
    QVERIFY(o->property("test").toBool());
}

void tst_qqmlecmascript::sharedAttachedObject()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("sharedAttachedObject.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(o->property("test1").toBool(), true);
    QCOMPARE(o->property("test2").toBool(), true);
    delete o;
}

// QTBUG-13999
void tst_qqmlecmascript::objectName()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("objectName.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test1").toString(), QString("hello"));
    QCOMPARE(o->property("test2").toString(), QString("ell"));

    o->setObjectName("world");

    QCOMPARE(o->property("test1").toString(), QString("world"));
    QCOMPARE(o->property("test2").toString(), QString("orl"));

    delete o;
}

void tst_qqmlecmascript::writeRemovesBinding()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("writeRemovesBinding.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);

    delete o;
}

// Test bindings assigned to alias properties actually assign to the alias' target
void tst_qqmlecmascript::aliasBindingsAssignCorrectly()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("aliasBindingsAssignCorrectly.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);

    delete o;
}

// Test bindings assigned to alias properties override a binding on the target (QTBUG-13719)
void tst_qqmlecmascript::aliasBindingsOverrideTarget()
{
    QQmlEngine engine;
    {
    QQmlComponent component(&engine, testFileUrl("aliasBindingsOverrideTarget.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);

    delete o;
    }

    {
    QQmlComponent component(&engine, testFileUrl("aliasBindingsOverrideTarget.2.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);

    delete o;
    }

    {
    QQmlComponent component(&engine, testFileUrl("aliasBindingsOverrideTarget.3.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);

    delete o;
    }
}

// Test that writes to alias properties override bindings on the alias target (QTBUG-13719)
void tst_qqmlecmascript::aliasWritesOverrideBindings()
{
    QQmlEngine engine;
    {
    QQmlComponent component(&engine, testFileUrl("aliasWritesOverrideBindings.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);

    delete o;
    }

    {
    QQmlComponent component(&engine, testFileUrl("aliasWritesOverrideBindings.2.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);

    delete o;
    }

    {
    QQmlComponent component(&engine, testFileUrl("aliasWritesOverrideBindings.3.qml"));
    QObject *o = component.create();
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);

    delete o;
    }
}

// Allow an alais to a composite element
// QTBUG-20200
void tst_qqmlecmascript::aliasToCompositeElement()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("aliasToCompositeElement.qml"));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    delete object;
}

void tst_qqmlecmascript::qtbug_20344()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_20344.qml"));

    QString warning = component.url().toString() + ":5: Error: Exception thrown from within QObject slot";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));

    QObject *object = component.create();
    QVERIFY(object != nullptr);

    delete object;
}

void tst_qqmlecmascript::revisionErrors()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("metaobjectRevisionErrors.qml"));
        QString url = component.url().toString();

        QString warning1 = url + ":8: ReferenceError: prop2 is not defined";
        QString warning2 = url + ":11: ReferenceError: prop2 is not defined";
        QString warning3 = url + ":13: ReferenceError: method2 is not defined";

        QTest::ignoreMessage(QtWarningMsg, warning1.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warning2.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warning3.toLatin1().constData());
        MyRevisionedClass *object = qobject_cast<MyRevisionedClass *>(component.create());
        QVERIFY(object != nullptr);
        delete object;
    }
    {
        QQmlComponent component(&engine, testFileUrl("metaobjectRevisionErrors2.qml"));
        QString url = component.url().toString();

        // MyRevisionedSubclass 1.0 uses MyRevisionedClass revision 0
        // method2, prop2 from MyRevisionedClass not available
        // method4, prop4 from MyRevisionedSubclass not available
        QString warning1 = url + ":8: ReferenceError: prop2 is not defined";
        QString warning2 = url + ":14: ReferenceError: prop2 is not defined";
        QString warning3 = url + ":10: ReferenceError: prop4 is not defined";
        QString warning4 = url + ":16: ReferenceError: prop4 is not defined";
        QString warning5 = url + ":20: ReferenceError: method2 is not defined";

        QTest::ignoreMessage(QtWarningMsg, warning1.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warning2.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warning3.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warning4.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warning5.toLatin1().constData());
        MyRevisionedClass *object = qobject_cast<MyRevisionedClass *>(component.create());
        QVERIFY(object != nullptr);
        delete object;
    }
    {
        QQmlComponent component(&engine, testFileUrl("metaobjectRevisionErrors3.qml"));
        QString url = component.url().toString();

        // MyRevisionedSubclass 1.1 uses MyRevisionedClass revision 1
        // All properties/methods available, except MyRevisionedBaseClassUnregistered rev 1
        QString warning1 = url + ":30: ReferenceError: methodD is not defined";
        QString warning2 = url + ":10: ReferenceError: propD is not defined";
        QString warning3 = url + ":20: ReferenceError: propD is not defined";
        QTest::ignoreMessage(QtWarningMsg, warning1.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warning2.toLatin1().constData());
        QTest::ignoreMessage(QtWarningMsg, warning3.toLatin1().constData());
        MyRevisionedClass *object = qobject_cast<MyRevisionedClass *>(component.create());
        QVERIFY(object != nullptr);
        delete object;
    }
}

void tst_qqmlecmascript::revision()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("metaobjectRevision.qml"));
        QString url = component.url().toString();

        MyRevisionedClass *object = qobject_cast<MyRevisionedClass *>(component.create());
        QVERIFY(object != nullptr);
        delete object;
    }
    {
        QQmlComponent component(&engine, testFileUrl("metaobjectRevision2.qml"));
        QString url = component.url().toString();

        MyRevisionedClass *object = qobject_cast<MyRevisionedClass *>(component.create());
        QVERIFY(object != nullptr);
        delete object;
    }
    {
        QQmlComponent component(&engine, testFileUrl("metaobjectRevision3.qml"));
        QString url = component.url().toString();

        MyRevisionedClass *object = qobject_cast<MyRevisionedClass *>(component.create());
        QVERIFY(object != nullptr);
        delete object;
    }
    // Test that non-root classes can resolve revisioned methods
    {
        QQmlComponent component(&engine, testFileUrl("metaobjectRevision4.qml"));

        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("test").toReal(), 11.);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("metaobjectRevision5.qml"));

        QObject *object = component.create();
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("test").toReal(), 11.);
        delete object;
    }
}

void tst_qqmlecmascript::realToInt()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("realToInt.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != nullptr);

    QMetaObject::invokeMethod(object, "test1");
    QCOMPARE(object->value(), int(4));
    QMetaObject::invokeMethod(object, "test2");
    QCOMPARE(object->value(), int(7));
}

void tst_qqmlecmascript::urlProperty()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("urlProperty.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
        QVERIFY(object != nullptr);
        object->setStringProperty("http://qt-project.org");
        QCOMPARE(object->urlProperty(), QUrl("http://qt-project.org/index.html"));
        QCOMPARE(object->intProperty(), 123);
        QCOMPARE(object->value(), 1);
        QCOMPARE(object->property("result").toBool(), true);
    }
}

void tst_qqmlecmascript::urlPropertyWithEncoding()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("urlProperty.2.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
        QVERIFY(object != nullptr);
        object->setStringProperty("http://qt-project.org");
        const QUrl encoded = QUrl::fromEncoded("http://qt-project.org/?get%3cDATA%3e", QUrl::TolerantMode);
        QCOMPARE(object->urlProperty(), encoded);
        QCOMPARE(object->value(), 0);   // Interpreting URL as string yields canonicalised version
        QCOMPARE(object->property("result").toBool(), true);
    }
}

void tst_qqmlecmascript::urlListPropertyWithEncoding()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("urlListProperty.qml"));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
        MySequenceConversionObject *msco1 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco1"));
        MySequenceConversionObject *msco2 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco2"));
        MySequenceConversionObject *msco3 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco3"));
        MySequenceConversionObject *msco4 = object->findChild<MySequenceConversionObject *>(QLatin1String("msco4"));
        QVERIFY(msco1 != nullptr && msco2 != nullptr && msco3 != nullptr && msco4 != nullptr);
        const QUrl encoded = QUrl::fromEncoded("http://qt-project.org/?get%3cDATA%3e", QUrl::TolerantMode);
        QCOMPARE(msco1->urlListProperty(), (QList<QUrl>() << encoded));
        QCOMPARE(msco2->urlListProperty(), (QList<QUrl>() << encoded));
        QCOMPARE(msco3->urlListProperty(), (QList<QUrl>() << encoded << encoded));
        QCOMPARE(msco4->urlListProperty(), (QList<QUrl>() << encoded << encoded));
        delete object;
    }
}

void tst_qqmlecmascript::dynamicString()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("dynamicString.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("stringProperty").toString(),
             QString::fromLatin1("string:Hello World false:0 true:1 uint32:100 int32:-100 double:3.14159 date:2011-02-11 05::30:50!"));
}

void tst_qqmlecmascript::deleteLaterObjectMethodCall()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deleteLaterObjectMethodCall.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
}

void tst_qqmlecmascript::automaticSemicolon()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("automaticSemicolon.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
}

void tst_qqmlecmascript::compatibilitySemicolon()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("compatibilitySemicolon.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
}

void tst_qqmlecmascript::incrDecrSemicolon1()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("incrDecrSemicolon1.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
}

void tst_qqmlecmascript::incrDecrSemicolon2()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("incrDecrSemicolon2.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
}

void tst_qqmlecmascript::incrDecrSemicolon_error1()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("incrDecrSemicolon_error1.qml"));
    QObject *object = component.create();
    QVERIFY(!object);
}

void tst_qqmlecmascript::unaryExpression()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("unaryExpression.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
}

// Makes sure that a binding isn't double re-evaluated when it depends on the same variable twice
void tst_qqmlecmascript::doubleEvaluate()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("doubleEvaluate.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    WriteCounter *wc = qobject_cast<WriteCounter *>(object);
    QVERIFY(wc != nullptr);
    QCOMPARE(wc->count(), 1);

    wc->setProperty("x", 9);

    QCOMPARE(wc->count(), 2);

    delete object;
}

void tst_qqmlecmascript::nonNotifyable()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("nonNotifyable.qml"));

    QQmlTestMessageHandler messageHandler;

    QObject *object = component.create();

    QVERIFY(object != nullptr);

    QString expected1 = QLatin1String("QQmlExpression: Expression ") +
                        component.url().toString() +
                        QLatin1String(":5:5 depends on non-NOTIFYable properties:");
    QString expected2 = QLatin1String("    ") +
                        QLatin1String(object->metaObject()->className()) +
                        QLatin1String("::value");

    QCOMPARE(messageHandler.messages().length(), 2);
    QCOMPARE(messageHandler.messages().at(0), expected1);
    QCOMPARE(messageHandler.messages().at(1), expected2);

    delete object;
}

void tst_qqmlecmascript::nonNotifyableConstant()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("nonNotifyableConstant.qml"));
    QQmlTestMessageHandler messageHandler;

    // Shouldn't produce an error message about non-NOTIFYable properties,
    // as the property has the CONSTANT attribute.
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object);
    QCOMPARE(messageHandler.messages().size(), 0);
}

void tst_qqmlecmascript::forInLoop()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("forInLoop.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);

    QMetaObject::invokeMethod(object, "listProperty");

    QStringList r = object->property("listResult").toString().split("|", QString::SkipEmptyParts);
    QCOMPARE(r.size(), 3);
    QCOMPARE(r[0],QLatin1String("0=obj1"));
    QCOMPARE(r[1],QLatin1String("1=obj2"));
    QCOMPARE(r[2],QLatin1String("2=obj3"));

    //TODO: should test for in loop for other objects (such as QObjects) as well.

    delete object;
}

// An object the binding depends on is deleted while the binding is still running
void tst_qqmlecmascript::deleteWhileBindingRunning()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deleteWhileBindingRunning.qml"));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
    delete object;
}

void tst_qqmlecmascript::qtbug_22679()
{
    MyQmlObject object;
    object.setStringProperty(QLatin1String("Please work correctly"));
    QQmlEngine engine;
    engine.rootContext()->setContextProperty("contextProp", &object);

    QQmlComponent component(&engine, testFileUrl("qtbug_22679.qml"));
    qRegisterMetaType<QList<QQmlError> >("QList<QQmlError>");
    QSignalSpy warningsSpy(&engine, SIGNAL(warnings(QList<QQmlError>)));

    QObject *o = component.create();
    QVERIFY(o != nullptr);
    QCOMPARE(warningsSpy.count(), 0);
    delete o;
}

void tst_qqmlecmascript::qtbug_22843_data()
{
    QTest::addColumn<bool>("library");

    QTest::newRow("without .pragma library") << false;
    QTest::newRow("with .pragma library") << true;
}

void tst_qqmlecmascript::qtbug_22843()
{
    QFETCH(bool, library);

    QString fileName("qtbug_22843");
    if (library)
        fileName += QLatin1String(".library");
    fileName += QLatin1String(".qml");

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl(fileName));

    QString url = component.url().toString();
    QString expectedError = url.left(url.length()-3) + QLatin1String("js:4:16: Expected token `;'");

    QVERIFY(component.isError());
    QCOMPARE(component.errors().value(1).toString(), expectedError);
}


void tst_qqmlecmascript::switchStatement()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("switchStatement.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        // `object->value()' is the number of executed statements

        object->setStringProperty("A");
        QCOMPARE(object->value(), 5);

        object->setStringProperty("S");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("D");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("F");
        QCOMPARE(object->value(), 4);

        object->setStringProperty("something else");
        QCOMPARE(object->value(), 1);
    }

    {
        QQmlComponent component(&engine, testFileUrl("switchStatement.2.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        // `object->value()' is the number of executed statements

        object->setStringProperty("A");
        QCOMPARE(object->value(), 5);

        object->setStringProperty("S");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("D");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("F");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("something else");
        QCOMPARE(object->value(), 4);
    }

    {
        QQmlComponent component(&engine, testFileUrl("switchStatement.3.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        // `object->value()' is the number of executed statements

        object->setStringProperty("A");
        QCOMPARE(object->value(), 5);

        object->setStringProperty("S");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("D");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("F");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("something else");
        QCOMPARE(object->value(), 6);
    }

    {
        QQmlComponent component(&engine, testFileUrl("switchStatement.4.qml"));

        QString warning = component.url().toString() + ":4:5: Unable to assign [undefined] to int";
        QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));

        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        // `object->value()' is the number of executed statements

        object->setStringProperty("A");
        QCOMPARE(object->value(), 5);

        object->setStringProperty("S");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("D");
        QCOMPARE(object->value(), 3);

        object->setStringProperty("F");
        QCOMPARE(object->value(), 3);

        QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));

        object->setStringProperty("something else");
    }

    {
        QQmlComponent component(&engine, testFileUrl("switchStatement.5.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        // `object->value()' is the number of executed statements

        object->setStringProperty("A");
        QCOMPARE(object->value(), 1);

        object->setStringProperty("S");
        QCOMPARE(object->value(), 1);

        object->setStringProperty("D");
        QCOMPARE(object->value(), 1);

        object->setStringProperty("F");
        QCOMPARE(object->value(), 1);

        object->setStringProperty("something else");
        QCOMPARE(object->value(), 1);
    }

    {
        QQmlComponent component(&engine, testFileUrl("switchStatement.6.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        // `object->value()' is the number of executed statements

        object->setStringProperty("A");
        QCOMPARE(object->value(), 123);

        object->setStringProperty("S");
        QCOMPARE(object->value(), 123);

        object->setStringProperty("D");
        QCOMPARE(object->value(), 321);

        object->setStringProperty("F");
        QCOMPARE(object->value(), 321);

        object->setStringProperty("something else");
        QCOMPARE(object->value(), 0);
    }
}

void tst_qqmlecmascript::withStatement()
{
    QQmlEngine engine;
    {
        QUrl url = testFileUrl("withStatement.1.qml");
        QQmlComponent component(&engine, url);
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->value(), 123);
    }
}

void tst_qqmlecmascript::tryStatement()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("tryStatement.1.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->value(), 123);
    }

    {
        QQmlComponent component(&engine, testFileUrl("tryStatement.2.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QCOMPARE(object->value(), 321);
    }

    {
        QQmlComponent component(&engine, testFileUrl("tryStatement.3.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QVERIFY(object->qjsvalue().isUndefined());
    }

    {
        QQmlComponent component(&engine, testFileUrl("tryStatement.4.qml"));
        MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
        QVERIFY(object != nullptr);

        QVERIFY(object->qjsvalue().isUndefined());
    }
}

class CppInvokableWithQObjectDerived : public QObject
{
    Q_OBJECT
public:
    CppInvokableWithQObjectDerived() {}
    ~CppInvokableWithQObjectDerived() {}

    Q_INVOKABLE MyQmlObject *createMyQmlObject(QString data)
    {
        MyQmlObject *obj = new MyQmlObject();
        obj->setStringProperty(data);
        return obj;
    }

    Q_INVOKABLE QString getStringProperty(MyQmlObject *obj)
    {
        return obj->stringProperty();
    }
};

void tst_qqmlecmascript::invokableWithQObjectDerived()
{
    CppInvokableWithQObjectDerived invokable;

    {
    QQmlEngine engine;
    engine.rootContext()->setContextProperty("invokable", &invokable);

    QQmlComponent component(&engine, testFileUrl("qobjectDerivedArgument.qml"));

    QObject *object = component.create();

    QVERIFY(object != nullptr);
    QVERIFY(object->property("result").value<bool>());

    delete object;
    }
}

void tst_qqmlecmascript::realTypePrecision()
{
    // Properties and signal parameters of type real should have double precision.
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("realTypePrecision.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("test").toDouble(), 1234567890.);
    QCOMPARE(object->property("test2").toDouble(), 1234567890.);
    QCOMPARE(object->property("test3").toDouble(), 1234567890.);
    QCOMPARE(object->property("test4").toDouble(), 1234567890.);
    QCOMPARE(object->property("test5").toDouble(), 1234567890.);
    QCOMPARE(object->property("test6").toDouble(), 1234567890.*2);
}

void tst_qqmlecmascript::registeredFlagMethod()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("registeredFlagMethod.qml"));
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != nullptr);

    QCOMPARE(object->buttons(), 0);
    emit object->basicSignal();
    QCOMPARE(object->buttons(), Qt::RightButton);

    delete object;
}

// QTBUG-23138
void tst_qqmlecmascript::replaceBinding()
{
    QQmlEngine engine;
    QQmlComponent c(&engine, testFileUrl("replaceBinding.qml"));
    QObject *obj = c.create();
    QVERIFY(obj != nullptr);

    QVERIFY(obj->property("success").toBool());
    delete obj;
}

void tst_qqmlecmascript::bindingBoundFunctions()
{
    QQmlEngine engine;
    QQmlComponent c(&engine, testFileUrl("bindingBoundFunctions.qml"));
    QObject *obj = c.create();
    QVERIFY(obj != nullptr);

    QVERIFY(obj->property("success").toBool());
    delete obj;
}

void tst_qqmlecmascript::deleteRootObjectInCreation()
{
    QQmlEngine engine;
    {
    QQmlComponent c(&engine, testFileUrl("deleteRootObjectInCreation.qml"));
    QObject *obj = c.create();
    QVERIFY(obj != nullptr);
    QVERIFY(obj->property("rootIndestructible").toBool());
    QVERIFY(!obj->property("childDestructible").toBool());
    QTest::qWait(1);
    QVERIFY(obj->property("childDestructible").toBool());
    delete obj;
    }

    {
    QQmlComponent c(&engine, testFileUrl("deleteRootObjectInCreation.2.qml"));
    QObject *object = c.create();
    QVERIFY(object != nullptr);
    QVERIFY(object->property("testConditionsMet").toBool());
    delete object;
    }
}

void tst_qqmlecmascript::onDestruction()
{
    {
        // Delete object manually to invoke the associated handlers,
        // prior to engine destruction.
        QQmlEngine engine;
        QQmlComponent c(&engine, testFileUrl("onDestruction.qml"));
        QObject *obj = c.create();
        QVERIFY(obj != nullptr);
        delete obj;
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    {
        // In this case, the teardown of the engine causes deletion
        // of contexts and child items.  This triggers the
        // onDestruction handler of a (previously .destroy()ed)
        // component instance.  This shouldn't crash.
        QQmlEngine engine;
        QQmlComponent c(&engine, testFileUrl("onDestruction.qml"));
        QObject *obj = c.create();
        QVERIFY(obj != nullptr);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
}

class WeakReferenceMutator : public QObject
{
    Q_OBJECT
public:
    WeakReferenceMutator()
        : resultPtr(nullptr)
        , weakRef(nullptr)
    {}

    void init(QV4::ExecutionEngine *v4, QV4::WeakValue *weakRef, bool *resultPtr)
    {
        QV4::QObjectWrapper::wrap(v4, this);
        QQmlEngine::setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);

        this->resultPtr = resultPtr;
        this->weakRef = weakRef;

        QObject::connect(QQmlComponent::qmlAttachedProperties(this), &QQmlComponentAttached::destruction, this, &WeakReferenceMutator::reviveFirstWeakReference);
    }

private slots:
    void reviveFirstWeakReference() {
        // weakRef is not required to be undefined here. The gc can clear it later.
        *resultPtr = weakRef->valueRef();
        if (!*resultPtr)
            return;
        QV4::ExecutionEngine *v4 = qmlEngine(this)->handle();
        weakRef->set(v4, v4->newObject());
        *resultPtr = weakRef->valueRef() && !weakRef->isNullOrUndefined();
    }

public:
    bool *resultPtr;

    QV4::WeakValue *weakRef;
};

QT_BEGIN_NAMESPACE

namespace QV4 {

namespace Heap {
struct WeakReferenceSentinel : public Object {
    void init(WeakValue *weakRef, bool *resultPtr)
    {
        Object::init();
        this->weakRef = weakRef;
        this->resultPtr = resultPtr;
    }

    void destroy() {
        *resultPtr = weakRef->isNullOrUndefined();
        Object::destroy();
    }

    WeakValue *weakRef;
    bool *resultPtr;
};
} // namespace Heap

struct WeakReferenceSentinel : public Object {
    V4_OBJECT2(WeakReferenceSentinel, Object)
    V4_NEEDS_DESTROY
};

} // namespace QV4

QT_END_NAMESPACE

DEFINE_OBJECT_VTABLE(QV4::WeakReferenceSentinel);

void tst_qqmlecmascript::onDestructionViaGC()
{
    qmlRegisterType<WeakReferenceMutator>("Test", 1, 0, "WeakReferenceMutator");

    QQmlEngine engine;
    QV4::ExecutionEngine *v4 =engine.handle();

    QQmlComponent component(&engine, testFileUrl("DestructionHelper.qml"));

    QScopedPointer<QV4::WeakValue> weakRef;

    bool mutatorResult = false;
    bool sentinelResult = false;

    {
        weakRef.reset(new QV4::WeakValue);
        weakRef->set(v4, v4->newObject());
        QVERIFY(!weakRef->isNullOrUndefined());

        QPointer<WeakReferenceMutator> weakReferenceMutator = qobject_cast<WeakReferenceMutator *>(component.create());
        QVERIFY2(!weakReferenceMutator.isNull(), qPrintable(component.errorString()));
        weakReferenceMutator->init(v4, weakRef.data(), &mutatorResult);

        v4->memoryManager->allocate<QV4::WeakReferenceSentinel>(weakRef.data(), &sentinelResult);
    }
    gc(engine);

    QVERIFY2(mutatorResult, "We failed to re-assign the weak reference a new value during GC");
    QVERIFY2(sentinelResult, "The weak reference was not cleared properly");
}

struct EventProcessor : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void process()
    {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    }
};

void tst_qqmlecmascript::bindingSuppression()
{
    QQmlEngine engine;
    EventProcessor processor;
    engine.rootContext()->setContextProperty("pendingEvents", &processor);

    QQmlTestMessageHandler messageHandler;

    QQmlComponent c(&engine, testFileUrl("bindingSuppression.qml"));
    QObject *obj = c.create();
    QVERIFY(obj != nullptr);
    delete obj;

    QVERIFY2(messageHandler.messages().isEmpty(), qPrintable(messageHandler.messageString()));
}

void tst_qqmlecmascript::signalEmitted()
{
    {
        // calling destroy on the sibling.
        QQmlEngine engine;
        QQmlComponent c(&engine, testFileUrl("signalEmitted.2.qml"));
        QObject *obj = c.create();
        QVERIFY(obj != nullptr);
        QTRY_VERIFY(obj->property("success").toBool());
        delete obj;
    }

    {
        // allowing gc to clean up the sibling.
        QQmlEngine engine;
        QQmlComponent c(&engine, testFileUrl("signalEmitted.3.qml"));
        QObject *obj = c.create();
        QVERIFY(obj != nullptr);
        gc(engine); // should collect c1.
        QTRY_VERIFY(obj->property("success").toBool());
        delete obj;
    }

    {
        // allowing gc to clean up the sibling after manually destroying target.
        QQmlEngine engine;
        QQmlComponent c(&engine, testFileUrl("signalEmitted.4.qml"));
        QObject *obj = c.create();
        QVERIFY(obj != nullptr);
        gc(engine); // should collect c1.
        QMetaObject::invokeMethod(obj, "destroyC2");
        QTRY_VERIFY(obj->property("success").toBool()); // handles events (incl. delete later).
        delete obj;
    }
}

// QTBUG-25647
void tst_qqmlecmascript::threadSignal()
{
    QQmlEngine engine;
    {
        QQmlComponent c(&engine, testFileUrl("threadSignal.qml"));
        QScopedPointer<QObject> object(c.create());
        QVERIFY(!object.isNull());
        QTRY_VERIFY(object->property("passed").toBool());
    }
    {
        QQmlComponent c(&engine, testFileUrl("threadSignal.2.qml"));
        QScopedPointer<QObject> object(c.create());
        QVERIFY(!object.isNull());
        QMetaObject::invokeMethod(object.data(), "doIt");
        QTRY_VERIFY(object->property("passed").toBool());
    }
}

// ensure that the qqmldata::destroyed() handler doesn't cause problems
void tst_qqmlecmascript::qqmldataDestroyed()
{
    QQmlEngine engine;
    // gc cleans up a qobject, later the qqmldata destroyed handler will run.
    {
        QQmlComponent c(&engine, testFileUrl("qqmldataDestroyed.qml"));
        QObject *object = c.create();
        QVERIFY(object != nullptr);
        // now gc causing the collection of the dynamically constructed object.
        engine.collectGarbage();
        engine.collectGarbage();
        // now process events to allow deletion (calling qqmldata::destroyed())
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
        // shouldn't crash.
        delete object;
    }

    // in this case, the object has CPP ownership, and the gc will
    // be triggered during its beginCreate stage.
    {
        QQmlComponent c(&engine, testFileUrl("qqmldataDestroyed.2.qml"));
        QObject *object = c.create();
        QVERIFY(object != nullptr);
        QVERIFY(object->property("testConditionsMet").toBool());
        // the gc() within the handler will have triggered the weak
        // qobject reference callback.  If that incorrectly disposes
        // the handle, when the qqmldata::destroyed() handler is
        // called due to object deletion we will see a crash.
        delete object;
        // shouldn't have crashed.
    }
}

void tst_qqmlecmascript::secondAlias()
{
    QQmlEngine engine;
    QQmlComponent c(&engine, testFileUrl("secondAlias.qml"));
    QObject *object = c.create();
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("test").toInt(), 200);
    delete object;
}

// An alias to a var property works
void tst_qqmlecmascript::varAlias()
{
    QQmlEngine engine;
    QQmlComponent c(&engine, testFileUrl("varAlias.qml"));
    QObject *object = c.create();
    QVERIFY(object != nullptr);
    QCOMPARE(object->property("test").toInt(), 192);
    delete object;
}

// Used to trigger an assert in the lazy meta object creation stage
void tst_qqmlecmascript::overrideDataAssert()
{
    QQmlEngine engine;
    QQmlComponent c(&engine, testFileUrl("overrideDataAssert.qml"));
    QObject *object = c.create();
    QVERIFY(object != nullptr);
    object->metaObject();
    delete object;
}

void tst_qqmlecmascript::fallbackBindings_data()
{
    QTest::addColumn<QString>("source");

    QTest::newRow("Property without fallback") << "fallbackBindings.1.qml";
    QTest::newRow("Property fallback") << "fallbackBindings.2.qml";
    QTest::newRow("SingletonType without fallback") << "fallbackBindings.3.qml";
    QTest::newRow("SingletonType fallback") << "fallbackBindings.4.qml";
    QTest::newRow("Attached without fallback") << "fallbackBindings.5.qml";
    QTest::newRow("Attached fallback") << "fallbackBindings.6.qml";
    QTest::newRow("Subproperty without fallback") << "fallbackBindings.7.qml";
    QTest::newRow("Subproperty fallback") << "fallbackBindings.8.qml";
}

void tst_qqmlecmascript::fallbackBindings()
{
    QFETCH(QString, source);

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl(source));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("success").toBool(), true);
}

void tst_qqmlecmascript::propertyOverride()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("propertyOverride.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);

    QCOMPARE(object->property("success").toBool(), true);
}

void tst_qqmlecmascript::sequenceSort_data()
{
    QTest::addColumn<QString>("function");
    QTest::addColumn<bool>("useComparer");

    QTest::newRow("qtbug_25269") << "test_qtbug_25269" << false;

    const char *types[] = { "alphabet", "numbers", "reals", "number_vector", "real_vector" };
    const char *sort[] = { "insertionSort", "quickSort" };

    for (size_t t=0 ; t < sizeof(types)/sizeof(types[0]) ; ++t) {
        for (size_t s=0 ; s < sizeof(sort)/sizeof(sort[0]) ; ++s) {
            for (int c=0 ; c < 2 ; ++c) {
                QString testName = QLatin1String(types[t]) + QLatin1Char('_') + QLatin1String(sort[s]);
                QString fnName = QLatin1String("test_") + testName;
                bool useComparer = c != 0;
                testName += useComparer ? QLatin1String("[custom]") : QLatin1String("[default]");
                QTest::newRow(testName.toLatin1().constData()) << fnName << useComparer;
            }
        }
    }
}

void tst_qqmlecmascript::sequenceSort()
{
    QFETCH(QString, function);
    QFETCH(bool, useComparer);

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("sequenceSort.qml"));

    QObject *object = component.create();
    if (object == nullptr)
        qDebug() << component.errorString();
    QVERIFY(object != nullptr);

    QVariant q;
    QMetaObject::invokeMethod(object, function.toLatin1().constData(), Q_RETURN_ARG(QVariant, q), Q_ARG(QVariant, useComparer));
    QVERIFY(q.toBool());

    delete object;
}

void tst_qqmlecmascript::dateParse()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("date.qml"));

    QObject *object = component.create();
    if (object == nullptr)
        qDebug() << component.errorString();
    QVERIFY(object != nullptr);

    QVariant q;
    QMetaObject::invokeMethod(object, "test_is_invalid_jsDateTime", Q_RETURN_ARG(QVariant, q));
    QVERIFY(q.toBool());

    QMetaObject::invokeMethod(object, "test_is_invalid_qtDateTime", Q_RETURN_ARG(QVariant, q));
    QVERIFY(q.toBool());

    QMetaObject::invokeMethod(object, "test_rfc2822_date", Q_RETURN_ARG(QVariant, q));
    QCOMPARE(q.toLongLong(), 1379512851000LL);
}

void tst_qqmlecmascript::utcDate()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("utcdate.qml"));

    QObject *object = component.create();
    if (object == nullptr)
        qDebug() << component.errorString();
    QVERIFY(object != nullptr);

    QVariant q;
    QVariant val = QString::fromLatin1("2014-07-16T23:30:31");
    QMetaObject::invokeMethod(object, "check_utc", Q_RETURN_ARG(QVariant, q), Q_ARG(QVariant, val));
    QVERIFY(q.toBool());
}

void tst_qqmlecmascript::negativeYear()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("negativeyear.qml"));

    QObject *object = component.create();
    if (object == nullptr)
        qDebug() << component.errorString();
    QVERIFY(object != nullptr);

    QVariant q;
    QMetaObject::invokeMethod(object, "check_negative_tostring", Q_RETURN_ARG(QVariant, q));

    // Only check for the year. We hope that every language writes the year in arabic numerals and
    // in relation to a specific dude's date of birth. We also hope that no language adds a "-2001"
    // junk string somewhere in the middle.
    QVERIFY(q.toString().indexOf(QStringLiteral("-2001")) != -1);

    QMetaObject::invokeMethod(object, "check_negative_toisostring", Q_RETURN_ARG(QVariant, q));
    QCOMPARE(q.toString().left(16), QStringLiteral("result: -002000-"));
}

void tst_qqmlecmascript::concatenatedStringPropertyAccess()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("concatenatedStringPropertyAccess.qml"));
    QObject *object = component.create();
    QVERIFY(object);
    QVERIFY(object->property("success").toBool());
    delete object;
}

void tst_qqmlecmascript::jsOwnedObjectsDeletedOnEngineDestroy()
{
    QQmlEngine *myEngine = new QQmlEngine;

    MyDeleteObject deleteObject;
    deleteObject.setObjectName("deleteObject");
    QObject * const object1 = new QObject;
    QObject * const object2 = new QObject;
    object1->setObjectName("object1");
    object2->setObjectName("object2");
    deleteObject.setObject1(object1);
    deleteObject.setObject2(object2);

    // Objects returned by function calls get marked as destructible, but objects returned by
    // property getters do not - therefore we explicitly set the object as destructible.
    QQmlEngine::setObjectOwnership(object2, QQmlEngine::JavaScriptOwnership);

    myEngine->rootContext()->setContextProperty("deleteObject", &deleteObject);
    QQmlComponent component(myEngine, testFileUrl("jsOwnedObjectsDeletedOnEngineDestroy.qml"));
    QObject *object = component.create();
    QVERIFY(object);

    // Destroying the engine should delete all JS owned QObjects
    QSignalSpy spy1(object1, SIGNAL(destroyed()));
    QSignalSpy spy2(object2, SIGNAL(destroyed()));
    delete myEngine;
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy2.count(), 1);

    delete object;
}

void tst_qqmlecmascript::updateCall()
{
    // update is a slot on QQuickItem. Even though it's not
    // documented it can be called from within QML. Make sure
    // we don't crash when calling it.
    QString file("updateCall.qml");
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl(file));
    QObject *object = component.create();
    QVERIFY(object != nullptr);
}

void tst_qqmlecmascript::numberParsing()
{
    QQmlEngine engine;
    for (int i = 1; i < 8; ++i) {
        QString file("numberParsing.%1.qml");
        file = file.arg(i);
        QQmlComponent component(&engine, testFileUrl(file));
        QObject *object = component.create();
        QVERIFY(object != nullptr);
    }
    for (int i = 1; i < 3; ++i) {
        QString file("numberParsing_error.%1.qml");
        file = file.arg(i);
        QQmlComponent component(&engine, testFileUrl(file));
        QVERIFY(!component.errors().isEmpty());
    }
}

void tst_qqmlecmascript::stringParsing()
{
    QQmlEngine engine;
    for (int i = 1; i < 7; ++i) {
        QString file("stringParsing_error.%1.qml");
        file = file.arg(i);
        QQmlComponent component(&engine, testFileUrl(file));
        QObject *object = component.create();
        QVERIFY(!object);
    }
}

void tst_qqmlecmascript::push_and_shift()
{
    QJSEngine e;
    const QString program =
            "var array = []; "
            "for (var i = 0; i < 10000; i++) {"
            "    array.push(5); array.unshift(5); array.push(5);"
            "}"
            "array.length;";
    QCOMPARE(e.evaluate(program).toNumber(), double(30000));
}

void tst_qqmlecmascript::qtbug_32801()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_32801.qml"));

    QScopedPointer<QObject> obj(component.create());
    QVERIFY(obj != nullptr);

    // do not crash when a QML signal is connected to a non-void slot
    connect(obj.data(), SIGNAL(testSignal(QString)), obj.data(), SLOT(slotWithReturnValue(QString)));
    QVERIFY(QMetaObject::invokeMethod(obj.data(), "emitTestSignal"));
}

void tst_qqmlecmascript::thisObject()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("thisObject.qml"));
    QObject *object = component.create();
    QVERIFY(object);
    QCOMPARE(qvariant_cast<QObject*>(object->property("subObject"))->property("test").toInt(), 2);
    delete object;
}

void tst_qqmlecmascript::qtbug_33754()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_33754.qml"));

    QScopedPointer<QObject> obj(component.create());
    QVERIFY(obj != nullptr);
}

void tst_qqmlecmascript::qtbug_34493()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_34493.qml"));

    QScopedPointer<QObject> obj(component.create());
    if (component.errors().size())
        qDebug() << component.errors();
    QVERIFY(component.errors().isEmpty());
    QVERIFY(obj != nullptr);
    QVERIFY(QMetaObject::invokeMethod(obj.data(), "doIt"));
    QTRY_VERIFY(obj->property("prop").toString() == QLatin1String("Hello World!"));
}

// Check that a Singleton can be passed from QML to C++
// as its type*, it's parent type* and as QObject*
void tst_qqmlecmascript::singletonFromQMLToCpp()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFile("singletonTest.qml"));
    QScopedPointer<QObject> obj(component.create());
    if (component.errors().size())
        qDebug() << component.errors();
    QVERIFY(component.errors().isEmpty());
    QVERIFY(obj != nullptr);

    QCOMPARE(obj->property("qobjectTest"), QVariant(true));
    QCOMPARE(obj->property("myQmlObjectTest"), QVariant(true));
    QCOMPARE(obj->property("myInheritedQmlObjectTest"), QVariant(true));
}

// Check that a Singleton can be passed from QML to C++
// as its type*, it's parent type* and as QObject*
// and correctly compares to itself
void tst_qqmlecmascript::singletonFromQMLAndBackAndCompare()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFile("singletonTest2.qml"));
    QScopedPointer<QObject> o(component.create());
    if (component.errors().size())
        qDebug() << component.errors();
    QVERIFY(component.errors().isEmpty());
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("myInheritedQmlObjectTest1"), QVariant(true));
    QCOMPARE(o->property("myInheritedQmlObjectTest2"), QVariant(true));
    QCOMPARE(o->property("myInheritedQmlObjectTest3"), QVariant(true));

    QCOMPARE(o->property("myQmlObjectTest1"), QVariant(true));
    QCOMPARE(o->property("myQmlObjectTest2"), QVariant(true));
    QCOMPARE(o->property("myQmlObjectTest3"), QVariant(true));

    QCOMPARE(o->property("qobjectTest1"), QVariant(true));
    QCOMPARE(o->property("qobjectTest2"), QVariant(true));
    QCOMPARE(o->property("qobjectTest3"), QVariant(true));

    QCOMPARE(o->property("singletonEqualToItself"), QVariant(true));
}

void tst_qqmlecmascript::setPropertyOnInvalid()
{
    QQmlEngine engine;
    {
        QQmlComponent component(&engine, testFileUrl("setPropertyOnNull.qml"));
        QString warning = component.url().toString() + ":4: TypeError: Value is null and could not be converted to an object";
        QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));
        QObject *object = component.create();
        QVERIFY(object);
        delete object;
    }

    {
        QQmlComponent component(&engine, testFileUrl("setPropertyOnUndefined.qml"));
        QString warning = component.url().toString() + ":4: TypeError: Value is undefined and could not be converted to an object";
        QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));
        QObject *object = component.create();
        QVERIFY(object);
        delete object;
    }
}

void tst_qqmlecmascript::miscTypeTest()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("misctypetest.qml"));

    QObject *object = component.create();
    if (object == nullptr)
        qDebug() << component.errorString();
    QVERIFY(object != nullptr);

    QVariant q;
    QMetaObject::invokeMethod(object, "test_invalid_url_equal", Q_RETURN_ARG(QVariant, q));
    QVERIFY(q.toBool());
    QMetaObject::invokeMethod(object, "test_invalid_url_strictequal", Q_RETURN_ARG(QVariant, q));
    QVERIFY(q.toBool());
    QMetaObject::invokeMethod(object, "test_valid_url_equal", Q_RETURN_ARG(QVariant, q));
    QVERIFY(q.toBool());
    QMetaObject::invokeMethod(object, "test_valid_url_strictequal", Q_RETURN_ARG(QVariant, q));
    QVERIFY(q.toBool());

    delete object;
}

void tst_qqmlecmascript::stackLimits()
{
    QJSEngine engine;
    engine.evaluate(QStringLiteral("function foo() {foo();} try {foo()} catch(e) { }"));
}

void tst_qqmlecmascript::idsAsLValues()
{
    QQmlEngine engine;
    QString err = QString(QLatin1String("%1:5: Error: left-hand side of assignment operator is not an lvalue")).arg(testFileUrl("idAsLValue.qml").toString());
    QQmlComponent component(&engine, testFileUrl("idAsLValue.qml"));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(err));
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(!object);
}

void tst_qqmlecmascript::qtbug_34792()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug34792.qml"));

    QObject *object = component.create();
    if (object == nullptr)
        qDebug() << component.errorString();
    QVERIFY(object != nullptr);
    delete object;
}

void tst_qqmlecmascript::noCaptureWhenWritingProperty()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("noCaptureWhenWritingProperty.qml"));
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());
    QCOMPARE(obj->property("somePropertyEvaluated").toBool(), false);
}

void tst_qqmlecmascript::singletonWithEnum()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("singletontype/singletonWithEnum.qml"));
    QScopedPointer<QObject> obj(component.create());
    if (obj.isNull())
        qDebug() << component.errors().first().toString();
    QVERIFY(!obj.isNull());
    QVariant prop = obj->property("testValue");
    QCOMPARE(prop.type(), QVariant::Int);
    QCOMPARE(prop.toInt(), int(SingletonWithEnum::TestValue));

    {
        QQmlExpression expr(qmlContext(obj.data()), obj.data(), "SingletonWithEnum.TestValue_MinusOne");
        bool valueUndefined = false;
        QVariant result = expr.evaluate(&valueUndefined);
        QVERIFY2(!expr.hasError(), qPrintable(expr.error().toString()));
        QVERIFY(!valueUndefined);
        QCOMPARE(result.toInt(), -1);
    }
}

void tst_qqmlecmascript::lazyBindingEvaluation()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("lazyBindingEvaluation.qml"));
    QScopedPointer<QObject> obj(component.create());
    if (obj.isNull())
       qDebug() << component.errors().first().toString();
    QVERIFY(!obj.isNull());
    QVariant prop = obj->property("arrayLength");
    QCOMPARE(prop.type(), QVariant::Int);
    QCOMPARE(prop.toInt(), 2);
}

void tst_qqmlecmascript::varPropertyAccessOnObjectWithInvalidContext()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("varPropertyAccessOnObjectWithInvalidContext.qml"));
    QScopedPointer<QObject> obj(component.create());
    if (obj.isNull())
       qDebug() << component.errors().first().toString();
    QVERIFY(!obj.isNull());
    QVERIFY(obj->property("success").toBool());
}

void tst_qqmlecmascript::importedScriptsAccessOnObjectWithInvalidContext()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("importedScriptsAccessOnObjectWithInvalidContext.qml"));
    QScopedPointer<QObject> obj(component.create());
    if (obj.isNull())
       qDebug() << component.errors().first().toString();
    QVERIFY(!obj.isNull());
    QTRY_VERIFY(obj->property("success").toBool());
}

void tst_qqmlecmascript::importedScriptsWithoutQmlMode()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("importScriptsWithoutQmlMode.qml"));
    QScopedPointer<QObject> obj(component.create());
    if (obj.isNull())
       qDebug() << component.errors().first().toString();
    QVERIFY(!obj.isNull());
    QTRY_VERIFY(obj->property("success").toBool());
}

void tst_qqmlecmascript::contextObjectOnLazyBindings()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("contextObjectOnLazyBindings.qml"));
    QScopedPointer<QObject> obj(component.create());
    if (obj.isNull())
        qDebug() << component.errors().first().toString();
    QVERIFY(!obj.isNull());
    QObject *subObject = qvariant_cast<QObject*>(obj->property("subObject"));
    QVERIFY(subObject);
    QCOMPARE(subObject->property("testValue").toInt(), int(42));
}

void tst_qqmlecmascript::garbageCollectionDuringCreation()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Qt.test 1.0\n"
                      "QObjectContainerWithGCOnAppend {\n"
                      "    objectName: \"root\"\n"
                      "    FloatingQObject {\n"
                      "        objectName: \"parentLessChild\"\n"
                      "        property var blah;\n" // Ensure we have JS wrapper
                      "    }\n"
                      "}\n",
                      QUrl());

    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QObjectContainer *container = qobject_cast<QObjectContainer*>(object.data());
    QCOMPARE(container->dataChildren.count(), 1);

    QObject *child = container->dataChildren.first();
    QQmlData *ddata = QQmlData::get(child);
    QVERIFY(!ddata->jsWrapper.isNullOrUndefined());

    gc(engine);
    QCOMPARE(container->dataChildren.count(), 0);
}

void tst_qqmlecmascript::qtbug_39520()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.0\n"
                      "Item {\n"
                      "    property string s\n"
                      "    Component.onCompleted: test()\n"
                      "    function test() {\n"
                      "    var count = 1 * 1000 * 1000\n"
                      "    var t = ''\n"
                      "    for (var i = 0; i < count; ++i)\n"
                      "        t += 'testtest ' + i + '\n'\n"
                      "    s = t\n"
                      "    }\n"
                      "}\n",
                      QUrl());

    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QString s = object->property("s").toString();
    QCOMPARE(s.count('\n'), 1 * 1000 * 1000);
}

class ContainedObject1 : public QObject
{
    Q_OBJECT
};

class ContainedObject2 : public QObject
{
    Q_OBJECT
};

class ObjectContainer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ContainedObject1 *object1 READ object1 WRITE setObject1)
    Q_PROPERTY(ContainedObject2 *object2 READ object2 WRITE setObject2)
public:
    explicit ObjectContainer(QObject *parent = nullptr) :
        QObject(parent),
        mGetterCalled(false),
        mSetterCalled(false)
    {
    }

    ContainedObject1 *object1()
    {
        mGetterCalled = true;
        return nullptr;
    }

    void setObject1(ContainedObject1 *)
    {
        mSetterCalled = true;
    }

    ContainedObject2 *object2()
    {
        mGetterCalled = true;
        return nullptr;
    }

    void setObject2(ContainedObject2 *)
    {
        mSetterCalled = true;
    }

public:
    bool mGetterCalled;
    bool mSetterCalled;
};

void tst_qqmlecmascript::readUnregisteredQObjectProperty()
{
    qmlRegisterType<ObjectContainer>("Test", 1, 0, "ObjectContainer");
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("accessUnregisteredQObjectProperty.qml"));
    QObject *root = component.create();
    QVERIFY(root);

    QMetaObject::invokeMethod(root, "readProperty");
    QCOMPARE(root->property("container").value<ObjectContainer*>()->mGetterCalled, true);
}

void tst_qqmlecmascript::writeUnregisteredQObjectProperty()
{
    qmlRegisterType<ObjectContainer>("Test", 1, 0, "ObjectContainer");
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("accessUnregisteredQObjectProperty.qml"));
    QObject *root = component.create();
    QVERIFY(root);

    QMetaObject::invokeMethod(root, "writeProperty");
    QCOMPARE(root->property("container").value<ObjectContainer*>()->mSetterCalled, true);
}

void tst_qqmlecmascript::switchExpression()
{
    // verify that we evaluate the expression inside switch() exactly once
    QJSEngine engine;
    QJSValue v = engine.evaluate(QString::fromLatin1(
            "var num = 0\n"
            "var x = 0\n"
            "function f() { ++num; return (Math.random() > 0.5) ? 0 : 1; }\n"
            "for (var i = 0; i < 1000; ++i) {\n"
            "   switch (f()) {\n"
            "   case 0:\n"
            "   case 1:\n"
            "       break;\n"
            "   default:\n"
            "       ++x;\n"
            "   }\n"
            "}\n"
            "(x == 0 && num == 1000) ? true : false\n"
                        ));
    QVERIFY(!v.isError());
    QCOMPARE(v.toBool(), true);
}

void tst_qqmlecmascript::qtbug_46022()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_46022.qml"));

    QScopedPointer<QObject> obj(component.create());
    QVERIFY(obj != nullptr);
    QCOMPARE(obj->property("test1").toBool(), true);
    QCOMPARE(obj->property("test2").toBool(), true);
}

void tst_qqmlecmascript::qtbug_52340()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_52340.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());
    QVariant returnValue;
    QVERIFY(QMetaObject::invokeMethod(object.data(), "testCall", Q_RETURN_ARG(QVariant, returnValue)));
    QVERIFY(returnValue.isValid());
    QVERIFY(returnValue.toBool());
}

void tst_qqmlecmascript::qtbug_54589()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_54589.qml"));

    QScopedPointer<QObject> obj(component.create());
    QVERIFY(obj != nullptr);
    QCOMPARE(obj->property("result").toBool(), true);
}

void tst_qqmlecmascript::qtbug_54687()
{
    QJSEngine e;
    // it's simple: this shouldn't crash.
    e.evaluate("12\n----12");
}

void tst_qqmlecmascript::stringify_qtbug_50592()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("stringify_qtbug_50592.qml"));

    QScopedPointer<QObject> obj(component.create());
    QVERIFY(obj != nullptr);
    QCOMPARE(obj->property("source").toString(),
             QString::fromLatin1("\"http://example.org/some_nonexistant_image.png\""));
}

// Tests for the JS-only instanceof. Tests for the QML extensions for
// instanceof belong in tst_qqmllanguage!
void tst_qqmlecmascript::instanceof_data()
{
    QTest::addColumn<QString>("setupCode");
    QTest::addColumn<QVariant>("expectedValue");

    // so the way this works is that the name of the test tag defines the test
    // to run. the code in setupCode defines code run before the actual test
    // (e.g. to create vars).
    //
    // the expectedValue is either a boolean true or false for whether the two
    // operands are indeed an instanceof each other, or a string for the
    // expected error message.
    QTest::newRow("String instanceof String")
            << ""
            << QVariant(false);
    QTest::newRow("s instanceof String")
            << "var s = \"hello\""
            << QVariant(false);
    QTest::newRow("objectString instanceof String")
            << "var objectString = new String(\"hello\")"
            << QVariant(true);
    QTest::newRow("o instanceof Object")
            << "var o = new Object()"
            << QVariant(true);
    QTest::newRow("o instanceof String")
            << "var o = new Object()"
            << QVariant(false);
    QTest::newRow("true instanceof true")
            << ""
            << QVariant("TypeError: Type error");
    QTest::newRow("1 instanceof Math")
            << ""
            << QVariant("TypeError: Type error");
    QTest::newRow("date instanceof Date")
            << "var date = new Date"
            << QVariant(true);
    QTest::newRow("date instanceof Object")
            << "var date = new Date"
            << QVariant(true);
    QTest::newRow("date instanceof String")
            << "var date = new Date"
            << QVariant(false);
}

void tst_qqmlecmascript::instanceof()
{
    QFETCH(QString, setupCode);
    QFETCH(QVariant, expectedValue);

    QJSEngine engine;
    QJSValue ret = engine.evaluate(setupCode + ";\n" + QTest::currentDataTag());

    if (expectedValue.type() == QVariant::Bool) {
        bool returnValue = ret.toBool();
        QVERIFY2(!ret.isError(), qPrintable(ret.toString()));
        QCOMPARE(returnValue, expectedValue.toBool());
    } else {
        QVERIFY2(ret.isError(), qPrintable(ret.toString()));
        QCOMPARE(ret.toString(), expectedValue.toString());
    }
}

void tst_qqmlecmascript::constkw_data()
{
    QTest::addColumn<QString>("sourceCode");
    QTest::addColumn<bool>("exceptionExpected");
    QTest::addColumn<QVariant>("expectedValue");

    QTest::newRow("simpleconst")
        << "const v = 5\n"
           "v\n"
        << false
        << QVariant(5);
    QTest::newRow("twoconst")
        << "const v = 5, i = 10\n"
           "v + i\n"
        << false
        << QVariant(15);
    QTest::newRow("constandvar")
        << "const v = 5\n"
           "var i = 20\n"
           "v + i\n"
        << false
        << QVariant(25);
    QTest::newRow("const-multiple-scopes-same-var")
        << "const v = 3\n"
           "function f() { const v = 1; return v; }\n"
           "v + f()\n"
        << false
        << QVariant(4);

    // error cases
    QTest::newRow("const-no-initializer")
        << "const v\n"
        << true
        << QVariant("SyntaxError: Missing initializer in const declaration");
    QTest::newRow("const-no-initializer-comma")
        << "const v = 1, i\n"
        << true
        << QVariant("SyntaxError: Missing initializer in const declaration");
    QTest::newRow("const-no-duplicate")
        << "const v = 1, v = 2\n"
        << true
        << QVariant("SyntaxError: Identifier v has already been declared");
    QTest::newRow("const-no-duplicate-2")
        << "const v = 1\n"
           "const v = 2\n"
        << true
        << QVariant("SyntaxError: Identifier v has already been declared");
    QTest::newRow("const-no-duplicate-var")
        << "const v = 1\n"
           "var v = 1\n"
        << true
        << QVariant("SyntaxError: Identifier v has already been declared");
    QTest::newRow("var-no-duplicate-const")
        << "var v = 1\n"
           "const v = 1\n"
        << true
        << QVariant("SyntaxError: Identifier v has already been declared");
    QTest::newRow("const-no-duplicate-let")
        << "const v = 1\n"
           "let v = 1\n"
        << true
        << QVariant("SyntaxError: Identifier v has already been declared");
    QTest::newRow("let-no-duplicate-const")
        << "let v = 1\n"
           "const v = 1\n"
        << true
        << QVariant("SyntaxError: Identifier v has already been declared");
}

void tst_qqmlecmascript::constkw()
{
    QFETCH(QString, sourceCode);
    QFETCH(bool, exceptionExpected);
    QFETCH(QVariant, expectedValue);

    QJSEngine engine;
    QJSValue ret = engine.evaluate(sourceCode);

    if (!exceptionExpected) {
        QVERIFY2(!ret.isError(), qPrintable(ret.toString()));
        QCOMPARE(ret.toVariant(), expectedValue);
    } else {
        QVERIFY2(ret.isError(), qPrintable(ret.toString()));
        QCOMPARE(ret.toString(), expectedValue.toString());
    }
}

// Redefine a property found on the global object. It shouldn't throw.
void tst_qqmlecmascript::redefineGlobalProp()
{
    {
        QJSEngine engine;
        QJSValue ret = engine.evaluate("\"use strict\"\n var toString = 1;");
        QVERIFY2(!ret.isError(), qPrintable(ret.toString()));
    }
    {
        QJSEngine engine;
        QJSValue ret = engine.evaluate("var toString = 1;");
        QVERIFY2(!ret.isError(), qPrintable(ret.toString()));
    }
}

void tst_qqmlecmascript::freeze_empty_object()
{
    // this shouldn't crash
    QJSEngine engine;
    QJSValue v = engine.evaluate(QString::fromLatin1(
            "var obj = {};\n"
            "Object.freeze(obj);\n"
    ));
    QVERIFY(!v.isError());
    QCOMPARE(v.toBool(), true);
}

void tst_qqmlecmascript::singleBlockLoops()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug_59012.qml"));

    QScopedPointer<QObject> obj(component.create());
    QVERIFY(obj != nullptr);
    QVERIFY(!component.isError());
}

// 'counter' was incorrectly resolved as a type rather than a variable.
// This fix ensures it looks up the right thing.
void tst_qqmlecmascript::qtbug_60547()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qtbug60547/main.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));
    QCOMPARE(object->property("counter"), QVariant(int(1)));
}

void tst_qqmlecmascript::anotherNaN()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("nans.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));
    object->setProperty("prop", std::numeric_limits<double>::quiet_NaN()); // don't crash

    std::uint64_t anotherNaN = 0xFFFFFF01000000F7ul;
    double d;
    std::memcpy(&d, &anotherNaN, sizeof(d));
    QVERIFY(std::isnan(d));
    object->setProperty("prop", d);  // don't crash
}

void tst_qqmlecmascript::delayLoadingArgs()
{
    QJSEngine engine;
    QJSValue ret = engine.evaluate("(function(x){return x + (x+=2)})(20)");
    QCOMPARE(ret.toInt(), 42); // esp. not 44.
}

void tst_qqmlecmascript::manyArguments()
{
    const char *testCase =
            "function x() { var sum; for (var i = 0; i < arguments.length; ++i) sum += arguments[i][0]; }"
            "x([0],[1],[2],[3],[4],[5],[6],[7],[8],[9], [0],[1],[2],[3],[4],[5],[6],[7],[8],[9], [0],[1],[2],[3],[4],[5],[6],[7],[8],[9])";

    QJSEngine engine;
    engine.evaluate(testCase);
}

void tst_qqmlecmascript::forInIterator()
{
    auto testCase =
            "(function(){\n"
            "var x = 'yoyo'\n"
            "var i\n"
            "for (i in x) {\n"
            "}\n"
            "return i\n"
            "})()";
    QJSEngine engine;
    QJSValue ret = engine.evaluate(testCase);
    QVERIFY(ret.isString());
    QCOMPARE(ret.toString(), QStringLiteral("3"));
}

void tst_qqmlecmascript::localForInIterator()
{
    auto testCase =
            "(function(){\n"
            "var x = 'yoyo'\n"
            "for (var i in x) {\n"
            "}\n"
            "return i\n"
            "})()";
    QJSEngine engine;
    QJSValue ret = engine.evaluate(testCase);
    QVERIFY(ret.isString());
    QCOMPARE(ret.toString(), QStringLiteral("3"));
}

void tst_qqmlecmascript::shadowedFunctionName()
{
    // verify that arguments shadow the function name
    QJSEngine engine;
    QJSValue v = engine.evaluate(QString::fromLatin1(
            "function f(f) { return f; }\n"
            "f(true)\n"
                        ));
    QVERIFY(!v.isError());
    QVERIFY(v.isBool());
    QCOMPARE(v.toBool(), true);
}

void tst_qqmlecmascript::callPropertyOnUndefined()
{
    QJSEngine engine;
    QJSValue v = engine.evaluate(QString::fromLatin1(
            "function f() {\n"
            "    var base;\n"
            "    base.push(1);"
            "}\n"
    ));
    QVERIFY(!v.isError()); // well, more importantly: this shouldn't fail on an assert.
}

void tst_qqmlecmascript::jumpStrictNotEqualUndefined()
{
    QJSEngine engine;
    QJSValue v = engine.evaluate(QString::fromLatin1(
        "var ok = 0\n"
        "var foo = 0\n"
        "if (foo !== void 1)\n"
        "    ++ok;\n"
        "else\n"
        "    --ok;\n"
        "if (foo === void 1)\n"
        "    --ok;\n"
        "else\n"
        "    ++ok;\n"
        "ok\n"
    ));
    QVERIFY(!v.isError());
    QCOMPARE(v.toInt(), 2);
}

void tst_qqmlecmascript::removeBindingsWithNoDependencies()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("removeBindingsWithNoDependencies.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());
    QVariant rect = object->property("placement");
    QCOMPARE(rect.toRectF(), QRectF(0, 0, 100, 100));
    const QMetaObject *metaObject = object->metaObject();

    {
        const QMetaProperty prop = metaObject->property(metaObject->indexOfProperty("placement"));
        QVERIFY(prop.isValid());
        QVERIFY(!QQmlPropertyPrivate::binding(object.data(), QQmlPropertyIndex(prop.propertyIndex())));
    }

    {
        const QMetaProperty prop = metaObject->property(metaObject->indexOfProperty("partialPlacement"));
        QVERIFY(prop.isValid());
        QQmlAbstractBinding *vtProxyBinding = QQmlPropertyPrivate::binding(object.data(), QQmlPropertyIndex(prop.propertyIndex()));
        QVERIFY(vtProxyBinding);
        QVERIFY(vtProxyBinding->isValueTypeProxy());

        QQmlValueTypeProxyBinding *proxy = static_cast<QQmlValueTypeProxyBinding*>(vtProxyBinding);
        QVERIFY(!proxy->subBindings());
    }

}

void tst_qqmlecmascript::preserveBindingWithUnresolvedNames()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("preserveBindingWithUnresolvedNames.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());
    QCOMPARE(object->property("testTypeOf").toString(), QString("undefined"));
    QObject obj;
    engine.rootContext()->setContextProperty("contextProp", &obj);
    QCOMPARE(object->property("testTypeOf").toString(), QString("object"));
}

void tst_qqmlecmascript::temporaryDeadZone()
{
    QJSEngine engine;
    QJSValue v = engine.evaluate(QString::fromLatin1("a; let a;"));
    QVERIFY(v.isError());
    v = engine.evaluate(QString::fromLatin1("a.name; let a;"));
    QVERIFY(v.isError());
    v = engine.evaluate(QString::fromLatin1("var a = {}; a[b]; let b;"));
    QVERIFY(v.isError());
    v = engine.evaluate(QString::fromLatin1("class C { constructor() { super[x]; let x; } }; new C()"));
    QVERIFY(v.isError());
}

void tst_qqmlecmascript::importLexicalVariables_data()
{
    QTest::addColumn<QUrl>("testFile");
    QTest::addColumn<QString>("expected");

    QTest::newRow("script")
        << testFileUrl("importLexicalVariables_script.qml")
        << QStringLiteral("000 100 210");
    QTest::newRow("pragmaLibrary")
        << testFileUrl("importLexicalVariables_pragmaLibrary.qml")
        << QStringLiteral("000 100 210");
    QTest::newRow("module")
        << testFileUrl("importLexicalVariables_module.qml")
        << QStringLiteral("000 000 110");
}

void tst_qqmlecmascript::importLexicalVariables()
{
    QFETCH(QUrl, testFile);
    QFETCH(QString, expected);

    QQmlEngine engine;
    QQmlComponent component(&engine, testFile);
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);
    QVERIFY(!component.isError());

    QVariant result;
    QMetaObject::invokeMethod(object.data(), "runTest", Qt::DirectConnection, Q_RETURN_ARG(QVariant, result));
    QCOMPARE(result, QVariant(expected));
}

void tst_qqmlecmascript::hugeObject()
{
    // mainly check that this doesn't crash
    QJSEngine engine;
    QJSValue v = engine.evaluate(QString::fromLatin1(
        "var known = {}, prefix = 'x'\n"
        "for (var i = 0; i < 150000; i++) known[prefix + i] = true;"
    ));
    QVERIFY(!v.isError());
}

void tst_qqmlecmascript::templateStringTerminator()
{
    QJSEngine engine;
    const QJSValue value = engine.evaluate("let a = 123; let b = `x${a}\ny^`; b;");
    QVERIFY(!value.isError());
    QCOMPARE(value.toString(), QLatin1String("x123\ny^"));
}

void tst_qqmlecmascript::arrayAndException()
{
    QJSEngine engine;
    const QJSValue value = engine.evaluate("[...[],[,,$]]");
    // Should not crash
    QVERIFY(value.isError());
}

void tst_qqmlecmascript::numberToStringWithRadix()
{
    QJSEngine engine;
    {
        const QJSValue value = engine.evaluate(".5.toString(5)");
        QVERIFY(!value.isError());
        QVERIFY(value.toString().startsWith("0.2222222222"));
    }
    {
        const QJSValue value = engine.evaluate(".05.toString(5)");
        QVERIFY(!value.isError());
        QVERIFY(value.toString().startsWith("0.01111111111"));
    }
}

void tst_qqmlecmascript::tailCallWithArguments()
{
    QJSEngine engine;
    const QJSValue value = engine.evaluate(
            "'use strict';\n"
            "[[1, 2]].map(function (a) {\n"
            "    return (function() { return Math.min.apply(this, arguments); })(a[0], a[1]);\n"
            "})[0];");
    QVERIFY(!value.isError());
    QCOMPARE(value.toInt(), 1);
}

void tst_qqmlecmascript::deleteSparseInIteration()
{
    QJSEngine engine;
    const QJSValue value = engine.evaluate(
            "(function() {\n"
            "    var obj = { 1: null, 2: null, 4096: null };\n"
            "    var iterated = [];\n"
            "    for (var t in obj) {\n"
            "        if (t == 2)\n"
            "            delete obj[t];\n"
            "        iterated.push(t);\n"
            "    }\n"
            "    return iterated;"
            "})()");
    QVERIFY(value.isArray());
    QCOMPARE(value.property("length").toInt(), 3);
    QCOMPARE(value.property("0").toInt(), 1);
    QCOMPARE(value.property("1").toInt(), 2);
    QCOMPARE(value.property("2").toInt(), 4096);
}

void tst_qqmlecmascript::saveAccumulatorBeforeToInt32()
{
    QJSEngine engine;

    // Infinite recursion produces a range error, but should not crash.
    // Also, any GC runs in between should not trash the temporary results of "a+a".
    const QJSValue value = engine.evaluate("function a(){a(a&a+a)}a()");
    QVERIFY(value.isError());
    QCOMPARE(value.toString(), QLatin1String("RangeError: Maximum call stack size exceeded."));
}

void tst_qqmlecmascript::intMinDividedByMinusOne()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(QByteArray("import QtQml 2.2\n"
                                 "QtObject {\n"
                                 "   property int intMin: -2147483648\n"
                                 "   property int minusOne: -1\n"
                                 "   property double doesNotFitInInt: intMin / minusOne\n"
                                 "}"), QUrl());
    QVERIFY(component.isReady());
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());
    QCOMPARE(object->property("doesNotFitInInt").toUInt(), 2147483648u);
}

void tst_qqmlecmascript::undefinedPropertiesInObjectWrapper()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFile("undefinedPropertiesInObjectWrapper.qml"));
    QVERIFY(component.isReady());
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());
}

void tst_qqmlecmascript::hugeRegexpQuantifiers()
{
    QJSEngine engine;
    QJSValue value = engine.evaluate("/({3072140529})?{3072140529}/");

    // It's a regular expression, but it won't match anything.
    // The RegExp compiler also shouldn't crash.
    QVERIFY(value.isRegExp());
}

struct CppSingleton1 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int testVar MEMBER testVar CONSTANT)
public:
    const int testVar = 0;
};

struct CppSingleton2 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int testVar MEMBER testVar CONSTANT)
public:
    const int testVar = 1;
};

void tst_qqmlecmascript::singletonTypeWrapperLookup()
{
    QQmlEngine engine;

    auto singletonTypeId1 = qmlRegisterSingletonType<CppSingleton1>("Test.Singletons", 1, 0, "CppSingleton1",
                                                                    [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new CppSingleton1;
    });

    auto singletonTypeId2 = qmlRegisterSingletonType<CppSingleton2>("Test.Singletons", 1, 0, "CppSingleton2",
                                                                    [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new CppSingleton2;
    });

    auto cleanup = qScopeGuard([&]() {
       qmlUnregisterType(singletonTypeId1);
       qmlUnregisterType(singletonTypeId2);
    });

    QQmlComponent component(&engine, testFileUrl("SingletonLookupTest.qml"));
    QScopedPointer<QObject> test(component.create());
    QVERIFY2(!test.isNull(), qPrintable(component.errorString()));

    auto singleton1 = engine.singletonInstance<CppSingleton1*>(singletonTypeId1);
    QVERIFY(singleton1);

    auto singleton2 = engine.singletonInstance<CppSingleton2*>(singletonTypeId2);
    QVERIFY(singleton2);

    QCOMPARE(test->property("firstLookup").toInt(), singleton1->testVar);
    QCOMPARE(test->property("secondLookup").toInt(), singleton2->testVar);
}

void tst_qqmlecmascript::getThisObject()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("getThis.qml"));
    QVERIFY(component.isReady());
    QScopedPointer<QObject> test(component.create());
    QVERIFY(!test.isNull());

    QTRY_COMPARE(qvariant_cast<QObject *>(test->property("self")), test.data());
}

// QTBUG-77954
void tst_qqmlecmascript::semicolonAfterProperty()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("semicolonAfterProperty.qml"));
    QVERIFY(component.isReady());
    QScopedPointer<QObject> test(component.create());
    QVERIFY(!test.isNull());
}

void tst_qqmlecmascript::gcCrashRegressionTest()
{
    const QString qmljs = QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qmljs";
    if (!QFile::exists(qmljs)) {
        QSKIP("Tets requires qmljs");
    }
    QProcess process;

    QTemporaryFile infile;
    QVERIFY(infile.open());
    infile.write(R"js(
        function i_want_to_break_free() {
            var n = 400;
            var m = 10;
            var regex = new RegExp("(ab)".repeat(n), "g"); // g flag to trigger the vulnerable path
            var part = "ab".repeat(n); // matches have to be at least size 2 to prevent interning
            var s = (part + "|").repeat(m);
            var cnt = 0;
            var ary = [];
            s.replace(regex, function() {
                for (var i = 1; i < arguments.length-2; ++i) {
                    if (typeof arguments[i] !== 'string') {
                        i_am_free = arguments[i];
                        throw "success";
                    }
                    ary[cnt++] = arguments[i];  // root everything to force GC
                }
                return "x";
            });
        }
        try { i_want_to_break_free(); } catch (e) {console.log("hi") }
        console.log(typeof(i_am_free));  // will print "object"
    )js");
    infile.close();

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("QV4_GC_MAX_STACK_SIZE", "32768");

    process.setProcessEnvironment(environment);
    process.start(qmljs, QStringList({infile.fileName()}));
    QVERIFY(process.waitForStarted());
    const qint64 pid = process.processId();
    QVERIFY(pid != 0);
    QVERIFY(process.waitForFinished());
    QCOMPARE(process.exitCode(), 0);
}

QTEST_MAIN(tst_qqmlecmascript)

#include "tst_qqmlecmascript.moc"
