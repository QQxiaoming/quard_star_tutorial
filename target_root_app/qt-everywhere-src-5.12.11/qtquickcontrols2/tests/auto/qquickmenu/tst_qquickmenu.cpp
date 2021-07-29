/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>
#include <QtGui/qcursor.h>
#include <QtGui/qstylehints.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlcontext.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/private/qquickitem_p.h>
#include "../shared/util.h"
#include "../shared/visualtestutil.h"
#include "../shared/qtest_quickcontrols.h"

#include <QtQuickTemplates2/private/qquickaction_p.h>
#include <QtQuickTemplates2/private/qquickapplicationwindow_p.h>
#include <QtQuickTemplates2/private/qquickoverlay_p.h>
#include <QtQuickTemplates2/private/qquickbutton_p.h>
#include <QtQuickTemplates2/private/qquickmenu_p.h>
#include <QtQuickTemplates2/private/qquickmenuitem_p.h>
#include <QtQuickTemplates2/private/qquickmenuseparator_p.h>

using namespace QQuickVisualTestUtil;

class tst_QQuickMenu : public QQmlDataTest
{
    Q_OBJECT

public:

private slots:
    void defaults();
    void count();
    void mouse();
    void pressAndHold();
    void contextMenuKeyboard();
    void disabledMenuItemKeyNavigation();
    void mnemonics();
    void menuButton();
    void addItem();
    void menuSeparator();
    void repeater();
    void order();
    void popup();
    void actions();
    void removeTakeItem();
    void subMenuMouse_data();
    void subMenuMouse();
    void subMenuDisabledMouse_data();
    void subMenuDisabledMouse();
    void subMenuKeyboard_data();
    void subMenuKeyboard();
    void subMenuDisabledKeyboard_data();
    void subMenuDisabledKeyboard();
    void subMenuPosition_data();
    void subMenuPosition();
    void addRemoveSubMenus();
    void scrollable_data();
    void scrollable();
    void disableWhenTriggered_data();
    void disableWhenTriggered();
    void menuItemWidth_data();
    void menuItemWidth();
    void menuItemWidthAfterMenuWidthChanged_data();
    void menuItemWidthAfterMenuWidthChanged();
    void menuItemWidthAfterImplicitWidthChanged_data();
    void menuItemWidthAfterImplicitWidthChanged();
    void menuItemWidthAfterRetranslate();
    void giveMenuItemFocusOnButtonPress();
};

void tst_QQuickMenu::defaults()
{
    QQuickApplicationHelper helper(this, QLatin1String("applicationwindow.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickMenu *emptyMenu = helper.appWindow->property("emptyMenu").value<QQuickMenu*>();
    QCOMPARE(emptyMenu->isVisible(), false);
    QCOMPARE(emptyMenu->currentIndex(), -1);
    QCOMPARE(emptyMenu->contentItem()->property("currentIndex"), QVariant(-1));
    QCOMPARE(emptyMenu->count(), 0);
}

void tst_QQuickMenu::count()
{
    QQuickApplicationHelper helper(this, QLatin1String("applicationwindow.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickMenu *menu = helper.window->property("emptyMenu").value<QQuickMenu*>();
    QVERIFY(menu);

    QSignalSpy countSpy(menu, &QQuickMenu::countChanged);
    QVERIFY(countSpy.isValid());

    menu->addItem(new QQuickItem);
    QCOMPARE(menu->count(), 1);
    QCOMPARE(countSpy.count(), 1);

    menu->insertItem(0, new QQuickItem);
    QCOMPARE(menu->count(), 2);
    QCOMPARE(countSpy.count(), 2);

    menu->removeItem(menu->itemAt(1));
    QCOMPARE(menu->count(), 1);
    QCOMPARE(countSpy.count(), 3);

    QScopedPointer<QQuickItem> item(menu->takeItem(0));
    QVERIFY(item);
    QCOMPARE(menu->count(), 0);
    QCOMPARE(countSpy.count(), 4);
}

void tst_QQuickMenu::mouse()
{
    if ((QGuiApplication::platformName() == QLatin1String("offscreen"))
        || (QGuiApplication::platformName() == QLatin1String("minimal")))
        QSKIP("Mouse hovering not functional on offscreen/minimal platforms");

    QQuickApplicationHelper helper(this, QLatin1String("applicationwindow.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    menu->open();
    QVERIFY(menu->isVisible());
    QVERIFY(window->overlay()->childItems().contains(menu->contentItem()->parentItem()));
    QTRY_VERIFY(menu->isOpened());

    QQuickItem *firstItem = menu->itemAt(0);
    QSignalSpy clickedSpy(firstItem, SIGNAL(clicked()));
    QSignalSpy triggeredSpy(firstItem, SIGNAL(triggered()));
    QSignalSpy visibleSpy(menu, SIGNAL(visibleChanged()));

    // Ensure that presses cause the current index to change,
    // so that the highlight acts as a way of illustrating press state.
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier,
        QPoint(menu->leftPadding() + firstItem->width() / 2, menu->topPadding() + firstItem->height() / 2));
    QVERIFY(firstItem->hasActiveFocus());
    QCOMPARE(menu->currentIndex(), 0);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(0));
    QVERIFY(menu->isVisible());

    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier,
        QPoint(menu->leftPadding() + firstItem->width() / 2, menu->topPadding() + firstItem->height() / 2));
    QCOMPARE(clickedSpy.count(), 1);
    QCOMPARE(triggeredSpy.count(), 1);
    QTRY_COMPARE(visibleSpy.count(), 1);
    QVERIFY(!menu->isVisible());
    QVERIFY(!window->overlay()->childItems().contains(menu->contentItem()));
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(-1));

    menu->open();
    QCOMPARE(visibleSpy.count(), 2);
    QVERIFY(menu->isVisible());
    QVERIFY(window->overlay()->childItems().contains(menu->contentItem()->parentItem()));
    QTRY_VERIFY(menu->isOpened());

    // Ensure that we have enough space to click outside of the menu.
    QVERIFY(window->width() > menu->contentItem()->width());
    QVERIFY(window->height() > menu->contentItem()->height());
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        QPoint(menu->contentItem()->width() + 1, menu->contentItem()->height() + 1));
    QTRY_COMPARE(visibleSpy.count(), 3);
    QVERIFY(!menu->isVisible());
    QVERIFY(!window->overlay()->childItems().contains(menu->contentItem()->parentItem()));

    menu->open();
    QCOMPARE(visibleSpy.count(), 4);
    QVERIFY(menu->isVisible());
    QVERIFY(window->overlay()->childItems().contains(menu->contentItem()->parentItem()));
    QTRY_VERIFY(menu->isOpened());

    // Hover-highlight through the menu items one by one
    QQuickItem *prevHoverItem = nullptr;
    QQuickItem *listView = menu->contentItem();
    for (int y = menu->topPadding(); y < listView->height(); ++y) {
        QQuickItem *hoverItem = nullptr;
        QVERIFY(QMetaObject::invokeMethod(listView, "itemAt", Q_RETURN_ARG(QQuickItem *, hoverItem), Q_ARG(qreal, 0), Q_ARG(qreal, listView->property("contentY").toReal() + y)));
        if (!hoverItem || !hoverItem->isVisible() || hoverItem == prevHoverItem)
            continue;
        QTest::mouseMove(window, QPoint(
            menu->leftPadding() + hoverItem->x() + hoverItem->width() / 2,
            menu->topPadding() + hoverItem->y() + hoverItem->height() / 2));
        QTRY_VERIFY(hoverItem->property("highlighted").toBool());
        if (prevHoverItem)
            QVERIFY(!prevHoverItem->property("highlighted").toBool());
        prevHoverItem = hoverItem;
    }

    // Try pressing within the menu and releasing outside of it; it should close.
    // TODO: won't work until QQuickPopup::releasedOutside() actually gets emitted
//    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(firstItem->width() / 2, firstItem->height() / 2));
//    QVERIFY(firstItem->hasActiveFocus());
//    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(0));
//    QVERIFY(menu->isVisible());
//    QCOMPARE(triggeredSpy.count(), 1);

//    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(menu->contentItem()->width() + 1, firstItem->height() / 2));
//    QCOMPARE(clickedSpy.count(), 1);
//    QCOMPARE(triggeredSpy.count(), 1);
//    QCOMPARE(visibleSpy.count(), 5);
//    QVERIFY(!menu->isVisible());
//    QVERIFY(!window->overlay()->childItems().contains(menu->contentItem()));
//    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(-1));
}

void tst_QQuickMenu::pressAndHold()
{
    QQuickApplicationHelper helper(this, QLatin1String("pressAndHold.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QVERIFY(menu);

    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QTRY_VERIFY(menu->isVisible());

    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QVERIFY(menu->isVisible());

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QTRY_VERIFY(!menu->isVisible());
}

void tst_QQuickMenu::contextMenuKeyboard()
{
    if (QGuiApplication::styleHints()->tabFocusBehavior() != Qt::TabFocusAllControls)
        QSKIP("This platform only allows tab focus for text controls");

    QQuickApplicationHelper helper(this, QLatin1String("applicationwindow.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));
    QVERIFY(QGuiApplication::focusWindow() == window);

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(-1));

    QQuickMenuItem *firstItem = qobject_cast<QQuickMenuItem *>(menu->itemAt(0));
    QVERIFY(firstItem);
    QSignalSpy visibleSpy(menu, SIGNAL(visibleChanged()));

    menu->setFocus(true);
    menu->open();
    QCOMPARE(visibleSpy.count(), 1);
    QVERIFY(menu->isVisible());
    QVERIFY(window->overlay()->childItems().contains(menu->contentItem()->parentItem()));
    QTRY_VERIFY(menu->isOpened());
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->property("highlighted").toBool());
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(-1));

    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(firstItem->hasActiveFocus());
    QVERIFY(firstItem->hasVisualFocus());
    QVERIFY(firstItem->isHighlighted());
    QCOMPARE(firstItem->focusReason(), Qt::TabFocusReason);
    QCOMPARE(menu->currentIndex(), 0);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(0));

    QQuickMenuItem *secondItem = qobject_cast<QQuickMenuItem *>(menu->itemAt(1));
    QVERIFY(secondItem);
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->hasVisualFocus());
    QVERIFY(!firstItem->isHighlighted());
    QVERIFY(secondItem->hasActiveFocus());
    QVERIFY(secondItem->hasVisualFocus());
    QVERIFY(secondItem->isHighlighted());
    QCOMPARE(secondItem->focusReason(), Qt::TabFocusReason);
    QCOMPARE(menu->currentIndex(), 1);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(1));

    QSignalSpy secondTriggeredSpy(secondItem, SIGNAL(triggered()));
    QTest::keyClick(window, Qt::Key_Space);
    QCOMPARE(secondTriggeredSpy.count(), 1);
    QTRY_COMPARE(visibleSpy.count(), 2);
    QVERIFY(!menu->isVisible());
    QVERIFY(!window->overlay()->childItems().contains(menu->contentItem()));
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->hasVisualFocus());
    QVERIFY(!firstItem->isHighlighted());
    QVERIFY(!secondItem->hasActiveFocus());
    QVERIFY(!secondItem->hasVisualFocus());
    QVERIFY(!secondItem->isHighlighted());
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(-1));

    // Enter/return should also work.
    // Open the menu.
    menu->open();
    QCOMPARE(visibleSpy.count(), 3);
    QVERIFY(menu->isVisible());
    QTRY_VERIFY(menu->isOpened());
    // Give the first item focus.
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(firstItem->hasActiveFocus());
    QVERIFY(firstItem->hasVisualFocus());
    QVERIFY(firstItem->isHighlighted());
    QCOMPARE(firstItem->focusReason(), Qt::TabFocusReason);
    QCOMPARE(menu->currentIndex(), 0);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(0));
    // Press enter.
    QSignalSpy firstTriggeredSpy(firstItem, SIGNAL(triggered()));
    QTest::keyClick(window, Qt::Key_Return);
    QCOMPARE(firstTriggeredSpy.count(), 1);
    QTRY_COMPARE(visibleSpy.count(), 4);
    QVERIFY(!menu->isVisible());
    QVERIFY(!window->overlay()->childItems().contains(menu->contentItem()));
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->hasVisualFocus());
    QVERIFY(!firstItem->isHighlighted());
    QVERIFY(!secondItem->hasActiveFocus());
    QVERIFY(!secondItem->hasVisualFocus());
    QVERIFY(!secondItem->isHighlighted());
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(-1));

    menu->open();
    QCOMPARE(visibleSpy.count(), 5);
    QVERIFY(menu->isVisible());
    QVERIFY(window->overlay()->childItems().contains(menu->contentItem()->parentItem()));
    QTRY_VERIFY(menu->isOpened());
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->hasVisualFocus());
    QVERIFY(!firstItem->isHighlighted());
    QVERIFY(!secondItem->hasActiveFocus());
    QVERIFY(!secondItem->hasVisualFocus());
    QVERIFY(!secondItem->isHighlighted());
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(-1));

    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(firstItem->hasActiveFocus());
    QVERIFY(firstItem->hasVisualFocus());
    QVERIFY(firstItem->isHighlighted());
    QCOMPARE(firstItem->focusReason(), Qt::TabFocusReason);

    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(secondItem->hasActiveFocus());
    QVERIFY(secondItem->hasVisualFocus());
    QVERIFY(secondItem->isHighlighted());
    QCOMPARE(secondItem->focusReason(), Qt::TabFocusReason);

    QTest::keyClick(window, Qt::Key_Down);
    QQuickMenuItem *thirdItem = qobject_cast<QQuickMenuItem *>(menu->itemAt(2));
    QVERIFY(thirdItem);
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->hasVisualFocus());
    QVERIFY(!firstItem->isHighlighted());
    QVERIFY(!secondItem->hasActiveFocus());
    QVERIFY(!secondItem->hasVisualFocus());
    QVERIFY(!secondItem->isHighlighted());
    QVERIFY(thirdItem->hasActiveFocus());
    QVERIFY(thirdItem->hasVisualFocus());
    QVERIFY(thirdItem->isHighlighted());
    QCOMPARE(thirdItem->focusReason(), Qt::TabFocusReason);

    // Key navigation shouldn't wrap by default.
    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->hasVisualFocus());
    QVERIFY(!firstItem->isHighlighted());
    QVERIFY(!secondItem->hasActiveFocus());
    QVERIFY(!secondItem->hasVisualFocus());
    QVERIFY(!secondItem->isHighlighted());
    QVERIFY(thirdItem->hasActiveFocus());
    QVERIFY(thirdItem->hasVisualFocus());
    QVERIFY(thirdItem->isHighlighted());
    QCOMPARE(thirdItem->focusReason(), Qt::TabFocusReason);

    QTest::keyClick(window, Qt::Key_Up);
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->hasVisualFocus());
    QVERIFY(!firstItem->isHighlighted());
    QVERIFY(secondItem->hasActiveFocus());
    QVERIFY(secondItem->hasVisualFocus());
    QVERIFY(secondItem->isHighlighted());
    QCOMPARE(secondItem->focusReason(), Qt::BacktabFocusReason);
    QVERIFY(!thirdItem->hasActiveFocus());
    QVERIFY(!thirdItem->hasVisualFocus());
    QVERIFY(!thirdItem->isHighlighted());

    QTest::keyClick(window, Qt::Key_Backtab);
    QVERIFY(firstItem->hasActiveFocus());
    QVERIFY(firstItem->hasVisualFocus());
    QVERIFY(firstItem->isHighlighted());
    QCOMPARE(firstItem->focusReason(), Qt::BacktabFocusReason);
    QVERIFY(!secondItem->hasActiveFocus());
    QVERIFY(!secondItem->hasVisualFocus());
    QVERIFY(!secondItem->isHighlighted());
    QVERIFY(!thirdItem->hasActiveFocus());
    QVERIFY(!thirdItem->hasVisualFocus());
    QVERIFY(!thirdItem->isHighlighted());

    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_COMPARE(visibleSpy.count(), 6);
    QVERIFY(!menu->isVisible());
}

