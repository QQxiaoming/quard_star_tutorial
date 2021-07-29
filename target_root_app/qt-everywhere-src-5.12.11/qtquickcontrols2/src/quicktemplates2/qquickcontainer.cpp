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

#include "qquickcontainer_p.h"
#include "qquickcontainer_p_p.h"

#include <QtQuick/private/qquickflickable_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Container
    \inherits Control
    \instantiates QQuickContainer
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-containers
    \brief Abstract base type providing functionality common to containers.

    Container is the base type of container-like user interface controls that
    allow dynamic insertion and removal of items.

    \section2 Using Containers

    Typically, items are statically declared as children of Container, but it
    is also possible to \l {addItem}{add}, \l {insertItem}{insert},
    \l {moveItem}{move} and \l {removeItem}{remove} items dynamically. The
    items in a container can be accessed using \l itemAt() or
    \l contentChildren.

    Most containers have the concept of a "current" item. The current item is
    specified via the \l currentIndex property, and can be accessed using the
    read-only \l currentItem property.

    The following example illustrates dynamic insertion of items to a \l TabBar,
    which is one of the concrete implementations of Container.

    \code
    Row {
        TabBar {
            id: tabBar

            currentIndex: 0
            width: parent.width - addButton.width

            TabButton { text: "TabButton" }
        }

        Component {
            id: tabButton
            TabButton { text: "TabButton" }
        }

        Button {
            id: addButton
            text: "+"
            flat: true
            onClicked: {
                tabBar.addItem(tabButton.createObject(tabBar))
                console.log("added:", tabBar.itemAt(tabBar.count - 1))
            }
        }
    }
    \endcode

    \section2 Managing the Current Index

    When using multiple containers, such as \l TabBar and \l SwipeView, together,
    their \l currentIndex properties can be bound to each other to keep them in
    sync. When the user interacts with either container, its current index changes
    automatically propagate to the other container.

    Notice, however, that assigning a \c currentIndex value in JavaScript removes
    the respective binding. In order to retain the bindings, use the following
    methods to alter the current index:

    \list
        \li \l incrementCurrentIndex()
        \li \l decrementCurrentIndex()
        \li \l setCurrentIndex(int index)
    \endlist

    \code
    TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
    }

    SwipeView {
        id: swipeView
        currentIndex: tabBar.currentIndex
    }

    Button {
        text: qsTr("Home")
        onClicked: swipeView.setCurrentIndex(0)
        enabled: swipeView.currentIndex != 0
    }

    Button {
        text: qsTr("Previous")
        onClicked: swipeView.decrementCurrentIndex()
        enabled: swipeView.currentIndex > 0
    }

    Button {
        text: qsTr("Next")
        onClicked: swipeView.incrementCurrentIndex()
        enabled: swipeView.currentIndex < swipeView.count - 1
    }
    \endcode


    \section2 Implementing Containers

    Container does not provide any default visualization. It is used to implement
    such containers as \l SwipeView and \l TabBar. When implementing a custom
    container, the most important part of the API is \l contentModel, which provides
    the contained items in a way that it can be used as a delegate model for item
    views and repeaters.

    \code
    Container {
        id: container

        contentItem: ListView {
            model: container.contentModel
            snapMode: ListView.SnapOneItem
            orientation: ListView.Horizontal
        }

        Text {
            text: "Page 1"
            width: container.width
            height: container.height
        }

        Text {
            text: "Page 2"
            width: container.width
            height: container.height
        }
    }
    \endcode

    Notice how the sizes of the page items are set by hand. This is because the
    example uses a plain Container, which does not make any assumptions on the
    visual layout. It is typically not necessary to specify sizes for items in
    concrete Container implementations, such as \l SwipeView and \l TabBar.

    \sa {Container Controls}
*/

static QQuickItem *effectiveContentItem(QQuickItem *item)
{
    QQuickFlickable *flickable = qobject_cast<QQuickFlickable *>(item);
    if (flickable)
        return flickable->contentItem();
    return item;
}

void QQuickContainerPrivate::init()
{
    Q_Q(QQuickContainer);
    contentModel = new QQmlObjectModel(q);
    QObject::connect(contentModel, &QQmlObjectModel::countChanged, q, &QQuickContainer::countChanged);
    QObject::connect(contentModel, &QQmlObjectModel::childrenChanged, q, &QQuickContainer::contentChildrenChanged);
    connect(q, &QQuickControl::implicitContentWidthChanged, this, &QQuickContainerPrivate::updateContentWidth);
    connect(q, &QQuickControl::implicitContentHeightChanged, this, &QQuickContainerPrivate::updateContentHeight);
}

