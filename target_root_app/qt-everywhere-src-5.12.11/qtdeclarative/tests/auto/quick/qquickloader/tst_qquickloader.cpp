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

#include <QSignalSpy>

#include <QtQml/QQmlContext>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlincubator.h>
#include <QtQuick/qquickview.h>
#include <private/qquickloader_p.h>
#include <private/qquickwindowmodule_p.h>
#include "testhttpserver.h"
#include "../../shared/util.h"
#include "../shared/geometrytestutil.h"

Q_LOGGING_CATEGORY(lcTests, "qt.quick.tests")

class SlowComponent : public QQmlComponent
{
    Q_OBJECT
public:
    SlowComponent() {
        QTest::qSleep(500);
    }
};

class PeriodicIncubationController : public QObject,
    public QQmlIncubationController
{
public:
    PeriodicIncubationController() {}

    void start() { startTimer(20); }

    bool incubated = false;

protected:
    virtual void timerEvent(QTimerEvent *) {
        incubateFor(15);
    }

    virtual void incubatingObjectCountChanged(int count) {
        if (count)
            incubated = true;
    }
};

class tst_QQuickLoader : public QQmlDataTest

{
    Q_OBJECT
public:
    tst_QQuickLoader();

private slots:
    void sourceOrComponent();
    void sourceOrComponent_data();
    void clear();
    void urlToComponent();
    void componentToUrl();
    void anchoredLoader();
    void sizeLoaderToItem();
    void sizeItemToLoader();
    void noResize();
    void networkRequestUrl();
    void failNetworkRequest();
    void networkComponent();
    void active();
    void initialPropertyValues_data();
    void initialPropertyValues();
    void initialPropertyValuesBinding();
    void initialPropertyValuesError_data();
    void initialPropertyValuesError();

    void deleteComponentCrash();
    void nonItem();
    void vmeErrors();
    void creationContext();
    void QTBUG_16928();
    void implicitSize();
    void QTBUG_17114();
    void asynchronous_data();
    void asynchronous();
    void asynchronous_clear();
    void simultaneousSyncAsync();
    void asyncToSync1();
    void asyncToSync2();
    void loadedSignal();

    void parented();
    void sizeBound();
    void QTBUG_30183();
    void transientWindow();
    void nestedTransientWindow();

    void sourceComponentGarbageCollection();

    void bindings();
    void parentErrors();

    void rootContext();
    void sourceURLKeepComponent();

};

Q_DECLARE_METATYPE(QList<QQmlError>)

tst_QQuickLoader::tst_QQuickLoader()
{
    qmlRegisterType<SlowComponent>("LoaderTest", 1, 0, "SlowComponent");
    qRegisterMetaType<QList<QQmlError>>();
}

void tst_QQuickLoader::sourceOrComponent()
{
    QFETCH(QString, sourceOrComponent);
    QFETCH(QString, sourceDefinition);
    QFETCH(QUrl, sourceUrl);
    QFETCH(QString, errorString);

    bool error = !errorString.isEmpty();
    if (error)
        QTest::ignoreMessage(QtWarningMsg, errorString.toUtf8().constData());

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(QByteArray(
            "import QtQuick 2.0\n"
            "Loader {\n"
            "   property int onItemChangedCount: 0\n"
            "   property int onSourceChangedCount: 0\n"
            "   property int onSourceComponentChangedCount: 0\n"
            "   property int onStatusChangedCount: 0\n"
            "   property int onProgressChangedCount: 0\n"
            "   property int onLoadedCount: 0\n")
            + sourceDefinition.toUtf8()
            + QByteArray(
            "   onItemChanged: onItemChangedCount += 1\n"
            "   onSourceChanged: onSourceChangedCount += 1\n"
            "   onSourceComponentChanged: onSourceComponentChangedCount += 1\n"
            "   onStatusChanged: onStatusChangedCount += 1\n"
            "   onProgressChanged: onProgressChangedCount += 1\n"
            "   onLoaded: onLoadedCount += 1\n"
            "}")
        , dataDirectoryUrl());

    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QVERIFY(loader != nullptr);
    QCOMPARE(loader->item() == nullptr, error);
    QCOMPARE(loader->source(), sourceUrl);
    QCOMPARE(loader->progress(), 1.0);

    QCOMPARE(loader->status(), error ? QQuickLoader::Error : QQuickLoader::Ready);
    QCOMPARE(static_cast<QQuickItem*>(loader.data())->childItems().count(), error ? 0: 1);

    if (!error) {
        bool sourceComponentIsChildOfLoader = false;
        for (int ii = 0; ii < loader->children().size(); ++ii) {
            QQmlComponent *c = qobject_cast<QQmlComponent*>(loader->children().at(ii));
            if (c && c == loader->sourceComponent()) {
                sourceComponentIsChildOfLoader = true;
            }
        }
        QVERIFY(sourceComponentIsChildOfLoader);
    }

    if (sourceOrComponent == "component") {
        QCOMPARE(loader->property("onSourceComponentChangedCount").toInt(), 1);
        QCOMPARE(loader->property("onSourceChangedCount").toInt(), 0);
    } else {
        QCOMPARE(loader->property("onSourceComponentChangedCount").toInt(), 0);
        QCOMPARE(loader->property("onSourceChangedCount").toInt(), 1);
    }
    QCOMPARE(loader->property("onStatusChangedCount").toInt(), 1);
    QCOMPARE(loader->property("onProgressChangedCount").toInt(), 1);

    QCOMPARE(loader->property("onItemChangedCount").toInt(), 1);
    QCOMPARE(loader->property("onLoadedCount").toInt(), error ? 0 : 1);
}

void tst_QQuickLoader::sourceOrComponent_data()
{
    QTest::addColumn<QString>("sourceOrComponent");
    QTest::addColumn<QString>("sourceDefinition");
    QTest::addColumn<QUrl>("sourceUrl");
    QTest::addColumn<QString>("errorString");

    QTest::newRow("source") << "source" << "source: 'Rect120x60.qml'\n" << testFileUrl("Rect120x60.qml") << "";
    QTest::newRow("source with subdir") << "source" << "source: 'subdir/Test.qml'\n" << testFileUrl("subdir/Test.qml") << "";
    QTest::newRow("source with encoded subdir literal") << "source" << "source: 'subdir%2fTest.qml'\n" << testFileUrl("subdir/Test.qml") << "";
    QTest::newRow("source with encoded subdir optimized binding") << "source" << "source: 'subdir' + '%2fTest.qml'\n" << testFileUrl("subdir/Test.qml") << "";
    QTest::newRow("source with encoded subdir binding") << "source" << "source: encodeURIComponent('subdir/Test.qml')\n" << testFileUrl("subdir/Test.qml") << "";
    QTest::newRow("sourceComponent") << "component" << "Component { id: comp; Rectangle { width: 100; height: 50 } }\n sourceComponent: comp\n" << QUrl() << "";
    QTest::newRow("invalid source") << "source" << "source: 'IDontExist.qml'\n" << testFileUrl("IDontExist.qml")
            << QString(testFileUrl("IDontExist.qml").toString() + ": No such file or directory");
}