// QTBUG-70181
void tst_QQuickMenu::disabledMenuItemKeyNavigation()
{
    if (QGuiApplication::styleHints()->tabFocusBehavior() != Qt::TabFocusAllControls)
        QSKIP("This platform only allows tab focus for text controls");

    QQuickApplicationHelper helper(this, QLatin1String("disabledMenuItemKeyNavigation.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));
    QVERIFY(QGuiApplication::focusWindow() == window);

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex"), QVariant(-1));

    QQuickMenuItem *firstItem = qobject_cast<QQuickMenuItem *>(menu->itemAt(0));
    QVERIFY(firstItem);

    QQuickMenuItem *secondItem = qobject_cast<QQuickMenuItem *>(menu->itemAt(1));
    QVERIFY(secondItem);

    QQuickMenuItem *thirdItem = qobject_cast<QQuickMenuItem *>(menu->itemAt(2));
    QVERIFY(thirdItem);

    menu->setFocus(true);
    menu->open();
    QVERIFY(menu->isVisible());
    QTRY_VERIFY(menu->isOpened());
    QVERIFY(!firstItem->hasActiveFocus());
    QVERIFY(!firstItem->property("highlighted").toBool());
    QCOMPARE(menu->currentIndex(), -1);

    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(firstItem->hasActiveFocus());
    QVERIFY(firstItem->hasVisualFocus());
    QVERIFY(firstItem->isHighlighted());
    QCOMPARE(firstItem->focusReason(), Qt::TabFocusReason);
    QCOMPARE(menu->currentIndex(), 0);

    // Shouldn't be possible to give focus to a disabled menu item.
    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(!secondItem->hasActiveFocus());
    QVERIFY(!secondItem->hasVisualFocus());
    QVERIFY(!secondItem->isHighlighted());
    QVERIFY(thirdItem->hasActiveFocus());
    QVERIFY(thirdItem->hasVisualFocus());
    QVERIFY(thirdItem->isHighlighted());
    QCOMPARE(thirdItem->focusReason(), Qt::TabFocusReason);

    QTest::keyClick(window, Qt::Key_Up);
    QVERIFY(firstItem->hasActiveFocus());
    QVERIFY(firstItem->hasVisualFocus());
    QVERIFY(firstItem->isHighlighted());
    QCOMPARE(firstItem->focusReason(), Qt::BacktabFocusReason);

    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!menu->isVisible());
}

void tst_QQuickMenu::mnemonics()
{
#ifdef Q_OS_MACOS
    QSKIP("Mnemonics are not used on macOS");
#endif

    QQuickApplicationHelper helper(this, QLatin1String("mnemonics.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QQuickAction *action = window->property("action").value<QQuickAction *>();
    QQuickMenuItem *menuItem = window->property("menuItem").value<QQuickMenuItem *>();
    QQuickMenu *subMenu = window->property("subMenu").value<QQuickMenu *>();
    QQuickMenuItem *subMenuItem = window->property("subMenuItem").value<QQuickMenuItem *>();
    QVERIFY(menu && action && menuItem && subMenu && subMenuItem);

    menu->open();
    QTRY_VERIFY(menu->isOpened());

    QSignalSpy actionSpy(action, &QQuickAction::triggered);
    QVERIFY(actionSpy.isValid());
    QTest::keyClick(window, Qt::Key_A, Qt::AltModifier); // "&Action"
    QCOMPARE(actionSpy.count(), 1);

    menu->open();
    QTRY_VERIFY(menu->isOpened());

    QSignalSpy menuItemSpy(menuItem, &QQuickMenuItem::triggered);
    QVERIFY(menuItemSpy.isValid());
    QTest::keyClick(window, Qt::Key_I, Qt::AltModifier); // "Menu &Item"
    QCOMPARE(menuItemSpy.count(), 1);

    menu->open();
    QTRY_VERIFY(menu->isOpened());

    QTest::keyClick(window, Qt::Key_M, Qt::AltModifier); // "Sub &Menu"
    QTRY_VERIFY(subMenu->isOpened());

    QSignalSpy subMenuItemSpy(subMenuItem, &QQuickMenuItem::triggered);
    QVERIFY(subMenuItemSpy.isValid());
    QTest::keyClick(window, Qt::Key_S, Qt::AltModifier); // "&Sub Menu Item"
    QCOMPARE(subMenuItemSpy.count(), 1);
}

void tst_QQuickMenu::menuButton()
{
    if (QGuiApplication::styleHints()->tabFocusBehavior() != Qt::TabFocusAllControls)
        QSKIP("This platform only allows tab focus for text controls");

    QQuickApplicationHelper helper(this, QLatin1String("applicationwindow.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));
    QVERIFY(QGuiApplication::focusWindow() == window);

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    QQuickButton *menuButton = window->property("menuButton").value<QQuickButton*>();
    QSignalSpy visibleSpy(menu, SIGNAL(visibleChanged()));

    menuButton->setVisible(true);
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        menuButton->mapToScene(QPointF(menuButton->width() / 2, menuButton->height() / 2)).toPoint());
    QCOMPARE(visibleSpy.count(), 1);
    QVERIFY(menu->isVisible());
    QTRY_VERIFY(menu->isOpened());

    QTest::keyClick(window, Qt::Key_Tab);
    QQuickItem *firstItem = menu->itemAt(0);
    QVERIFY(firstItem->hasActiveFocus());
}

void tst_QQuickMenu::addItem()
{
    QQuickApplicationHelper helper(this, QLatin1String("addItem.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    QVERIFY(menu);
    menu->open();
    QVERIFY(menu->isVisible());

    QQuickItem *menuItem = menu->itemAt(0);
    QVERIFY(menuItem);
    QTRY_VERIFY(!QQuickItemPrivate::get(menuItem)->culled); // QTBUG-53262

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        menuItem->mapToScene(QPointF(menuItem->width() / 2, menuItem->height() / 2)).toPoint());
    QTRY_VERIFY(!menu->isVisible());
}

void tst_QQuickMenu::menuSeparator()
{
    QQuickApplicationHelper helper(this, QLatin1String("menuSeparator.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    QVERIFY(menu);
    menu->open();
    QVERIFY(menu->isVisible());

    QQuickMenuItem *newMenuItem = qobject_cast<QQuickMenuItem*>(menu->itemAt(0));
    QVERIFY(newMenuItem);
    QCOMPARE(newMenuItem->text(), QStringLiteral("New"));

    QQuickMenuSeparator *menuSeparator = qobject_cast<QQuickMenuSeparator*>(menu->itemAt(1));
    QVERIFY(menuSeparator);

    QQuickMenuItem *saveMenuItem = qobject_cast<QQuickMenuItem*>(menu->itemAt(2));
    QVERIFY(saveMenuItem);
    QCOMPARE(saveMenuItem->text(), QStringLiteral("Save"));
    QTRY_VERIFY(!QQuickItemPrivate::get(saveMenuItem)->culled); // QTBUG-53262

    // Clicking on items should still close the menu.
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        newMenuItem->mapToScene(QPointF(newMenuItem->width() / 2, newMenuItem->height() / 2)).toPoint());
    QTRY_VERIFY(!menu->isVisible());

    menu->open();
    QVERIFY(menu->isVisible());

    // Clicking on a separator shouldn't close the menu.
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        menuSeparator->mapToScene(QPointF(menuSeparator->width() / 2, menuSeparator->height() / 2)).toPoint());
    QVERIFY(menu->isVisible());

    // Clicking on items should still close the menu.
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        saveMenuItem->mapToScene(QPointF(saveMenuItem->width() / 2, saveMenuItem->height() / 2)).toPoint());
    QTRY_VERIFY(!menu->isVisible());

    moveMouseAway(window);

    menu->open();
    QVERIFY(menu->isVisible());
    QTRY_VERIFY(menu->isOpened());

    // Key navigation skips separators
    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(newMenuItem->hasActiveFocus());
    QVERIFY(newMenuItem->hasVisualFocus());
    QCOMPARE(newMenuItem->focusReason(), Qt::TabFocusReason);

    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(saveMenuItem->hasActiveFocus());
    QVERIFY(saveMenuItem->hasVisualFocus());
    QCOMPARE(saveMenuItem->focusReason(), Qt::TabFocusReason);

    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(saveMenuItem->hasActiveFocus());
    QVERIFY(saveMenuItem->hasVisualFocus());
    QCOMPARE(saveMenuItem->focusReason(), Qt::TabFocusReason);

    QTest::keyClick(window, Qt::Key_Up);
    QVERIFY(newMenuItem->hasActiveFocus());
    QVERIFY(newMenuItem->hasVisualFocus());
    QCOMPARE(newMenuItem->focusReason(), Qt::BacktabFocusReason);

    QTest::keyClick(window, Qt::Key_Up);
    QVERIFY(newMenuItem->hasActiveFocus());
    QVERIFY(newMenuItem->hasVisualFocus());
    QCOMPARE(newMenuItem->focusReason(), Qt::BacktabFocusReason);
}

void tst_QQuickMenu::repeater()
{
    QQuickApplicationHelper helper(this, QLatin1String("repeater.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    QVERIFY(menu);
    menu->open();
    QVERIFY(menu->isVisible());

    QObject *repeater = window->property("repeater").value<QObject*>();
    QVERIFY(repeater);

    int count = repeater->property("count").toInt();
    QCOMPARE(count, 5);

    for (int i = 0; i < count; ++i) {
        QQuickItem *item = menu->itemAt(i);
        QVERIFY(item);
        QCOMPARE(item->property("idx").toInt(), i);

        QQuickItem *repeaterItem = nullptr;
        QVERIFY(QMetaObject::invokeMethod(repeater, "itemAt", Q_RETURN_ARG(QQuickItem*, repeaterItem), Q_ARG(int, i)));
        QCOMPARE(item, repeaterItem);
    }

    repeater->setProperty("model", 3);

    count = repeater->property("count").toInt();
    QCOMPARE(count, 3);

    for (int i = 0; i < count; ++i) {
        QQuickItem *item = menu->itemAt(i);
        QVERIFY(item);
        QCOMPARE(item->property("idx").toInt(), i);

        QQuickItem *repeaterItem = nullptr;
        QVERIFY(QMetaObject::invokeMethod(repeater, "itemAt", Q_RETURN_ARG(QQuickItem*, repeaterItem), Q_ARG(int, i)));
        QCOMPARE(item, repeaterItem);
    }
}

void tst_QQuickMenu::order()
{
    QQuickApplicationHelper helper(this, QLatin1String("order.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    QVERIFY(menu);
    menu->open();
    QVERIFY(menu->isVisible());

    const QStringList texts = {"dynamic_0", "static_1", "repeated_2", "repeated_3", "static_4", "dynamic_5", "dynamic_6"};

    for (int i = 0; i < texts.count(); ++i) {
        QQuickItem *item = menu->itemAt(i);
        QVERIFY(item);
        QCOMPARE(item->property("text").toString(), texts.at(i));
    }
}

void tst_QQuickMenu::popup()
{
    QQuickApplicationHelper helper(this, QLatin1String("popup.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QVERIFY(menu);

    QQuickMenuItem *menuItem1 = window->property("menuItem1").value<QQuickMenuItem *>();
    QVERIFY(menuItem1);

    QQuickMenuItem *menuItem2 = window->property("menuItem2").value<QQuickMenuItem *>();
    QVERIFY(menuItem2);

    QQuickMenuItem *menuItem3 = window->property("menuItem3").value<QQuickMenuItem *>();
    QVERIFY(menuItem3);

    QQuickItem *button = window->property("button").value<QQuickItem *>();
    QVERIFY(button);

#if QT_CONFIG(cursor)
    QPoint oldCursorPos = QCursor::pos();
    QPoint cursorPos = window->mapToGlobal(QPoint(11, 22));
    QCursor::setPos(cursorPos);
    QTRY_COMPARE(QCursor::pos(), cursorPos);

    QVERIFY(QMetaObject::invokeMethod(window, "popupAtCursor"));
    QCOMPARE(menu->parentItem(), window->contentItem());
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), -1);
    const qreal elevenOrLeftMargin = qMax(qreal(11), menu->leftMargin());
    const qreal twentyTwoOrTopMargin = qMax(qreal(22), menu->topMargin());
    // If the Menu has large margins, it may be moved to stay within them.
    // QTBUG-75503: QTRY_COMPARE doesn't use qFuzzyCompare() in all cases,
    // meaning a lot of these comparisons could trigger a 10 second wait;
    // use QTRY_VERIFY and qFuzzyCompare instead.
    QTRY_VERIFY(qFuzzyCompare(menu->x(), elevenOrLeftMargin));
    QTRY_VERIFY(qFuzzyCompare(menu->y(), twentyTwoOrTopMargin));
    menu->close();

    QVERIFY(QMetaObject::invokeMethod(window, "popupAtPos", Q_ARG(QVariant, QPointF(33, 44))));
    QCOMPARE(menu->parentItem(), window->contentItem());
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), -1);
    QTRY_VERIFY(qFuzzyCompare(menu->x(), 33));
    QTRY_VERIFY(qFuzzyCompare(menu->y(), 44));
    menu->close();

    QVERIFY(QMetaObject::invokeMethod(window, "popupAtCoord", Q_ARG(QVariant, 55), Q_ARG(QVariant, 66)));
    QCOMPARE(menu->parentItem(), window->contentItem());
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), -1);
    QTRY_VERIFY(qFuzzyCompare(menu->x(), 55));
    QTRY_VERIFY(qFuzzyCompare(menu->y(), 66));
    menu->close();

    menu->setParentItem(nullptr);
    QVERIFY(QMetaObject::invokeMethod(window, "popupAtParentCursor", Q_ARG(QVariant, QVariant::fromValue(button))));
    QCOMPARE(menu->parentItem(), button);
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), -1);
    QTRY_VERIFY(qFuzzyCompare(menu->x(), button->mapFromScene(QPointF(elevenOrLeftMargin, twentyTwoOrTopMargin)).x()));
    QTRY_VERIFY(qFuzzyCompare(menu->y(), button->mapFromScene(QPointF(elevenOrLeftMargin, twentyTwoOrTopMargin)).y()));
    menu->close();

    menu->setParentItem(nullptr);
    QVERIFY(QMetaObject::invokeMethod(window, "popupAtParentPos", Q_ARG(QVariant, QVariant::fromValue(button)), Q_ARG(QVariant, QPointF(-11, -22))));
    QCOMPARE(menu->parentItem(), button);
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), -1);
    // Don't need to worry about margins here because we're opening close
    // to the center of the window.
    QTRY_VERIFY(qFuzzyCompare(menu->x(), -11));
    QTRY_VERIFY(qFuzzyCompare(menu->y(), -22));
    QCOMPARE(menu->popupItem()->position(), button->mapToScene(QPointF(-11, -22)));
    menu->close();

    menu->setParentItem(nullptr);
    QVERIFY(QMetaObject::invokeMethod(window, "popupAtParentCoord", Q_ARG(QVariant, QVariant::fromValue(button)), Q_ARG(QVariant, -33), Q_ARG(QVariant, -44)));
    QCOMPARE(menu->parentItem(), button);
    QCOMPARE(menu->currentIndex(), -1);
    QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), -1);
    QTRY_VERIFY(qFuzzyCompare(menu->x(), -33));
    QTRY_VERIFY(qFuzzyCompare(menu->y(), -44));
    QCOMPARE(menu->popupItem()->position(), button->mapToScene(QPointF(-33, -44)));
    menu->close();

    const qreal twelveOrLeftMargin = qMax(qreal(12), menu->leftMargin());
    cursorPos = window->mapToGlobal(QPoint(twelveOrLeftMargin, window->height() / 2));
    QCursor::setPos(cursorPos);
    QTRY_COMPARE(QCursor::pos(), cursorPos);

    const QList<QQuickMenuItem *> menuItems = QList<QQuickMenuItem *>() << menuItem1 << menuItem2 << menuItem3;
    for (QQuickMenuItem *menuItem : menuItems) {
        menu->resetParentItem();

        QVERIFY(QMetaObject::invokeMethod(window, "popupItemAtCursor", Q_ARG(QVariant, QVariant::fromValue(menuItem))));
        QCOMPARE(menu->currentIndex(), menuItems.indexOf(menuItem));
        QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), menuItems.indexOf(menuItem));
        QTRY_VERIFY(qFuzzyCompare(menu->x(), twelveOrLeftMargin));
        QTRY_VERIFY(qFuzzyCompare(menu->y(), window->height() / 2 - menu->topPadding() - menuItem->y()));
        menu->close();

        QVERIFY(QMetaObject::invokeMethod(window, "popupItemAtPos", Q_ARG(QVariant, QPointF(33, window->height() / 3)), Q_ARG(QVariant, QVariant::fromValue(menuItem))));
        QCOMPARE(menu->currentIndex(), menuItems.indexOf(menuItem));
        QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), menuItems.indexOf(menuItem));
        QTRY_VERIFY(qFuzzyCompare(menu->x(), 33));
        QTRY_VERIFY(qFuzzyCompare(menu->y(), window->height() / 3 - menu->topPadding() - menuItem->y()));
        menu->close();

        QVERIFY(QMetaObject::invokeMethod(window, "popupItemAtCoord", Q_ARG(QVariant, 55), Q_ARG(QVariant, window->height() / 3 * 2), Q_ARG(QVariant, QVariant::fromValue(menuItem))));
        QCOMPARE(menu->currentIndex(), menuItems.indexOf(menuItem));
        QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), menuItems.indexOf(menuItem));
        QTRY_VERIFY(qFuzzyCompare(menu->x(), 55));
        QTRY_COMPARE_WITH_TIMEOUT(menu->y(), window->height() / 3 * 2 - menu->topPadding() - menuItem->y(), 500);
        menu->close();

        menu->setParentItem(nullptr);
        QVERIFY(QMetaObject::invokeMethod(window, "popupItemAtParentCursor", Q_ARG(QVariant, QVariant::fromValue(button)), Q_ARG(QVariant, QVariant::fromValue(menuItem))));
        QCOMPARE(menu->parentItem(), button);
        QCOMPARE(menu->currentIndex(), menuItems.indexOf(menuItem));
        QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), menuItems.indexOf(menuItem));
        QTRY_VERIFY(qFuzzyCompare(menu->x(), button->mapFromScene(QPoint(twelveOrLeftMargin, window->height() / 2)).x()));
        QTRY_VERIFY(qFuzzyCompare(menu->y(), button->mapFromScene(QPoint(twelveOrLeftMargin, window->height() / 2)).y() - menu->topPadding() - menuItem->y()));
        menu->close();

        menu->setParentItem(nullptr);
        QVERIFY(QMetaObject::invokeMethod(window, "popupItemAtParentPos", Q_ARG(QVariant, QVariant::fromValue(button)), Q_ARG(QVariant, QPointF(-11, -22)), Q_ARG(QVariant, QVariant::fromValue(menuItem))));
        QCOMPARE(menu->parentItem(), button);
        QCOMPARE(menu->currentIndex(), menuItems.indexOf(menuItem));
        QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), menuItems.indexOf(menuItem));
        QTRY_VERIFY(qFuzzyCompare(menu->x(), -11));
        QTRY_VERIFY(qFuzzyCompare(menu->y(), -22 - menu->topPadding() - menuItem->y()));
        QCOMPARE(menu->popupItem()->position(), button->mapToScene(QPointF(-11, -22 - menu->topPadding() - menuItem->y())));
        menu->close();

        menu->setParentItem(nullptr);
        QVERIFY(QMetaObject::invokeMethod(window, "popupItemAtParentCoord", Q_ARG(QVariant, QVariant::fromValue(button)), Q_ARG(QVariant, -33), Q_ARG(QVariant, -44), Q_ARG(QVariant, QVariant::fromValue(menuItem))));
        QCOMPARE(menu->parentItem(), button);
        QCOMPARE(menu->currentIndex(), menuItems.indexOf(menuItem));
        QCOMPARE(menu->contentItem()->property("currentIndex").toInt(), menuItems.indexOf(menuItem));
        QTRY_VERIFY(qFuzzyCompare(menu->x(), -33));
        QTRY_VERIFY(qFuzzyCompare(menu->y(), -44 - menu->topPadding() - menuItem->y()));
        QCOMPARE(menu->popupItem()->position(), button->mapToScene(QPointF(-33, -44 - menu->topPadding() - menuItem->y())));
        menu->close();
    }

    QCursor::setPos(oldCursorPos);
    QTRY_COMPARE(QCursor::pos(), oldCursorPos);
