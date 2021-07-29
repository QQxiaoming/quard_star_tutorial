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

#include "qquickscrollview_p.h"
#include "qquickpane_p_p.h"
#include "qquickscrollbar_p_p.h"

#include <QtQuick/private/qquickflickable_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ScrollView
    \inherits Pane
    \instantiates QQuickScrollView
    \inqmlmodule QtQuick.Controls
    \since 5.9
    \ingroup qtquickcontrols2-containers
    \ingroup qtquickcontrols2-focusscopes
    \brief Scrollable view.

    ScrollView provides scrolling for user-defined content. It can be used to
    either replace a \l Flickable, or to decorate an existing one.

    \image qtquickcontrols2-scrollview.png

    The first example demonstrates the simplest usage of ScrollView.

    \snippet qtquickcontrols2-scrollview.qml file

    \note ScrollView does not automatically clip its contents. If it is not used as
    a full-screen item, you should consider setting the \l {Item::}{clip} property
    to \c true, as shown above.

    The second example illustrates using an existing \l Flickable, that is,
    a \l ListView.

    \snippet qtquickcontrols2-scrollview-listview.qml file

    \section2 Sizing

    As with Flickable, there are several things to keep in mind when using
    ScrollView:
    \list
        \li If only a single item is used within a ScrollView, the content size is
            automatically calculated based on the implicit size of its contained item.
            However, if more than one item is used (or an implicit size is not
            provided), the \l {QtQuick.Controls::Pane::}{contentWidth} and
            \l {QtQuick.Controls::Pane::}{contentHeight} properties must
            be set to the combined size of its contained items.
        \li If the content size is less than or equal to the size of the ScrollView,
            it will not be flickable.
    \endlist

    \section2 Scroll Bars

    The horizontal and vertical scroll bars can be accessed and customized using
    the \l {ScrollBar::horizontal}{ScrollBar.horizontal} and \l {ScrollBar::vertical}
    {ScrollBar.vertical} attached properties. The following example adjusts the scroll
    bar policies so that the horizontal scroll bar is always off, and the vertical
    scroll bar is always on.

    \snippet qtquickcontrols2-scrollview-policy.qml file

    \section2 Touch vs. Mouse Interaction

    On touch, ScrollView enables flicking and makes the scroll bars non-interactive.

    \image qtquickcontrols2-scrollindicator.gif

    When interacted with a mouse device, flicking is disabled and the scroll bars
    are interactive.

    \image qtquickcontrols2-scrollbar.gif

    Scroll bars can be made interactive on touch, or non-interactive when interacted
    with a mouse device, by setting the \l {ScrollBar::}{interactive} property explicitly
    to \c true or \c false, respectively.

    \snippet qtquickcontrols2-scrollview-interactive.qml file

    \sa ScrollBar, ScrollIndicator, {Customizing ScrollView}, {Container Controls},
        {Focus Management in Qt Quick Controls 2}
*/

class QQuickScrollViewPrivate : public QQuickPanePrivate
{
    Q_DECLARE_PUBLIC(QQuickScrollView)

public:
    QQmlListProperty<QObject> contentData() override;
    QQmlListProperty<QQuickItem> contentChildren() override;
    QList<QQuickItem *> contentChildItems() const override;

    QQuickItem *getContentItem() override;

    QQuickFlickable *ensureFlickable(bool content);
    bool setFlickable(QQuickFlickable *flickable, bool content);

    void flickableContentWidthChanged();
    void flickableContentHeightChanged();

    qreal getContentWidth() const override;
    qreal getContentHeight() const override;

    QQuickScrollBar *verticalScrollBar() const;
    QQuickScrollBar *horizontalScrollBar() const;

    void setScrollBarsInteractive(bool interactive);

    static void contentData_append(QQmlListProperty<QObject> *prop, QObject *obj);
    static int contentData_count(QQmlListProperty<QObject> *prop);
    static QObject *contentData_at(QQmlListProperty<QObject> *prop, int index);
    static void contentData_clear(QQmlListProperty<QObject> *prop);