void tst_QQuickLoader::clear()
{
    QQmlEngine engine;

    {
        QQmlComponent component(&engine);
        component.setData(QByteArray(
                    "import QtQuick 2.0\n"
                    " Loader { id: loader\n"
                    "  source: 'Rect120x60.qml'\n"
                    "  Timer { interval: 200; running: true; onTriggered: loader.source = '' }\n"
                    " }")
                , dataDirectoryUrl());
        QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
        QVERIFY(loader != nullptr);
        QVERIFY(loader->item());
        QCOMPARE(loader->progress(), 1.0);
        QCOMPARE(static_cast<QQuickItem*>(loader.data())->childItems().count(), 1);

        QTRY_VERIFY(!loader->item());
        QCOMPARE(loader->progress(), 0.0);
        QCOMPARE(loader->status(), QQuickLoader::Null);
        QCOMPARE(static_cast<QQuickItem*>(loader.data())->childItems().count(), 0);
    }
    {
        QQmlComponent component(&engine, testFileUrl("/SetSourceComponent.qml"));
        QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
        QVERIFY(item);

        QQuickLoader *loader = qobject_cast<QQuickLoader*>(item->QQuickItem::childItems().at(0));
        QVERIFY(loader);
        QVERIFY(loader->item());
        QCOMPARE(loader->progress(), 1.0);
        QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 1);

        loader->setSourceComponent(nullptr);

        QVERIFY(!loader->item());
        QCOMPARE(loader->progress(), 0.0);
        QCOMPARE(loader->status(), QQuickLoader::Null);
        QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 0);
    }
    {
        QQmlComponent component(&engine, testFileUrl("/SetSourceComponent.qml"));
        QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
        QVERIFY(item);

        QQuickLoader *loader = qobject_cast<QQuickLoader*>(item->QQuickItem::childItems().at(0));
        QVERIFY(loader);
        QVERIFY(loader->item());
        QCOMPARE(loader->progress(), 1.0);
        QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 1);

        QMetaObject::invokeMethod(item.data(), "clear");

        QVERIFY(!loader->item());
        QCOMPARE(loader->progress(), 0.0);
        QCOMPARE(loader->status(), QQuickLoader::Null);
        QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 0);
    }
}

void tst_QQuickLoader::urlToComponent()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(QByteArray("import QtQuick 2.0\n"
                "Loader {\n"
                " id: loader\n"
                " Component { id: myComp; Rectangle { width: 10; height: 10 } }\n"
                " source: \"Rect120x60.qml\"\n"
                " Timer { interval: 100; running: true; onTriggered: loader.sourceComponent = myComp }\n"
                "}" )
            , dataDirectoryUrl());
    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QTest::qWait(200);
    QTRY_VERIFY(loader != nullptr);
    QVERIFY(loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(static_cast<QQuickItem*>(loader.data())->childItems().count(), 1);
    QCOMPARE(loader->width(), 10.0);
    QCOMPARE(loader->height(), 10.0);
}

void tst_QQuickLoader::componentToUrl()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("/SetSourceComponent.qml"));
    QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(item);

    QQuickLoader *loader = qobject_cast<QQuickLoader*>(item->QQuickItem::childItems().at(0));
    QVERIFY(loader);
    QVERIFY(loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 1);

    loader->setSource(testFileUrl("/Rect120x60.qml"));
    QVERIFY(loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 1);
    QCOMPARE(loader->width(), 120.0);
    QCOMPARE(loader->height(), 60.0);
}

void tst_QQuickLoader::anchoredLoader()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("/AnchoredLoader.qml"));
    QScopedPointer<QQuickItem> rootItem(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(rootItem != nullptr);
    QQuickItem *loader = rootItem->findChild<QQuickItem*>("loader");
    QQuickItem *sourceElement = rootItem->findChild<QQuickItem*>("sourceElement");

    QVERIFY(loader != nullptr);
    QVERIFY(sourceElement != nullptr);

    QCOMPARE(rootItem->width(), 300.0);
    QCOMPARE(rootItem->height(), 200.0);

    QCOMPARE(loader->width(), 300.0);
    QCOMPARE(loader->height(), 200.0);

    QCOMPARE(sourceElement->width(), 300.0);
    QCOMPARE(sourceElement->height(), 200.0);
}

void tst_QQuickLoader::sizeLoaderToItem()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("/SizeToItem.qml"));
    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QVERIFY(loader != nullptr);
    QCOMPARE(loader->width(), 120.0);
    QCOMPARE(loader->height(), 60.0);

    // Check resize
    QQuickItem *rect = qobject_cast<QQuickItem*>(loader->item());
    QVERIFY(rect);
    rect->setWidth(150);
    rect->setHeight(45);
    QCOMPARE(loader->width(), 150.0);
    QCOMPARE(loader->height(), 45.0);

    // Check explicit width
    loader->setWidth(200.0);
    QCOMPARE(loader->width(), 200.0);
    QCOMPARE(rect->width(), 200.0);
    rect->setWidth(100.0); // when rect changes ...
    QCOMPARE(rect->width(), 100.0); // ... it changes
    QCOMPARE(loader->width(), 200.0); // ... but loader stays the same

    // Check explicit height
    loader->setHeight(200.0);
    QCOMPARE(loader->height(), 200.0);
    QCOMPARE(rect->height(), 200.0);
    rect->setHeight(100.0); // when rect changes ...
    QCOMPARE(rect->height(), 100.0); // ... it changes
    QCOMPARE(loader->height(), 200.0); // ... but loader stays the same

    // Switch mode
    loader->setWidth(180);
    loader->setHeight(30);
    QCOMPARE(rect->width(), 180.0);
    QCOMPARE(rect->height(), 30.0);
}

