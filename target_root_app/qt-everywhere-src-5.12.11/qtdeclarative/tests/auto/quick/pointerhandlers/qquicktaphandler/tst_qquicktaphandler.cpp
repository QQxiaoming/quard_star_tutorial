/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include <QtGui/qstylehints.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/private/qquickpointerhandler_p.h>
#include <QtQuick/private/qquicktaphandler_p.h>
#include <qpa/qwindowsysteminterface.h>

#include <private/qquickwindow_p.h>

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlproperty.h>

#include "../../../shared/util.h"
#include "../../shared/viewtestutil.h"

Q_LOGGING_CATEGORY(lcPointerTests, "qt.quick.pointer.tests")

class tst_TapHandler : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_TapHandler()
        :touchDevice(QTest::createTouchDevice())
    {}

private slots:
    void initTestCase();

    void touchGesturePolicyDragThreshold();
    void mouseGesturePolicyDragThreshold();
    void touchMouseGesturePolicyDragThreshold();
    void touchGesturePolicyWithinBounds();
    void mouseGesturePolicyWithinBounds();
    void touchGesturePolicyReleaseWithinBounds();
    void mouseGesturePolicyReleaseWithinBounds();
    void touchMultiTap();
    void mouseMultiTap();
    void touchLongPress();
    void mouseLongPress();
    void buttonsMultiTouch();
    void componentUserBehavioralOverride();
    void rightLongPressIgnoreWheel();

private:
    void createView(QScopedPointer<QQuickView> &window, const char *fileName);
    QTouchDevice *touchDevice;
};

void tst_TapHandler::createView(QScopedPointer<QQuickView> &window, const char *fileName)
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

void tst_TapHandler::initTestCase()
{
    // This test assumes that we don't get synthesized mouse events from QGuiApplication
    qApp->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);

    QQmlDataTest::initTestCase();
}

void tst_TapHandler::touchGesturePolicyDragThreshold()
{
    const int dragThreshold = QGuiApplication::styleHints()->startDragDistance();
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *buttonDragThreshold = window->rootObject()->findChild<QQuickItem*>("DragThreshold");
    QVERIFY(buttonDragThreshold);
    QQuickTapHandler *tapHandler = buttonDragThreshold->findChild<QQuickTapHandler*>();
    QVERIFY(tapHandler);
    QSignalSpy dragThresholdTappedSpy(buttonDragThreshold, SIGNAL(tapped()));

    // DragThreshold button stays pressed while touchpoint stays within dragThreshold, emits tapped on release
    QPoint p1 = buttonDragThreshold->mapToScene(QPointF(20, 20)).toPoint();
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    p1 += QPoint(dragThreshold, 0);
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(buttonDragThreshold->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonDragThreshold->property("pressed").toBool());
    QCOMPARE(dragThresholdTappedSpy.count(), 1);
    QCOMPARE(buttonDragThreshold->property("tappedPosition").toPoint(), p1);
    QCOMPARE(tapHandler->point().position(), QPointF());

    // DragThreshold button is no longer pressed if touchpoint goes beyond dragThreshold
    dragThresholdTappedSpy.clear();
    p1 = buttonDragThreshold->mapToScene(QPointF(20, 20)).toPoint();
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    p1 += QPoint(dragThreshold, 0);
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(buttonDragThreshold->property("pressed").toBool());
    p1 += QPoint(1, 0);
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonDragThreshold->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(!buttonDragThreshold->property("pressed").toBool());
    QCOMPARE(dragThresholdTappedSpy.count(), 0);
}

