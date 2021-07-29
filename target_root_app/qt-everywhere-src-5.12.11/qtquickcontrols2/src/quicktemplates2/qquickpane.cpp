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

#include "qquickpane_p.h"
#include "qquickpane_p_p.h"
#include "qquickcontentitem_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Pane
    \inherits Control
    \instantiates QQuickPane
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-containers
    \ingroup qtquickcontrols2-focusscopes
    \brief Provides a background matching with the application style and theme.

    Pane provides a background color that matches with the application style
    and theme. Pane does not provide a layout of its own, but requires you to
    position its contents, for instance by creating a \l RowLayout or a
    \l ColumnLayout.

    Items declared as children of a Pane are automatically parented to the
    Pane's \l {Control::}{contentItem}. Items created dynamically need to be
    explicitly parented to the contentItem.

    \section1 Content Sizing

    If only a single item is used within a Pane, it will resize to fit the
    implicit size of its contained item. This makes it particularly suitable
    for use together with layouts.

    \image qtquickcontrols2-pane.png

    \snippet qtquickcontrols2-pane.qml 1

    Sometimes there might be two items within the pane:

    \code
    Pane {
        SwipeView {
            // ...
        }
        PageIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
        }
    }
    \endcode

    In this case, Pane cannot calculate a sensible implicit size. Since we're
    anchoring the \l PageIndicator over the \l SwipeView, we can simply set the
    content size to the view's implicit size:

    \code
    Pane {
        contentWidth: view.implicitWidth
        contentHeight: view.implicitHeight

        SwipeView {
            id: view
            // ...
        }
        PageIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
        }
     }
    \endcode

    If the \l contentItem has no implicit size and only one child, Pane will
    use the implicit size of that child. For example, in the following code,
    the Pane will assume the size of the Rectangle:

    \code
    Pane {
        Item {
            Rectangle {
                implicitWidth: 200
                implicitHeight: 200
                color: "salmon"
            }
        }
    }
    \endcode

    \sa {Customizing Pane}, {Container Controls},
        {Focus Management in Qt Quick Controls 2}, {Event Handling}
*/

void QQuickPanePrivate::init()
{
    Q_Q(QQuickPane);
    q->setFlag(QQuickItem::ItemIsFocusScope);
    q->setAcceptedMouseButtons(Qt::AllButtons);
#if QT_CONFIG(cursor)
    q->setCursor(Qt::ArrowCursor);
#endif
    connect(q, &QQuickControl::implicitContentWidthChanged, this, &QQuickPanePrivate::updateContentWidth);
    connect(q, &QQuickControl::implicitContentHeightChanged, this, &QQuickPanePrivate::updateContentHeight);
}

QList<QQuickItem *> QQuickPanePrivate::contentChildItems() const
{
    if (!contentItem)
        return QList<QQuickItem *>();

    return contentItem->childItems();
}

QQuickItem *QQuickPanePrivate::getContentItem()
{
    Q_Q(QQuickPane);
    if (QQuickItem *item = QQuickControlPrivate::getContentItem())
        return item;

    return new QQuickContentItem(q);
}

void QQuickPanePrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    QQuickControlPrivate::itemImplicitWidthChanged(item);

    if (item == firstChild)
        updateImplicitContentWidth();
}

void QQuickPanePrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    QQuickControlPrivate::itemImplicitHeightChanged(item);

    if (item == firstChild)
        updateImplicitContentHeight();
}

void QQuickPanePrivate::contentChildrenChange()
{
    Q_Q(QQuickPane);
    QQuickItem *newFirstChild = contentChildItems().value(0);
    if (newFirstChild != firstChild) {
        if (firstChild)
            removeImplicitSizeListener(firstChild);
        if (newFirstChild)
            addImplicitSizeListener(newFirstChild);
        firstChild = newFirstChild;
    }

    updateImplicitContentSize();
    emit q->contentChildrenChanged();
}

qreal QQuickPanePrivate::getContentWidth() const
{
    if (!contentItem)
        return 0;

    const qreal cw = contentItem->implicitWidth();
    if (!qFuzzyIsNull(cw))
        return cw;

    const auto contentChildren = contentChildItems();
    if (contentChildren.count() == 1)
        return contentChildren.first()->implicitWidth();

    return 0;
}

qreal QQuickPanePrivate::getContentHeight() const
{
    if (!contentItem)
        return 0;

    const qreal ch = contentItem->implicitHeight();
    if (!qFuzzyIsNull(ch))
        return ch;

    const auto contentChildren = contentChildItems();
    if (contentChildren.count() == 1)
        return contentChildren.first()->implicitHeight();

    return 0;
}

void QQuickPanePrivate::updateContentWidth()
{
    Q_Q(QQuickPane);
    if (hasContentWidth || qFuzzyCompare(contentWidth, implicitContentWidth))
        return;

    const qreal oldContentWidth = contentWidth;
    contentWidth = implicitContentWidth;
    q->contentSizeChange(QSizeF(contentWidth, contentHeight), QSizeF(oldContentWidth, contentHeight));
    emit q->contentWidthChanged();
}

void QQuickPanePrivate::updateContentHeight()
{
    Q_Q(QQuickPane);
    if (hasContentHeight || qFuzzyCompare(contentHeight, implicitContentHeight))
        return;

    const qreal oldContentHeight = contentHeight;
    contentHeight = implicitContentHeight;
    q->contentSizeChange(QSizeF(contentWidth, contentHeight), QSizeF(contentWidth, oldContentHeight));
    emit q->contentHeightChanged();
}