void tst_QQuickLoader::sizeItemToLoader()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("/SizeToLoader.qml"));
    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QVERIFY(loader != nullptr);
    QCOMPARE(loader->width(), 200.0);
    QCOMPARE(loader->height(), 80.0);

    QQuickItem *rect = qobject_cast<QQuickItem*>(loader->item());
    QVERIFY(rect);
    QCOMPARE(rect->width(), 200.0);
    QCOMPARE(rect->height(), 80.0);

    // Check resize
    QSizeChangeListener sizeListener(rect);
    const QSizeF size(180, 30);
    loader->setSize(size);
    QVERIFY2(!sizeListener.isEmpty(), "There should be at least one signal about the size changed");
    for (const QSizeF sizeOnGeometryChanged : sizeListener) {
        // Check that we have the correct size on all signals
        QCOMPARE(sizeOnGeometryChanged, size);
    }
    QCOMPARE(rect->width(), size.width());
    QCOMPARE(rect->height(), size.height());

    // Switch mode
    loader->resetWidth(); // reset explicit size
    loader->resetHeight();
    rect->setWidth(160);
    rect->setHeight(45);
    QCOMPARE(loader->width(), 160.0);
    QCOMPARE(loader->height(), 45.0);
}

void tst_QQuickLoader::noResize()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("/NoResize.qml"));
    QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(item != nullptr);
    QCOMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 80.0);
}