void tst_TapHandler::mouseGesturePolicyDragThreshold()
{
    const int dragThreshold = QGuiApplication::styleHints()->startDragDistance();
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *buttonDragThreshold = window->rootObject()->findChild<QQuickItem*>("DragThreshold");
    QVERIFY(buttonDragThreshold);
    QQuickTapHandler *tapHandler = buttonDragThreshold->findChild<QQuickTapHandler*>();
    QVERIFY(tapHandler);
    QSignalSpy dragThresholdTappedSpy(buttonDragThreshold, SIGNAL(tapped()));

    // DragThreshold button stays pressed while mouse stays within dragThreshold, emits tapped on release
    QPoint p1 = buttonDragThreshold->mapToScene(QPointF(20, 20)).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    p1 += QPoint(dragThreshold, 0);
    QTest::mouseMove(window, p1);
    QVERIFY(buttonDragThreshold->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(!buttonDragThreshold->property("pressed").toBool());
    QTRY_COMPARE(dragThresholdTappedSpy.count(), 1);
    QCOMPARE(buttonDragThreshold->property("tappedPosition").toPoint(), p1);
    QCOMPARE(tapHandler->point().position(), QPointF());

    // DragThreshold button is no longer pressed if mouse goes beyond dragThreshold
    dragThresholdTappedSpy.clear();
    p1 = buttonDragThreshold->mapToScene(QPointF(20, 20)).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    p1 += QPoint(dragThreshold, 0);
    QTest::mouseMove(window, p1);
    QVERIFY(buttonDragThreshold->property("pressed").toBool());
    p1 += QPoint(1, 0);
    QTest::mouseMove(window, p1);
    QTRY_VERIFY(!buttonDragThreshold->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QVERIFY(!buttonDragThreshold->property("pressed").toBool());
    QCOMPARE(dragThresholdTappedSpy.count(), 0);
}

void tst_TapHandler::touchMouseGesturePolicyDragThreshold()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *buttonDragThreshold = window->rootObject()->findChild<QQuickItem*>("DragThreshold");
    QVERIFY(buttonDragThreshold);
    QSignalSpy tappedSpy(buttonDragThreshold, SIGNAL(tapped()));
    QSignalSpy canceledSpy(buttonDragThreshold, SIGNAL(canceled()));

    // Press mouse, drag it outside the button, release
    QPoint p1 = buttonDragThreshold->mapToScene(QPointF(20, 20)).toPoint();
    QPoint p2 = p1 + QPoint(int(buttonDragThreshold->height()), 0);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    QTest::mouseMove(window, p2);
    QTRY_COMPARE(canceledSpy.count(), 1);
    QCOMPARE(tappedSpy.count(), 0);
    QCOMPARE(buttonDragThreshold->property("pressed").toBool(), false);
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p2);

    // Press and release touch, verify that it still works (QTBUG-71466)
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonDragThreshold->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 1);

    // Press touch, drag it outside the button, release
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).move(1, p2, window);
    QQuickTouchUtils::flush(window);
    QTRY_COMPARE(buttonDragThreshold->property("pressed").toBool(), false);
    QTest::touchEvent(window, touchDevice).release(1, p2, window);
    QQuickTouchUtils::flush(window);
    QTRY_COMPARE(canceledSpy.count(), 2);
    QCOMPARE(tappedSpy.count(), 1); // didn't increase
    QCOMPARE(buttonDragThreshold->property("pressed").toBool(), false);

    // Press and release mouse, verify that it still works
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(tappedSpy.count(), 2);
    QCOMPARE(canceledSpy.count(), 2); // didn't increase
    QCOMPARE(buttonDragThreshold->property("pressed").toBool(), false);
}

void tst_TapHandler::touchGesturePolicyWithinBounds()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *buttonWithinBounds = window->rootObject()->findChild<QQuickItem*>("WithinBounds");
    QVERIFY(buttonWithinBounds);
    QSignalSpy withinBoundsTappedSpy(buttonWithinBounds, SIGNAL(tapped()));

    // WithinBounds button stays pressed while touchpoint stays within bounds, emits tapped on release
    QPoint p1 = buttonWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonWithinBounds->property("pressed").toBool());
    p1 += QPoint(50, 0);
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonWithinBounds->property("pressed").toBool());
    QCOMPARE(withinBoundsTappedSpy.count(), 1);

    // WithinBounds button is no longer pressed if touchpoint leaves bounds
    withinBoundsTappedSpy.clear();
    p1 = buttonWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonWithinBounds->property("pressed").toBool());
    p1 += QPoint(0, 100);
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonWithinBounds->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(!buttonWithinBounds->property("pressed").toBool());
    QCOMPARE(withinBoundsTappedSpy.count(), 0);
}