void QQuickContainerPrivate::cleanup()
{
    Q_Q(QQuickContainer);
    // ensure correct destruction order (QTBUG-46798)
    const int count = contentModel->count();
    for (int i = 0; i < count; ++i) {
        QQuickItem *item = itemAt(i);
        if (item)
            QQuickItemPrivate::get(item)->removeItemChangeListener(this, changeTypes);
    }

    if (contentItem) {
        QQuickItem *focusItem = QQuickItemPrivate::get(contentItem)->subFocusItem;
        if (focusItem && window)
            QQuickWindowPrivate::get(window)->clearFocusInScope(contentItem, focusItem, Qt::OtherFocusReason);

        q->contentItemChange(nullptr, contentItem);
        delete contentItem;
    }

    QObject::disconnect(contentModel, &QQmlObjectModel::countChanged, q, &QQuickContainer::countChanged);
    QObject::disconnect(contentModel, &QQmlObjectModel::childrenChanged, q, &QQuickContainer::contentChildrenChanged);
    delete contentModel;
}

QQuickItem *QQuickContainerPrivate::itemAt(int index) const
{
    return qobject_cast<QQuickItem *>(contentModel->get(index));
}

void QQuickContainerPrivate::insertItem(int index, QQuickItem *item)
{
    Q_Q(QQuickContainer);
    if (!q->isContent(item))
        return;
    contentData.append(item);

    updatingCurrent = true;

    item->setParentItem(effectiveContentItem(getContentItem()));
    QQuickItemPrivate::get(item)->addItemChangeListener(this, changeTypes);
    contentModel->insert(index, item);

    q->itemAdded(index, item);

    int count = contentModel->count();
    for (int i = index + 1; i < count; ++i)
        q->itemMoved(i, itemAt(i));

    if (count == 1 && currentIndex == -1)
        q->setCurrentIndex(index);

    updatingCurrent = false;
}

void QQuickContainerPrivate::moveItem(int from, int to, QQuickItem *item)
{
    Q_Q(QQuickContainer);
    int oldCurrent = currentIndex;
    contentModel->move(from, to);

    updatingCurrent = true;

    q->itemMoved(to, item);

    if (from < to) {
        for (int i = from; i < to; ++i)
            q->itemMoved(i, itemAt(i));
    } else {
        for (int i = from; i > to; --i)
            q->itemMoved(i, itemAt(i));
    }

    if (from == oldCurrent)
        q->setCurrentIndex(to);
    else if (from < oldCurrent && to >= oldCurrent)
        q->setCurrentIndex(oldCurrent - 1);
    else if (from > oldCurrent && to <= oldCurrent)
        q->setCurrentIndex(oldCurrent + 1);

    updatingCurrent = false;
}

void QQuickContainerPrivate::removeItem(int index, QQuickItem *item)
{
    Q_Q(QQuickContainer);
    if (!q->isContent(item))
        return;
    contentData.removeOne(item);

    updatingCurrent = true;

    int count = contentModel->count();
    bool currentChanged = false;
    if (index == currentIndex && (index != 0 || count == 1)) {
        q->setCurrentIndex(currentIndex - 1);
    } else if (index < currentIndex) {
        --currentIndex;
        currentChanged = true;
    }

    QQuickItemPrivate::get(item)->removeItemChangeListener(this, changeTypes);
    item->setParentItem(nullptr);
    contentModel->remove(index);
    --count;

    q->itemRemoved(index, item);

    for (int i = index; i < count; ++i)
        q->itemMoved(i, itemAt(i));

    if (currentChanged)
        emit q->currentIndexChanged();

    updatingCurrent = false;
}

void QQuickContainerPrivate::reorderItems()
{
    Q_Q(QQuickContainer);
    if (!contentItem)
        return;

    QList<QQuickItem *> siblings = effectiveContentItem(contentItem)->childItems();

    int to = 0;
    for (int i = 0; i < siblings.count(); ++i) {
        QQuickItem* sibling = siblings.at(i);
        if (QQuickItemPrivate::get(sibling)->isTransparentForPositioner())
            continue;
        int index = contentModel->indexOf(sibling, nullptr);
        q->moveItem(index, to++);
    }
}

