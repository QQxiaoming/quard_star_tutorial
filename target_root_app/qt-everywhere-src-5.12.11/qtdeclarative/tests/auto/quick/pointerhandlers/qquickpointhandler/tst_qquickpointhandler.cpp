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

#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/private/qquickpointhandler_p.h>
#include <qpa/qwindowsysteminterface.h>

#include <private/qquickwindow_p.h>

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlproperty.h>

#include "../../../shared/util.h"
#include "../../shared/viewtestutil.h"

Q_LOGGING_CATEGORY(lcPointerTests, "qt.quick.pointer.tests")

class tst_PointHandler : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_PointHandler()
        : touchDevice(QTest::createTouchDevice())
    {}

private slots:
    void initTestCase();

    void singleTouch();
    void simultaneousMultiTouch();
    void pressedMultipleButtons_data();
    void pressedMultipleButtons();

private:
    void createView(QScopedPointer<QQuickView> &window, const char *fileName);
    QTouchDevice *touchDevice;
};

void tst_PointHandler::createView(QScopedPointer<QQuickView> &window, const char *fileName)
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

void tst_PointHandler::initTestCase()
{
    // This test assumes that we don't get synthesized mouse events from QGuiApplication
    qApp->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);

    QQmlDataTest::initTestCase();
}

void tst_PointHandler::singleTouch()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "pointTracker.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem *tracker = window->rootObject()->findChild<QQuickItem *>("pointTracker");
    QVERIFY(tracker);
    QQuickPointHandler *handler = window->rootObject()->findChild<QQuickPointHandler *>("pointHandler");
    QVERIFY(handler);

    QSignalSpy activeSpy(handler, SIGNAL(activeChanged()));
    QSignalSpy pointSpy(handler, SIGNAL(pointChanged()));
    QSignalSpy translationSpy(handler, SIGNAL(translationChanged()));

    QPoint point(100,100);
    QTest::touchEvent(window, touchDevice).press(1, point, window);
    QQuickTouchUtils::flush(window);
    QTRY_COMPARE(handler->active(), true);
    QCOMPARE(activeSpy.count(), 1);
    QCOMPARE(pointSpy.count(), 1);
    QCOMPARE(handler->point().position().toPoint(), point);
    QCOMPARE(handler->point().scenePosition().toPoint(), point);
    QCOMPARE(handler->point().pressedButtons(), Qt::NoButton);
    QCOMPARE(handler->translation(), QVector2D());
    QCOMPARE(translationSpy.count(), 1);

    point += QPoint(10, 10);
    QTest::touchEvent(window, touchDevice).move(1, point, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(handler->active(), true);
    QCOMPARE(activeSpy.count(), 1);
    QCOMPARE(pointSpy.count(), 2);
    QCOMPARE(handler->point().position().toPoint(), point);
    QCOMPARE(handler->point().scenePosition().toPoint(), point);
    QCOMPARE(handler->point().pressPosition().toPoint(), QPoint(100, 100));
    QCOMPARE(handler->point().scenePressPosition().toPoint(), QPoint(100, 100));
    QCOMPARE(handler->point().pressedButtons(), Qt::NoButton);
    QVERIFY(handler->point().velocity().x() > 0);
    QVERIFY(handler->point().velocity().y() > 0);
    QCOMPARE(handler->translation(), QVector2D(10, 10));
    QCOMPARE(translationSpy.count(), 2);

    QTest::touchEvent(window, touchDevice).release(1, point, window);
    QQuickTouchUtils::flush(window);
    QTRY_COMPARE(handler->active(), false);
    QCOMPARE(activeSpy.count(), 2);
    QCOMPARE(pointSpy.count(), 3);
    QCOMPARE(handler->translation(), QVector2D());
    QCOMPARE(translationSpy.count(), 3);
}