void tst_QQuickLoader::networkRequestUrl()
{
    ThreadedTestHTTPServer server(dataDirectory());

    QQmlEngine engine;
    QQmlComponent component(&engine);
    const QString qml = "import QtQuick 2.0\nLoader { property int signalCount : 0; source: \"" + server.baseUrl().toString() + "/Rect120x60.qml\"; onLoaded: signalCount += 1 }";
    component.setData(qml.toUtf8(), testFileUrl("../dummy.qml"));
    if (component.isError())
        qDebug() << component.errors();
    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QVERIFY(loader != nullptr);

    QTRY_COMPARE(loader->status(), QQuickLoader::Ready);

    QVERIFY(loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(loader->property("signalCount").toInt(), 1);
    QCOMPARE(static_cast<QQuickItem*>(loader.data())->childItems().count(), 1);
}

/* XXX Component waits until all dependencies are loaded.  Is this actually possible? */
void tst_QQuickLoader::networkComponent()
{
    ThreadedTestHTTPServer server(dataDirectory(), TestHTTPServer::Delay);

    QQmlEngine engine;
    QQmlComponent component(&engine);
    const QString qml = "import QtQuick 2.0\n"
                        "import \"" + server.baseUrl().toString() + "/\" as NW\n"
                        "Item {\n"
                        " Component { id: comp; NW.Rect120x60 {} }\n"
                        " Loader { sourceComponent: comp } }";
    component.setData(qml.toUtf8(), dataDirectory());
    // The component may be loaded synchronously or asynchronously, so we cannot test for
    // status == Loading here. Also, it makes no sense to instruct the server to send here
    // because in the synchronous case we're already done loading.
    QTRY_COMPARE(component.status(), QQmlComponent::Ready);

    QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(item);

    QQuickLoader *loader = qobject_cast<QQuickLoader*>(item->children().at(1));
    QVERIFY(loader);
    QTRY_COMPARE(loader->status(), QQuickLoader::Ready);

    QVERIFY(loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(loader->status(), QQuickLoader::Ready);
    QCOMPARE(static_cast<QQuickItem*>(loader)->children().count(), 1);

}

void tst_QQuickLoader::failNetworkRequest()
{
    ThreadedTestHTTPServer server(dataDirectory());

    QTest::ignoreMessage(QtWarningMsg, QString(server.baseUrl().toString() + "/IDontExist.qml: File not found").toUtf8());

    QQmlEngine engine;
    QQmlComponent component(&engine);
    const QString qml = "import QtQuick 2.0\nLoader { property int did_load: 123; source: \"" + server.baseUrl().toString() + "/IDontExist.qml\"; onLoaded: did_load=456 }";
    component.setData(qml.toUtf8(), server.url("/dummy.qml"));
    QTRY_COMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QVERIFY(loader != nullptr);

    QTRY_COMPARE(loader->status(), QQuickLoader::Error);

    QVERIFY(!loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(loader->property("did_load").toInt(), 123);
    QCOMPARE(static_cast<QQuickItem*>(loader.data())->childItems().count(), 0);
}

void tst_QQuickLoader::active()
{
    QQmlEngine engine;

    // check that the item isn't instantiated until active is set to true
    {
        QQmlComponent component(&engine, testFileUrl("active.1.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY(object != nullptr);
        QQuickLoader *loader = object->findChild<QQuickLoader*>("loader");

        QVERIFY(loader->active() == false); // set manually to false
        QVERIFY(!loader->item());
        QMetaObject::invokeMethod(object.data(), "doSetSourceComponent");
        QVERIFY(!loader->item());
        QMetaObject::invokeMethod(object.data(), "doSetSource");
        QVERIFY(!loader->item());
        QMetaObject::invokeMethod(object.data(), "doSetActive");
        QVERIFY(loader->item() != nullptr);
    }

    // check that the status is Null if active is set to false
    {
        QQmlComponent component(&engine, testFileUrl("active.2.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY(object != nullptr);
        QQuickLoader *loader = object->findChild<QQuickLoader*>("loader");

        QVERIFY(loader->active() == true); // active is true by default
        QCOMPARE(loader->status(), QQuickLoader::Ready);
        int currStatusChangedCount = loader->property("statusChangedCount").toInt();
        QMetaObject::invokeMethod(object.data(), "doSetInactive");
        QCOMPARE(loader->status(), QQuickLoader::Null);
        QCOMPARE(loader->property("statusChangedCount").toInt(), (currStatusChangedCount+1));
    }

    // check that the source is not cleared if active is set to false
    {
        QQmlComponent component(&engine, testFileUrl("active.3.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY(object != nullptr);
        QQuickLoader *loader = object->findChild<QQuickLoader*>("loader");

        QVERIFY(loader->active() == true); // active is true by default
        QVERIFY(!loader->source().isEmpty());
        int currSourceChangedCount = loader->property("sourceChangedCount").toInt();
        QMetaObject::invokeMethod(object.data(), "doSetInactive");
        QVERIFY(!loader->source().isEmpty());
        QCOMPARE(loader->property("sourceChangedCount").toInt(), currSourceChangedCount);
    }

    // check that the sourceComponent is not cleared if active is set to false
    {
        QQmlComponent component(&engine, testFileUrl("active.4.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY(object != nullptr);
        QQuickLoader *loader = object->findChild<QQuickLoader*>("loader");

        QVERIFY(loader->active() == true); // active is true by default
        QVERIFY(loader->sourceComponent() != nullptr);
        int currSourceComponentChangedCount = loader->property("sourceComponentChangedCount").toInt();
        QMetaObject::invokeMethod(object.data(), "doSetInactive");
        QVERIFY(loader->sourceComponent() != nullptr);
        QCOMPARE(loader->property("sourceComponentChangedCount").toInt(), currSourceComponentChangedCount);
    }

    // check that the item is released if active is set to false
    {
        QQmlComponent component(&engine, testFileUrl("active.5.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY(object != nullptr);
        QQuickLoader *loader = object->findChild<QQuickLoader*>("loader");

        QVERIFY(loader->active() == true); // active is true by default
        QVERIFY(loader->item() != nullptr);
        int currItemChangedCount = loader->property("itemChangedCount").toInt();
        QMetaObject::invokeMethod(object.data(), "doSetInactive");
        QVERIFY(!loader->item());
        QCOMPARE(loader->property("itemChangedCount").toInt(), (currItemChangedCount+1));
    }

    // check that the activeChanged signal is emitted correctly
    {
        QQmlComponent component(&engine, testFileUrl("active.6.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY(object != nullptr);
        QQuickLoader *loader = object->findChild<QQuickLoader*>("loader");

        QVERIFY(loader->active() == true); // active is true by default
        loader->setActive(true);           // no effect
        QCOMPARE(loader->property("activeChangedCount").toInt(), 0);
        loader->setActive(false);          // change signal should be emitted
        QCOMPARE(loader->property("activeChangedCount").toInt(), 1);
        loader->setActive(false);          // no effect
        QCOMPARE(loader->property("activeChangedCount").toInt(), 1);
        loader->setActive(true);           // change signal should be emitted
        QCOMPARE(loader->property("activeChangedCount").toInt(), 2);
        loader->setActive(false);          // change signal should be emitted
        QCOMPARE(loader->property("activeChangedCount").toInt(), 3);
        QMetaObject::invokeMethod(object.data(), "doSetActive");
        QCOMPARE(loader->property("activeChangedCount").toInt(), 4);
        QMetaObject::invokeMethod(object.data(), "doSetActive");
        QCOMPARE(loader->property("activeChangedCount").toInt(), 4);
        QMetaObject::invokeMethod(object.data(), "doSetInactive");
        QCOMPARE(loader->property("activeChangedCount").toInt(), 5);
        loader->setActive(true);           // change signal should be emitted
        QCOMPARE(loader->property("activeChangedCount").toInt(), 6);
    }

    // check that the component isn't loaded until active is set to true
    {
        QQmlComponent component(&engine, testFileUrl("active.7.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("success").toBool(), true);
    }

    // check that the component is loaded if active is not set (true by default)
    {
        QQmlComponent component(&engine, testFileUrl("active.8.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY(object != nullptr);
        QCOMPARE(object->property("success").toBool(), true);
    }
}

void tst_QQuickLoader::initialPropertyValues_data()
{
    QTest::addColumn<QUrl>("qmlFile");
    QTest::addColumn<QStringList>("expectedWarnings");
    QTest::addColumn<QStringList>("propertyNames");
    QTest::addColumn<QVariantList>("propertyValues");

    QTest::newRow("source url with value set in onLoaded, initially active = true") << testFileUrl("initialPropertyValues.1.qml")
            << QStringList()
            << (QStringList() << "initialValue" << "behaviorCount")
            << (QVariantList() << 1 << 1);

    QTest::newRow("set source with initial property values specified, active = true") << testFileUrl("initialPropertyValues.2.qml")
            << QStringList()
            << (QStringList() << "initialValue" << "behaviorCount")
            << (QVariantList() << 2 << 0);

    QTest::newRow("set source with initial property values specified, active = false") << testFileUrl("initialPropertyValues.3.qml")
            << (QStringList() << QString(testFileUrl("initialPropertyValues.3.qml").toString() + QLatin1String(":16: TypeError: Cannot read property 'canary' of null")))
            << (QStringList())
            << (QVariantList());

    QTest::newRow("set source with initial property values specified, active = false, with active set true later") << testFileUrl("initialPropertyValues.4.qml")
            << QStringList()
            << (QStringList() << "initialValue" << "behaviorCount")
            << (QVariantList() << 4 << 0);

    QTest::newRow("set source without initial property values specified, active = true") << testFileUrl("initialPropertyValues.5.qml")
            << QStringList()
            << (QStringList() << "initialValue" << "behaviorCount")
            << (QVariantList() << 0 << 0);

    QTest::newRow("set source with initial property values specified with binding, active = true") << testFileUrl("initialPropertyValues.6.qml")
            << QStringList()
            << (QStringList() << "initialValue" << "behaviorCount")
            << (QVariantList() << 6 << 0);

    QTest::newRow("ensure initial property value semantics mimic createObject") << testFileUrl("initialPropertyValues.7.qml")
            << QStringList()
            << (QStringList() << "loaderValue" << "createObjectValue")
            << (QVariantList() << 1 << 1);

    QTest::newRow("ensure initial property values aren't disposed prior to component completion") << testFileUrl("initialPropertyValues.8.qml")
            << QStringList()
            << (QStringList() << "initialValue")
            << (QVariantList() << 6);
}

void tst_QQuickLoader::initialPropertyValues()
{
    QFETCH(QUrl, qmlFile);
    QFETCH(QStringList, expectedWarnings);
    QFETCH(QStringList, propertyNames);
    QFETCH(QVariantList, propertyValues);

    ThreadedTestHTTPServer server(dataDirectory());

    foreach (const QString &warning, expectedWarnings)
        QTest::ignoreMessage(QtWarningMsg, warning.toLatin1().constData());

    QQmlEngine engine;
    QQmlComponent component(&engine, qmlFile);
    QScopedPointer<QObject> object(component.beginCreate(engine.rootContext()));
    QVERIFY(object != nullptr);

    const int serverBaseUrlPropertyIndex = object->metaObject()->indexOfProperty("serverBaseUrl");
    if (serverBaseUrlPropertyIndex != -1) {
        QMetaProperty prop = object->metaObject()->property(serverBaseUrlPropertyIndex);
        QVERIFY(prop.write(object.data(), server.baseUrl().toString()));
    }

    component.completeCreate();
    if (expectedWarnings.isEmpty()) {
        QQuickLoader *loader = object->findChild<QQuickLoader*>("loader");
        QTRY_VERIFY(loader->item());
    }

    for (int i = 0; i < propertyNames.size(); ++i)
        QCOMPARE(object->property(propertyNames.at(i).toLatin1().constData()), propertyValues.at(i));
}

void tst_QQuickLoader::initialPropertyValuesBinding()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("initialPropertyValues.binding.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);

    QVERIFY(object->setProperty("bindable", QVariant(8)));
    QCOMPARE(object->property("canaryValue").toInt(), 8);
}

void tst_QQuickLoader::initialPropertyValuesError_data()
{
    QTest::addColumn<QUrl>("qmlFile");
    QTest::addColumn<QStringList>("expectedWarnings");

    QTest::newRow("invalid initial property values object") << testFileUrl("initialPropertyValues.error.1.qml")
            << (QStringList() << QString(testFileUrl("initialPropertyValues.error.1.qml").toString() + ":6:5: QML Loader: setSource: value is not an object"));

    QTest::newRow("nonexistent source url") << testFileUrl("initialPropertyValues.error.2.qml")
            << (QStringList() << QString(testFileUrl("NonexistentSourceComponent.qml").toString() + ": No such file or directory"));

    QTest::newRow("invalid source url") << testFileUrl("initialPropertyValues.error.3.qml")
            << (QStringList() << QString(testFileUrl("InvalidSourceComponent.qml").toString() + ":5:1: Expected token `:'"));

    QTest::newRow("invalid initial property values object with invalid property access") << testFileUrl("initialPropertyValues.error.4.qml")
            << (QStringList() << QString(testFileUrl("initialPropertyValues.error.4.qml").toString() + ":7:5: QML Loader: setSource: value is not an object")
                              << QString(testFileUrl("initialPropertyValues.error.4.qml").toString() + ":5: TypeError: Cannot read property 'canary' of null"));
}

void tst_QQuickLoader::initialPropertyValuesError()
{
    QFETCH(QUrl, qmlFile);
    QFETCH(QStringList, expectedWarnings);

    foreach (const QString &warning, expectedWarnings)
        QTest::ignoreMessage(QtWarningMsg, warning.toUtf8().constData());

    QQmlEngine engine;
    QQmlComponent component(&engine, qmlFile);
    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != nullptr);
    QQuickLoader *loader = object->findChild<QQuickLoader*>("loader");
    QVERIFY(loader != nullptr);
    QVERIFY(!loader->item());
}

// QTBUG-9241
void tst_QQuickLoader::deleteComponentCrash()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("crash.qml"));
    QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(item);

    item->metaObject()->invokeMethod(item.data(), "setLoaderSource");

    QQuickLoader *loader = qobject_cast<QQuickLoader*>(item->QQuickItem::childItems().at(0));
    QVERIFY(loader);
    QVERIFY(loader->item());
    QCOMPARE(loader->item()->objectName(), QLatin1String("blue"));
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(loader->status(), QQuickLoader::Ready);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    QTRY_COMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 1);
    QCOMPARE(loader->source(), testFileUrl("BlueRect.qml"));
}

void tst_QQuickLoader::nonItem()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("nonItem.qml"));

    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QVERIFY(loader);
    QVERIFY(loader->item());

    QCOMPARE(loader.data(), loader->item()->parent());

    QPointer<QObject> item = loader->item();
    loader->setActive(false);
    QVERIFY(!loader->item());
    QTRY_VERIFY(!item);
}

void tst_QQuickLoader::vmeErrors()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("vmeErrors.qml"));
    QString err = testFileUrl("VmeError.qml").toString() + ":6:26: Cannot assign object type QObject with no default method";
    QTest::ignoreMessage(QtWarningMsg, err.toLatin1().constData());
    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QVERIFY(loader);
    QVERIFY(!loader->item());
}

// QTBUG-13481
void tst_QQuickLoader::creationContext()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("creationContext.qml"));

    QScopedPointer<QObject> o(component.create());
    QVERIFY(o != nullptr);

    QCOMPARE(o->property("test").toBool(), true);
}

void tst_QQuickLoader::QTBUG_16928()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("QTBUG_16928.qml"));
    QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(item);

    QCOMPARE(item->width(), 250.);
    QCOMPARE(item->height(), 250.);
}

void tst_QQuickLoader::implicitSize()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("implicitSize.qml"));
    QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(item);

    QCOMPARE(item->width(), 150.);
    QCOMPARE(item->height(), 150.);

    QCOMPARE(item->property("implHeight").toReal(), 100.);
    QCOMPARE(item->property("implWidth").toReal(), 100.);

    QQuickLoader *loader = item->findChild<QQuickLoader*>("loader");
    QSignalSpy implWidthSpy(loader, SIGNAL(implicitWidthChanged()));
    QSignalSpy implHeightSpy(loader, SIGNAL(implicitHeightChanged()));

    QMetaObject::invokeMethod(item.data(), "changeImplicitSize");

    QCOMPARE(loader->property("implicitWidth").toReal(), 200.);
    QCOMPARE(loader->property("implicitHeight").toReal(), 300.);

    QCOMPARE(implWidthSpy.count(), 1);
    QCOMPARE(implHeightSpy.count(), 1);
}

void tst_QQuickLoader::QTBUG_17114()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("QTBUG_17114.qml"));
    QScopedPointer<QQuickItem> item(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(item);

    QCOMPARE(item->property("loaderWidth").toReal(), 32.);
    QCOMPARE(item->property("loaderHeight").toReal(), 32.);
}

void tst_QQuickLoader::asynchronous_data()
{
    QTest::addColumn<QUrl>("qmlFile");
    QTest::addColumn<QStringList>("expectedWarnings");

    QTest::newRow("Valid component") << testFileUrl("BigComponent.qml")
            << QStringList();

    QTest::newRow("Non-existent component") << testFileUrl("IDoNotExist.qml")
            << (QStringList() << QString(testFileUrl("IDoNotExist.qml").toString() + ": No such file or directory"));

    QTest::newRow("Invalid component") << testFileUrl("InvalidSourceComponent.qml")
            << (QStringList() << QString(testFileUrl("InvalidSourceComponent.qml").toString() + ":5:1: Expected token `:'"));
}

void tst_QQuickLoader::asynchronous()
{
    QFETCH(QUrl, qmlFile);
    QFETCH(QStringList, expectedWarnings);

    QQmlEngine engine;
    PeriodicIncubationController *controller = new PeriodicIncubationController;
    QQmlIncubationController *previous = engine.incubationController();
    engine.setIncubationController(controller);
    delete previous;

    QQmlComponent component(&engine, testFileUrl("asynchronous.qml"));
    QScopedPointer<QQuickItem> root(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(root);

    QQuickLoader *loader = root->findChild<QQuickLoader*>("loader");
    QVERIFY(loader);

    foreach (const QString &warning, expectedWarnings)
        QTest::ignoreMessage(QtWarningMsg, warning.toUtf8().constData());

    QVERIFY(!loader->item());
    QCOMPARE(loader->progress(), 0.0);
    root->setProperty("comp", qmlFile.toString());
    QMetaObject::invokeMethod(root.data(), "loadComponent");
    QVERIFY(!loader->item());

    if (expectedWarnings.isEmpty()) {
        QCOMPARE(loader->status(), QQuickLoader::Loading);

        controller->start();
        QVERIFY(!controller->incubated); // asynchronous compilation means not immediately compiled/incubating.
        QTRY_VERIFY(controller->incubated); // but should start incubating once compilation is complete.
        QTRY_VERIFY(loader->item());
        QCOMPARE(loader->progress(), 1.0);
        QCOMPARE(loader->status(), QQuickLoader::Ready);
    } else {
        QTRY_COMPARE(loader->progress(), 1.0);
        QTRY_COMPARE(loader->status(), QQuickLoader::Error);
    }
}

void tst_QQuickLoader::asynchronous_clear()
{
    QQmlEngine engine;
    PeriodicIncubationController *controller = new PeriodicIncubationController;
    QQmlIncubationController *previous = engine.incubationController();
    engine.setIncubationController(controller);
    delete previous;

    QQmlComponent component(&engine, testFileUrl("asynchronous.qml"));
    QScopedPointer<QQuickItem> root(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(root);

    QQuickLoader *loader = root->findChild<QQuickLoader*>("loader");
    QVERIFY(loader);

    QVERIFY(!loader->item());
    root->setProperty("comp", "BigComponent.qml");
    QMetaObject::invokeMethod(root.data(), "loadComponent");
    QVERIFY(!loader->item());

    controller->start();
    QCOMPARE(loader->status(), QQuickLoader::Loading);
    QTRY_COMPARE(engine.incubationController()->incubatingObjectCount(), 1);

    // clear before component created
    root->setProperty("comp", "");
    QMetaObject::invokeMethod(root.data(), "loadComponent");
    QVERIFY(!loader->item());
    QCOMPARE(engine.incubationController()->incubatingObjectCount(), 0);

    QCOMPARE(loader->progress(), 0.0);
    QCOMPARE(loader->status(), QQuickLoader::Null);
    QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 0);

    // check loading component
    root->setProperty("comp", "BigComponent.qml");
    QMetaObject::invokeMethod(root.data(), "loadComponent");
    QVERIFY(!loader->item());

    QCOMPARE(loader->status(), QQuickLoader::Loading);
    QCOMPARE(engine.incubationController()->incubatingObjectCount(), 1);

    QTRY_VERIFY(loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(loader->status(), QQuickLoader::Ready);
    QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 1);
}

void tst_QQuickLoader::simultaneousSyncAsync()
{
    QQmlEngine engine;
    PeriodicIncubationController *controller = new PeriodicIncubationController;
    QQmlIncubationController *previous = engine.incubationController();
    engine.setIncubationController(controller);
    delete previous;

    QQmlComponent component(&engine, testFileUrl("simultaneous.qml"));
    QScopedPointer<QQuickItem> root(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(root);

    QQuickLoader *asyncLoader = root->findChild<QQuickLoader*>("asyncLoader");
    QQuickLoader *syncLoader = root->findChild<QQuickLoader*>("syncLoader");
    QVERIFY(asyncLoader);
    QVERIFY(syncLoader);

    QVERIFY(!asyncLoader->item());
    QVERIFY(!syncLoader->item());
    QMetaObject::invokeMethod(root.data(), "loadComponents");
    QVERIFY(!asyncLoader->item());
    QVERIFY(syncLoader->item());

    controller->start();
    QCOMPARE(asyncLoader->status(), QQuickLoader::Loading);
    QVERIFY(!controller->incubated); // asynchronous compilation means not immediately compiled/incubating.
    QTRY_VERIFY(controller->incubated); // but should start incubating once compilation is complete.
    QTRY_VERIFY(asyncLoader->item());
    QCOMPARE(asyncLoader->progress(), 1.0);
    QCOMPARE(asyncLoader->status(), QQuickLoader::Ready);
}

void tst_QQuickLoader::asyncToSync1()
{
    QQmlEngine engine;
    PeriodicIncubationController *controller = new PeriodicIncubationController;
    QQmlIncubationController *previous = engine.incubationController();
    engine.setIncubationController(controller);
    delete previous;

    QQmlComponent component(&engine, testFileUrl("asynchronous.qml"));
    QScopedPointer<QQuickItem> root(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(root);

    QQuickLoader *loader = root->findChild<QQuickLoader*>("loader");
    QVERIFY(loader);

    QVERIFY(!loader->item());
    root->setProperty("comp", "BigComponent.qml");
    QMetaObject::invokeMethod(root.data(), "loadComponent");
    QVERIFY(!loader->item());

    controller->start();
    QCOMPARE(loader->status(), QQuickLoader::Loading);
    QCOMPARE(engine.incubationController()->incubatingObjectCount(), 0);

    // force completion before component created
    loader->setAsynchronous(false);
    QVERIFY(loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(loader->status(), QQuickLoader::Ready);
    QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 1);
}

void tst_QQuickLoader::asyncToSync2()
{
    QQmlEngine engine;
    PeriodicIncubationController *controller = new PeriodicIncubationController;
    QQmlIncubationController *previous = engine.incubationController();
    engine.setIncubationController(controller);
    delete previous;

    QQmlComponent component(&engine, testFileUrl("asynchronous.qml"));
    QScopedPointer<QQuickItem> root(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(root);

    QQuickLoader *loader = root->findChild<QQuickLoader*>("loader");
    QVERIFY(loader);

    QVERIFY(!loader->item());
    root->setProperty("comp", "BigComponent.qml");
    QMetaObject::invokeMethod(root.data(), "loadComponent");
    QVERIFY(!loader->item());

    controller->start();
    QCOMPARE(loader->status(), QQuickLoader::Loading);
    QTRY_COMPARE(engine.incubationController()->incubatingObjectCount(), 1);

    // force completion after component created but before incubation complete
    loader->setAsynchronous(false);
    QVERIFY(loader->item());
    QCOMPARE(loader->progress(), 1.0);
    QCOMPARE(loader->status(), QQuickLoader::Ready);
    QCOMPARE(static_cast<QQuickItem*>(loader)->childItems().count(), 1);
}

void tst_QQuickLoader::loadedSignal()
{
    QQmlEngine engine;
    PeriodicIncubationController *controller = new PeriodicIncubationController;
    QQmlIncubationController *previous = engine.incubationController();
    engine.setIncubationController(controller);
    delete previous;

    {
        // ensure that triggering loading (by setting active = true)
        // and then immediately setting active to false, causes the
        // loader to be deactivated, including disabling the incubator.
        QQmlComponent component(&engine, testFileUrl("loadedSignal.qml"));
        QScopedPointer<QObject> obj(component.create());

        QMetaObject::invokeMethod(obj.data(), "triggerLoading");
        QTest::qWait(100); // ensure that loading would have finished if it wasn't deactivated
        QCOMPARE(obj->property("loadCount").toInt(), 0);
        QVERIFY(obj->property("success").toBool());

        QMetaObject::invokeMethod(obj.data(), "triggerLoading");
        QTest::qWait(100);
        QCOMPARE(obj->property("loadCount").toInt(), 0);
        QVERIFY(obj->property("success").toBool());

        QMetaObject::invokeMethod(obj.data(), "triggerMultipleLoad");
        controller->start();
        QTest::qWait(100);
        QTRY_COMPARE(obj->property("loadCount").toInt(), 1); // only one loaded signal should be emitted.
        QVERIFY(obj->property("success").toBool());
    }

    {
        // ensure that an error doesn't result in the onLoaded signal being emitted.
        QQmlComponent component(&engine, testFileUrl("loadedSignal.2.qml"));
        QScopedPointer<QObject> obj(component.create());

        QMetaObject::invokeMethod(obj.data(), "triggerLoading");
        QTest::qWait(100);
        QCOMPARE(obj->property("loadCount").toInt(), 0);
        QVERIFY(obj->property("success").toBool());
    }
}

void tst_QQuickLoader::parented()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("parented.qml"));
    QScopedPointer<QQuickItem> root(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(root);

    QQuickItem *item = root->findChild<QQuickItem*>("comp");
    QVERIFY(item);

    QCOMPARE(item->parentItem(), root.data());

    QCOMPARE(item->width(), 300.);
    QCOMPARE(item->height(), 300.);
}

void tst_QQuickLoader::sizeBound()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("sizebound.qml"));
    QScopedPointer<QQuickItem> root(qobject_cast<QQuickItem*>(component.create()));
    QVERIFY(root);
    QQuickLoader *loader = root->findChild<QQuickLoader*>("loader");
    QVERIFY(loader != nullptr);

    QVERIFY(loader->item());

    QCOMPARE(loader->width(), 50.0);
    QCOMPARE(loader->height(), 60.0);

    QMetaObject::invokeMethod(root.data(), "switchComponent");

    QCOMPARE(loader->width(), 80.0);
    QCOMPARE(loader->height(), 90.0);
}

void tst_QQuickLoader::QTBUG_30183()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("/QTBUG_30183.qml"));
    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    QVERIFY(loader != nullptr);
    QCOMPARE(loader->width(), 240.0);
    QCOMPARE(loader->height(), 120.0);

    // the loaded item must follow the size
    QQuickItem *rect = qobject_cast<QQuickItem*>(loader->item());
    QVERIFY(rect);
    QCOMPARE(rect->width(), 240.0);
    QCOMPARE(rect->height(), 120.0);
}

void tst_QQuickLoader::transientWindow() // QTBUG-52944
{
    QQuickView view;
    view.setSource(testFileUrl("itemLoaderWindow.qml"));
    QQuickItem *root = qobject_cast<QQuickItem*>(view.rootObject());
    QVERIFY(root);
    QQuickLoader *loader = root->findChild<QQuickLoader *>();
    QVERIFY(loader);
    QTRY_COMPARE(loader->status(), QQuickLoader::Ready);
    QQuickWindowQmlImpl *loadedWindow = qobject_cast<QQuickWindowQmlImpl *>(loader->item());
    QVERIFY(loadedWindow);
    QCOMPARE(loadedWindow->visibility(), QWindow::Hidden);

    QElapsedTimer timer;
    qint64 viewVisibleTime = -1;
    qint64 loadedWindowVisibleTime = -1;
    connect(&view, &QWindow::visibleChanged,
            [&viewVisibleTime, &timer]() { viewVisibleTime = timer.elapsed(); } );
    connect(loadedWindow, &QQuickWindowQmlImpl::visibilityChanged,
            [&loadedWindowVisibleTime, &timer]() { loadedWindowVisibleTime = timer.elapsed(); } );
    timer.start();
    view.show();

    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QTRY_VERIFY(loadedWindowVisibleTime >= 0);
    QVERIFY(viewVisibleTime >= 0);

    // now that we're sure they are both visible, which one became visible first?
    qCDebug(lcTests) << "transient Window became visible" << (loadedWindowVisibleTime - viewVisibleTime) << "ms after the root Item";
    QVERIFY((loadedWindowVisibleTime - viewVisibleTime) >= 0);

    QWindowList windows = QGuiApplication::topLevelWindows();
    QTRY_COMPARE(windows.size(), 2);

    // TODO Ideally we would now close the outer window and make sure the transient window closes too.
    // It works during manual testing because of QWindowPrivate::maybeQuitOnLastWindowClosed()
    // but quitting an autotest doesn't make sense.
}

void tst_QQuickLoader::nestedTransientWindow() // QTBUG-52944
{
    QQuickView view;
    view.setSource(testFileUrl("itemLoaderItemWindow.qml"));
    QQuickItem *root = qobject_cast<QQuickItem*>(view.rootObject());
    QVERIFY(root);
    QQuickLoader *loader = root->findChild<QQuickLoader *>();
    QVERIFY(loader);
    QTRY_COMPARE(loader->status(), QQuickLoader::Ready);
    QQuickItem *loadedItem = qobject_cast<QQuickItem *>(loader->item());
    QVERIFY(loadedItem);
    QQuickWindowQmlImpl *loadedWindow = loadedItem->findChild<QQuickWindowQmlImpl *>();
    QVERIFY(loadedWindow);
    QCOMPARE(loadedWindow->visibility(), QWindow::Hidden);

    QElapsedTimer timer;
    qint64 viewVisibleTime = -1;
    qint64 loadedWindowVisibleTime = -1;
    connect(&view, &QWindow::visibleChanged,
            [&viewVisibleTime, &timer]() { viewVisibleTime = timer.elapsed(); } );
    connect(loadedWindow, &QQuickWindowQmlImpl::visibilityChanged,
            [&loadedWindowVisibleTime, &timer]() { loadedWindowVisibleTime = timer.elapsed(); } );
    timer.start();
    view.show();

    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QTRY_VERIFY(loadedWindowVisibleTime >= 0);
    QVERIFY(viewVisibleTime >= 0);

    // now that we're sure they are both visible, which one became visible first?
    qCDebug(lcTests) << "transient Window became visible" << (loadedWindowVisibleTime - viewVisibleTime) << "ms after the root Item";
    QVERIFY((loadedWindowVisibleTime - viewVisibleTime) >= 0);

    QWindowList windows = QGuiApplication::topLevelWindows();
    QTRY_COMPARE(windows.size(), 2);

    // TODO Ideally we would now close the outer window and make sure the transient window closes too.
    // It works during manual testing because of QWindowPrivate::maybeQuitOnLastWindowClosed()
    // but quitting an autotest doesn't make sense.
}

void tst_QQuickLoader::sourceComponentGarbageCollection()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("sourceComponentGarbageCollection.qml"));
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());

    QMetaObject::invokeMethod(obj.data(), "setSourceComponent");
    engine.collectGarbage();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QSignalSpy spy(obj.data(), SIGNAL(loaded()));

    obj->setProperty("active", true);

    if (spy.isEmpty())
        QVERIFY(spy.wait());

    QCOMPARE(spy.count(), 1);
}

