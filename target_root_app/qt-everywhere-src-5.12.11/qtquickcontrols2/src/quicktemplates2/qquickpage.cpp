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

#include "qquickpage_p.h"
#include "qquickpage_p_p.h"
#include "qquicktabbar_p.h"
#include "qquicktoolbar_p.h"
#include "qquickdialogbuttonbox_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Page
    \inherits Pane
    \instantiates QQuickPage
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-containers
    \ingroup qtquickcontrols2-focusscopes
    \brief Styled page control with support for a header and footer.

    Page is a container control which makes it convenient to add
    a \l header and \l footer item to a page.

    \image qtquickcontrols2-page-wireframe.png

    The following example snippet illustrates how to use a page-specific
    toolbar header and an application-wide tabbar footer.

    \qml
    import QtQuick.Controls 2.12

    ApplicationWindow {
        visible: true

        StackView {
            anchors.fill: parent

            initialItem: Page {
                header: ToolBar {
                    // ...
                }
            }
        }

        footer: TabBar {
            // ...
        }
    }
    \endqml

    \sa ApplicationWindow, {Container Controls},
        {Focus Management in Qt Quick Controls 2}
*/

static const QQuickItemPrivate::ChangeTypes LayoutChanges = QQuickItemPrivate::Geometry | QQuickItemPrivate::Visibility | QQuickItemPrivate::Destroyed
                                                          | QQuickItemPrivate::ImplicitWidth | QQuickItemPrivate::ImplicitHeight;

namespace {
    enum Position {
        Header,
        Footer
    };

    Q_STATIC_ASSERT(int(Header) == int(QQuickTabBar::Header));
    Q_STATIC_ASSERT(int(Footer) == int(QQuickTabBar::Footer));

    Q_STATIC_ASSERT(int(Header) == int(QQuickToolBar::Header));
    Q_STATIC_ASSERT(int(Footer) == int(QQuickToolBar::Footer));

    Q_STATIC_ASSERT(int(Header) == int(QQuickDialogButtonBox::Header));
    Q_STATIC_ASSERT(int(Footer) == int(QQuickDialogButtonBox::Footer));

    static void setPos(QQuickItem *item, Position position)
    {
        if (QQuickToolBar *toolBar = qobject_cast<QQuickToolBar *>(item))
            toolBar->setPosition(static_cast<QQuickToolBar::Position>(position));
        else if (QQuickTabBar *tabBar = qobject_cast<QQuickTabBar *>(item))
            tabBar->setPosition(static_cast<QQuickTabBar::Position>(position));
        else if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(item))
            buttonBox->setPosition(static_cast<QQuickDialogButtonBox::Position>(position));
    }
}

void QQuickPagePrivate::relayout()
{
    Q_Q(QQuickPage);
    const qreal hh = header && header->isVisible() ? header->height() : 0;
    const qreal fh = footer && footer->isVisible() ? footer->height() : 0;
    const qreal hsp = hh > 0 ? spacing : 0;
    const qreal fsp = fh > 0 ? spacing : 0;

    if (contentItem) {
        contentItem->setY(q->topPadding() + hh + hsp);
        contentItem->setX(q->leftPadding());
        contentItem->setWidth(q->availableWidth());
        contentItem->setHeight(q->availableHeight() - hh - fh - hsp - fsp);
    }

    if (header)
        header->setWidth(q->width());

    if (footer) {
        footer->setY(q->height() - footer->height());
        footer->setWidth(q->width());
    }
}

void QQuickPagePrivate::resizeContent()
{
    relayout();
}

void QQuickPagePrivate::itemVisibilityChanged(QQuickItem *item)
{
    Q_Q(QQuickPage);
    QQuickPanePrivate::itemVisibilityChanged(item);
    if (item == header) {
        QBoolBlocker signalGuard(emittingImplicitSizeChangedSignals);
        emit q->implicitHeaderWidthChanged();
        emit q->implicitHeaderHeightChanged();
        relayout();
    } else if (item == footer) {
        QBoolBlocker signalGuard(emittingImplicitSizeChangedSignals);
        emit q->implicitFooterWidthChanged();
        emit q->implicitFooterHeightChanged();
        relayout();
    }
}

void QQuickPagePrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    Q_Q(QQuickPage);
    QQuickPanePrivate::itemImplicitWidthChanged(item);

    // Avoid binding loops by skipping signal emission if we're already doing it.
    if (emittingImplicitSizeChangedSignals)
        return;

    if (item == header)
        emit q->implicitHeaderWidthChanged();
    else if (item == footer)
        emit q->implicitFooterWidthChanged();
}

void QQuickPagePrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    Q_Q(QQuickPage);
    QQuickPanePrivate::itemImplicitHeightChanged(item);

    // Avoid binding loops by skipping signal emission if we're already doing it.
    if (emittingImplicitSizeChangedSignals)
        return;

    if (item == header)
        emit q->implicitHeaderHeightChanged();
    else if (item == footer)
        emit q->implicitFooterHeightChanged();
}

void QQuickPagePrivate::itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF & diff)
{
    QQuickPanePrivate::itemGeometryChanged(item, change, diff);
    if (item == header || item == footer)
        relayout();
}

void QQuickPagePrivate::itemDestroyed(QQuickItem *item)
{
    Q_Q(QQuickPage);
    QQuickPanePrivate::itemDestroyed(item);
    if (item == header) {
        header = nullptr;
        relayout();
        emit q->implicitHeaderWidthChanged();
        emit q->implicitHeaderHeightChanged();
        emit q->headerChanged();
    } else if (item == footer) {
        footer = nullptr;
        relayout();
        emit q->implicitFooterWidthChanged();
        emit q->implicitFooterHeightChanged();
        emit q->footerChanged();
    }
}

QQuickPage::QQuickPage(QQuickItem *parent)
    : QQuickPane(*(new QQuickPagePrivate), parent)
{
}

QQuickPage::QQuickPage(QQuickPagePrivate &dd, QQuickItem *parent)
    : QQuickPane(dd, parent)
{
}

QQuickPage::~QQuickPage()
{
    Q_D(QQuickPage);
    if (d->header)
        QQuickItemPrivate::get(d->header)->removeItemChangeListener(d, LayoutChanges);
    if (d->footer)
        QQuickItemPrivate::get(d->footer)->removeItemChangeListener(d, LayoutChanges);
}

/*!
    \qmlproperty string QtQuick.Controls::Page::title

    This property holds the page title.

    The title is often displayed at the top of a page to give
    the user context about the page they are viewing.

    \code
    ApplicationWindow {
        visible: true
        width: 400
        height: 400

        header: Label {
            text: view.currentItem.title
            horizontalAlignment: Text.AlignHCenter
        }

        SwipeView {
            id: view
            anchors.fill: parent

            Page {
                title: qsTr("Home")
            }
            Page {
                title: qsTr("Discover")
            }
            Page {
                title: qsTr("Activity")
            }
        }
    }
    \endcode
*/

QString QQuickPage::title() const
{
    return d_func()->title;
}

void QQuickPage::setTitle(const QString &title)
{
    Q_D(QQuickPage);
    if (d->title == title)
        return;

    d->title = title;
    setAccessibleName(title);
    emit titleChanged();
}

/*!
    \qmlproperty Item QtQuick.Controls::Page::header

    This property holds the page header item. The header item is positioned to
    the top, and resized to the width of the page. The default value is \c null.

    \note Assigning a ToolBar, TabBar, or DialogButtonBox as a page header
    automatically sets the respective \l ToolBar::position, \l TabBar::position,
    or \l DialogButtonBox::position property to \c Header.

    \sa footer, ApplicationWindow::header
*/
QQuickItem *QQuickPage::header() const
{
    Q_D(const QQuickPage);
    return d->header;
}