void QQuickContainerPrivate::_q_currentIndexChanged()
{
    Q_Q(QQuickContainer);
    if (!updatingCurrent)
        q->setCurrentIndex(contentItem ? contentItem->property("currentIndex").toInt() : -1);
}

void QQuickContainerPrivate::itemChildAdded(QQuickItem *, QQuickItem *child)
{
    // add dynamically reparented items (eg. by a Repeater)
    if (!QQuickItemPrivate::get(child)->isTransparentForPositioner() && !contentData.contains(child))
        insertItem(contentModel->count(), child);
}

void QQuickContainerPrivate::itemParentChanged(QQuickItem *item, QQuickItem *parent)
{
    // remove dynamically unparented items (eg. by a Repeater)
    if (!parent)
        removeItem(contentModel->indexOf(item, nullptr), item);
}

void QQuickContainerPrivate::itemSiblingOrderChanged(QQuickItem *)
{
    if (!componentComplete)
        return;

    // reorder the restacked items (eg. by a Repeater)
    reorderItems();
}

void QQuickContainerPrivate::itemDestroyed(QQuickItem *item)
{
    int index = contentModel->indexOf(item, nullptr);
    if (index != -1)
        removeItem(index, item);
    else
        QQuickControlPrivate::itemDestroyed(item);
}

void QQuickContainerPrivate::contentData_append(QQmlListProperty<QObject> *prop, QObject *obj)
{
    QQuickContainer *q = static_cast<QQuickContainer *>(prop->object);
    QQuickContainerPrivate *p = QQuickContainerPrivate::get(q);
    QQuickItem *item = qobject_cast<QQuickItem *>(obj);
    if (item) {
        if (QQuickItemPrivate::get(item)->isTransparentForPositioner())
            item->setParentItem(effectiveContentItem(p->contentItem));
        else if (p->contentModel->indexOf(item, nullptr) == -1)
            q->addItem(item);
    } else {
        p->contentData.append(obj);
    }
}

int QQuickContainerPrivate::contentData_count(QQmlListProperty<QObject> *prop)
{
    QQuickContainer *q = static_cast<QQuickContainer *>(prop->object);
    return QQuickContainerPrivate::get(q)->contentData.count();
}

QObject *QQuickContainerPrivate::contentData_at(QQmlListProperty<QObject> *prop, int index)
{
    QQuickContainer *q = static_cast<QQuickContainer *>(prop->object);
    return QQuickContainerPrivate::get(q)->contentData.value(index);
}

void QQuickContainerPrivate::contentData_clear(QQmlListProperty<QObject> *prop)
{
    QQuickContainer *q = static_cast<QQuickContainer *>(prop->object);
    return QQuickContainerPrivate::get(q)->contentData.clear();
}

void QQuickContainerPrivate::contentChildren_append(QQmlListProperty<QQuickItem> *prop, QQuickItem *item)
{
    QQuickContainer *q = static_cast<QQuickContainer *>(prop->object);
    q->addItem(item);
}

int QQuickContainerPrivate::contentChildren_count(QQmlListProperty<QQuickItem> *prop)
{
    QQuickContainer *q = static_cast<QQuickContainer *>(prop->object);
    return QQuickContainerPrivate::get(q)->contentModel->count();
}

QQuickItem *QQuickContainerPrivate::contentChildren_at(QQmlListProperty<QQuickItem> *prop, int index)
{
    QQuickContainer *q = static_cast<QQuickContainer *>(prop->object);
    return q->itemAt(index);
}

void QQuickContainerPrivate::contentChildren_clear(QQmlListProperty<QQuickItem> *prop)
{
    QQuickContainer *q = static_cast<QQuickContainer *>(prop->object);
    return QQuickContainerPrivate::get(q)->contentModel->clear();
}

void QQuickContainerPrivate::updateContentWidth()
{
    Q_Q(QQuickContainer);
    if (hasContentWidth || qFuzzyCompare(contentWidth, implicitContentWidth))
        return;

    contentWidth = implicitContentWidth;
    emit q->contentWidthChanged();
}

void QQuickContainerPrivate::updateContentHeight()
{
    Q_Q(QQuickContainer);
    if (hasContentHeight || qFuzzyCompare(contentHeight, implicitContentHeight))
        return;

    contentHeight = implicitContentHeight;
    emit q->contentHeightChanged();
}