void tst_PointHandler::simultaneousMultiTouch()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "multiPointTracker.qml");
    QQuickView * window = windowPtr.data();
    QList<QQuickPointHandler *> handlers = window->rootObject()->findChildren<QQuickPointHandler *>();
    QCOMPARE(handlers.count(), 3);

    QVector<QSignalSpy*> activeSpies;
    QVector<QSignalSpy*> pointSpies;
    QVector<QSignalSpy*> translationSpies;
    QVector<QPoint> points{{100,100}, {200, 200}, {100, 300}};
    QVector<QPoint> pressPoints = points;
    for (auto h : handlers) {
        activeSpies << new QSignalSpy(h, SIGNAL(activeChanged()));
        pointSpies << new QSignalSpy(h, SIGNAL(pointChanged()));
        translationSpies << new QSignalSpy(h, SIGNAL(translationChanged()));
    }

    QTest::touchEvent(window, touchDevice).press(1, points[0], window).press(2, points[1], window).press(3, points[2], window);
    QQuickTouchUtils::flush(window);
    QVector<int> pointIndexPerHandler;
    int i = 0;
    for (auto h : handlers) {
        QTRY_COMPARE(h->active(), true);
        QCOMPARE(activeSpies[i]->count(), 1);
        QCOMPARE(pointSpies[i]->count(), 1);
        int chosenPointIndex = points.indexOf(h->point().position().toPoint());
        QVERIFY(chosenPointIndex != -1);
        // Verify that each handler chose a unique point
        QVERIFY(!pointIndexPerHandler.contains(chosenPointIndex));
        pointIndexPerHandler.append(chosenPointIndex);
        QPoint point = points[chosenPointIndex];
        QCOMPARE(h->point().scenePosition().toPoint(), point);
        QCOMPARE(h->point().pressedButtons(), Qt::NoButton);
        QCOMPARE(h->translation(), QVector2D());
        QCOMPARE(translationSpies[i]->count(), 1);
        ++i;
    }

    for (int i = 0; i < 3; ++i)
        points[i] += QPoint(10 + 10 * i, 10 + 10 * i % 2);
    QTest::touchEvent(window, touchDevice).move(1, points[0], window).move(2, points[1], window).move(3, points[2], window);
    QQuickTouchUtils::flush(window);
    i = 0;
    for (auto h : handlers) {
        QCOMPARE(h->active(), true);
        QCOMPARE(activeSpies[i]->count(), 1);
        QCOMPARE(pointSpies[i]->count(), 2);
        QCOMPARE(h->point().position().toPoint(), points[pointIndexPerHandler[i]]);
        QCOMPARE(h->point().scenePosition().toPoint(), points[pointIndexPerHandler[i]]);
        QCOMPARE(h->point().pressPosition().toPoint(), pressPoints[pointIndexPerHandler[i]]);
        QCOMPARE(h->point().scenePressPosition().toPoint(), pressPoints[pointIndexPerHandler[i]]);
        QCOMPARE(h->point().pressedButtons(), Qt::NoButton);
        QVERIFY(h->point().velocity().x() > 0);
        QVERIFY(h->point().velocity().y() > 0);
        QCOMPARE(h->translation(), QVector2D(10 + 10 * pointIndexPerHandler[i], 10 + 10 * pointIndexPerHandler[i] % 2));
        QCOMPARE(translationSpies[i]->count(), 2);
        ++i;
    }

    QTest::touchEvent(window, touchDevice).release(1, points[0], window).release(2, points[1], window).release(3, points[2], window);
    QQuickTouchUtils::flush(window);
    i = 0;
    for (auto h : handlers) {
        QTRY_COMPARE(h->active(), false);
        QCOMPARE(activeSpies[i]->count(), 2);
        QCOMPARE(pointSpies[i]->count(), 3);
        QCOMPARE(h->translation(), QVector2D());
        QCOMPARE(translationSpies[i]->count(), 3);
        ++i;
    }

    qDeleteAll(activeSpies);
    qDeleteAll(pointSpies);
    qDeleteAll(translationSpies);
}