#endif
}

void tst_QQuickMenu::actions()
{
    QQuickApplicationHelper helper(this, QLatin1String("actions.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QVERIFY(menu);

    QPointer<QQuickAction> action1 = menu->actionAt(0);
    QVERIFY(!action1.isNull());

    QPointer<QQuickAction> action3 = menu->actionAt(2);
    QVERIFY(!action3.isNull());

    QVERIFY(!menu->actionAt(1));
    QVERIFY(!menu->actionAt(3));

    QPointer<QQuickMenuItem> menuItem1 = qobject_cast<QQuickMenuItem *>(menu->itemAt(0));
    QVERIFY(!menuItem1.isNull());
    QCOMPARE(menuItem1->action(), action1.data());
    QCOMPARE(menuItem1->text(), "action1");

    QPointer<QQuickMenuItem> menuItem2 = qobject_cast<QQuickMenuItem *>(menu->itemAt(1));
    QVERIFY(!menuItem2.isNull());
    QVERIFY(!menuItem2->action());
    QCOMPARE(menuItem2->text(), "menuitem2");

    QPointer<QQuickMenuItem> menuItem3 = qobject_cast<QQuickMenuItem *>(menu->itemAt(2));
    QVERIFY(!menuItem3.isNull());
    QCOMPARE(menuItem3->action(), action3.data());
    QCOMPARE(menuItem3->text(), "action3");

    QPointer<QQuickMenuItem> menuItem4 = qobject_cast<QQuickMenuItem *>(menu->itemAt(3));
    QVERIFY(!menuItem4.isNull());
    QVERIFY(!menuItem4->action());
    QCOMPARE(menuItem4->text(), "menuitem4");

    // takeAction(int) does not destroy the action, but does destroy the respective item
    QCOMPARE(menu->takeAction(0), action1.data());
    QVERIFY(!menu->itemAt(3));
    QCoreApplication::sendPostedEvents(action1, QEvent::DeferredDelete);
    QVERIFY(!action1.isNull());
    QCoreApplication::sendPostedEvents(menuItem1, QEvent::DeferredDelete);
    QVERIFY(menuItem1.isNull());

    // takeAction(int) does not destroy an item that doesn't have an action
    QVERIFY(!menuItem2->subMenu());
    QVERIFY(!menu->takeAction(0));
    QCoreApplication::sendPostedEvents(menuItem2, QEvent::DeferredDelete);
    QVERIFY(!menuItem2.isNull());

    // addAction(Action) re-creates the respective item in the menu
    menu->addAction(action1);
    menuItem1 = qobject_cast<QQuickMenuItem *>(menu->itemAt(3));
    QVERIFY(!menuItem1.isNull());
    QCOMPARE(menuItem1->action(), action1.data());

    // removeAction(Action) destroys both the action and the respective item
    menu->removeAction(action1);
    QVERIFY(!menu->itemAt(3));
    QCoreApplication::sendPostedEvents(action1, QEvent::DeferredDelete);
    QVERIFY(action1.isNull());
    QCoreApplication::sendPostedEvents(menuItem1, QEvent::DeferredDelete);
    QVERIFY(menuItem1.isNull());
}

void tst_QQuickMenu::removeTakeItem()
{
    QQuickApplicationHelper helper(this, QLatin1String("removeTakeItem.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QVERIFY(menu);

    QPointer<QQuickMenuItem> menuItem1 = window->property("menuItem1").value<QQuickMenuItem *>();
    QVERIFY(!menuItem1.isNull());
    QCOMPARE(menuItem1->menu(), menu);

    QPointer<QQuickMenuItem> menuItem2 = window->property("menuItem2").value<QQuickMenuItem *>();
    QVERIFY(!menuItem2.isNull());
    QCOMPARE(menuItem2->menu(), menu);

    QPointer<QQuickMenuItem> menuItem3 = window->property("menuItem3").value<QQuickMenuItem *>();
    QVERIFY(!menuItem3.isNull());
    QCOMPARE(menuItem3->menu(), menu);

    // takeItem(int) does not destroy
    QVariant ret;
    QVERIFY(QMetaObject::invokeMethod(window, "takeSecondItem", Q_RETURN_ARG(QVariant, ret)));
    QCOMPARE(ret.value<QQuickMenuItem *>(), menuItem2);
    QVERIFY(!menuItem2->menu());
    QCoreApplication::sendPostedEvents(menuItem2, QEvent::DeferredDelete);
    QVERIFY(!menuItem2.isNull());

    // removeItem(Item) destroys
    QVERIFY(QMetaObject::invokeMethod(window, "removeFirstItem"));
    QVERIFY(!menuItem1->menu());
    QCoreApplication::sendPostedEvents(menuItem1, QEvent::DeferredDelete);
    QVERIFY(menuItem1.isNull());

    // removeItem(null) must not call removeItem(0)
    QVERIFY(QMetaObject::invokeMethod(window, "removeNullItem"));
    QCOMPARE(menuItem3->menu(), menu);
    QCoreApplication::sendPostedEvents(menuItem3, QEvent::DeferredDelete);
    QVERIFY(!menuItem3.isNull());

    // deprecated removeItem(int) does not destroy
    QVERIFY(QMetaObject::invokeMethod(window, "removeFirstIndex"));
    QVERIFY(!menuItem3->menu());
    QCoreApplication::sendPostedEvents(menuItem3, QEvent::DeferredDelete);
    QVERIFY(!menuItem3.isNull());
}

void tst_QQuickMenu::subMenuMouse_data()
{
    QTest::addColumn<bool>("cascade");

    QTest::newRow("cascading") << true;
    QTest::newRow("non-cascading") << false;
}

void tst_QQuickMenu::subMenuMouse()
{
    if ((QGuiApplication::platformName() == QLatin1String("offscreen"))
        || (QGuiApplication::platformName() == QLatin1String("minimal")))
        QSKIP("Mouse hovering not functional on offscreen/minimal platforms");

    QFETCH(bool, cascade);

    QQuickApplicationHelper helper(this, QLatin1String("subMenus.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *mainMenu = window->property("mainMenu").value<QQuickMenu *>();
    QVERIFY(mainMenu);
    mainMenu->setCascade(cascade);
    QCOMPARE(mainMenu->cascade(), cascade);

    QQuickMenu *subMenu1 = window->property("subMenu1").value<QQuickMenu *>();
    QVERIFY(subMenu1);

    QQuickMenu *subMenu2 = window->property("subMenu2").value<QQuickMenu *>();
    QVERIFY(subMenu2);

    QQuickMenu *subSubMenu1 = window->property("subSubMenu1").value<QQuickMenu *>();
    QVERIFY(subSubMenu1);

    mainMenu->open();
    QVERIFY(mainMenu->isVisible());
    QTRY_VERIFY(mainMenu->isOpened());
    QVERIFY(!subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());

    // open the sub-menu with mouse click
    QQuickMenuItem *subMenu1Item = qobject_cast<QQuickMenuItem *>(mainMenu->itemAt(1));
    QVERIFY(subMenu1Item);
    QCOMPARE(subMenu1Item->subMenu(), subMenu1);
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, subMenu1Item->mapToScene(QPoint(1, 1)).toPoint());
    QTRY_COMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu1->isVisible());
    QTRY_VERIFY(subMenu1->isOpened());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());

    // open the cascading sub-sub-menu with mouse hover
    QQuickMenuItem *subSubMenu1Item = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(2));
    QVERIFY(subSubMenu1Item);
    QCOMPARE(subSubMenu1Item->subMenu(), subSubMenu1);
    QTest::mouseMove(window, subSubMenu1Item->mapToScene(QPoint(1, 1)).toPoint());
    QCOMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());
    if (cascade) {
        QTRY_VERIFY(subSubMenu1->isVisible());
        QTRY_VERIFY(subSubMenu1->isOpened());
    }

    // close the sub-sub-menu with mouse hover over another parent menu item
    QQuickMenuItem *subMenuItem1 = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(0));
    QVERIFY(subMenuItem1);
    QVERIFY(!subMenuItem1->subMenu());
    QTest::mouseMove(window, subMenuItem1->mapToScene(QPoint(1, 1)).toPoint());
    QCOMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QTRY_VERIFY(!subSubMenu1->isVisible());

    // re-open the sub-sub-menu with mouse hover
    QTest::mouseMove(window, subSubMenu1Item->mapToScene(QPoint(1, 1)).toPoint());
    QCOMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    if (!cascade) {
        QVERIFY(!subSubMenu1->isVisible());
    } else {
        QTRY_VERIFY(subSubMenu1->isVisible());
        QTRY_VERIFY(subSubMenu1->isOpened());
    }

    // close sub-menu and sub-sub-menu with mouse hover in the main menu
    QQuickMenuItem *mainMenuItem1 = qobject_cast<QQuickMenuItem *>(mainMenu->itemAt(0));
    QVERIFY(mainMenuItem1);
    QTest::mouseMove(window, mainMenuItem1->mapToScene(QPoint(1, 1)).toPoint());
    QCOMPARE(mainMenu->isVisible(), cascade);
    QTRY_COMPARE(subMenu1->isVisible(), !cascade);
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());

    // close all menus by click triggering an item
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, mainMenuItem1->mapToScene(QPoint(1, 1)).toPoint());
    QTRY_VERIFY(!mainMenu->isVisible());
    QTRY_VERIFY(!subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());
}