    static void contentChildren_append(QQmlListProperty<QQuickItem> *prop, QQuickItem *obj);
    static int contentChildren_count(QQmlListProperty<QQuickItem> *prop);
    static QQuickItem *contentChildren_at(QQmlListProperty<QQuickItem> *prop, int index);
    static void contentChildren_clear(QQmlListProperty<QQuickItem> *prop);

    void itemImplicitWidthChanged(QQuickItem *item) override;

    bool wasTouched = false;
    QQuickFlickable *flickable = nullptr;
    bool flickableHasExplicitContentWidth = true;
    bool flickableHasExplicitContentHeight = true;
};

QList<QQuickItem *> QQuickScrollViewPrivate::contentChildItems() const
{
    if (!flickable)
        return QList<QQuickItem *>();

    return flickable->contentItem()->childItems();
}

QQuickItem *QQuickScrollViewPrivate::getContentItem()
{
    if (!contentItem)
        executeContentItem();
    return ensureFlickable(false);
}

QQuickFlickable *QQuickScrollViewPrivate::ensureFlickable(bool content)
{
    Q_Q(QQuickScrollView);
    if (!flickable) {
        flickableHasExplicitContentWidth = false;
        flickableHasExplicitContentHeight = false;
        setFlickable(new QQuickFlickable(q), content);
    }
    return flickable;
}

bool QQuickScrollViewPrivate::setFlickable(QQuickFlickable *item, bool content)
{
    Q_Q(QQuickScrollView);
    if (item == flickable)
        return false;

    QQuickScrollBarAttached *attached = qobject_cast<QQuickScrollBarAttached *>(qmlAttachedPropertiesObject<QQuickScrollBar>(q, false));

    if (flickable) {
        flickable->removeEventFilter(q);

        if (attached)
            QQuickScrollBarAttachedPrivate::get(attached)->setFlickable(nullptr);

        QObjectPrivate::disconnect(flickable->contentItem(), &QQuickItem::childrenChanged, this, &QQuickPanePrivate::contentChildrenChange);
        QObjectPrivate::disconnect(flickable, &QQuickFlickable::contentWidthChanged, this, &QQuickScrollViewPrivate::flickableContentWidthChanged);
        QObjectPrivate::disconnect(flickable, &QQuickFlickable::contentHeightChanged, this, &QQuickScrollViewPrivate::flickableContentHeightChanged);
    }

    flickable = item;
    if (content)
        q->setContentItem(flickable);

    if (flickable) {
        flickable->installEventFilter(q);
        if (hasContentWidth)
            flickable->setContentWidth(contentWidth);
        else
            flickableContentWidthChanged();
        if (hasContentHeight)
            flickable->setContentHeight(contentHeight);
        else
            flickableContentHeightChanged();

        if (attached)
            QQuickScrollBarAttachedPrivate::get(attached)->setFlickable(flickable);

        QObjectPrivate::connect(flickable->contentItem(), &QQuickItem::childrenChanged, this, &QQuickPanePrivate::contentChildrenChange);
        QObjectPrivate::connect(flickable, &QQuickFlickable::contentWidthChanged, this, &QQuickScrollViewPrivate::flickableContentWidthChanged);
        QObjectPrivate::connect(flickable, &QQuickFlickable::contentHeightChanged, this, &QQuickScrollViewPrivate::flickableContentHeightChanged);
    }

    return true;
}

void QQuickScrollViewPrivate::flickableContentWidthChanged()
{
    Q_Q(QQuickScrollView);
    if (!flickable || !componentComplete)
        return;

    const qreal cw = flickable->contentWidth();
    if (qFuzzyCompare(cw, implicitContentWidth))
        return;

    flickableHasExplicitContentWidth = true;
    implicitContentWidth = cw;
    emit q->implicitContentWidthChanged();
}

void QQuickScrollViewPrivate::flickableContentHeightChanged()
{
    Q_Q(QQuickScrollView);
    if (!flickable || !componentComplete)
        return;

    const qreal ch = flickable->contentHeight();
    if (qFuzzyCompare(ch, implicitContentHeight))
        return;

    flickableHasExplicitContentHeight = true;
    implicitContentHeight = ch;
    emit q->implicitContentHeightChanged();
}

