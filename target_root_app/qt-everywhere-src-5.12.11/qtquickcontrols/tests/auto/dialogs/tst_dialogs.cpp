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
#include "../shared/util.h"
#include "qquickabstractdialog_p.h"
#include <QtQml/QQmlComponent>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QSignalSpy>

class tst_dialogs : public QQmlDataTest
{
    Q_OBJECT
public:

private slots:
    void initTestCase()
    {
        QQmlDataTest::initTestCase();
    }

    //Dialog
    void dialogImplicitWidth_data();
    void dialogImplicitWidth();
    void dialogContentResize();
    void dialogButtonHandler_data();
    void dialogButtonHandler();
    void dialogKeyHandler_data();
    void dialogKeyHandler();
    void dialogWithDynamicTitle();

    // FileDialog
    void fileDialogDefaultModality();
    void fileDialogNonModal();
    void fileDialogNameFilters();
    void fileDialogDefaultSuffix();

private:
};

void tst_dialogs::dialogImplicitWidth_data()
{
    QTest::addColumn<int>("standardButtons");
    QTest::addColumn<int>("minimumHeight");

    QTest::newRow("No buttons") <<
        int(QQuickAbstractDialog::NoButton) <<
        150;
    QTest::newRow("OK button") <<
        int(QQuickAbstractDialog::Ok) <<
        160;
}

void tst_dialogs::dialogImplicitWidth()
{
    QFETCH(int, standardButtons);
    QFETCH(int, minimumHeight);

    /* This is the outerSpacing from DefaultDialogWrapper.qml,
     * which is always present */
    int heightMargins = 12 * 2;
    QQmlEngine engine;
    engine.rootContext()->setContextProperty("buttonsFromTest", standardButtons);
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("DialogImplicitSize.qml"));
    QObject *created = component.create();
    QScopedPointer<QObject> cleanup(created);
    QVERIFY2(created, qPrintable(component.errorString()));

    QTRY_VERIFY(created->property("width").toInt() >= 400);
    QTRY_VERIFY(created->property("height").toInt() >= minimumHeight + heightMargins);
}

void tst_dialogs::dialogContentResize()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("DialogMinimumSize.qml"));
    QObject *created = component.create();
    QScopedPointer<QObject> cleanup(created);
    QVERIFY2(created, qPrintable(component.errorString()));

    QTRY_COMPARE(created->property("width").toInt(), 400);
    QTRY_COMPARE(created->property("height").toInt(), 300);

    // Check that the content item has been sized up from its implicit size
    QQuickItem *userContent = created->findChild<QQuickItem*>("userContent");
    QVERIFY(userContent);
    QVERIFY(userContent->width() > 350);
    QVERIFY(userContent->height() > 200);
}

void tst_dialogs::dialogButtonHandler_data()
{
    QTest::addColumn<int>("standardButtons");
    QTest::addColumn<bool>("mustBlock");
    QTest::addColumn<QString>("expectedAction");

    QTest::newRow("Cancel, ignored") <<
        int(QQuickAbstractDialog::Cancel) <<
        false <<
        "rejected";
    QTest::newRow("Cancel, blocked") <<
        int(QQuickAbstractDialog::Cancel) <<
        true <<
        "";
    QTest::newRow("OK, ignored") <<
        int(QQuickAbstractDialog::Ok) <<
        false <<
        "accepted";
    QTest::newRow("OK, blocked") <<
        int(QQuickAbstractDialog::Ok) <<
        true <<
        "";
}

void tst_dialogs::dialogButtonHandler()
{
    QFETCH(int, standardButtons);
    QFETCH(bool, mustBlock);
    QFETCH(QString, expectedAction);

    QQmlEngine engine;
    engine.rootContext()->setContextProperty("buttonsFromTest", standardButtons);
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("DialogButtonHandler.qml"));
    QObject *root = component.create();
    QScopedPointer<QObject> cleanup(root);
    QVERIFY2(root, qPrintable(component.errorString()));

    root->setProperty("visible", true);
    root->setProperty("mustBlock", mustBlock);

    QQuickWindow *window = root->findChild<QQuickWindow*>();
    QTest::qWaitForWindowExposed(window);

    /* Hack to find the created buttons: since they are created by a
     * QQuickRepeater, they don't appear on the hierarchy tree; therefore, we
     * first need to find the repeater, and then to get its child. */
    const QList<QQuickItem*> children = root->findChildren<QQuickItem*>();
    QQuickItem *buttonWidget = nullptr;
    for (QQuickItem *child: children) {
        if (qstrcmp(child->metaObject()->className(), "QQuickRepeater") == 0 &&
            child->property("count").toInt() > 0) {
            int index = 0;
            QMetaObject::invokeMethod(child,
                                      "itemAt",
                                      Q_RETURN_ARG(QQuickItem *, buttonWidget),
                                      Q_ARG(int, index));
            break;
        }
    }
    QVERIFY(buttonWidget);

    const QPointF buttonCenterF(buttonWidget->width() / 2,
                                buttonWidget->height() / 2);
    const QPoint buttonCenter = buttonWidget->mapToScene(buttonCenterF).toPoint();

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, buttonCenter);
    QTRY_VERIFY(root->property("handlerWasCalled").toBool());

    QCOMPARE(root->property("buttonCode").toInt(), standardButtons);
    QCOMPARE(root->property("keyCode").toInt(), 0);

    QCOMPARE(root->property("actionCalled").toString(), expectedAction);
}