void tst_TapHandler::mouseGesturePolicyWithinBounds()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *buttonWithinBounds = window->rootObject()->findChild<QQuickItem*>("WithinBounds");
    QVERIFY(buttonWithinBounds);
    QSignalSpy withinBoundsTappedSpy(buttonWithinBounds, SIGNAL(tapped()));

    // WithinBounds button stays pressed while touchpoint stays within bounds, emits tapped on release
    QPoint p1 = buttonWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(buttonWithinBounds->property("pressed").toBool());
    p1 += QPoint(50, 0);
    QTest::mouseMove(window, p1);
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(!buttonWithinBounds->property("pressed").toBool());
    QCOMPARE(withinBoundsTappedSpy.count(), 1);

    // WithinBounds button is no longer pressed if touchpoint leaves bounds
    withinBoundsTappedSpy.clear();
    p1 = buttonWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(buttonWithinBounds->property("pressed").toBool());
    p1 += QPoint(0, 100);
    QTest::mouseMove(window, p1);
    QTRY_VERIFY(!buttonWithinBounds->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QVERIFY(!buttonWithinBounds->property("pressed").toBool());
    QCOMPARE(withinBoundsTappedSpy.count(), 0);
}

void tst_TapHandler::touchGesturePolicyReleaseWithinBounds()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *buttonReleaseWithinBounds = window->rootObject()->findChild<QQuickItem*>("ReleaseWithinBounds");
    QVERIFY(buttonReleaseWithinBounds);
    QSignalSpy releaseWithinBoundsTappedSpy(buttonReleaseWithinBounds, SIGNAL(tapped()));

    // ReleaseWithinBounds button stays pressed while touchpoint wanders anywhere,
    // then if it comes back within bounds, emits tapped on release
    QPoint p1 = buttonReleaseWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    p1 += QPoint(50, 0);
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    p1 += QPoint(250, 100);
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    p1 = buttonReleaseWithinBounds->mapToScene(QPointF(25, 15)).toPoint();
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonReleaseWithinBounds->property("pressed").toBool());
    QCOMPARE(releaseWithinBoundsTappedSpy.count(), 1);

    // ReleaseWithinBounds button does not emit tapped if released out of bounds
    releaseWithinBoundsTappedSpy.clear();
    p1 = buttonReleaseWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    p1 += QPoint(0, 100);
    QTest::touchEvent(window, touchDevice).move(1, p1, window);
    QQuickTouchUtils::flush(window);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonReleaseWithinBounds->property("pressed").toBool());
    QCOMPARE(releaseWithinBoundsTappedSpy.count(), 0);
}

void tst_TapHandler::mouseGesturePolicyReleaseWithinBounds()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *buttonReleaseWithinBounds = window->rootObject()->findChild<QQuickItem*>("ReleaseWithinBounds");
    QVERIFY(buttonReleaseWithinBounds);
    QSignalSpy releaseWithinBoundsTappedSpy(buttonReleaseWithinBounds, SIGNAL(tapped()));

    // ReleaseWithinBounds button stays pressed while touchpoint wanders anywhere,
    // then if it comes back within bounds, emits tapped on release
    QPoint p1 = buttonReleaseWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    p1 += QPoint(50, 0);
    QTest::mouseMove(window, p1);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    p1 += QPoint(250, 100);
    QTest::mouseMove(window, p1);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    p1 = buttonReleaseWithinBounds->mapToScene(QPointF(25, 15)).toPoint();
    QTest::mouseMove(window, p1);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(!buttonReleaseWithinBounds->property("pressed").toBool());
    QCOMPARE(releaseWithinBoundsTappedSpy.count(), 1);

    // ReleaseWithinBounds button does not emit tapped if released out of bounds
    releaseWithinBoundsTappedSpy.clear();
    p1 = buttonReleaseWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    p1 += QPoint(0, 100);
    QTest::mouseMove(window, p1);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(!buttonReleaseWithinBounds->property("pressed").toBool());
    QCOMPARE(releaseWithinBoundsTappedSpy.count(), 0);
}

void tst_TapHandler::touchMultiTap()
{
    const int dragThreshold = QGuiApplication::styleHints()->startDragDistance();
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *button = window->rootObject()->findChild<QQuickItem*>("DragThreshold");
    QVERIFY(button);
    QSignalSpy tappedSpy(button, SIGNAL(tapped()));

    // Tap once
    QPoint p1 = button->mapToScene(QPointF(2, 2)).toPoint();
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 1);

    // Tap again in exactly the same place (not likely with touch in the real world)
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 2);

    // Tap a third time, nearby
    p1 += QPoint(dragThreshold, dragThreshold);
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 3);

    // Tap a fourth time, drifting farther away
    p1 += QPoint(dragThreshold, dragThreshold);
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 4);
}