qreal QQuickScrollViewPrivate::getContentWidth() const
{
    if (flickable && flickableHasExplicitContentWidth)
        return flickable->contentWidth();

    // The scrollview wraps a flickable created by us, and nobody searched for it and
    // modified its contentWidth. In that case, since the application does not control
    // this flickable, we fall back to calculate the content width based on the child
    // items inside it.
    return QQuickPanePrivate::getContentWidth();
}

qreal QQuickScrollViewPrivate::getContentHeight() const
{
    if (flickable && flickableHasExplicitContentHeight)
        return flickable->contentHeight();

    // The scrollview wraps a flickable created by us, and nobody searched for it and
    // modified its contentHeight. In that case, since the application does not control
    // this flickable, we fall back to calculate the content height based on the child
    // items inside it.
    return QQuickPanePrivate::getContentHeight();
}

QQuickScrollBar *QQuickScrollViewPrivate::verticalScrollBar() const
{
    Q_Q(const QQuickScrollView);
    QQuickScrollBarAttached *attached = qobject_cast<QQuickScrollBarAttached *>(qmlAttachedPropertiesObject<QQuickScrollBar>(q, false));
    if (!attached)
        return nullptr;
    return attached->vertical();
}

QQuickScrollBar *QQuickScrollViewPrivate::horizontalScrollBar() const
{
    Q_Q(const QQuickScrollView);
    QQuickScrollBarAttached *attached = qobject_cast<QQuickScrollBarAttached *>(qmlAttachedPropertiesObject<QQuickScrollBar>(q, false));
    if (!attached)
        return nullptr;
    return attached->horizontal();
}

void QQuickScrollViewPrivate::setScrollBarsInteractive(bool interactive)
{
    QQuickScrollBar *hbar = horizontalScrollBar();
    if (hbar) {
        QQuickScrollBarPrivate *p = QQuickScrollBarPrivate::get(hbar);
        if (!p->explicitInteractive)
            p->setInteractive(interactive);
    }

    QQuickScrollBar *vbar = verticalScrollBar();
    if (vbar) {
        QQuickScrollBarPrivate *p = QQuickScrollBarPrivate::get(vbar);
        if (!p->explicitInteractive)
            p->setInteractive(interactive);
    }
}

void QQuickScrollViewPrivate::contentData_append(QQmlListProperty<QObject> *prop, QObject *obj)
{
    QQuickScrollViewPrivate *p = static_cast<QQuickScrollViewPrivate *>(prop->data);
    if (!p->flickable && p->setFlickable(qobject_cast<QQuickFlickable *>(obj), true))
        return;

    QQuickFlickable *flickable = p->ensureFlickable(true);
    Q_ASSERT(flickable);
    QQmlListProperty<QObject> data = flickable->flickableData();
    data.append(&data, obj);
}

int QQuickScrollViewPrivate::contentData_count(QQmlListProperty<QObject> *prop)
{
    QQuickScrollViewPrivate *p = static_cast<QQuickScrollViewPrivate *>(prop->data);
    if (!p->flickable)
        return 0;

    QQmlListProperty<QObject> data = p->flickable->flickableData();
    return data.count(&data);
}

QObject *QQuickScrollViewPrivate::contentData_at(QQmlListProperty<QObject> *prop, int index)
{
    QQuickScrollViewPrivate *p = static_cast<QQuickScrollViewPrivate *>(prop->data);
    if (!p->flickable)
        return nullptr;

    QQmlListProperty<QObject> data = p->flickable->flickableData();
    return data.at(&data, index);
}

void QQuickScrollViewPrivate::contentData_clear(QQmlListProperty<QObject> *prop)
{
    QQuickScrollViewPrivate *p = static_cast<QQuickScrollViewPrivate *>(prop->data);
    if (!p->flickable)
        return;

    QQmlListProperty<QObject> data = p->flickable->flickableData();
    return data.clear(&data);
}

