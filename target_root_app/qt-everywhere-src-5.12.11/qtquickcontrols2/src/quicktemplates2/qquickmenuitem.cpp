/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
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

#include "qquickmenuitem_p.h"
#include "qquickmenuitem_p_p.h"
#include "qquickmenu_p.h"
#include "qquickdeferredexecute_p_p.h"

#include <QtGui/qpa/qplatformtheme.h>
#include <QtQuick/private/qquickevents_p_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MenuItem
    \inherits AbstractButton
    \instantiates QQuickMenuItem
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-menus
    \brief Presents an item within a Menu.

    MenuItem is a convenience type that implements the AbstractButton API,
    providing a familiar way to respond to menu items being \l triggered, for
    example.

    MenuItem inherits its API from AbstractButton. For instance, you can set
    \l {AbstractButton::text}{text} and \l {Icons in Qt Quick Controls 2}{icon}
    using the AbstractButton API.

    \code
    Button {
        id: fileButton
        text: "File"
        onClicked: menu.open()

        Menu {
            id: menu

            MenuItem {
                text: "New..."
                onTriggered: document.reset()
            }
            MenuItem {
                text: "Open..."
                onTriggered: openDialog.open()
            }
            MenuItem {
                text: "Save"
                onTriggered: saveDialog.open()
            }
        }
    }
    \endcode

    \sa {Customizing Menu}, Menu, {Menu Controls}
*/

void QQuickMenuItemPrivate::setMenu(QQuickMenu *newMenu)
{
    Q_Q(QQuickMenuItem);
    if (menu == newMenu)
        return;

    menu = newMenu;
    emit q->menuChanged();
}

void QQuickMenuItemPrivate::setSubMenu(QQuickMenu *newSubMenu)
{
    Q_Q(QQuickMenuItem);
    if (subMenu == newSubMenu)
        return;

    if (subMenu) {
        QObject::disconnect(subMenu, &QQuickMenu::titleChanged, q, &QQuickAbstractButton::setText);
        QObjectPrivate::disconnect(subMenu, &QQuickPopup::enabledChanged, this, &QQuickMenuItemPrivate::updateEnabled);
    }

    if (newSubMenu) {
        QObject::connect(newSubMenu, &QQuickMenu::titleChanged, q, &QQuickAbstractButton::setText);
        QObjectPrivate::connect(newSubMenu, &QQuickPopup::enabledChanged, this, &QQuickMenuItemPrivate::updateEnabled);
        q->setText(newSubMenu->title());
    }

    subMenu = newSubMenu;
    updateEnabled();
    emit q->subMenuChanged();
}

void QQuickMenuItemPrivate::updateEnabled()
{
    Q_Q(QQuickMenuItem);
    q->setEnabled(subMenu && subMenu->isEnabled());
}

static inline QString arrowName() { return QStringLiteral("arrow"); }

void QQuickMenuItemPrivate::cancelArrow()
{
    Q_Q(QQuickAbstractButton);
    quickCancelDeferred(q, arrowName());
}

void QQuickMenuItemPrivate::executeArrow(bool complete)
{
    Q_Q(QQuickMenuItem);
    if (arrow.wasExecuted())
        return;

    if (!arrow || complete)
        quickBeginDeferred(q, arrowName(), arrow);
    if (complete)
        quickCompleteDeferred(q, arrowName(), arrow);
}

bool QQuickMenuItemPrivate::acceptKeyClick(Qt::Key key) const
{
    return key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter;
}

/*!
    \qmlsignal void QtQuick.Controls::MenuItem::triggered()

    This signal is emitted when the menu item is triggered by the user.
*/

QQuickMenuItem::QQuickMenuItem(QQuickItem *parent)
    : QQuickAbstractButton(*(new QQuickMenuItemPrivate), parent)
{
    connect(this, &QQuickAbstractButton::clicked, this, &QQuickMenuItem::triggered);
}

/*!
    \qmlproperty bool QtQuick.Controls::MenuItem::highlighted

    This property holds whether the menu item is highlighted by the user.

    A menu item can be highlighted by mouse hover or keyboard navigation.

    The default value is \c false.

    \sa Menu::currentIndex
*/
bool QQuickMenuItem::isHighlighted() const
{
    Q_D(const QQuickMenuItem);
    return d->highlighted;
}

void QQuickMenuItem::setHighlighted(bool highlighted)
{
    Q_D(QQuickMenuItem);
    if (highlighted == d->highlighted)
        return;

    d->highlighted = highlighted;
    emit highlightedChanged();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty Item QtQuick.Controls::MenuItem::arrow

    This property holds the sub-menu arrow item.

    \sa {Customizing Menu}
*/
QQuickItem *QQuickMenuItem::arrow() const
{
    QQuickMenuItemPrivate *d = const_cast<QQuickMenuItemPrivate *>(d_func());
    if (!d->arrow)
        d->executeArrow();
    return d->arrow;
}

void QQuickMenuItem::setArrow(QQuickItem *arrow)
{
    Q_D(QQuickMenuItem);
    if (d->arrow == arrow)
        return;

    if (!d->arrow.isExecuting())
        d->cancelArrow();

    delete d->arrow;
    d->arrow = arrow;
    if (arrow && !arrow->parentItem())
        arrow->setParentItem(this);
    if (!d->arrow.isExecuting())
        emit arrowChanged();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty Menu QtQuick.Controls::MenuItem::menu
    \readonly

    This property holds the menu that contains this menu item,
    or \c null if the item is not in a menu.
*/
QQuickMenu *QQuickMenuItem::menu() const
{
    Q_D(const QQuickMenuItem);
    return d->menu;
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty Menu QtQuick.Controls::MenuItem::subMenu
    \readonly

    This property holds the sub-menu that this item presents in
    the parent menu, or \c null if this item is not a sub-menu item.
*/
QQuickMenu *QQuickMenuItem::subMenu() const
{
    Q_D(const QQuickMenuItem);
    return d->subMenu;
}

void QQuickMenuItem::componentComplete()
{
    Q_D(QQuickMenuItem);
    d->executeArrow(true);
    QQuickAbstractButton::componentComplete();
}

QFont QQuickMenuItem::defaultFont() const
{
    return QQuickTheme::font(QQuickTheme::Menu);
}

QPalette QQuickMenuItem::defaultPalette() const
{
    return QQuickTheme::palette(QQuickTheme::Menu);
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickMenuItem::accessibleRole() const
{
    return QAccessible::MenuItem;
}
#endif

QT_END_NAMESPACE