void tst_TapHandler::mouseMultiTap()
{
    const int dragThreshold = QGuiApplication::styleHints()->startDragDistance();
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *button = window->rootObject()->findChild<QQuickItem*>("DragThreshold");
    QVERIFY(button);
    QSignalSpy tappedSpy(button, SIGNAL(tapped()));

    // Tap once
    QPoint p1 = button->mapToScene(QPointF(2, 2)).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 1);

    // Tap again in exactly the same place (not likely with touch in the real world)
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 2);

    // Tap a third time, nearby
    p1 += QPoint(dragThreshold, dragThreshold);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 3);

    // Tap a fourth time, drifting farther away
    p1 += QPoint(dragThreshold, dragThreshold);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 4);
}

void tst_TapHandler::touchLongPress()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *button = window->rootObject()->findChild<QQuickItem*>("DragThreshold");
    QVERIFY(button);
    QQuickTapHandler *tapHandler = button->findChild<QQuickTapHandler*>("DragThreshold");
    QVERIFY(tapHandler);
    QSignalSpy tappedSpy(button, SIGNAL(tapped()));
    QSignalSpy longPressThresholdChangedSpy(tapHandler, SIGNAL(longPressThresholdChanged()));
    QSignalSpy timeHeldSpy(tapHandler, SIGNAL(timeHeldChanged()));
    QSignalSpy longPressedSpy(tapHandler, SIGNAL(longPressed()));

    // Reduce the threshold so that we can get a long press quickly
    tapHandler->setLongPressThreshold(0.5);
    QCOMPARE(longPressThresholdChangedSpy.count(), 1);

    // Press and hold
    QPoint p1 = button->mapToScene(button->clipRect().center()).toPoint();
    QTest::touchEvent(window, touchDevice).press(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTRY_COMPARE(longPressedSpy.count(), 1);
    timeHeldSpy.wait(); // the longer we hold it, the more this will occur
    qDebug() << "held" << tapHandler->timeHeld() << "secs; timeHeld updated" << timeHeldSpy.count() << "times";
    QVERIFY(timeHeldSpy.count() > 0);
    QVERIFY(tapHandler->timeHeld() > 0.4); // Should be > 0.5 but slow CI and timer granularity can interfere

    // Release and verify that tapped was not emitted
    QTest::touchEvent(window, touchDevice).release(1, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 0);
}

void tst_TapHandler::mouseLongPress()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *button = window->rootObject()->findChild<QQuickItem*>("DragThreshold");
    QVERIFY(button);
    QQuickTapHandler *tapHandler = button->findChild<QQuickTapHandler*>("DragThreshold");
    QVERIFY(tapHandler);
    QSignalSpy tappedSpy(button, SIGNAL(tapped()));
    QSignalSpy longPressThresholdChangedSpy(tapHandler, SIGNAL(longPressThresholdChanged()));
    QSignalSpy timeHeldSpy(tapHandler, SIGNAL(timeHeldChanged()));
    QSignalSpy longPressedSpy(tapHandler, SIGNAL(longPressed()));

    // Reduce the threshold so that we can get a long press quickly
    tapHandler->setLongPressThreshold(0.5);
    QCOMPARE(longPressThresholdChangedSpy.count(), 1);

    // Press and hold
    QPoint p1 = button->mapToScene(button->clipRect().center()).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_VERIFY(button->property("pressed").toBool());
    QTRY_COMPARE(longPressedSpy.count(), 1);
    timeHeldSpy.wait(); // the longer we hold it, the more this will occur
    qDebug() << "held" << tapHandler->timeHeld() << "secs; timeHeld updated" << timeHeldSpy.count() << "times";
    QVERIFY(timeHeldSpy.count() > 0);
    QVERIFY(tapHandler->timeHeld() > 0.4); // Should be > 0.5 but slow CI and timer granularity can interfere

    // Release and verify that tapped was not emitted
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1, 500);
    QTRY_VERIFY(!button->property("pressed").toBool());
    QCOMPARE(tappedSpy.count(), 0);
}