// QTBUG-51995
void tst_QQuickLoader::bindings()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("bindings.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QQuickItem *game = object->property("game").value<QQuickItem*>();
    QVERIFY(game);

    QQuickLoader *loader = object->property("loader").value<QQuickLoader*>();
    QVERIFY(loader);

    QSignalSpy warningsSpy(&engine, SIGNAL(warnings(QList<QQmlError>)));

    // Causes the Loader to become active
    game->setState(QLatin1String("running"));
    QTRY_VERIFY(loader->item());

    // Causes the Loader to become inactive - should not cause binding errors
    game->setState(QLatin1String("invalid"));
    QTRY_VERIFY(!loader->item());

    QString failureMessage;
    if (!warningsSpy.isEmpty()) {
        QDebug stream(&failureMessage);
        stream << warningsSpy.first().first().value<QList<QQmlError>>();
    }
    QVERIFY2(warningsSpy.isEmpty(), qPrintable(failureMessage));
}

// QTBUG-47321
void tst_QQuickLoader::parentErrors()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("parentErrors.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QQuickLoader *loader = object->property("loader").value<QQuickLoader*>();
    QVERIFY(loader);

    QSignalSpy warningsSpy(&engine, SIGNAL(warnings(QList<QQmlError>)));

    // Give the loader a component
    loader->setSourceComponent(object->property("component").value<QQmlComponent*>());
    QTRY_VERIFY(loader->item());

    // Clear the loader's component; should not cause binding errors
    loader->setSourceComponent(nullptr);
    QTRY_VERIFY(!loader->item());

    QString failureMessage;
    if (!warningsSpy.isEmpty()) {
        QDebug stream(&failureMessage);
        stream << warningsSpy.first().first().value<QList<QQmlError>>();
    }
    QVERIFY2(warningsSpy.isEmpty(), qPrintable(failureMessage));
}