QQuickContainer::QQuickContainer(QQuickItem *parent)
    : QQuickControl(*(new QQuickContainerPrivate), parent)
{
    Q_D(QQuickContainer);
    d->init();
}

QQuickContainer::QQuickContainer(QQuickContainerPrivate &dd, QQuickItem *parent)
    : QQuickControl(dd, parent)
{
    Q_D(QQuickContainer);
    d->init();
}

QQuickContainer::~QQuickContainer()
{
    Q_D(QQuickContainer);
    d->cleanup();
}

/*!
    \qmlproperty int QtQuick.Controls::Container::count
    \readonly

    This property holds the number of items.
*/
int QQuickContainer::count() const
{
    Q_D(const QQuickContainer);
    return d->contentModel->count();
}

/*!
    \qmlmethod Item QtQuick.Controls::Container::itemAt(int index)

    Returns the item at \a index, or \c null if it does not exist.
*/
QQuickItem *QQuickContainer::itemAt(int index) const
{
    Q_D(const QQuickContainer);
    return d->itemAt(index);
}

/*!
    \qmlmethod void QtQuick.Controls::Container::addItem(Item item)

    Adds an \a item.
*/
void QQuickContainer::addItem(QQuickItem *item)
{
    Q_D(QQuickContainer);
    insertItem(d->contentModel->count(), item);
}

/*!
    \qmlmethod void QtQuick.Controls::Container::insertItem(int index, Item item)

    Inserts an \a item at \a index.
*/
void QQuickContainer::insertItem(int index, QQuickItem *item)
{
    Q_D(QQuickContainer);
    if (!item)
        return;
    const int count = d->contentModel->count();
    if (index < 0 || index > count)
        index = count;

    int oldIndex = d->contentModel->indexOf(item, nullptr);
    if (oldIndex != -1) {
        if (oldIndex < index)
            --index;
        if (oldIndex != index)
            d->moveItem(oldIndex, index, item);
    } else {
        d->insertItem(index, item);
    }
}

/*!
    \qmlmethod void QtQuick.Controls::Container::moveItem(int from, int to)

    Moves an item \a from one index \a to another.
*/
void QQuickContainer::moveItem(int from, int to)
{
    Q_D(QQuickContainer);
    const int count = d->contentModel->count();
    if (from < 0 || from > count - 1)
        return;
    if (to < 0 || to > count - 1)
        to = count - 1;

    if (from != to)
        d->moveItem(from, to, d->itemAt(from));
}