void tst_TapHandler::buttonsMultiTouch()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttons.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *buttonDragThreshold = window->rootObject()->findChild<QQuickItem*>("DragThreshold");
    QVERIFY(buttonDragThreshold);
    QSignalSpy dragThresholdTappedSpy(buttonDragThreshold, SIGNAL(tapped()));

    QQuickItem *buttonWithinBounds = window->rootObject()->findChild<QQuickItem*>("WithinBounds");
    QVERIFY(buttonWithinBounds);
    QSignalSpy withinBoundsTappedSpy(buttonWithinBounds, SIGNAL(tapped()));

    QQuickItem *buttonReleaseWithinBounds = window->rootObject()->findChild<QQuickItem*>("ReleaseWithinBounds");
    QVERIFY(buttonReleaseWithinBounds);
    QSignalSpy releaseWithinBoundsTappedSpy(buttonReleaseWithinBounds, SIGNAL(tapped()));
    QTest::QTouchEventSequence touchSeq = QTest::touchEvent(window, touchDevice, false);

    // can press multiple buttons at the same time
    QPoint p1 = buttonDragThreshold->mapToScene(QPointF(20, 20)).toPoint();
    touchSeq.press(1, p1, window).commit();
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    QPoint p2 = buttonWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    touchSeq.stationary(1).press(2, p2, window).commit();
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonWithinBounds->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("active").toBool());
    QPoint p3 = buttonReleaseWithinBounds->mapToScene(QPointF(20, 20)).toPoint();
    touchSeq.stationary(1).stationary(2).press(3, p3, window).commit();
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QVERIFY(buttonReleaseWithinBounds->property("active").toBool());
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("active").toBool());
    QVERIFY(buttonDragThreshold->property("pressed").toBool());

    // combinations of small touchpoint movements and stationary points should not cause state changes
    p1 += QPoint(2, 0);
    p2 += QPoint(3, 0);
    touchSeq.move(1, p1).move(2, p2).stationary(3).commit();
    QVERIFY(buttonDragThreshold->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("active").toBool());
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QVERIFY(buttonReleaseWithinBounds->property("active").toBool());
    p3 += QPoint(4, 0);
    touchSeq.stationary(1).stationary(2).move(3, p3).commit();
    QVERIFY(buttonDragThreshold->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("active").toBool());
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QVERIFY(buttonReleaseWithinBounds->property("active").toBool());

    // can release top button and press again: others stay pressed the whole time
    touchSeq.stationary(2).stationary(3).release(1, p1, window).commit();
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonDragThreshold->property("pressed").toBool());
    QCOMPARE(dragThresholdTappedSpy.count(), 1);
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QCOMPARE(withinBoundsTappedSpy.count(), 0);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QCOMPARE(releaseWithinBoundsTappedSpy.count(), 0);
    touchSeq.stationary(2).stationary(3).press(1, p1, window).commit();
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());

    // can release middle button and press again: others stay pressed the whole time
    touchSeq.stationary(1).stationary(3).release(2, p2, window).commit();
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(!buttonWithinBounds->property("pressed").toBool());
    QCOMPARE(withinBoundsTappedSpy.count(), 1);
    QVERIFY(buttonDragThreshold->property("pressed").toBool());
    QCOMPARE(dragThresholdTappedSpy.count(), 1);
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
    QCOMPARE(releaseWithinBoundsTappedSpy.count(), 0);
    touchSeq.stationary(1).stationary(3).press(2, p2, window).commit();
    QQuickTouchUtils::flush(window);
    QVERIFY(buttonDragThreshold->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());

    // can release bottom button and press again: others stay pressed the whole time
    touchSeq.stationary(1).stationary(2).release(3, p3, window).commit();
    QQuickTouchUtils::flush(window);
    QCOMPARE(releaseWithinBoundsTappedSpy.count(), 1);
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QCOMPARE(withinBoundsTappedSpy.count(), 1);
    QVERIFY(!buttonReleaseWithinBounds->property("pressed").toBool());
    QCOMPARE(dragThresholdTappedSpy.count(), 1);
    touchSeq.stationary(1).stationary(2).press(3, p3, window).commit();
    QQuickTouchUtils::flush(window);
    QTRY_VERIFY(buttonDragThreshold->property("pressed").toBool());
    QVERIFY(buttonWithinBounds->property("pressed").toBool());
    QVERIFY(buttonReleaseWithinBounds->property("pressed").toBool());
}