class ObjectInRootContext: public QObject
{
    Q_OBJECT

public:
    int didIt = 0;

public slots:
    void doIt() {
        didIt += 1;
    }
};

void tst_QQuickLoader::rootContext()
{
    QQmlEngine engine;
    ObjectInRootContext objectInRootContext;
    engine.rootContext()->setContextProperty("objectInRootContext", &objectInRootContext);

    QQmlComponent component(&engine, testFileUrl("rootContext.qml"));
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QQuickLoader *loader = object->property("loader").value<QQuickLoader*>();
    QVERIFY(loader);

    QSignalSpy warningsSpy(&engine, SIGNAL(warnings(QList<QQmlError>)));

    // Give the loader a component
    loader->setSourceComponent(object->property("component").value<QQmlComponent*>());
    QTRY_VERIFY(loader->active());
    QTRY_VERIFY(loader->item());

    QString failureMessage;
    if (!warningsSpy.isEmpty()) {
        QDebug stream(&failureMessage);
        stream << warningsSpy.first().first().value<QList<QQmlError>>();
    }
    QVERIFY2(warningsSpy.isEmpty(), qPrintable(failureMessage));
    QCOMPARE(objectInRootContext.didIt, 0);

    // Deactivate the loader, which deletes the item.
    // Check that a) there are no errors, and b) the objectInRootContext can still be resolved even
    // after deactivating the loader. If it cannot, a ReferenceError for objectInRootContext is
    // generated (and the 'doIt' counter in objectInRootContext will be 1 for the call before
    // the deactivation).
    loader->item()->setProperty("trigger", true);
    QTRY_VERIFY(!loader->active());
    QTRY_VERIFY(!loader->item());

    if (!warningsSpy.isEmpty()) {
        QDebug stream(&failureMessage);
        stream << warningsSpy.first().first().value<QList<QQmlError>>();
    }
    QVERIFY2(warningsSpy.isEmpty(), qPrintable(failureMessage));
    QCOMPARE(objectInRootContext.didIt, 2);
}