void tst_PointHandler::pressedMultipleButtons_data()
{
    QTest::addColumn<Qt::MouseButtons>("accepted");
    QTest::addColumn<QList<Qt::MouseButtons> >("buttons");
    QTest::addColumn<QList<bool> >("active");
    QTest::addColumn<QList<Qt::MouseButtons> >("pressedButtons");
    QTest::addColumn<int>("changeCount");

    QList<Qt::MouseButtons> buttons;
    QList<bool> active;
    QList<Qt::MouseButtons> pressedButtons;
    buttons << Qt::LeftButton
            << (Qt::LeftButton | Qt::RightButton)
            << Qt::LeftButton
            << Qt::NoButton;
    active << true
           << true
           << true
           << false;
    pressedButtons << Qt::LeftButton
                   << (Qt::LeftButton | Qt::RightButton)
                   << Qt::LeftButton
                   << Qt::NoButton;
    QTest::newRow("Accept Left - Press left, Press Right, Release Right")
            << Qt::MouseButtons(Qt::LeftButton) << buttons << active << pressedButtons << 4;

    buttons.clear();
    active.clear();
    pressedButtons.clear();
    buttons << Qt::LeftButton
            << (Qt::LeftButton | Qt::RightButton)
            << Qt::RightButton
            << Qt::NoButton;
    active << true
           << true
           << false
           << false;
    pressedButtons << Qt::LeftButton
                   << (Qt::LeftButton | Qt::RightButton)
                   << Qt::NoButton // Not the "truth" but filtered according to this handler's acceptedButtons
                   << Qt::NoButton;
    QTest::newRow("Accept Left - Press left, Press Right, Release Left")
            << Qt::MouseButtons(Qt::LeftButton) << buttons << active << pressedButtons << 3;

    buttons.clear();
    active.clear();
    pressedButtons.clear();
    buttons << Qt::LeftButton
            << (Qt::LeftButton | Qt::RightButton)
            << Qt::LeftButton
            << Qt::NoButton;
    active << true
           << true
           << true
           << false;
    pressedButtons << Qt::LeftButton
                   << (Qt::LeftButton | Qt::RightButton)
                   << Qt::LeftButton
                   << Qt::NoButton;
    QTest::newRow("Accept Left|Right - Press left, Press Right, Release Right")
            << (Qt::LeftButton | Qt::RightButton) << buttons << active << pressedButtons << 4;

    buttons.clear();
    active.clear();
    pressedButtons.clear();
    buttons << Qt::RightButton
            << (Qt::LeftButton | Qt::RightButton)
            << Qt::LeftButton
            << Qt::NoButton;
    active << true
           << true
           << false
           << false;
    pressedButtons << Qt::RightButton
                   << (Qt::LeftButton | Qt::RightButton)
                   << Qt::NoButton // Not the "truth" but filtered according to this handler's acceptedButtons
                   << Qt::NoButton;
    QTest::newRow("Accept Right - Press Right, Press Left, Release Right")
            << Qt::MouseButtons(Qt::RightButton) << buttons << active << pressedButtons << 3;
}

void tst_PointHandler::pressedMultipleButtons()
{
    QFETCH(Qt::MouseButtons, accepted);
    QFETCH(QList<Qt::MouseButtons>, buttons);
    QFETCH(QList<bool>, active);
    QFETCH(QList<Qt::MouseButtons>, pressedButtons);
    QFETCH(int, changeCount);

    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "pointTracker.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem *tracker = window->rootObject()->findChild<QQuickItem *>("pointTracker");
    QVERIFY(tracker);
    QQuickPointHandler *handler = window->rootObject()->findChild<QQuickPointHandler *>("pointHandler");
    QVERIFY(handler);

    QSignalSpy activeSpy(handler, SIGNAL(activeChanged()));
    QSignalSpy pointSpy(handler, SIGNAL(pointChanged()));
    handler->setAcceptedButtons(accepted);

    QPoint point(100,100);

    for (int i = 0; i < buttons.count(); ++i) {
        int btns = int(buttons.at(i));
        int release = 0;
        if (i > 0) {
            int lastBtns = int(buttons.at(i - 1));
            release = lastBtns & ~btns;
        }
        if (release)
            QTest::mouseRelease(windowPtr.data(), Qt::MouseButton(release), Qt::NoModifier, point);
        else
            QTest::mousePress(windowPtr.data(), Qt::MouseButton(btns), Qt::NoModifier, point);

        qCDebug(lcPointerTests) << i << ": acceptedButtons" << handler->acceptedButtons()
                                << "; comparing" << handler->point().pressedButtons() << pressedButtons.at(i);
        QCOMPARE(handler->point().pressedButtons(), pressedButtons.at(i));
        QCOMPARE(handler->active(), active.at(i));
    }

    QTest::mousePress(windowPtr.data(), Qt::NoButton, Qt::NoModifier, point);
    QCOMPARE(handler->active(), false);
    QCOMPARE(activeSpy.count(), 2);
    QCOMPARE(pointSpy.count(), changeCount);
}

QTEST_MAIN(tst_PointHandler)

#include "tst_qquickpointhandler.moc"