void tst_TapHandler::componentUserBehavioralOverride()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "buttonOverrideHandler.qml");
    QQuickView * window = windowPtr.data();

    QQuickItem *button = window->rootObject()->findChild<QQuickItem*>("Overridden");
    QVERIFY(button);
    QQuickTapHandler *innerTapHandler = button->findChild<QQuickTapHandler*>("Overridden");
    QVERIFY(innerTapHandler);
    QQuickTapHandler *userTapHandler = button->findChild<QQuickTapHandler*>("override");
    QVERIFY(userTapHandler);
    QSignalSpy tappedSpy(button, SIGNAL(tapped()));
    QSignalSpy innerGrabChangedSpy(innerTapHandler, SIGNAL(grabChanged(QQuickEventPoint::GrabTransition, QQuickEventPoint *)));
    QSignalSpy userGrabChangedSpy(userTapHandler, SIGNAL(grabChanged(QQuickEventPoint::GrabTransition, QQuickEventPoint *)));
    QSignalSpy innerPressedChangedSpy(innerTapHandler, SIGNAL(pressedChanged()));
    QSignalSpy userPressedChangedSpy(userTapHandler, SIGNAL(pressedChanged()));

    // Press
    QPoint p1 = button->mapToScene(button->clipRect().center()).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(userPressedChangedSpy.count(), 1);
    QCOMPARE(innerPressedChangedSpy.count(), 0);
    QCOMPARE(innerGrabChangedSpy.count(), 0);
    QCOMPARE(userGrabChangedSpy.count(), 1);

    // Release
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(userPressedChangedSpy.count(), 2);
    QCOMPARE(innerPressedChangedSpy.count(), 0);
    QCOMPARE(tappedSpy.count(), 1); // only because the override handler makes that happen
    QCOMPARE(innerGrabChangedSpy.count(), 0);
    QCOMPARE(userGrabChangedSpy.count(), 2);
}

void tst_TapHandler::rightLongPressIgnoreWheel()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "rightTapHandler.qml");
    QQuickView * window = windowPtr.data();

    QQuickTapHandler *tap = window->rootObject()->findChild<QQuickTapHandler*>();
    QVERIFY(tap);
    QSignalSpy tappedSpy(tap, SIGNAL(tapped(QQuickEventPoint *)));
    QSignalSpy longPressedSpy(tap, SIGNAL(longPressed()));
    QPoint p1(100, 100);

    // Mouse wheel with ScrollBegin phase (because as soon as two fingers are touching
    // the trackpad, it will send such an event: QTBUG-71955)
    {
        QWheelEvent wheelEvent(p1, p1, QPoint(0, 0), QPoint(0, 0),
                               Qt::NoButton, Qt::NoModifier, Qt::ScrollBegin, false, Qt::MouseEventNotSynthesized);
        QGuiApplication::sendEvent(window, &wheelEvent);
    }

    // Press
    QTest::mousePress(window, Qt::RightButton, Qt::NoModifier, p1);
    QTRY_COMPARE(tap->isPressed(), true);

    // Mouse wheel ScrollEnd phase
    QWheelEvent wheelEvent(p1, p1, QPoint(0, 0), QPoint(0, 0),
                           Qt::NoButton, Qt::NoModifier, Qt::ScrollEnd, false, Qt::MouseEventNotSynthesized);
    QGuiApplication::sendEvent(window, &wheelEvent);
    QTRY_COMPARE(longPressedSpy.count(), 1);
    QCOMPARE(tap->isPressed(), true);
    QCOMPARE(tappedSpy.count(), 0);

    // Release
    QTest::mouseRelease(window, Qt::RightButton, Qt::NoModifier, p1, 500);
    QTRY_COMPARE(tap->isPressed(), false);
    QCOMPARE(tappedSpy.count(), 0);
}

QTEST_MAIN(tst_TapHandler)

#include "tst_qquicktaphandler.moc"