void QQuickScrollViewPrivate::contentChildren_append(QQmlListProperty<QQuickItem> *prop, QQuickItem *item)
{
    QQuickScrollViewPrivate *p = static_cast<QQuickScrollViewPrivate *>(prop->data);
    if (!p->flickable)
        p->setFlickable(qobject_cast<QQuickFlickable *>(item), true);

    QQuickFlickable *flickable = p->ensureFlickable(true);
    Q_ASSERT(flickable);
    QQmlListProperty<QQuickItem> children = flickable->flickableChildren();
    children.append(&children, item);
}

int QQuickScrollViewPrivate::contentChildren_count(QQmlListProperty<QQuickItem> *prop)
{
    QQuickScrollViewPrivate *p = static_cast<QQuickScrollViewPrivate *>(prop->data);
    if (!p->flickable)
        return 0;

    QQmlListProperty<QQuickItem> children = p->flickable->flickableChildren();
    return children.count(&children);
}

QQuickItem *QQuickScrollViewPrivate::contentChildren_at(QQmlListProperty<QQuickItem> *prop, int index)
{
    QQuickScrollViewPrivate *p = static_cast<QQuickScrollViewPrivate *>(prop->data);
    if (!p->flickable)
        return nullptr;

    QQmlListProperty<QQuickItem> children = p->flickable->flickableChildren();
    return children.at(&children, index);
}

void QQuickScrollViewPrivate::contentChildren_clear(QQmlListProperty<QQuickItem> *prop)
{
    QQuickScrollViewPrivate *p = static_cast<QQuickScrollViewPrivate *>(prop->data);
    if (!p->flickable)
        return;

    QQmlListProperty<QQuickItem> children = p->flickable->flickableChildren();
    children.clear(&children);
}

void QQuickScrollViewPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    // a special case for width<->height dependent content (wrapping text) in ScrollView
    if (contentWidth < 0 && !componentComplete)
        return;

    QQuickPanePrivate::itemImplicitWidthChanged(item);
}

QQuickScrollView::QQuickScrollView(QQuickItem *parent)
    : QQuickPane(*(new QQuickScrollViewPrivate), parent)
{
    Q_D(QQuickScrollView);
    d->contentWidth = -1;
    d->contentHeight = -1;

    setFiltersChildMouseEvents(true);
    setWheelEnabled(true);
}

/*!
    \qmlproperty list<Object> QtQuick.Controls::ScrollView::contentData
    \default

    This property holds the list of content data.

    The list contains all objects that have been declared in QML as children of the view.

    \note Unlike \c contentChildren, \c contentData does include non-visual QML objects.

    \sa Item::data, contentChildren
*/
QQmlListProperty<QObject> QQuickScrollViewPrivate::contentData()
{
    Q_Q(QQuickScrollView);
    return QQmlListProperty<QObject>(q, this,
                                     QQuickScrollViewPrivate::contentData_append,
                                     QQuickScrollViewPrivate::contentData_count,
                                     QQuickScrollViewPrivate::contentData_at,
                                     QQuickScrollViewPrivate::contentData_clear);
}

/*!
    \qmlproperty list<Item> QtQuick.Controls::ScrollView::contentChildren

    This property holds the list of content children.

    The list contains all items that have been declared in QML as children of the view.

    \note Unlike \c contentData, \c contentChildren does not include non-visual QML objects.

    \sa Item::children, contentData
*/
QQmlListProperty<QQuickItem> QQuickScrollViewPrivate::contentChildren()
{
    Q_Q(QQuickScrollView);
    return QQmlListProperty<QQuickItem>(q, this,
                                        QQuickScrollViewPrivate::contentChildren_append,
                                        QQuickScrollViewPrivate::contentChildren_count,
                                        QQuickScrollViewPrivate::contentChildren_at,
                                        QQuickScrollViewPrivate::contentChildren_clear);
}