void tst_QQuickMenu::subMenuDisabledMouse_data()
{
    subMenuMouse_data();
}

// QTBUG-69540
void tst_QQuickMenu::subMenuDisabledMouse()
{
    if ((QGuiApplication::platformName() == QLatin1String("offscreen"))
        || (QGuiApplication::platformName() == QLatin1String("minimal")))
        QSKIP("Mouse hovering not functional on offscreen/minimal platforms");

    QFETCH(bool, cascade);

    QQuickApplicationHelper helper(this, QLatin1String("subMenuDisabled.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *mainMenu = window->property("mainMenu").value<QQuickMenu *>();
    QVERIFY(mainMenu);
    mainMenu->setCascade(cascade);
    QCOMPARE(mainMenu->cascade(), cascade);

    QQuickMenuItem *menuItem1 = qobject_cast<QQuickMenuItem *>(mainMenu->itemAt(0));
    QVERIFY(menuItem1);

    QQuickMenu *subMenu = window->property("subMenu").value<QQuickMenu *>();
    QVERIFY(subMenu);

    mainMenu->open();
    QVERIFY(mainMenu->isVisible());
    QVERIFY(!menuItem1->isHighlighted());
    QVERIFY(!subMenu->isVisible());

    // Open the sub-menu with a mouse click.
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, menuItem1->mapToScene(QPoint(1, 1)).toPoint());
    // Need to use the TRY variant here when cascade is false,
    // as e.g. Material style menus have transitions and don't close immediately.
    QTRY_COMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu->isVisible());
    QVERIFY(menuItem1->isHighlighted());
    // Now the sub-menu is open. The current behavior is that the first menu item
    // in the new menu is highlighted; make sure that we choose the next item if
    // the first is disabled.
    QQuickMenuItem *subMenuItem1 = qobject_cast<QQuickMenuItem *>(subMenu->itemAt(0));
    QVERIFY(subMenuItem1);
    QQuickMenuItem *subMenuItem2 = qobject_cast<QQuickMenuItem *>(subMenu->itemAt(1));
    QVERIFY(subMenuItem2);
    QVERIFY(!subMenuItem1->isHighlighted());
    QVERIFY(subMenuItem2->isHighlighted());

    // Close all menus by clicking on the item that isn't disabled.
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, subMenuItem2->mapToScene(QPoint(1, 1)).toPoint());
    QTRY_VERIFY(!mainMenu->isVisible());
    QTRY_VERIFY(!subMenu->isVisible());
}

