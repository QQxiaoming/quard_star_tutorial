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
#include <QtQuick>
#include <QtQuick/private/qquickmousearea_p.h>
#include <QDebug>
#include "../../../auto/shared/util.h"
#include "../../../auto/quick/shared/viewtestutil.h"


class TestView : public QQuickView
{
public:
    void handleEvent(QEvent *ev) { event(ev); }
};


class tst_events : public QQmlDataTest
{
    Q_OBJECT

private slots:
    void mousePressRelease();
    void mouseMove();
    void touchToMousePressRelease();
    void touchToMousePressMove();

public slots:
    void initTestCase() {
        QQmlDataTest::initTestCase();
        window.setBaseSize(QSize(400, 400));
        window.setSource(testFileUrl("mouseevent.qml"));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
    }

private:
    TestView window;
};

void tst_events::mousePressRelease()
{
    QQuickMouseArea *mouseArea = window.rootObject()->findChild<QQuickMouseArea *>("mouseArea");
    QCOMPARE(mouseArea->pressed(), false);

    QBENCHMARK {
        QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(100, 100), Qt::LeftButton, Qt::LeftButton, 0);
        window.handleEvent(&pressEvent);
        QCOMPARE(mouseArea->pressed(), true);
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(100, 100), Qt::LeftButton, Qt::LeftButton, 0);
        window.handleEvent(&releaseEvent);
    }
    QCOMPARE(mouseArea->pressed(), false);
}

void tst_events::mouseMove()
{
    QQuickMouseArea *mouseArea = window.rootObject()->findChild<QQuickMouseArea *>("mouseArea");
    QCOMPARE(mouseArea->pressed(), false);

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(100, 100), Qt::LeftButton, Qt::LeftButton, 0);
    window.handleEvent(&pressEvent);
    QCOMPARE(mouseArea->pressed(), true);
    QMouseEvent moveEvent1(QEvent::MouseMove, QPoint(101, 100), Qt::LeftButton, Qt::LeftButton, 0);
    QMouseEvent moveEvent2(QEvent::MouseMove, QPoint(100, 100), Qt::LeftButton, Qt::LeftButton, 0);
    QBENCHMARK {
        window.handleEvent(&moveEvent1);
        window.handleEvent(&moveEvent2);
    }
    QCOMPARE(mouseArea->pressed(), true);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(100, 100), Qt::LeftButton, Qt::LeftButton, 0);
    window.handleEvent(&releaseEvent);
    QCOMPARE(mouseArea->pressed(), false);
}

void tst_events::touchToMousePressRelease()
{
    QQuickMouseArea *mouseArea = window.rootObject()->findChild<QQuickMouseArea *>("mouseArea");
    QCOMPARE(mouseArea->pressed(), false);

    auto device = QTest::createTouchDevice();
    auto p = QPoint(80, 80);

    QBENCHMARK {
        QTest::touchEvent(&window, device).press(0, p, &window).commit();
        QCOMPARE(mouseArea->pressed(), true);
        QTest::touchEvent(&window, device).release(0, p, &window).commit();
    }
    QCOMPARE(mouseArea->pressed(), false);
}

void tst_events::touchToMousePressMove()
{
    QQuickMouseArea *mouseArea = window.rootObject()->findChild<QQuickMouseArea *>("mouseArea");
    QCOMPARE(mouseArea->pressed(), false);

    auto device = QTest::createTouchDevice();
    auto p = QPoint(80, 80);
    auto p2 = QPoint(81, 80);

    QTest::touchEvent(&window, device).press(0, p, &window).commit();
    QQuickTouchUtils::flush(&window);
    QCOMPARE(mouseArea->pressed(), true);

    QBENCHMARK {
        QTest::touchEvent(&window, device).move(0, p, &window).commit();
        QCOMPARE(mouseArea->pressed(), true);
        QTest::touchEvent(&window, device).move(0, p2, &window).commit();
    }
    QCOMPARE(mouseArea->pressed(), true);
    QTest::touchEvent(&window, device).release(0, p, &window).commit();
    QQuickTouchUtils::flush(&window);
    QCOMPARE(mouseArea->pressed(), false);
}

QTEST_MAIN(tst_events)
#include "tst_events.moc"