void tst_dialogs::dialogKeyHandler_data()
{
    QTest::addColumn<int>("key");
    QTest::addColumn<bool>("mustBlock");
    QTest::addColumn<int>("expectedButton");
    QTest::addColumn<QString>("expectedAction");

    QTest::newRow("Escape, ignored") <<
        int(Qt::Key_Escape) <<
        false <<
        int(QQuickAbstractDialog::Cancel) <<
        "rejected";
    QTest::newRow("Cancel, blocked") <<
        int(Qt::Key_Escape) <<
        true <<
        int(QQuickAbstractDialog::Cancel) <<
        "";
    QTest::newRow("Enter, ignored") <<
        int(Qt::Key_Enter) <<
        false <<
        int(QQuickAbstractDialog::Ok) <<
        "accepted";
    QTest::newRow("Enter, blocked") <<
        int(Qt::Key_Enter) <<
        true <<
        int(QQuickAbstractDialog::Ok) <<
        "";
}

void tst_dialogs::dialogKeyHandler()
{
    QFETCH(int, key);
    QFETCH(bool, mustBlock);
    QFETCH(int, expectedButton);
    QFETCH(QString, expectedAction);

    QQmlEngine engine;
    QQuickAbstractDialog::StandardButtons buttons =
        QQuickAbstractDialog::Ok | QQuickAbstractDialog::Cancel;
    engine.rootContext()->setContextProperty("buttonsFromTest", int(buttons));
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("DialogButtonHandler.qml"));
    QObject *root = component.create();
    QScopedPointer<QObject> cleanup(root);
    QVERIFY2(root, qPrintable(component.errorString()));

    root->setProperty("visible", true);
    root->setProperty("mustBlock", mustBlock);

    QQuickWindow *window = root->findChild<QQuickWindow*>();
    QTest::qWaitForWindowExposed(window);

    QTest::keyClick(window, Qt::Key(key));
    QTRY_VERIFY(root->property("handlerWasCalled").toBool());

    QCOMPARE(root->property("buttonCode").toInt(), expectedButton);
    QCOMPARE(root->property("keyCode").toInt(), key);

    QCOMPARE(root->property("actionCalled").toString(), expectedAction);
}

void tst_dialogs::fileDialogDefaultModality()
{
    QQuickView *window = new QQuickView;
    QScopedPointer<QQuickWindow> cleanup(window);

    window->setSource(testFileUrl("RectWithFileDialog.qml"));
    window->setGeometry(240,240,1024,320);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    QVERIFY2(window->rootObject(), qPrintable(window->errors().value(0).toString()));

    // Click to show
    QObject *dlg = qvariant_cast<QObject *>(window->rootObject()->property("fileDialog"));
    QSignalSpy spyVisibilityChanged(dlg, SIGNAL(visibilityChanged()));
    QTest::mouseClick(window, Qt::LeftButton, 0, QPoint(1000, 100));  // show
    QTRY_VERIFY(spyVisibilityChanged.count() > 0);
    int visibilityChangedCount = spyVisibilityChanged.count();
    // Can't hide by clicking the main window, because dialog is modal.
    QTest::mouseClick(window, Qt::LeftButton, 0, QPoint(1000, 100));
    /*
        On OS X, if you send an event directly to a window, the modal dialog
        doesn't block the event, so the window will process it normally. This
        is a different code path compared to having a user click the mouse and
        generate a native event; in that case the OS does the filtering itself,
        and Qt will not even see the event. But simulating real events in the
        test framework is generally unstable. So there isn't a good way to test
        modality on OS X.
        This test sometimes fails on other platforms too.  Maybe it's not reliable
        to try to click the main window in a location which is outside the
        dialog, without checking or guaranteeing it somehow.
    */
    QSKIP("Modality test is flaky in general and doesn't work at all on OS X");
    // So we expect no change in visibility.
    QCOMPARE(spyVisibilityChanged.count(), visibilityChangedCount);

    QCOMPARE(dlg->property("visible").toBool(), true);
    QMetaObject::invokeMethod(dlg, "close");
    QTRY_VERIFY(spyVisibilityChanged.count() > visibilityChangedCount);
    visibilityChangedCount = spyVisibilityChanged.count();
    QCOMPARE(dlg->property("visible").toBool(), false);
    QMetaObject::invokeMethod(dlg, "open");
    QTRY_VERIFY(spyVisibilityChanged.count() > visibilityChangedCount);
    QCOMPARE(dlg->property("visible").toBool(), true);
    QCOMPARE(dlg->property("modality").toInt(), (int)Qt::WindowModal);
}