void QQuickPage::setHeader(QQuickItem *header)
{
    Q_D(QQuickPage);
    if (d->header == header)
        return;

    if (d->header) {
        QQuickItemPrivate::get(d->header)->removeItemChangeListener(d, LayoutChanges);
        d->header->setParentItem(nullptr);
    }
    d->header = header;
    if (header) {
        header->setParentItem(this);
        QQuickItemPrivate::get(header)->addItemChangeListener(d, LayoutChanges);
        if (qFuzzyIsNull(header->z()))
            header->setZ(1);
        setPos(header, Header);
    }
    if (isComponentComplete())
        d->relayout();
    emit headerChanged();
}

/*!
    \qmlproperty Item QtQuick.Controls::Page::footer

    This property holds the page footer item. The footer item is positioned to
    the bottom, and resized to the width of the page. The default value is \c null.

    \note Assigning a ToolBar, TabBar, or DialogButtonBox as a page footer
    automatically sets the respective \l ToolBar::position, \l TabBar::position,
    or \l DialogButtonBox::position property to \c Footer.

    \sa header, ApplicationWindow::footer
*/
QQuickItem *QQuickPage::footer() const
{
    Q_D(const QQuickPage);
    return d->footer;
}

void QQuickPage::setFooter(QQuickItem *footer)
{
    Q_D(QQuickPage);
    if (d->footer == footer)
        return;

    if (d->footer) {
        QQuickItemPrivate::get(d->footer)->removeItemChangeListener(d, LayoutChanges);
        d->footer->setParentItem(nullptr);
    }
    d->footer = footer;
    if (footer) {
        footer->setParentItem(this);
        QQuickItemPrivate::get(footer)->addItemChangeListener(d, LayoutChanges);
        if (qFuzzyIsNull(footer->z()))
            footer->setZ(1);
        setPos(footer, Footer);
    }
    if (isComponentComplete())
        d->relayout();
    emit footerChanged();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Page::implicitHeaderWidth
    \readonly

    This property holds the implicit header width.

    The value is equal to \c {header && header.visible ? header.implicitWidth : 0}.

    \sa implicitHeaderHeight, implicitFooterWidth
*/
qreal QQuickPage::implicitHeaderWidth() const
{
    Q_D(const QQuickPage);
    if (!d->header || !d->header->isVisible())
        return 0;
    return d->header->implicitWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Page::implicitHeaderHeight
    \readonly

    This property holds the implicit header height.

    The value is equal to \c {header && header.visible ? header.implicitHeight : 0}.

    \sa implicitHeaderWidth, implicitFooterHeight
*/
qreal QQuickPage::implicitHeaderHeight() const
{
    Q_D(const QQuickPage);
    if (!d->header || !d->header->isVisible())
        return 0;
    return d->header->implicitHeight();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Page::implicitFooterWidth
    \readonly

    This property holds the implicit footer width.

    The value is equal to \c {footer && footer.visible ? footer.implicitWidth : 0}.

    \sa implicitFooterHeight, implicitHeaderWidth
*/
qreal QQuickPage::implicitFooterWidth() const
{
    Q_D(const QQuickPage);
    if (!d->footer || !d->footer->isVisible())
        return 0;
    return d->footer->implicitWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Page::implicitFooterHeight
    \readonly

    This property holds the implicit footer height.

    The value is equal to \c {footer && footer.visible ? footer.implicitHeight : 0}.

    \sa implicitFooterWidth, implicitHeaderHeight
*/
qreal QQuickPage::implicitFooterHeight() const
{
    Q_D(const QQuickPage);
    if (!d->footer || !d->footer->isVisible())
        return 0;
    return d->footer->implicitHeight();
}

void QQuickPage::componentComplete()
{
    Q_D(QQuickPage);
    QQuickPane::componentComplete();
    d->relayout();
}

void QQuickPage::spacingChange(qreal newSpacing, qreal oldSpacing)
{
    Q_D(QQuickPage);
    QQuickPane::spacingChange(newSpacing, oldSpacing);
    d->relayout();
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickPage::accessibleRole() const
{
    return QAccessible::PageTab;
}

void QQuickPage::accessibilityActiveChanged(bool active)
{
    Q_D(QQuickPage);
    QQuickPane::accessibilityActiveChanged(active);

    if (active)
        setAccessibleName(d->title);
}
#endif

QT_END_NAMESPACE
