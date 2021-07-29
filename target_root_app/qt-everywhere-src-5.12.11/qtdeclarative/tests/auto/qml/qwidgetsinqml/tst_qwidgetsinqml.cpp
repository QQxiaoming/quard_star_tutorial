/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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
#include <QQmlEngine>
#include <QtQml>
#include <QWidget>

class tst_QWidgetsInQml : public QObject
{
    Q_OBJECT
public:
    tst_QWidgetsInQml();

private slots:
    void instantiateWidget();
    void instantiateWidgetWithoutParentWidget();
    void widgetAsDefaultPropertyCollected();
    void widgetAsDefaultPropertyKept();
    void widgetAsDefaultPropertyKeptDuringCreation();
};

static void gc(QQmlEngine &engine)
{
    engine.collectGarbage();
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

// Like QtObject, but with default property
class QObjectContainer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("DefaultProperty", "data");
    Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false);
public:
    QObjectContainer()
        : widgetParent(0)
        , gcOnAppend(false)
    {}

    QQmlListProperty<QObject> data() {
        return QQmlListProperty<QObject>(this, 0, children_append, children_count, children_at, children_clear);
    }

    static void children_append(QQmlListProperty<QObject> *prop, QObject *o)
    {
        QObjectContainer *that = static_cast<QObjectContainer*>(prop->object);
        that->dataChildren.append(o);
        QObject::connect(o, SIGNAL(destroyed(QObject*)), prop->object, SLOT(childDestroyed(QObject*)));
        QWidget *widget = qobject_cast<QWidget*>(o);
        if (widget && that->widgetParent)
            widget->setParent(that->widgetParent);

        if (that->gcOnAppend) {
            QQmlEngine *engine = qmlEngine(that);
            gc(*engine);
        }
    }

    static int children_count(QQmlListProperty<QObject> *prop)
    {
        return static_cast<QObjectContainer*>(prop->object)->dataChildren.count();
    }

    static QObject *children_at(QQmlListProperty<QObject> *prop, int index)
    {
        return static_cast<QObjectContainer*>(prop->object)->dataChildren.at(index);
    }

    static void children_clear(QQmlListProperty<QObject> *prop)
    {
        QObjectContainer *that = static_cast<QObjectContainer*>(prop->object);
        foreach (QObject *c, that->dataChildren)
            QObject::disconnect(c, SIGNAL(destroyed(QObject*)), that, SLOT(childDestroyed(QObject*)));
        that->dataChildren.clear();
    }

    QList<QObject*> dataChildren;
    QWidget *widgetParent;
    bool gcOnAppend;

protected slots:
    void childDestroyed(QObject *child) {
        dataChildren.removeAll(child);
    }
};

class QWidgetContainer : public QObjectContainer
{
    Q_OBJECT
public:
    QWidgetContainer()
    {
        widgetParent = new QWidget;
        QQmlEngine::setObjectOwnership(widgetParent, QQmlEngine::CppOwnership);
    }
    virtual ~QWidgetContainer()
    {
        delete widgetParent;
        widgetParent = 0;
    }
};

class QObjectContainerWithGCOnAppend : public QObjectContainer
{
    Q_OBJECT
public:
    QObjectContainerWithGCOnAppend()
    {
        gcOnAppend = true;
    }
};

tst_QWidgetsInQml::tst_QWidgetsInQml()
{
    qmlRegisterType<QWidget>("Qt.Widgets", 1, 0, "QWidget");
    qmlRegisterType<QObjectContainer>("Qt.Widgets", 1, 0, "QObjectContainer");
    qmlRegisterType<QWidgetContainer>("Qt.Widgets", 1, 0, "QWidgetContainer");
    qmlRegisterType<QObjectContainerWithGCOnAppend>("Qt.Widgets", 1, 0, "QObjectContainerWithGCOnAppend");
}

void tst_QWidgetsInQml::instantiateWidget()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Qt.Widgets 1.0;\nQWidget { property QWidget child: QWidget { objectName: 'child' } }", QUrl());
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());
    QWidget *rootWidget = qobject_cast<QWidget*>(object.data());
    QVERIFY(rootWidget != 0);
    QCOMPARE(rootWidget->children().count(), 1);
    QWidget *firstChildWidget = qobject_cast<QWidget*>(rootWidget->children().first());
    QVERIFY(firstChildWidget != 0);

    QWidget *widgetProperty = qvariant_cast<QWidget*>(object->property("child"));
    QVERIFY(widgetProperty != 0);
    QCOMPARE(firstChildWidget, widgetProperty);
    QCOMPARE(firstChildWidget->objectName(), QString("child"));
}