/*!
    \deprecated
    \qmlmethod void QtQuick.Controls::Container::removeItem(int index)

    Use Container::removeItem(Item) or Container::takeItem(int) instead.
*/
void QQuickContainer::removeItem(const QVariant &var)
{
    if (var.userType() == QMetaType::Nullptr)
        return;

    if (QQuickItem *item = var.value<QQuickItem *>())
        removeItem(item);
    else
        takeItem(var.toInt());
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlmethod void QtQuick.Controls::Container::removeItem(Item item)

    Removes and destroys the specified \a item.
*/
void QQuickContainer::removeItem(QQuickItem *item)
{
    Q_D(QQuickContainer);
    if (!item)
        return;

    const int index = d->contentModel->indexOf(item, nullptr);
    if (index == -1)
        return;

    d->removeItem(index, item);
    item->deleteLater();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlmethod Item QtQuick.Controls::Container::takeItem(int index)

    Removes and returns the item at \a index.

    \note The ownership of the item is transferred to the caller.
*/
QQuickItem *QQuickContainer::takeItem(int index)
{
    Q_D(QQuickContainer);
    const int count = d->contentModel->count();
    if (index < 0 || index >= count)
        return nullptr;

    QQuickItem *item = itemAt(index);
    if (item)
        d->removeItem(index, item);
    return item;
}

/*!
    \qmlproperty model QtQuick.Controls::Container::contentModel
    \readonly

    This property holds the content model of items.

    The content model is provided for visualization purposes. It can be assigned
    as a model to a content item that presents the contents of the container.

    \code
    Container {
        id: container
        contentItem: ListView {
            model: container.contentModel
        }
    }
    \endcode

    \sa contentData, contentChildren
*/
QVariant QQuickContainer::contentModel() const
{
    Q_D(const QQuickContainer);
    return QVariant::fromValue(d->contentModel);
}

/*!
    \qmlproperty list<Object> QtQuick.Controls::Container::contentData
    \default

    This property holds the list of content data.

    The list contains all objects that have been declared in QML as children
    of the container, and also items that have been dynamically added or
    inserted using the \l addItem() and \l insertItem() methods, respectively.

    \note Unlike \c contentChildren, \c contentData does include non-visual QML
    objects. It is not re-ordered when items are inserted or moved.

    \sa Item::data, contentChildren
*/
QQmlListProperty<QObject> QQuickContainer::contentData()
{
    Q_D(QQuickContainer);
    if (!d->contentItem)
        d->executeContentItem();
    return QQmlListProperty<QObject>(this, nullptr,
                                     QQuickContainerPrivate::contentData_append,
                                     QQuickContainerPrivate::contentData_count,
                                     QQuickContainerPrivate::contentData_at,
                                     QQuickContainerPrivate::contentData_clear);
}

/*!
    \qmlproperty list<Item> QtQuick.Controls::Container::contentChildren

    This property holds the list of content children.

    The list contains all items that have been declared in QML as children
    of the container, and also items that have been dynamically added or
    inserted using the \l addItem() and \l insertItem() methods, respectively.

    \note Unlike \c contentData, \c contentChildren does not include non-visual
    QML objects. It is re-ordered when items are inserted or moved.

    \sa Item::children, contentData
*/
QQmlListProperty<QQuickItem> QQuickContainer::contentChildren()
{
    return QQmlListProperty<QQuickItem>(this, nullptr,
                                        QQuickContainerPrivate::contentChildren_append,
                                        QQuickContainerPrivate::contentChildren_count,
                                        QQuickContainerPrivate::contentChildren_at,
                                        QQuickContainerPrivate::contentChildren_clear);
}

/*!
    \qmlproperty int QtQuick.Controls::Container::currentIndex

    This property holds the index of the current item.

    \sa currentItem, {Managing the Current Index}
*/
int QQuickContainer::currentIndex() const
{
    Q_D(const QQuickContainer);
    return d->currentIndex;
}

/*!
    \qmlmethod void QtQuick.Controls::Container::setCurrentIndex(int index)

    Sets the current index of the container.

    This method can be called to set a specific current index without breaking
    existing \c currentIndex bindings.

    \sa currentIndex, {Managing the Current Index}
*/
void QQuickContainer::setCurrentIndex(int index)
{
    Q_D(QQuickContainer);
    if (d->currentIndex == index)
        return;

    d->currentIndex = index;
    emit currentIndexChanged();
    emit currentItemChanged();
}

/*!
    \qmlmethod void QtQuick.Controls::Container::incrementCurrentIndex()
    \since QtQuick.Controls 2.1 (Qt 5.8)

    Increments the current index of the container.

    This method can be called to alter the current index without breaking
    existing \c currentIndex bindings.

    \sa currentIndex, {Managing the Current Index}
*/
void QQuickContainer::incrementCurrentIndex()
{
    Q_D(QQuickContainer);
    if (d->currentIndex < count() - 1)
        setCurrentIndex(d->currentIndex + 1);
}

/*!
    \qmlmethod void QtQuick.Controls::Container::decrementCurrentIndex()
    \since QtQuick.Controls 2.1 (Qt 5.8)

    Decrements the current index of the container.

    This method can be called to alter the current index without breaking
    existing \c currentIndex bindings.

    \sa currentIndex, {Managing the Current Index}
*/
void QQuickContainer::decrementCurrentIndex()
{
    Q_D(QQuickContainer);
    if (d->currentIndex > 0)
        setCurrentIndex(d->currentIndex - 1);
}

/*!
    \qmlproperty Item QtQuick.Controls::Container::currentItem
    \readonly

    This property holds the current item.

    \sa currentIndex
*/
QQuickItem *QQuickContainer::currentItem() const
{
    Q_D(const QQuickContainer);
    return itemAt(d->currentIndex);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Container::contentWidth

    This property holds the content width. It is used for calculating the total
    implicit width of the container.

    Unless explicitly overridden, the content width is automatically calculated
    based on the implicit width of the items in the container.

    \sa contentHeight
*/
qreal QQuickContainer::contentWidth() const
{
    Q_D(const QQuickContainer);
    return d->contentWidth;
}

void QQuickContainer::setContentWidth(qreal width)
{
    Q_D(QQuickContainer);
    d->hasContentWidth = true;
    if (qFuzzyCompare(d->contentWidth, width))
        return;

    d->contentWidth = width;
    d->resizeContent();
    emit contentWidthChanged();
}

void QQuickContainer::resetContentWidth()
{
    Q_D(QQuickContainer);
    if (!d->hasContentWidth)
        return;

    d->hasContentWidth = false;
    d->updateContentWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Container::contentHeight

    This property holds the content height. It is used for calculating the total
    implicit height of the container.

    Unless explicitly overridden, the content height is automatically calculated
    based on the implicit height of the items in the container.

    \sa contentWidth
*/
qreal QQuickContainer::contentHeight() const
{
    Q_D(const QQuickContainer);
    return d->contentHeight;
}

void QQuickContainer::setContentHeight(qreal height)
{
    Q_D(QQuickContainer);
    d->hasContentHeight = true;
    if (qFuzzyCompare(d->contentHeight, height))
        return;

    d->contentHeight = height;
    d->resizeContent();
    emit contentHeightChanged();
}

void QQuickContainer::resetContentHeight()
{
    Q_D(QQuickContainer);
    if (!d->hasContentHeight)
        return;

    d->hasContentHeight = false;
    d->updateContentHeight();
}

void QQuickContainer::componentComplete()
{
    Q_D(QQuickContainer);
    QQuickControl::componentComplete();
    d->reorderItems();
}

void QQuickContainer::itemChange(ItemChange change, const ItemChangeData &data)
{
    Q_D(QQuickContainer);
    QQuickControl::itemChange(change, data);
    if (change == QQuickItem::ItemChildAddedChange && isComponentComplete() && data.item != d->background && data.item != d->contentItem) {
        if (!QQuickItemPrivate::get(data.item)->isTransparentForPositioner() && d->contentModel->indexOf(data.item, nullptr) == -1)
            addItem(data.item);
    }
}

void QQuickContainer::contentItemChange(QQuickItem *newItem, QQuickItem *oldItem)
{
    Q_D(QQuickContainer);
    QQuickControl::contentItemChange(newItem, oldItem);

    static const int slotIndex = metaObject()->indexOfSlot("_q_currentIndexChanged()");

    if (oldItem) {
        QQuickItemPrivate::get(oldItem)->removeItemChangeListener(d, QQuickItemPrivate::Children | QQuickItemPrivate::ImplicitWidth | QQuickItemPrivate::ImplicitHeight);
        QQuickItem *oldContentItem = effectiveContentItem(oldItem);
        if (oldContentItem != oldItem)
            QQuickItemPrivate::get(oldContentItem)->removeItemChangeListener(d, QQuickItemPrivate::Children);

        int signalIndex = oldItem->metaObject()->indexOfSignal("currentIndexChanged()");
        if (signalIndex != -1)
            QMetaObject::disconnect(oldItem, signalIndex, this, slotIndex);
    }

    if (newItem) {
        QQuickItemPrivate::get(newItem)->addItemChangeListener(d, QQuickItemPrivate::Children | QQuickItemPrivate::ImplicitWidth | QQuickItemPrivate::ImplicitHeight);
        QQuickItem *newContentItem = effectiveContentItem(newItem);
        if (newContentItem != newItem)
            QQuickItemPrivate::get(newContentItem)->addItemChangeListener(d, QQuickItemPrivate::Children);

        int signalIndex = newItem->metaObject()->indexOfSignal("currentIndexChanged()");
        if (signalIndex != -1)
            QMetaObject::connect(newItem, signalIndex, this, slotIndex);
    }
}

bool QQuickContainer::isContent(QQuickItem *item) const
{
    // If the item has a QML context associated to it (it was created in QML),
    // we add it to the content model. Otherwise, it's probably the default
    // highlight item that is always created by the item views, which we need
    // to exclude.
    //
    // TODO: Find a better way to identify/exclude the highlight item...
    return qmlContext(item);
}

void QQuickContainer::itemAdded(int index, QQuickItem *item)
{
    Q_UNUSED(index);
    Q_UNUSED(item);
}

void QQuickContainer::itemMoved(int index, QQuickItem *item)
{
    Q_UNUSED(index);
    Q_UNUSED(item);
}

void QQuickContainer::itemRemoved(int index, QQuickItem *item)
{
    Q_UNUSED(index);
    Q_UNUSED(item);
}

QT_END_NAMESPACE

#include "moc_qquickcontainer_p.cpp"