bool QQuickScrollView::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    Q_D(QQuickScrollView);
    switch (event->type()) {
    case QEvent::TouchBegin:
        d->wasTouched = true;
        d->setScrollBarsInteractive(false);
        return false;

    case QEvent::TouchEnd:
        d->wasTouched = false;
        return false;

    case QEvent::MouseButtonPress:
        // NOTE: Flickable does not handle touch events, only synthesized mouse events
        if (static_cast<QMouseEvent *>(event)->source() == Qt::MouseEventNotSynthesized) {
            d->wasTouched = false;
            d->setScrollBarsInteractive(true);
            return false;
        }
        return !d->wasTouched && item == d->flickable;

    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
        if (static_cast<QMouseEvent *>(event)->source() == Qt::MouseEventNotSynthesized)
            return item == d->flickable;
        break;

    case QEvent::HoverEnter:
    case QEvent::HoverMove:
        if (d->wasTouched && (item == d->verticalScrollBar() || item == d->horizontalScrollBar()))
            d->setScrollBarsInteractive(true);
        break;

    default:
        break;
    }

    return false;
}

bool QQuickScrollView::eventFilter(QObject *object, QEvent *event)
{
    Q_D(QQuickScrollView);
    if (event->type() == QEvent::Wheel) {
        d->setScrollBarsInteractive(true);
        if (!d->wheelEnabled)
            return true;
    }
    return QQuickPane::eventFilter(object, event);
}

void QQuickScrollView::keyPressEvent(QKeyEvent *event)
{
    Q_D(QQuickScrollView);
    QQuickPane::keyPressEvent(event);
    switch (event->key()) {
    case Qt::Key_Up:
        if (QQuickScrollBar *vbar = d->verticalScrollBar()) {
            vbar->decrease();
            event->accept();
        }
        break;
    case Qt::Key_Down:
        if (QQuickScrollBar *vbar = d->verticalScrollBar()) {
            vbar->increase();
            event->accept();
        }
        break;
    case Qt::Key_Left:
        if (QQuickScrollBar *hbar = d->horizontalScrollBar()) {
            hbar->decrease();
            event->accept();
        }
        break;
    case Qt::Key_Right:
        if (QQuickScrollBar *hbar = d->horizontalScrollBar()) {
            hbar->increase();
            event->accept();
        }
        break;
    default:
        event->ignore();
        break;
    }
}

void QQuickScrollView::componentComplete()
{
    Q_D(QQuickScrollView);
    QQuickPane::componentComplete();
    if (!d->contentItem)
        d->ensureFlickable(true);
}

void QQuickScrollView::contentItemChange(QQuickItem *newItem, QQuickItem *oldItem)
{
    Q_D(QQuickScrollView);
    if (newItem != d->flickable) {
        // The new flickable was not created by us. In that case, we always
        // assume/require that it has an explicit content size assigned.
        d->flickableHasExplicitContentWidth = true;
        d->flickableHasExplicitContentHeight = true;
        d->setFlickable(qobject_cast<QQuickFlickable *>(newItem), false);
    }
    QQuickPane::contentItemChange(newItem, oldItem);
}

void QQuickScrollView::contentSizeChange(const QSizeF &newSize, const QSizeF &oldSize)
{
    Q_D(QQuickScrollView);
    QQuickPane::contentSizeChange(newSize, oldSize);
    if (d->flickable) {
        // Only set the content size on the flickable if the flickable doesn't
        // have an explicit assignment from before. Otherwise we can end up overwriting
        // assignments done to those properties by the application. The
        // exception is if the application has assigned a content size
        // directly to the scrollview, which will then win even if the
        // application has assigned something else to the flickable.
        if (d->hasContentWidth || !d->flickableHasExplicitContentWidth)
            d->flickable->setContentWidth(newSize.width());
        if (d->hasContentHeight || !d->flickableHasExplicitContentHeight)
            d->flickable->setContentHeight(newSize.height());
    }
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickScrollView::accessibleRole() const
{
    return QAccessible::Pane;
}
#endif

QT_END_NAMESPACE

#include "moc_qquickscrollview_p.cpp"