void tst_QQuickLoader::sourceURLKeepComponent()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(QByteArray(
                      "import QtQuick 2.0\n"
                      " Loader { id: loader\n }"),
                      dataDirectoryUrl());

    QScopedPointer<QQuickLoader> loader(qobject_cast<QQuickLoader*>(component.create()));
    loader->setSource(testFileUrl("/Rect120x60.qml"));

    QVERIFY(loader);
    QVERIFY(loader->item());
    QVERIFY(loader->sourceComponent());
    QCOMPARE(loader->progress(), 1.0);

    const QPointer<QQmlComponent> sourceComponent =  loader->sourceComponent();

    //Ensure toggling active status does not recreate component
    loader->setActive(false);
    QVERIFY(!loader->item());
    QVERIFY(loader->sourceComponent());
    QCOMPARE(sourceComponent.data(), loader->sourceComponent());

    loader->setActive(true);
    QVERIFY(loader->item());
    QVERIFY(loader->sourceComponent());
    QCOMPARE(sourceComponent.data(), loader->sourceComponent());

    loader->setActive(false);
    QVERIFY(!loader->item());
    QVERIFY(loader->sourceComponent());
    QCOMPARE(sourceComponent.data(), loader->sourceComponent());

    //Ensure changing source url causes component to be recreated when inactive
    loader->setSource(testFileUrl("/BlueRect.qml"));

    loader->setActive(true);
    QVERIFY(loader->item());
    QVERIFY(loader->sourceComponent());

    const QPointer<QQmlComponent> newSourceComponent =  loader->sourceComponent();
    QVERIFY(sourceComponent.data() != newSourceComponent.data());

    //Ensure changing source url causes component to be recreated when active
    loader->setSource(testFileUrl("/Rect120x60.qml"));
    QVERIFY(loader->sourceComponent() != newSourceComponent.data());

}

QTEST_MAIN(tst_QQuickLoader)

#include "tst_qquickloader.moc"
