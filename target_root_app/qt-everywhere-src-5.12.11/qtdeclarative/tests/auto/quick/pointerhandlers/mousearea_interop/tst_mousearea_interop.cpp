/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlproperty.h>
#include <QtQuick/private/qquickdraghandler_p.h>
#include <QtQuick/private/qquickmousearea_p.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

#include "../../../shared/util.h"
#include "../../shared/viewtestutil.h"

Q_LOGGING_CATEGORY(lcPointerTests, "qt.quick.pointer.tests")

class tst_MouseAreaInterop : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_MouseAreaInterop()
      : touchDevice(QTest::createTouchDevice())
      , touchPointerDevice(QQuickPointerDevice::touchDevice(touchDevice))
    {}

private slots:
    void dragHandlerInSiblingStealingGrabFromMouseAreaViaMouse();
    void dragHandlerInSiblingStealingGrabFromMouseAreaViaTouch();

private:
    void createView(QScopedPointer<QQuickView> &window, const char *fileName);
    QTouchDevice *touchDevice;
    QQuickPointerDevice *touchPointerDevice;
};

void tst_MouseAreaInterop::createView(QScopedPointer<QQuickView> &window, const char *fileName)
{
    window.reset(new QQuickView);
    window->setSource(testFileUrl(fileName));
    QTRY_COMPARE(window->status(), QQuickView::Ready);
    QQuickViewTestUtil::centerOnScreen(window.data());
    QQuickViewTestUtil::moveMouseAway(window.data());

    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window.data()));
    QVERIFY(window->rootObject() != nullptr);
}

void tst_MouseAreaInterop::dragHandlerInSiblingStealingGrabFromMouseAreaViaMouse()
{
    const int dragThreshold = QGuiApplication::styleHints()->startDragDistance();
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "dragTakeOverFromSibling.qml");
    QQuickView * window = windowPtr.data();
    auto pointerEvent = QQuickWindowPrivate::get(window)->pointerEventInstance(QQuickPointerDevice::genericMouseDevice());

    QPointer<QQuickPointerHandler> handler = window->rootObject()->findChild<QQuickPointerHandler*>();
    QVERIFY(handler);
    QQuickMouseArea *ma = window->rootObject()->findChild<QQuickMouseArea*>();
    QVERIFY(ma);

    QPoint p1(150, 150);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QCOMPARE(window->mouseGrabberItem(), ma);
    QCOMPARE(ma->pressed(), true);

    // Start dragging
    // DragHandler keeps monitoring, due to its passive grab,
    // and eventually steals the exclusive grab from MA
    int dragStoleGrab = 0;
    for (int i = 0; i < 4; ++i) {
        p1 += QPoint(dragThreshold / 2, 0);
        QTest::mouseMove(window, p1);
        if (!dragStoleGrab && pointerEvent->point(0)->exclusiveGrabber() == handler)
            dragStoleGrab = i;
    }
    if (dragStoleGrab)
        qCDebug(lcPointerTests, "DragHandler stole the grab after %d events", dragStoleGrab);
    QVERIFY(dragStoleGrab > 1);
    QCOMPARE(handler->active(), true);
    QCOMPARE(ma->pressed(), false);

    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QCOMPARE(handler->active(), false);
}

void tst_MouseAreaInterop::dragHandlerInSiblingStealingGrabFromMouseAreaViaTouch() // QTBUG-77624
{
    const int dragThreshold = QGuiApplication::styleHints()->startDragDistance();
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "dragTakeOverFromSibling.qml");
    QQuickView * window = windowPtr.data();
    auto pointerEvent = QQuickWindowPrivate::get(window)->pointerEventInstance(touchPointerDevice);

    QPointer<QQuickPointerHandler> handler = window->rootObject()->findChild<QQuickPointerHandler*>();
    QVERIFY(handler);
    QQuickMouseArea *ma = window->rootObject()->findChild<QQuickMouseArea*>();
    QVERIFY(ma);

    QPoint p1(150, 150);
    QTest::QTouchEventSequence touch = QTest::touchEvent(window, touchDevice);

    touch.press(1, p1).commit();
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(pointerEvent->point(0)->passiveGrabbers().contains(handler));
    QCOMPARE(pointerEvent->point(0)->grabberItem(), ma);
    QCOMPARE(window->mouseGrabberItem(), ma);
    QCOMPARE(ma->pressed(), true);

    // Start dragging
    // DragHandler keeps monitoring, due to its passive grab,
    // and eventually steals the exclusive grab from MA
    int dragStoleGrab = 0;
    for (int i = 0; i < 4; ++i) {
        p1 += QPoint(dragThreshold / 2, 0);
        touch.move(1, p1).commit();
        QQuickTouchUtils::flush(window);
        if (!dragStoleGrab && pointerEvent->point(0)->exclusiveGrabber() == handler)
            dragStoleGrab = i;
    }
    if (dragStoleGrab)
        qCDebug(lcPointerTests, "DragHandler stole the grab after %d events", dragStoleGrab);
    QVERIFY(dragStoleGrab > 1);
    QCOMPARE(handler->active(), true);
    QCOMPARE(ma->pressed(), false);

    touch.release(1, p1).commit();
    QQuickTouchUtils::flush(window);
    QCOMPARE(handler->active(), false);
}

QTEST_MAIN(tst_MouseAreaInterop)

#include "tst_mousearea_interop.moc"