QQuickPane::QQuickPane(QQuickItem *parent)
    : QQuickControl(*(new QQuickPanePrivate), parent)
{
    Q_D(QQuickPane);
    d->init();
}

QQuickPane::~QQuickPane()
{
    Q_D(QQuickPane);
    d->removeImplicitSizeListener(d->contentItem);
    d->removeImplicitSizeListener(d->firstChild);
}

QQuickPane::QQuickPane(QQuickPanePrivate &dd, QQuickItem *parent)
    : QQuickControl(dd, parent)
{
    Q_D(QQuickPane);
    d->init();
}

/*!
    \qmlproperty real QtQuick.Controls::Pane::contentWidth

    This property holds the content width. It is used for calculating the total
    implicit width of the pane.

    For more information, see \l {Content Sizing}.

    \sa contentHeight
*/
qreal QQuickPane::contentWidth() const
{
    Q_D(const QQuickPane);
    return d->contentWidth;
}

void QQuickPane::setContentWidth(qreal width)
{
    Q_D(QQuickPane);
    d->hasContentWidth = true;
    if (qFuzzyCompare(d->contentWidth, width))
        return;

    const qreal oldWidth = d->contentWidth;
    d->contentWidth = width;
    contentSizeChange(QSizeF(width, d->contentHeight), QSizeF(oldWidth, d->contentHeight));
    emit contentWidthChanged();
}

void QQuickPane::resetContentWidth()
{
    Q_D(QQuickPane);
    if (!d->hasContentWidth)
        return;

    d->hasContentHeight = false;
    d->updateContentWidth();
}

/*!
    \qmlproperty real QtQuick.Controls::Pane::contentHeight

    This property holds the content height. It is used for calculating the total
    implicit height of the pane.

    For more information, see \l {Content Sizing}.

    \sa contentWidth
*/
qreal QQuickPane::contentHeight() const
{
    Q_D(const QQuickPane);
    return d->contentHeight;
}

void QQuickPane::setContentHeight(qreal height)
{
    Q_D(QQuickPane);
    d->hasContentHeight = true;
    if (qFuzzyCompare(d->contentHeight, height))
        return;

    const qreal oldHeight = d->contentHeight;
    d->contentHeight = height;
    contentSizeChange(QSizeF(d->contentWidth, height), QSizeF(d->contentWidth, oldHeight));
    emit contentHeightChanged();
}

void QQuickPane::resetContentHeight()
{
    Q_D(QQuickPane);
    if (!d->hasContentHeight)
        return;

    d->hasContentHeight = false;
    d->updateContentHeight();
}

/*!
    \qmlproperty list<Object> QtQuick.Controls::Pane::contentData
    \default

    This property holds the list of content data.

    The list contains all objects that have been declared in QML as children
    of the pane.

    \note Unlike \c contentChildren, \c contentData does include non-visual QML
    objects.

    \sa Item::data, contentChildren
*/
QQmlListProperty<QObject> QQuickPanePrivate::contentData()
{
    Q_Q(QQuickPane);
    return QQmlListProperty<QObject>(q->contentItem(), nullptr,
                                     QQuickItemPrivate::data_append,
                                     QQuickItemPrivate::data_count,
                                     QQuickItemPrivate::data_at,
                                     QQuickItemPrivate::data_clear);
}

/*!
    \qmlproperty list<Item> QtQuick.Controls::Pane::contentChildren

    This property holds the list of content children.

    The list contains all items that have been declared in QML as children
    of the pane.

    \note Unlike \c contentData, \c contentChildren does not include non-visual
    QML objects.

    \sa Item::children, contentData
*/
QQmlListProperty<QQuickItem> QQuickPanePrivate::contentChildren()
{
    Q_Q(QQuickPane);
    return QQmlListProperty<QQuickItem>(q->contentItem(), nullptr,
                                        QQuickItemPrivate::children_append,
                                        QQuickItemPrivate::children_count,
                                        QQuickItemPrivate::children_at,
                                        QQuickItemPrivate::children_clear);
}

void QQuickPane::componentComplete()
{
    Q_D(QQuickPane);
    QQuickControl::componentComplete();
    d->updateImplicitContentSize();
}

void QQuickPane::contentItemChange(QQuickItem *newItem, QQuickItem *oldItem)
{
    Q_D(QQuickPane);
    QQuickControl::contentItemChange(newItem, oldItem);
    if (oldItem) {
        d->removeImplicitSizeListener(oldItem);
        QObjectPrivate::disconnect(oldItem, &QQuickItem::childrenChanged, d, &QQuickPanePrivate::contentChildrenChange);
    }
    if (newItem) {
        d->addImplicitSizeListener(newItem);
        QObjectPrivate::connect(newItem, &QQuickItem::childrenChanged, d, &QQuickPanePrivate::contentChildrenChange);
    }
    d->contentChildrenChange();
}

void QQuickPane::contentSizeChange(const QSizeF &newSize, const QSizeF &oldSize)
{
    Q_UNUSED(newSize)
    Q_UNUSED(oldSize)
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickPane::accessibleRole() const
{
    return QAccessible::Pane;
}
#endif

QT_END_NAMESPACE

#include "moc_qquickpane_p.cpp"