void tst_QWidgetsInQml::instantiateWidgetWithoutParentWidget()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Qt.Widgets 1.0;\n"
                      "import QtQml 2.0;\n"
                      "QtObject { property QtObject child: QWidget { objectName: 'child' } }", QUrl());
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QPointer<QWidget> widgetProperty = qvariant_cast<QWidget*>(object->property("child"));
    QVERIFY(!widgetProperty.isNull());
    QCOMPARE(widgetProperty->objectName(), QString("child"));

    QVERIFY(!widgetProperty->parent());
    gc(engine);
    // Don't collect, the property reference should keep it alive
    QVERIFY(!widgetProperty.isNull());
}

void tst_QWidgetsInQml::widgetAsDefaultPropertyCollected()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Qt.Widgets 1.0;\n"
                      "import QtQml 2.0;\n"
                      "QObjectContainer {\n"
                      "    QWidget {\n"
                      "        id: parentLessChild;\n"
                      "        objectName: 'child'\n"
                      "    }\n"
                      "    property var widgetHolder;\n"
                      "    Component.onCompleted: {\n"
                      "        widgetHolder = parentLessChild;\n"
                      "    }\n"
                      "}", QUrl());
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QObjectContainer *container = qobject_cast<QObjectContainer*>(object.data());
    QCOMPARE(container->dataChildren.count(), 1);

    QJSValue holder = qvariant_cast<QJSValue>(object->property("widgetHolder"));
    QVERIFY(!holder.isNull());
    gc(engine);
    QCOMPARE(container->dataChildren.count(), 1);

    holder = QJSValue();
    object->setProperty("widgetHolder", QVariant::fromValue(holder));

    gc(engine);
    // The QWidget is without a parent and nobody is referencing it anymore (the children
    // list in QObjectContainer is weak!), so it should get collected.
    QCOMPARE(container->dataChildren.count(), 0);
}

void tst_QWidgetsInQml::widgetAsDefaultPropertyKept()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Qt.Widgets 1.0;\n"
                      "import QtQml 2.0;\n"
                      "QWidgetContainer {\n"
                      "    QWidget {\n"
                      "        id: parentLessChild;\n"
                      "        objectName: 'child'\n"
                      "    }\n"
                      "    property var widgetHolder;\n"
                      "    Component.onCompleted: {\n"
                      "        widgetHolder = parentLessChild;\n"
                      "    }\n"
                      "}", QUrl());
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QWidgetContainer *container = qobject_cast<QWidgetContainer*>(object.data());
    QCOMPARE(container->dataChildren.count(), 1);

    QJSValue holder = qvariant_cast<QJSValue>(object->property("widgetHolder"));
    QVERIFY(!holder.isNull());
    gc(engine);
    QCOMPARE(container->dataChildren.count(), 1);

    holder = QJSValue();
    object->setProperty("widgetHolder", QVariant::fromValue(holder));

    gc(engine);
    QCOMPARE(container->dataChildren.count(), 1);
}

void tst_QWidgetsInQml::widgetAsDefaultPropertyKeptDuringCreation()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Qt.Widgets 1.0;\n"
                      "import QtQml 2.0;\n"
                      "QObjectContainerWithGCOnAppend {\n"
                      "    QWidget {\n"
                      "        id: parentLessChild;\n"
                      "        objectName: 'child'\n"
                      "        property var blah;\n" // Ensures that we have a JS wrapper
                      "    }\n"
                      "}", QUrl());
    QScopedPointer<QObject> object(component.create());
    QVERIFY(!object.isNull());

    QObjectContainer *container = qobject_cast<QObjectContainer*>(object.data());
    QCOMPARE(container->dataChildren.count(), 1);

    gc(engine);
    QCOMPARE(container->dataChildren.count(), 0);

}

QTEST_MAIN(tst_QWidgetsInQml)

#include "tst_qwidgetsinqml.moc"