void tst_QQuickMenu::subMenuKeyboard_data()
{
    QTest::addColumn<bool>("cascade");
    QTest::addColumn<bool>("mirrored");

    QTest::newRow("cascading") << true << false;
    QTest::newRow("cascading,mirrored") << true << true;
    QTest::newRow("non-cascading") << false << false;
    QTest::newRow("non-cascading,mirrored") << false << true;
}

void tst_QQuickMenu::subMenuKeyboard()
{
    QFETCH(bool, cascade);
    QFETCH(bool, mirrored);

    QQuickApplicationHelper helper(this, QLatin1String("subMenus.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    if (mirrored)
        window->setLocale(QLocale("ar_EG"));

    QQuickMenu *mainMenu = window->property("mainMenu").value<QQuickMenu *>();
    QVERIFY(mainMenu);
    mainMenu->setCascade(cascade);
    QCOMPARE(mainMenu->cascade(), cascade);

    QQuickMenu *subMenu1 = window->property("subMenu1").value<QQuickMenu *>();
    QVERIFY(subMenu1);

    QQuickMenu *subMenu2 = window->property("subMenu2").value<QQuickMenu *>();
    QVERIFY(subMenu2);

    QQuickMenu *subSubMenu1 = window->property("subSubMenu1").value<QQuickMenu *>();
    QVERIFY(subSubMenu1);

    mainMenu->open();
    QVERIFY(mainMenu->isVisible());
    QTRY_VERIFY(mainMenu->isOpened());
    QVERIFY(!subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());

    // navigate to the sub-menu item and trigger it to open the sub-menu
    QQuickMenuItem *subMenu1Item = qobject_cast<QQuickMenuItem *>(mainMenu->itemAt(1));
    QVERIFY(subMenu1Item);
    QVERIFY(!subMenu1Item->isHighlighted());
    QCOMPARE(subMenu1Item->subMenu(), subMenu1);
    QTest::keyClick(window, Qt::Key_Down);
    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(subMenu1Item->isHighlighted());
    QTest::keyClick(window, Qt::Key_Space);
    QTRY_COMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu1->isVisible());
    QTRY_VERIFY(subMenu1->isOpened());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());

    // navigate to the sub-sub-menu item and open it with the arrow key
    QQuickMenuItem *subSubMenu1Item = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(2));
    QVERIFY(subSubMenu1Item);
    QVERIFY(!subSubMenu1Item->isHighlighted());
    QCOMPARE(subSubMenu1Item->subMenu(), subSubMenu1);
    QTest::keyClick(window, Qt::Key_Down);
    QTest::keyClick(window, Qt::Key_Down);
    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(subSubMenu1Item->isHighlighted());
    QTRY_COMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());
    QTest::keyClick(window, mirrored ? Qt::Key_Left : Qt::Key_Right);
    QCOMPARE(mainMenu->isVisible(), cascade);
    QTRY_COMPARE(subMenu1->isVisible(), cascade);
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(subSubMenu1->isVisible());
    QTRY_VERIFY(subSubMenu1->isOpened());

    // navigate within the sub-sub-menu
    QQuickMenuItem *subSubMenuItem1 = qobject_cast<QQuickMenuItem *>(subSubMenu1->itemAt(0));
    QVERIFY(subSubMenuItem1);
    QQuickMenuItem *subSubMenuItem2 = qobject_cast<QQuickMenuItem *>(subSubMenu1->itemAt(1));
    QVERIFY(subSubMenuItem2);
    QVERIFY(subSubMenuItem1->isHighlighted());
    QVERIFY(!subSubMenuItem2->isHighlighted());
    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(!subSubMenuItem1->isHighlighted());
    QVERIFY(subSubMenuItem2->isHighlighted());

    // navigate to the parent menu with the arrow key
    QTest::keyClick(window, mirrored ? Qt::Key_Right : Qt::Key_Left);
    QVERIFY(subSubMenu1Item->isHighlighted());
    QCOMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QTRY_VERIFY(!subSubMenu1->isVisible());

    // navigate within the sub-menu
    QQuickMenuItem *subMenuItem1 = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(0));
    QVERIFY(subMenuItem1);
    QQuickMenuItem *subMenuItem2 = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(1));
    QVERIFY(subMenuItem2);
    QVERIFY(!subMenuItem1->isHighlighted());
    QVERIFY(!subMenuItem2->isHighlighted());
    QVERIFY(subSubMenu1Item->isHighlighted());
    QTest::keyClick(window, Qt::Key_Up);
    QVERIFY(!subMenuItem1->isHighlighted());
    QVERIFY(subMenuItem2->isHighlighted());
    QVERIFY(!subSubMenu1Item->isHighlighted());

    // close the menus with esc
    QTest::keyClick(window, Qt::Key_Escape);
    QCOMPARE(mainMenu->isVisible(), cascade);
    QTRY_VERIFY(!subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!mainMenu->isVisible());
    QVERIFY(!subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());
}