void tst_dialogs::fileDialogNonModal()
{
    QQuickView *window = new QQuickView;
    QScopedPointer<QQuickWindow> cleanup(window);

    window->setSource(testFileUrl("RectWithFileDialog.qml"));
    window->setGeometry(240,240,1024,320);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    QVERIFY2(window->rootObject(), qPrintable(window->errors().value(0).toString()));

    // Click to toggle visibility
    QObject *dlg = qvariant_cast<QObject *>(window->rootObject()->property("fileDialog"));
    dlg->setProperty("modality", QVariant((int)Qt::NonModal));
    QSignalSpy spyVisibilityChanged(dlg, SIGNAL(visibilityChanged()));
    QTest::mouseClick(window, Qt::LeftButton, 0, QPoint(1000, 100));  // show
    QTRY_VERIFY(spyVisibilityChanged.count() > 0);
    int visibilityChangedCount = spyVisibilityChanged.count();
    QCOMPARE(dlg->property("visible").toBool(), true);
    QTest::mouseClick(window, Qt::LeftButton, 0, QPoint(1000, 100));  // hide
    QTRY_VERIFY(spyVisibilityChanged.count() > visibilityChangedCount);
    QCOMPARE(dlg->property("visible").toBool(), false);
#ifdef Q_OS_WIN
    QCOMPARE(dlg->property("modality").toInt(), (int)Qt::ApplicationModal);
#else
    QCOMPARE(dlg->property("modality").toInt(), (int)Qt::NonModal);
#endif
}

void tst_dialogs::fileDialogNameFilters()
{
    QQuickView *window = new QQuickView;
    QScopedPointer<QQuickWindow> cleanup(window);

    window->setSource(testFileUrl("RectWithFileDialog.qml"));
    window->setGeometry(240,240,1024,320);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    QVERIFY2(window->rootObject(), qPrintable(window->errors().value(0).toString()));

    QObject *dlg = qvariant_cast<QObject *>(window->rootObject()->property("fileDialog"));
    QStringList filters;
    filters << "QML files (*.qml)";
    filters << "Image files (*.jpg, *.png, *.gif)";
    filters << "All files (*)";
    dlg->setProperty("nameFilters", QVariant(filters));
    QCOMPARE(dlg->property("selectedNameFilter").toString(), filters.first());
}

void tst_dialogs::fileDialogDefaultSuffix()
{
    QQuickView *window = new QQuickView;
    QScopedPointer<QQuickWindow> cleanup(window);

    const QUrl sourceUrl = testFileUrl("RectWithFileDialog.qml");
    window->setSource(sourceUrl);
    window->setGeometry(240, 240, 1024, 320);
    window->show();
    QTRY_VERIFY(QTest::qWaitForWindowExposed(window));
    QVERIFY(window->rootObject());

    QObject *dlg = qvariant_cast<QObject *>(window->rootObject()->property("fileDialog"));
    QCOMPARE(dlg->property("defaultSuffix").toString(), QString());
    dlg->setProperty("defaultSuffix", "txt");
    QCOMPARE(dlg->property("defaultSuffix").toString(), QString("txt"));
    dlg->setProperty("defaultSuffix", ".txt");
    QCOMPARE(dlg->property("defaultSuffix").toString(), QString("txt"));
    dlg->setProperty("defaultSuffix", QString());
    QCOMPARE(dlg->property("defaultSuffix").toString(), QString());
}

void tst_dialogs::dialogWithDynamicTitle()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("DialogWithDynamicTitle.qml"));
    QObject *dlg = component.create();
    QScopedPointer<QObject> cleanup(dlg);
    QVERIFY2(dlg, qPrintable(component.errorString()));
    QWindow *window = dlg->findChild<QWindow *>();
    QVERIFY(window);
    QTRY_COMPARE(window->title(), QLatin1String("Title"));
    dlg->setProperty("newTitle", true);
    QTRY_COMPARE(window->title(), QLatin1String("New Title"));
}

QTEST_MAIN(tst_dialogs)

#include "tst_dialogs.moc"
