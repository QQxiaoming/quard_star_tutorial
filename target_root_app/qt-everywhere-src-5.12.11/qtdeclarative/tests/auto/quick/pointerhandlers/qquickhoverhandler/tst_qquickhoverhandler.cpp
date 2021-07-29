/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/private/qquickhoverhandler_p.h>
#include <QtQuick/private/qquickmousearea_p.h>
#include <qpa/qwindowsysteminterface.h>

#include <private/qquickwindow_p.h>

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlproperty.h>

#include "../../../shared/util.h"
#include "../../shared/viewtestutil.h"

Q_LOGGING_CATEGORY(lcPointerTests, "qt.quick.pointer.tests")

static bool isPlatformWayland()
{
    return !QGuiApplication::platformName().compare(QLatin1String("wayland"), Qt::CaseInsensitive);
}

class tst_HoverHandler : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_HoverHandler()
    {}

private slots:
    void hoverHandlerAndUnderlyingHoverHandler();
    void mouseAreaAndUnderlyingHoverHandler();
    void hoverHandlerAndUnderlyingMouseArea();
    void movingItemWithHoverHandler();

private:
    void createView(QScopedPointer<QQuickView> &window, const char *fileName);
};

void tst_HoverHandler::createView(QScopedPointer<QQuickView> &window, const char *fileName)
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

void tst_HoverHandler::hoverHandlerAndUnderlyingHoverHandler()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * topSidebar = window->rootObject()->findChild<QQuickItem *>("topSidebar");
    QVERIFY(topSidebar);
    QQuickItem * button = topSidebar->findChild<QQuickItem *>("buttonWithHH");
    QVERIFY(button);
    QQuickHoverHandler *topSidebarHH = topSidebar->findChild<QQuickHoverHandler *>("topSidebarHH");
    QVERIFY(topSidebarHH);
    QQuickHoverHandler *buttonHH = button->findChild<QQuickHoverHandler *>("buttonHH");
    QVERIFY(buttonHH);

    QPoint buttonCenter(button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QPoint rightOfButton(button->mapToScene(QPointF(button->width() + 2, button->height() / 2)).toPoint());
    QPoint outOfSidebar(topSidebar->mapToScene(QPointF(topSidebar->width() + 2, topSidebar->height() / 2)).toPoint());
    QSignalSpy sidebarHoveredSpy(topSidebarHH, SIGNAL(hoveredChanged()));
    QSignalSpy buttonHoveredSpy(buttonHH, SIGNAL(hoveredChanged()));

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(topSidebarHH->isHovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 0);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);

    QTest::mouseMove(window, buttonCenter);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), true);
    QCOMPARE(buttonHoveredSpy.count(), 1);

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(topSidebarHH->isHovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 2);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
}

void tst_HoverHandler::mouseAreaAndUnderlyingHoverHandler()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * topSidebar = window->rootObject()->findChild<QQuickItem *>("topSidebar");
    QVERIFY(topSidebar);
    QQuickMouseArea * buttonMA = topSidebar->findChild<QQuickMouseArea *>("buttonMA");
    QVERIFY(buttonMA);
    QQuickHoverHandler *topSidebarHH = topSidebar->findChild<QQuickHoverHandler *>("topSidebarHH");
    QVERIFY(topSidebarHH);

    QPoint buttonCenter(buttonMA->mapToScene(QPointF(buttonMA->width() / 2, buttonMA->height() / 2)).toPoint());
    QPoint rightOfButton(buttonMA->mapToScene(QPointF(buttonMA->width() + 2, buttonMA->height() / 2)).toPoint());
    QPoint outOfSidebar(topSidebar->mapToScene(QPointF(topSidebar->width() + 2, topSidebar->height() / 2)).toPoint());
    QSignalSpy sidebarHoveredSpy(topSidebarHH, SIGNAL(hoveredChanged()));
    QSignalSpy buttonHoveredSpy(buttonMA, SIGNAL(hoveredChanged()));

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(topSidebarHH->isHovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 0);
    QCOMPARE(buttonMA->hovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonMA->hovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);

    QTest::mouseMove(window, buttonCenter);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonMA->hovered(), true);
    QCOMPARE(buttonHoveredSpy.count(), 1);

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonMA->hovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(topSidebarHH->isHovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 2);
    QCOMPARE(buttonMA->hovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
}

void tst_HoverHandler::hoverHandlerAndUnderlyingMouseArea()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * bottomSidebar = window->rootObject()->findChild<QQuickItem *>("bottomSidebar");
    QVERIFY(bottomSidebar);
    QQuickMouseArea *bottomSidebarMA = bottomSidebar->findChild<QQuickMouseArea *>("bottomSidebarMA");
    QVERIFY(bottomSidebarMA);
    QQuickItem * button = bottomSidebar->findChild<QQuickItem *>("buttonWithHH");
    QVERIFY(button);
    QQuickHoverHandler *buttonHH = button->findChild<QQuickHoverHandler *>("buttonHH");
    QVERIFY(buttonHH);

    QPoint buttonCenter(button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QPoint rightOfButton(button->mapToScene(QPointF(button->width() + 2, button->height() / 2)).toPoint());
    QPoint outOfSidebar(bottomSidebar->mapToScene(QPointF(bottomSidebar->width() + 2, bottomSidebar->height() / 2)).toPoint());
    QSignalSpy sidebarHoveredSpy(bottomSidebarMA, SIGNAL(hoveredChanged()));
    QSignalSpy buttonHoveredSpy(buttonHH, SIGNAL(hoveredChanged()));

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(bottomSidebarMA->hovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 0);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(bottomSidebarMA->hovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);

    QTest::mouseMove(window, buttonCenter);
    QCOMPARE(bottomSidebarMA->hovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 2);
    QCOMPARE(buttonHH->isHovered(), true);
    QCOMPARE(buttonHoveredSpy.count(), 1);

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(bottomSidebarMA->hovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 3);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(bottomSidebarMA->hovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 4);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
}

void tst_HoverHandler::movingItemWithHoverHandler()
{
   if (isPlatformWayland())
        QSKIP("Wayland: QCursor::setPos() doesn't work.");

    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * paddle = window->rootObject()->findChild<QQuickItem *>("paddle");
    QVERIFY(paddle);
    QQuickHoverHandler *paddleHH = paddle->findChild<QQuickHoverHandler *>("paddleHH");
    QVERIFY(paddleHH);

    // Find the global coordinate of the paddle
    const QPoint p(paddle->mapToScene(paddle->clipRect().center()).toPoint());
    const QPoint paddlePos = window->mapToGlobal(p);

    // Now hide the window, put the cursor where the paddle was and show it again
    window->hide();
    QTRY_COMPARE(window->isVisible(), false);
    QCursor::setPos(paddlePos);
    window->show();
    QTest::qWaitForWindowExposed(window);

    QTRY_COMPARE(paddleHH->isHovered(), true);

    paddle->setX(100);
    QTRY_COMPARE(paddleHH->isHovered(), false);

    paddle->setX(p.x());
    QTRY_COMPARE(paddleHH->isHovered(), true);

    paddle->setX(540);
    QTRY_COMPARE(paddleHH->isHovered(), false);
}

QTEST_MAIN(tst_HoverHandler)

#include "tst_qquickhoverhandler.moc"