void tst_QQuickMenu::subMenuDisabledKeyboard_data()
{
    subMenuKeyboard_data();
}

// QTBUG-69540
void tst_QQuickMenu::subMenuDisabledKeyboard()
{
    QFETCH(bool, cascade);
    QFETCH(bool, mirrored);

    QQuickApplicationHelper helper(this, QLatin1String("subMenuDisabled.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    if (mirrored)
        window->setLocale(QLocale("ar_EG"));

    QQuickMenu *mainMenu = window->property("mainMenu").value<QQuickMenu *>();
    QVERIFY(mainMenu);
    mainMenu->setCascade(cascade);
    QCOMPARE(mainMenu->cascade(), cascade);

    QQuickMenuItem *menuItem1 = qobject_cast<QQuickMenuItem *>(mainMenu->itemAt(0));
    QVERIFY(menuItem1);

    QQuickMenu *subMenu = window->property("subMenu").value<QQuickMenu *>();
    QVERIFY(subMenu);

    mainMenu->open();
    QVERIFY(mainMenu->isVisible());
    QTRY_VERIFY(mainMenu->isOpened());
    QVERIFY(!menuItem1->isHighlighted());
    QVERIFY(!subMenu->isVisible());

    // Highlight the top-level menu item.
    QTest::keyClick(window, Qt::Key_Down);
    QVERIFY(menuItem1->isHighlighted());

    QQuickMenuItem *subMenuItem1 = qobject_cast<QQuickMenuItem *>(subMenu->itemAt(0));
    QVERIFY(subMenuItem1);
    QQuickMenuItem *subMenuItem2 = qobject_cast<QQuickMenuItem *>(subMenu->itemAt(1));
    QVERIFY(subMenuItem2);

    // Open the sub-menu.
    QTest::keyClick(window, mirrored ? Qt::Key_Left : Qt::Key_Right);
    // The first sub-menu item is disabled, so it should highlight the second one.
    QVERIFY(!subMenuItem1->isHighlighted());
    QVERIFY(subMenuItem2->isHighlighted());

    // Close the menus with escape.
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_COMPARE(mainMenu->isVisible(), cascade);
    QTRY_VERIFY(!subMenu->isVisible());
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!mainMenu->isVisible());
    QVERIFY(!subMenu->isVisible());
}

void tst_QQuickMenu::subMenuPosition_data()
{
    QTest::addColumn<bool>("cascade");
    QTest::addColumn<bool>("flip");
    QTest::addColumn<bool>("mirrored");
    QTest::addColumn<qreal>("overlap");

    QTest::newRow("cascading") << true << false << false << 0.0;
    QTest::newRow("cascading,flip") << true << true << false << 0.0;
    QTest::newRow("cascading,overlap") << true << false << false << 10.0;
    QTest::newRow("cascading,flip,overlap") << true << true << false << 10.0;
    QTest::newRow("cascading,mirrored") << true << false << true << 0.0;
    QTest::newRow("cascading,mirrored,flip") << true << true << true << 0.0;
    QTest::newRow("cascading,mirrored,overlap") << true << false << true << 10.0;
    QTest::newRow("cascading,mirrored,flip,overlap") << true << true << true << 10.0;
    QTest::newRow("non-cascading") << false << false << false << 0.0;
}

void tst_QQuickMenu::subMenuPosition()
{
    QFETCH(bool, cascade);
    QFETCH(bool, flip);
    QFETCH(bool, mirrored);
    QFETCH(qreal, overlap);

    QQuickApplicationHelper helper(this, QLatin1String("subMenus.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;

    // Ensure that the default size of the window fits three menus side by side.
    QQuickMenu *mainMenu = window->property("mainMenu").value<QQuickMenu *>();
    QVERIFY(mainMenu);
    window->setWidth(mainMenu->width() * 3 + mainMenu->leftMargin() + mainMenu->rightMargin());

    // the default size of the window fits three menus side by side.
    // when testing flipping, we resize the window so that the first
    // sub-menu fits, but the second doesn't
    if (flip)
        window->setWidth(window->width() - mainMenu->width());

    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    if (mirrored)
        window->setLocale(QLocale("ar_EG"));

    mainMenu->setCascade(cascade);
    QCOMPARE(mainMenu->cascade(), cascade);
    mainMenu->setOverlap(overlap);
    QCOMPARE(mainMenu->overlap(), overlap);

    QQuickMenu *subMenu1 = window->property("subMenu1").value<QQuickMenu *>();
    QVERIFY(subMenu1);
    subMenu1->setCascade(cascade);
    QCOMPARE(subMenu1->cascade(), cascade);
    subMenu1->setOverlap(overlap);
    QCOMPARE(subMenu1->overlap(), overlap);

    QQuickMenu *subMenu2 = window->property("subMenu2").value<QQuickMenu *>();
    QVERIFY(subMenu2);
    subMenu2->setCascade(cascade);
    QCOMPARE(subMenu2->cascade(), cascade);
    subMenu2->setOverlap(overlap);
    QCOMPARE(subMenu2->overlap(), overlap);

    QQuickMenu *subSubMenu1 = window->property("subSubMenu1").value<QQuickMenu *>();
    QVERIFY(subSubMenu1);
    subSubMenu1->setCascade(cascade);
    QCOMPARE(subSubMenu1->cascade(), cascade);
    subSubMenu1->setOverlap(overlap);
    QCOMPARE(subSubMenu1->overlap(), overlap);

    // choose the main menu position so that there's room for the
    // sub-menus to cascade to the left when mirrored
    if (mirrored)
        mainMenu->setX(window->width() - mainMenu->width());

    mainMenu->open();
    QVERIFY(mainMenu->isVisible());
    QTRY_VERIFY(mainMenu->isOpened());
    QVERIFY(!subMenu1->isVisible());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());

    // open the sub-menu (never flips)
    QQuickMenuItem *subMenu1Item = qobject_cast<QQuickMenuItem *>(mainMenu->itemAt(1));
    QVERIFY(subMenu1Item);
    QCOMPARE(subMenu1Item->subMenu(), subMenu1);
    emit subMenu1Item->triggered();
    QTRY_COMPARE(mainMenu->isVisible(), cascade);
    QVERIFY(subMenu1->isVisible());
    QTRY_VERIFY(subMenu1->isOpened());
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(!subSubMenu1->isVisible());

    if (cascade) {
        QCOMPARE(subMenu1->parentItem(), subMenu1Item);
        // vertically aligned to the parent menu item
        QCOMPARE(subMenu1->popupItem()->y(), mainMenu->popupItem()->y() + subMenu1Item->y());
        if (mirrored) {
            // on the left of the parent menu
            QCOMPARE(subMenu1->popupItem()->x(), mainMenu->popupItem()->x() - subMenu1->width() + overlap);
        } else {
            // on the right of the parent menu
            QCOMPARE(subMenu1->popupItem()->x(), mainMenu->popupItem()->x() + mainMenu->width() - overlap);
        }
    } else {
        QCOMPARE(subMenu1->parentItem(), mainMenu->parentItem());
        // centered over the parent menu
        QCOMPARE(subMenu1->popupItem()->x(), mainMenu->popupItem()->x() + (mainMenu->width() - subMenu1->width()) / 2);
        QCOMPARE(subMenu1->popupItem()->y(), mainMenu->popupItem()->y() + (mainMenu->height() - subMenu1->height()) / 2);
    }

    // open the sub-sub-menu (can flip)
    QQuickMenuItem *subSubMenu1Item = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(2));
    QVERIFY(subSubMenu1Item);
    QCOMPARE(subSubMenu1Item->subMenu(), subSubMenu1);
    emit subSubMenu1Item->triggered();
    QTRY_COMPARE(mainMenu->isVisible(), cascade);
    QTRY_COMPARE(subMenu1->isVisible(), cascade);
    QVERIFY(!subMenu2->isVisible());
    QVERIFY(subSubMenu1->isVisible());
    QTRY_VERIFY(subSubMenu1->isOpened());

    if (cascade) {
        QCOMPARE(subSubMenu1->parentItem(), subSubMenu1Item);
        // vertically aligned to the parent menu item
        QCOMPARE(subSubMenu1->popupItem()->y(), subMenu1->popupItem()->y() + subSubMenu1Item->y());
        if (mirrored != flip) {
            // on the left of the parent menu
            QCOMPARE(subSubMenu1->popupItem()->x(), subMenu1->popupItem()->x() - subSubMenu1->width() + overlap);
        } else {
            // on the right of the parent menu
            QCOMPARE(subSubMenu1->popupItem()->x(), subMenu1->popupItem()->x() + subMenu1->width() - overlap);
        }
    } else {
        QCOMPARE(subSubMenu1->parentItem(), subMenu1->parentItem());
        // centered over the parent menu
        QCOMPARE(subSubMenu1->popupItem()->x(), subMenu1->popupItem()->x() + (subMenu1->width() - subSubMenu1->width()) / 2);
        QCOMPARE(subSubMenu1->popupItem()->y(), subMenu1->popupItem()->y() + (subMenu1->height() - subSubMenu1->height()) / 2);
    }
}

void tst_QQuickMenu::addRemoveSubMenus()
{
    QQuickApplicationHelper helper(this, QLatin1String("subMenus.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *mainMenu = window->property("mainMenu").value<QQuickMenu *>();
    QVERIFY(mainMenu);

    QVERIFY(!mainMenu->menuAt(0));

    QPointer<QQuickMenu> subMenu1 = window->property("subMenu1").value<QQuickMenu *>();
    QVERIFY(!subMenu1.isNull());
    QCOMPARE(mainMenu->menuAt(1), subMenu1.data());

    QVERIFY(!mainMenu->menuAt(2));

    QPointer<QQuickMenu> subMenu2 = window->property("subMenu2").value<QQuickMenu *>();
    QVERIFY(!subMenu2.isNull());
    QCOMPARE(mainMenu->menuAt(3), subMenu2.data());

    QVERIFY(!mainMenu->menuAt(4));

    QPointer<QQuickMenu> subSubMenu1 = window->property("subSubMenu1").value<QQuickMenu *>();
    QVERIFY(!subSubMenu1.isNull());

    // takeMenu(int) does not destroy the menu, but does destroy the respective item in the parent menu
    QPointer<QQuickMenuItem> subSubMenu1Item = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(2));
    QVERIFY(subSubMenu1Item);
    QCOMPARE(subSubMenu1Item->subMenu(), subSubMenu1.data());
    QCOMPARE(subMenu1->takeMenu(2), subSubMenu1.data());
    QVERIFY(!subMenu1->itemAt(2));
    QCoreApplication::sendPostedEvents(subSubMenu1, QEvent::DeferredDelete);
    QVERIFY(!subSubMenu1.isNull());
    QCoreApplication::sendPostedEvents(subSubMenu1Item, QEvent::DeferredDelete);
    QVERIFY(subSubMenu1Item.isNull());

    // takeMenu(int) does not destroy an item that doesn't present a menu
    QPointer<QQuickMenuItem> subMenuItem1 = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(0));
    QVERIFY(subMenuItem1);
    QVERIFY(!subMenuItem1->subMenu());
    QVERIFY(!subMenu1->takeMenu(0));
    QCoreApplication::sendPostedEvents(subMenuItem1, QEvent::DeferredDelete);
    QVERIFY(!subMenuItem1.isNull());

    // addMenu(Menu) re-creates the respective item in the parent menu
    subMenu1->addMenu(subSubMenu1);
    subSubMenu1Item = qobject_cast<QQuickMenuItem *>(subMenu1->itemAt(2));
    QVERIFY(!subSubMenu1Item.isNull());

    // removeMenu(Menu) destroys both the menu and the respective item in the parent menu
    subMenu1->removeMenu(subSubMenu1);
    QVERIFY(!subMenu1->itemAt(2));
    QCoreApplication::sendPostedEvents(subSubMenu1, QEvent::DeferredDelete);
    QVERIFY(subSubMenu1.isNull());
    QCoreApplication::sendPostedEvents(subSubMenu1Item, QEvent::DeferredDelete);
    QVERIFY(subSubMenu1Item.isNull());
}

void tst_QQuickMenu::scrollable_data()
{
    QTest::addColumn<QString>("qmlFilePath");

    QTest::addRow("Window") << QString::fromLatin1("windowScrollable.qml");
    QTest::addRow("ApplicationWindow") << QString::fromLatin1("applicationWindowScrollable.qml");
}

void tst_QQuickMenu::scrollable()
{
    QFETCH(QString, qmlFilePath);

    QQuickApplicationHelper helper(this, qmlFilePath);
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    menu->open();
    QVERIFY(menu->isVisible());

    QQuickItem *contentItem = menu->contentItem();
    QCOMPARE(contentItem->property("interactive").toBool(), true);
}

void tst_QQuickMenu::disableWhenTriggered_data()
{
    QTest::addColumn<int>("menuItemIndex");
    QTest::addColumn<int>("subMenuItemIndex");

    QTest::addRow("Action") << 0 << -1;
    QTest::addRow("MenuItem with Action") << 1 << -1;
    QTest::addRow("MenuItem with Action declared outside menu") << 2 << -1;
    QTest::addRow("MenuItem with no Action") << 3 << -1;

    QTest::addRow("Sub-Action") << 4 << 0;
    QTest::addRow("Sub-MenuItem with Action declared inside") << 4 << 1;
    QTest::addRow("Sub-MenuItem with Action declared outside menu") << 4 << 2;
    QTest::addRow("Sub-MenuItem with no Action") << 4 << 3;
}

// Tests that the menu is dismissed when a menu item sets "enabled = false" in onTriggered().
void tst_QQuickMenu::disableWhenTriggered()
{
    if ((QGuiApplication::platformName() == QLatin1String("offscreen"))
        || (QGuiApplication::platformName() == QLatin1String("minimal")))
        QSKIP("Mouse hovering not functional on offscreen/minimal platforms");

    QFETCH(int, menuItemIndex);
    QFETCH(int, subMenuItemIndex);

    QQuickApplicationHelper helper(this, QLatin1String("disableWhenTriggered.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->findChild<QQuickMenu*>("Menu");
    QVERIFY(menu);

    menu->open();
    QVERIFY(menu->isVisible());
    QTRY_VERIFY(menu->isOpened());

    QPointer<QQuickMenuItem> menuItem = qobject_cast<QQuickMenuItem*>(menu->itemAt(menuItemIndex));
    QVERIFY(menuItem);

    if (subMenuItemIndex == -1) {
        // Click a top-level menu item.
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
            menuItem->mapToScene(QPointF(menuItem->width() / 2, menuItem->height() / 2)).toPoint());
        QCOMPARE(menuItem->isEnabled(), false);
        QTRY_VERIFY(!menu->isVisible());
    } else {
        // Click a sub-menu item.
        QPointer<QQuickMenu> subMenu = menuItem->subMenu();
        QVERIFY(subMenu);

        QPointer<QQuickMenuItem> subMenuItem = qobject_cast<QQuickMenuItem*>(subMenu->itemAt(subMenuItemIndex));
        QVERIFY(subMenuItem);

        // First, open the sub-menu.
        QTest::mouseMove(window, menuItem->mapToScene(QPoint(1, 1)).toPoint());
        QTRY_VERIFY(subMenu->isVisible());
        QVERIFY(menuItem->isHovered());
        QTRY_VERIFY(subMenu->contentItem()->property("contentHeight").toReal() > 0.0);

        // Click the item.
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
            subMenuItem->mapToScene(QPointF(subMenuItem->width() / 2, subMenuItem->height() / 2)).toPoint());
        QCOMPARE(subMenuItem->isEnabled(), false);
        QTRY_VERIFY(!menu->isVisible());
    }
}

void tst_QQuickMenu::menuItemWidth_data()
{
    QTest::addColumn<bool>("mirrored");

    QTest::newRow("non-mirrored") << false;
    QTest::newRow("mirrored") << true;
}

void tst_QQuickMenu::menuItemWidth()
{
    QFETCH(bool, mirrored);

    QQuickApplicationHelper helper(this, QLatin1String("menuItemWidths.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    if (mirrored)
        window->setLocale(QLocale("ar_EG"));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QVERIFY(menu);
    menu->open();
    QTRY_VERIFY(menu->isOpened());
    for (int i = 0; i < menu->count(); ++i)
        QCOMPARE(menu->itemAt(i)->width(), menu->availableWidth());
}

void tst_QQuickMenu::menuItemWidthAfterMenuWidthChanged_data()
{
    QTest::addColumn<bool>("mirrored");

    QTest::newRow("non-mirrored") << false;
    QTest::newRow("mirrored") << true;
}

void tst_QQuickMenu::menuItemWidthAfterMenuWidthChanged()
{
    QFETCH(bool, mirrored);

    QQuickApplicationHelper helper(this, QLatin1String("menuItemWidths.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    if (mirrored)
        window->setLocale(QLocale("ar_EG"));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QVERIFY(menu);
    menu->open();
    QTRY_VERIFY(menu->isOpened());
    for (int i = 0; i < menu->count(); ++i) {
        // Check that the width of menu items is correct before we resize the menu.
        const QQuickItem *item = menu->itemAt(i);
        QVERIFY2(qFuzzyCompare(item->width(), menu->availableWidth()),
            qPrintable(QString::fromLatin1("Expected width of %1 to be %2, but it's %3")
                .arg(item->objectName()).arg(menu->availableWidth()).arg(item->width())));
    }

    menu->setWidth(menu->width() + 10);

    // Check that the width of menu items is correct after we resize the menu.
    for (int i = 0; i < menu->count(); ++i) {
        // Check that the width of menu items is correct after we resize the menu.
        const QQuickItem *item = menu->itemAt(i);
        QVERIFY2(qFuzzyCompare(item->width(), menu->availableWidth()),
            qPrintable(QString::fromLatin1("Expected width of %1 to be %2, but it's %3")
                .arg(item->objectName()).arg(menu->availableWidth()).arg(item->width())));
    }
}

void tst_QQuickMenu::menuItemWidthAfterImplicitWidthChanged_data()
{
    QTest::addColumn<bool>("mirrored");

    QTest::newRow("non-mirrored") << false;
    QTest::newRow("mirrored") << true;
}

void tst_QQuickMenu::menuItemWidthAfterImplicitWidthChanged()
{
    QFETCH(bool, mirrored);

    QQuickApplicationHelper helper(this, QLatin1String("menuItemWidths.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    if (mirrored)
        window->setLocale(QLocale("ar_EG"));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QVERIFY(menu);
    menu->open();
    QTRY_VERIFY(menu->isOpened());
    // Check that the width of the menu item is correct before we change its font size.
    QQuickMenuItem *menuItem = qobject_cast<QQuickMenuItem*>(menu->itemAt(0));
    QCOMPARE(menuItem->width(), menu->availableWidth());

    // Add some text to increase the implicitWidth of the MenuItem.
    const qreal oldImplicitWidth = menuItem->implicitWidth();
    for (int i = 0; menuItem->implicitWidth() <= oldImplicitWidth; ++i) {
        menuItem->setText(menuItem->text() + QLatin1String("---"));
        if (i == 100)
            QFAIL("Shouldn't need 100 iterations to increase MenuItem's implicitWidth; something is wrong here");
    }

    // Check that the width of the menu item is correct after we change its font size.
    QCOMPARE(menuItem->width(), menu->availableWidth());
}

void tst_QQuickMenu::menuItemWidthAfterRetranslate()
{
    QQuickApplicationHelper helper(this, QLatin1String("menuItemWidths.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickMenu *menu = window->property("menu").value<QQuickMenu *>();
    QVERIFY(menu);
    menu->open();
    QTRY_VERIFY(menu->isOpened());
    for (int i = 0; i < menu->count(); ++i) {
        // Check that the width of each menu item is correct before we retranslate.
        const QQuickItem *item = menu->itemAt(i);
        QVERIFY2(qFuzzyCompare(item->width(), menu->availableWidth()),
            qPrintable(QString::fromLatin1("Expected width of %1 to be %2, but it's %3")
                .arg(item->objectName()).arg(menu->availableWidth()).arg(item->width())));
    }

    // Call retranslate() and cause all bindings to be re-evaluated.
    helper.engine.retranslate();

    for (int i = 0; i < menu->count(); ++i) {
        // Check that the width of each menu item is correct after we retranslate.
        const QQuickItem *item = menu->itemAt(i);
        QVERIFY2(qFuzzyCompare(item->width(), menu->availableWidth()),
            qPrintable(QString::fromLatin1("Expected width of %1 to be %2, but it's %3")
                .arg(item->objectName()).arg(menu->availableWidth()).arg(item->width())));
    }
}

void tst_QQuickMenu::giveMenuItemFocusOnButtonPress()
{
    QQuickApplicationHelper helper(this, QLatin1String("giveMenuItemFocusOnButtonPress.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    // Press enter on the button to open the menu.
    QQuickButton *menuButton = window->property("menuButton").value<QQuickButton*>();
    QVERIFY(menuButton);
    menuButton->forceActiveFocus();
    QVERIFY(menuButton->hasActiveFocus());

    QSignalSpy clickedSpy(window, SIGNAL(menuButtonClicked()));
    QVERIFY(clickedSpy.isValid());

    QTest::keyClick(window, Qt::Key_Return);
    QCOMPARE(clickedSpy.count(), 1);

    // The menu should still be open.
    QQuickMenu *menu = window->property("menu").value<QQuickMenu*>();
    QVERIFY(menu);
    QTRY_VERIFY(menu->isOpened());
}

QTEST_QUICKCONTROLS_MAIN(tst_QQuickMenu)

#include "tst_qquickmenu.moc"
