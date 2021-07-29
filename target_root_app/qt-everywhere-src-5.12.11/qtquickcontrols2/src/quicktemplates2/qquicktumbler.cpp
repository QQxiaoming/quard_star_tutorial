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

#include "qquicktumbler_p.h"

#include <QtGui/qpa/qplatformtheme.h>
#include <QtQml/qqmlinfo.h>
#include <QtQuick/private/qquickflickable_p.h>
#include <QtQuickTemplates2/private/qquickcontrol_p_p.h>
#include <QtQuickTemplates2/private/qquicktumbler_p_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Tumbler
    \inherits Control
    \instantiates QQuickTumbler
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-input
    \brief Spinnable wheel of items that can be selected.

    \image qtquickcontrols2-tumbler-wrap.gif

    \code
    Tumbler {
        model: 5
        // ...
    }
    \endcode

    Tumbler allows the user to select an option from a spinnable \e "wheel" of
    items. It is useful for when there are too many options to use, for
    example, a RadioButton, and too few options to require the use of an
    editable SpinBox. It is convenient in that it requires no keyboard usage
    and wraps around at each end when there are a large number of items.

    The API is similar to that of views like \l ListView and \l PathView; a
    \l model and \l delegate can be set, and the \l count and \l currentItem
    properties provide read-only access to information about the view. To
    position the view at a certain index, use \l positionViewAtIndex().

    Unlike views like \l PathView and \l ListView, however, there is always a
    current item (when the model isn't empty). This means that when \l count is
    equal to \c 0, \l currentIndex will be \c -1. In all other cases, it will
    be greater than or equal to \c 0.

    By default, Tumbler \l {wrap}{wraps} when it reaches the top and bottom, as
    long as there are more items in the model than there are visible items;
    that is, when \l count is greater than \l visibleItemCount:

    \snippet qtquickcontrols2-tumbler-timePicker.qml tumbler

    \sa {Customizing Tumbler}, {Input Controls}
*/

namespace {
    static inline qreal delegateHeight(const QQuickTumbler *tumbler)
    {
        return tumbler->availableHeight() / tumbler->visibleItemCount();
    }
}

/*
    Finds the contentItem of the view that is a child of the control's \a contentItem.
    The type is stored in \a type.
*/
QQuickItem *QQuickTumblerPrivate::determineViewType(QQuickItem *contentItem)
{
    if (!contentItem) {
        resetViewData();
        return nullptr;
    }

    if (contentItem->inherits("QQuickPathView")) {
        view = contentItem;
        viewContentItem = contentItem;
        viewContentItemType = PathViewContentItem;
        viewOffset = 0;

        return contentItem;
    } else if (contentItem->inherits("QQuickListView")) {
        view = contentItem;
        viewContentItem = qobject_cast<QQuickFlickable*>(contentItem)->contentItem();
        viewContentItemType = ListViewContentItem;
        viewContentY = 0;

        return contentItem;
    } else {
        const auto childItems = contentItem->childItems();
        for (QQuickItem *childItem : childItems) {
            QQuickItem *item = determineViewType(childItem);
            if (item)
                return item;
        }
    }

    resetViewData();
    viewContentItemType = UnsupportedContentItemType;
    return nullptr;
}

void QQuickTumblerPrivate::resetViewData()
{
    view = nullptr;
    viewContentItem = nullptr;
    if (viewContentItemType == PathViewContentItem)
        viewOffset = 0;
    else if (viewContentItemType == ListViewContentItem)
        viewContentY = 0;
    viewContentItemType = NoContentItem;
}

QList<QQuickItem *> QQuickTumblerPrivate::viewContentItemChildItems() const
{
    if (!viewContentItem)
        return QList<QQuickItem *>();

    return viewContentItem->childItems();
}

QQuickTumblerPrivate *QQuickTumblerPrivate::get(QQuickTumbler *tumbler)
{
    return tumbler->d_func();
}

void QQuickTumblerPrivate::_q_updateItemHeights()
{
    if (ignoreSignals)
        return;

    // Can't use our own private padding members here, as the padding property might be set,
    // which doesn't affect them, only their getters.
    Q_Q(const QQuickTumbler);
    const qreal itemHeight = delegateHeight(q);
    const auto items = viewContentItemChildItems();
    for (QQuickItem *childItem : items)
        childItem->setHeight(itemHeight);
}

void QQuickTumblerPrivate::_q_updateItemWidths()
{
    if (ignoreSignals)
        return;

    Q_Q(const QQuickTumbler);
    const qreal availableWidth = q->availableWidth();
    const auto items = viewContentItemChildItems();
    for (QQuickItem *childItem : items)
        childItem->setWidth(availableWidth);
}

void QQuickTumblerPrivate::_q_onViewCurrentIndexChanged()
{
    Q_Q(QQuickTumbler);
    if (!view || ignoreCurrentIndexChanges || currentIndexSetDuringModelChange) {
        // If the user set currentIndex in the onModelChanged handler,
        // we have to respect that currentIndex by ignoring changes in the view
        // until the model has finished being set.
        return;
    }

    const int oldCurrentIndex = currentIndex;
    currentIndex = view->property("currentIndex").toInt();
    if (oldCurrentIndex != currentIndex)
        emit q->currentIndexChanged();
}

void QQuickTumblerPrivate::_q_onViewCountChanged()
{
    Q_Q(QQuickTumbler);
    if (ignoreSignals)
        return;

    setCount(view->property("count").toInt());

    if (count > 0) {
        if (pendingCurrentIndex != -1) {
            // If there was an attempt to set currentIndex at creation, try to finish that attempt now.
            // componentComplete() is too early, because the count might only be known sometime after completion.
            setCurrentIndex(pendingCurrentIndex);
            // If we could successfully set the currentIndex, consider it done.
            // Otherwise, we'll try again later in updatePolish().
            if (currentIndex == pendingCurrentIndex)
                setPendingCurrentIndex(-1);
            else
                q->polish();
        } else if (currentIndex == -1) {
            // If new items were added and our currentIndex was -1, we must
            // enforce our rule of a non-negative currentIndex when count > 0.
            setCurrentIndex(0);
        }
    } else {
        setCurrentIndex(-1);
    }
}

void QQuickTumblerPrivate::_q_onViewOffsetChanged()
{
    viewOffset = view->property("offset").toReal();
    calculateDisplacements();
}

void QQuickTumblerPrivate::_q_onViewContentYChanged()
{
    viewContentY = view->property("contentY").toReal();
    calculateDisplacements();
}

void QQuickTumblerPrivate::calculateDisplacements()
{
    const auto items = viewContentItemChildItems();
    for (QQuickItem *childItem : items) {
        QQuickTumblerAttached *attached = qobject_cast<QQuickTumblerAttached *>(qmlAttachedPropertiesObject<QQuickTumbler>(childItem, false));
        if (attached)
            QQuickTumblerAttachedPrivate::get(attached)->calculateDisplacement();
    }
}

void QQuickTumblerPrivate::itemChildAdded(QQuickItem *, QQuickItem *)
{
    _q_updateItemWidths();
    _q_updateItemHeights();
}

void QQuickTumblerPrivate::itemChildRemoved(QQuickItem *, QQuickItem *)
{
    _q_updateItemWidths();
    _q_updateItemHeights();
}

void QQuickTumblerPrivate::itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &diff)
{
    QQuickControlPrivate::itemGeometryChanged(item, change, diff);
    if (change.sizeChange())
        calculateDisplacements();
}

QQuickTumbler::QQuickTumbler(QQuickItem *parent)
    : QQuickControl(*(new QQuickTumblerPrivate), parent)
{
    setActiveFocusOnTab(true);

    connect(this, SIGNAL(leftPaddingChanged()), this, SLOT(_q_updateItemWidths()));
    connect(this, SIGNAL(rightPaddingChanged()), this, SLOT(_q_updateItemWidths()));
    connect(this, SIGNAL(topPaddingChanged()), this, SLOT(_q_updateItemHeights()));
    connect(this, SIGNAL(bottomPaddingChanged()), this, SLOT(_q_updateItemHeights()));
}

QQuickTumbler::~QQuickTumbler()
{
    Q_D(QQuickTumbler);
    // Ensure that the item change listener is removed.
    d->disconnectFromView();
}

/*!
    \qmlproperty variant QtQuick.Controls::Tumbler::model

    This property holds the model that provides data for this tumbler.
*/
QVariant QQuickTumbler::model() const
{
    Q_D(const QQuickTumbler);
    return d->model;
}

void QQuickTumbler::setModel(const QVariant &model)
{
    Q_D(QQuickTumbler);
    if (model == d->model)
        return;

    d->beginSetModel();

    d->model = model;
    emit modelChanged();

    d->endSetModel();

    d->currentIndexSetDuringModelChange = false;

    // Don't try to correct the currentIndex if count() isn't known yet.
    // We can check in setupViewData() instead.
    if (isComponentComplete() && d->view && count() == 0)
        d->setCurrentIndex(-1);
}

/*!
    \qmlproperty int QtQuick.Controls::Tumbler::count
    \readonly

    This property holds the number of items in the model.
*/
int QQuickTumbler::count() const
{
    Q_D(const QQuickTumbler);
    return d->count;
}

/*!
    \qmlproperty int QtQuick.Controls::Tumbler::currentIndex

    This property holds the index of the current item.

    The value of this property is \c -1 when \l count is equal to \c 0. In all
    other cases, it will be greater than or equal to \c 0.

    \sa currentItem, positionViewAtIndex()
*/
int QQuickTumbler::currentIndex() const
{
    Q_D(const QQuickTumbler);
    return d->currentIndex;
}

void QQuickTumbler::setCurrentIndex(int currentIndex)
{
    Q_D(QQuickTumbler);
    if (d->modelBeingSet)
        d->currentIndexSetDuringModelChange = true;
    d->setCurrentIndex(currentIndex, QQuickTumblerPrivate::UserChange);
}

/*!
    \qmlproperty Item QtQuick.Controls::Tumbler::currentItem
    \readonly

    This property holds the item at the current index.

    \sa currentIndex, positionViewAtIndex()
*/
QQuickItem *QQuickTumbler::currentItem() const
{
    Q_D(const QQuickTumbler);
    return d->view ? d->view->property("currentItem").value<QQuickItem*>() : nullptr;
}

/*!
    \qmlproperty Component QtQuick.Controls::Tumbler::delegate

    This property holds the delegate used to display each item.
*/
QQmlComponent *QQuickTumbler::delegate() const
{
    Q_D(const QQuickTumbler);
    return d->delegate;
}

void QQuickTumbler::setDelegate(QQmlComponent *delegate)
{
    Q_D(QQuickTumbler);
    if (delegate == d->delegate)
        return;

    d->delegate = delegate;
    emit delegateChanged();
}

/*!
    \qmlproperty int QtQuick.Controls::Tumbler::visibleItemCount

    This property holds the number of items visible in the tumbler. It must be
    an odd number, as the current item is always vertically centered.
*/
int QQuickTumbler::visibleItemCount() const
{
    Q_D(const QQuickTumbler);
    return d->visibleItemCount;
}

void QQuickTumbler::setVisibleItemCount(int visibleItemCount)
{
    Q_D(QQuickTumbler);
    if (visibleItemCount == d->visibleItemCount)
        return;

    d->visibleItemCount = visibleItemCount;
    d->_q_updateItemHeights();
    emit visibleItemCountChanged();
}

QQuickTumblerAttached *QQuickTumbler::qmlAttachedProperties(QObject *object)
{
    return new QQuickTumblerAttached(object);
}

/*!
    \qmlproperty bool QtQuick.Controls::Tumbler::wrap
    \since QtQuick.Controls 2.1 (Qt 5.8)

    This property determines whether or not the tumbler wraps around when it
    reaches the top or bottom.

    The default value is \c false when \l count is less than
    \l visibleItemCount, as it is simpler to interact with a non-wrapping Tumbler
    when there are only a few items. To override this behavior, explicitly set
    the value of this property. To return to the default behavior, set this
    property to \c undefined.
*/
bool QQuickTumbler::wrap() const
{
    Q_D(const QQuickTumbler);
    return d->wrap;
}

void QQuickTumbler::setWrap(bool wrap)
{
    Q_D(QQuickTumbler);
    d->setWrap(wrap, true);
}

void QQuickTumbler::resetWrap()
{
    Q_D(QQuickTumbler);
    d->explicitWrap = false;
    d->setWrapBasedOnCount();
}

/*!
    \qmlproperty bool QtQuick.Controls::Tumbler::moving
    \since QtQuick.Controls 2.2 (Qt 5.9)

    This property describes whether the tumbler is currently moving, due to
    the user either dragging or flicking it.
*/
bool QQuickTumbler::isMoving() const
{
    Q_D(const QQuickTumbler);
    return d->view && d->view->property("moving").toBool();
}

/*!
    \qmlmethod void QtQuick.Controls::Tumbler::positionViewAtIndex(int index, PositionMode mode)
    \since QtQuick.Controls 2.5 (Qt 5.12)

    Positions the view so that the \a index is at the position specified by \a mode.

    For example:

    \code
    positionViewAtIndex(10, Tumbler.Center)
    \endcode

    If \l wrap is true (the default), the modes available to \l {PathView}'s
    \l {PathView::}{positionViewAtIndex()} function
    are available, otherwise the modes available to \l {ListView}'s
    \l {ListView::}{positionViewAtIndex()} function
    are available.

    \note There is a known limitation that using \c Tumbler.Beginning when \l
    wrap is \c true will result in the wrong item being positioned at the top
    of view. As a workaround, pass \c {index - 1}.

    \sa currentIndex
*/
void QQuickTumbler::positionViewAtIndex(int index, QQuickTumbler::PositionMode mode)
{
    Q_D(QQuickTumbler);
    if (!d->view) {
        d->warnAboutIncorrectContentItem();
        return;
    }

    QMetaObject::invokeMethod(d->view, "positionViewAtIndex", Q_ARG(int, index), Q_ARG(int, mode));
}

void QQuickTumbler::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickTumbler);

    QQuickControl::geometryChanged(newGeometry, oldGeometry);

    d->_q_updateItemHeights();

    if (newGeometry.width() != oldGeometry.width())
        d->_q_updateItemWidths();
}

void QQuickTumbler::componentComplete()
{
    Q_D(QQuickTumbler);
    QQuickControl::componentComplete();

    if (!d->view) {
        // Force the view to be created.
        emit wrapChanged();
        // Determine the type of view for attached properties, etc.
        d->setupViewData(d->contentItem);
    }

    // If there was no contentItem or it was of an unsupported type,
    // we don't have anything else to do.
    if (!d->view)
        return;

    // Update item heights after we've populated the model,
    // otherwise ignoreSignals will cause these functions to return early.
    d->_q_updateItemHeights();
    d->_q_updateItemWidths();
    d->_q_onViewCountChanged();
}

void QQuickTumbler::contentItemChange(QQuickItem *newItem, QQuickItem *oldItem)
{
    Q_D(QQuickTumbler);

    QQuickControl::contentItemChange(newItem, oldItem);

    if (oldItem)
        d->disconnectFromView();

    if (newItem) {
        // We wait until wrap is set to that we know which type of view to create.
        // If we try to set up the view too early, we'll issue warnings about it not existing.
        if (isComponentComplete()) {
            // Make sure we use the new content item and not the current one, as that won't
            // be changed until after contentItemChange() has finished.
            d->setupViewData(newItem);

            d->_q_updateItemHeights();
            d->_q_updateItemWidths();
        }
    }
}

void QQuickTumblerPrivate::disconnectFromView()
{
    Q_Q(QQuickTumbler);
    if (!view) {
        // If a custom content item is declared, it can happen that
        // the original contentItem exists without the view etc. having been
        // determined yet, and then this is called when the custom content item
        // is eventually set.
        return;
    }

    QObject::disconnect(view, SIGNAL(currentIndexChanged()), q, SLOT(_q_onViewCurrentIndexChanged()));
    QObject::disconnect(view, SIGNAL(currentItemChanged()), q, SIGNAL(currentItemChanged()));
    QObject::disconnect(view, SIGNAL(countChanged()), q, SLOT(_q_onViewCountChanged()));
    QObject::disconnect(view, SIGNAL(movingChanged()), q, SIGNAL(movingChanged()));

    if (viewContentItemType == PathViewContentItem)
        QObject::disconnect(view, SIGNAL(offsetChanged()), q, SLOT(_q_onViewOffsetChanged()));
    else
        QObject::disconnect(view, SIGNAL(contentYChanged()), q, SLOT(_q_onViewContentYChanged()));

    QQuickItemPrivate *oldViewContentItemPrivate = QQuickItemPrivate::get(viewContentItem);
    oldViewContentItemPrivate->removeItemChangeListener(this, QQuickItemPrivate::Children | QQuickItemPrivate::Geometry);

    resetViewData();
}

void QQuickTumblerPrivate::setupViewData(QQuickItem *newControlContentItem)
{
    // Don't do anything if we've already set up.
    if (view)
        return;

    determineViewType(newControlContentItem);

    if (viewContentItemType == QQuickTumblerPrivate::NoContentItem)
        return;

    if (viewContentItemType == QQuickTumblerPrivate::UnsupportedContentItemType) {
        warnAboutIncorrectContentItem();
        return;
    }

    Q_Q(QQuickTumbler);
    QObject::connect(view, SIGNAL(currentIndexChanged()), q, SLOT(_q_onViewCurrentIndexChanged()));
    QObject::connect(view, SIGNAL(currentItemChanged()), q, SIGNAL(currentItemChanged()));
    QObject::connect(view, SIGNAL(countChanged()), q, SLOT(_q_onViewCountChanged()));
    QObject::connect(view, SIGNAL(movingChanged()), q, SIGNAL(movingChanged()));

    if (viewContentItemType == PathViewContentItem) {
        QObject::connect(view, SIGNAL(offsetChanged()), q, SLOT(_q_onViewOffsetChanged()));
        _q_onViewOffsetChanged();
    } else {
        QObject::connect(view, SIGNAL(contentYChanged()), q, SLOT(_q_onViewContentYChanged()));
        _q_onViewContentYChanged();
    }

    QQuickItemPrivate *viewContentItemPrivate = QQuickItemPrivate::get(viewContentItem);
    viewContentItemPrivate->addItemChangeListener(this, QQuickItemPrivate::Children | QQuickItemPrivate::Geometry);

    // Sync the view's currentIndex with ours.
    syncCurrentIndex();

    calculateDisplacements();
}

void QQuickTumblerPrivate::warnAboutIncorrectContentItem()
{
    Q_Q(QQuickTumbler);
    qmlWarning(q) << "Tumbler: contentItem must contain either a PathView or a ListView";
}

void QQuickTumblerPrivate::syncCurrentIndex()
{
    const int actualViewIndex = view->property("currentIndex").toInt();
    Q_Q(QQuickTumbler);

    const bool isPendingCurrentIndex = pendingCurrentIndex != -1;
    const int indexToSet = isPendingCurrentIndex ? pendingCurrentIndex : currentIndex;

    // Nothing to do.
    if (actualViewIndex == indexToSet) {
        setPendingCurrentIndex(-1);
        return;
    }

    // PathView likes to use 0 as currentIndex for empty models, but we use -1 for that.
    if (q->count() == 0 && actualViewIndex == 0)
        return;

    ignoreCurrentIndexChanges = true;
    view->setProperty("currentIndex", QVariant(indexToSet));
    ignoreCurrentIndexChanges = false;

    if (view->property("currentIndex").toInt() == indexToSet)
        setPendingCurrentIndex(-1);
    else if (isPendingCurrentIndex)
        q->polish();
}

void QQuickTumblerPrivate::setPendingCurrentIndex(int index)
{
    pendingCurrentIndex = index;
}

void QQuickTumblerPrivate::setCurrentIndex(int newCurrentIndex,
    QQuickTumblerPrivate::PropertyChangeReason changeReason)
{
    Q_Q(QQuickTumbler);
    if (newCurrentIndex == currentIndex || newCurrentIndex < -1)
        return;

    if (!q->isComponentComplete()) {
        // Views can't set currentIndex until they're ready.
        setPendingCurrentIndex(newCurrentIndex);
        return;
    }

    if (modelBeingSet && changeReason == UserChange) {
        // If modelBeingSet is true and the user set the currentIndex,
        // the model is in the process of being set and the user has set
        // the currentIndex in onModelChanged. We have to queue the currentIndex
        // change until we're ready.
        setPendingCurrentIndex(newCurrentIndex);
        return;
    }

    // -1 doesn't make sense for a non-empty Tumbler, because unlike
    // e.g. ListView, there's always one item selected.
    // Wait until the component has finished before enforcing this rule, though,
    // because the count might not be known yet.
    if ((count > 0 && newCurrentIndex == -1) || (newCurrentIndex >= count)) {
        return;
    }

    // The view might not have been created yet, as is the case
    // if you create a Tumbler component and pass e.g. { currentIndex: 2 }
    // to createObject().
    if (view) {
        // Only actually set our currentIndex if the view was able to set theirs.
        bool couldSet = false;
        if (count == 0 && newCurrentIndex == -1) {
            // PathView insists on using 0 as the currentIndex when there are no items.
            couldSet = true;
        } else {
            ignoreCurrentIndexChanges = true;
            ignoreSignals = true;
            view->setProperty("currentIndex", newCurrentIndex);
            ignoreSignals = false;
            ignoreCurrentIndexChanges = false;

            couldSet = view->property("currentIndex").toInt() == newCurrentIndex;
        }

        if (couldSet) {
            // The view's currentIndex might not have actually changed, but ours has,
            // and that's what user code sees.
            currentIndex = newCurrentIndex;
            emit q->currentIndexChanged();
        }
    }
}

void QQuickTumblerPrivate::setCount(int newCount)
{
    if (newCount == count)
        return;

    count = newCount;

    Q_Q(QQuickTumbler);
    setWrapBasedOnCount();

    emit q->countChanged();
}

void QQuickTumblerPrivate::setWrapBasedOnCount()
{
    if (count == 0 || explicitWrap || modelBeingSet)
        return;

    setWrap(count >= visibleItemCount, false);
}

void QQuickTumblerPrivate::setWrap(bool shouldWrap, bool isExplicit)
{
    if (isExplicit)
        explicitWrap = true;

    Q_Q(QQuickTumbler);
    if (q->isComponentComplete() && shouldWrap == wrap)
        return;

    // Since we use the currentIndex of the contentItem directly, we must
    // ensure that we keep track of the currentIndex so it doesn't get lost
    // between view changes.
    const int oldCurrentIndex = currentIndex;

    disconnectFromView();

    wrap = shouldWrap;

    // New views will set their currentIndex upon creation, which we'd otherwise
    // take as the correct one, so we must ignore them.
    ignoreCurrentIndexChanges = true;

    // This will cause the view to be created if our contentItem is a TumblerView.
    emit q->wrapChanged();

    ignoreCurrentIndexChanges = false;

    // If isComponentComplete() is true, we require a contentItem. If it's not
    // true, it might not have been created yet, so we wait until
    // componentComplete() is called.
    //
    // When the contentItem (usually QQuickTumblerView) has been created, we
    // can start determining its type, etc. If the delegates use attached
    // properties, this will have already been called, in which case it will
    // return early. If the delegate doesn't use attached properties, we need
    // to call it here.
    if (q->isComponentComplete() || contentItem)
        setupViewData(contentItem);

    setCurrentIndex(oldCurrentIndex);
}

void QQuickTumblerPrivate::beginSetModel()
{
    modelBeingSet = true;
}

void QQuickTumblerPrivate::endSetModel()
{
    modelBeingSet = false;
    setWrapBasedOnCount();
}

void QQuickTumbler::keyPressEvent(QKeyEvent *event)
{
    QQuickControl::keyPressEvent(event);

    Q_D(QQuickTumbler);
    if (event->isAutoRepeat() || !d->view)
        return;

    if (event->key() == Qt::Key_Up) {
        QMetaObject::invokeMethod(d->view, "decrementCurrentIndex");
    } else if (event->key() == Qt::Key_Down) {
        QMetaObject::invokeMethod(d->view, "incrementCurrentIndex");
    }
}

void QQuickTumbler::updatePolish()
{
    Q_D(QQuickTumbler);
    if (d->pendingCurrentIndex != -1) {
        // Update our count, as ignoreSignals might have been true
        // when _q_onViewCountChanged() was last called.
        d->setCount(d->view->property("count").toInt());

        // If the count is still 0, it's not going to happen.
        if (d->count == 0) {
            d->setPendingCurrentIndex(-1);
            return;
        }

        // If there is a pending currentIndex at this stage, it means that
        // the view wouldn't set our currentIndex in _q_onViewCountChanged
        // because it wasn't ready. Try one last time here.
        d->setCurrentIndex(d->pendingCurrentIndex);

        if (d->currentIndex != d->pendingCurrentIndex && d->currentIndex == -1) {
            // If we *still* couldn't set it, it's probably invalid.
            // See if we can at least enforce our rule of "non-negative currentIndex when count > 0" instead.
            d->setCurrentIndex(0);
        }

        d->setPendingCurrentIndex(-1);
    }
}

QFont QQuickTumbler::defaultFont() const
{
    return QQuickTheme::font(QQuickTheme::Tumbler);
}

QPalette QQuickTumbler::defaultPalette() const
{
    return QQuickTheme::palette(QQuickTheme::Tumbler);
}

void QQuickTumblerAttachedPrivate::init(QQuickItem *delegateItem)
{
    if (!delegateItem->parentItem()) {
        qWarning() << "Tumbler: attached properties must be accessed through a delegate item that has a parent";
        return;
    }

    QVariant indexContextProperty = qmlContext(delegateItem)->contextProperty(QStringLiteral("index"));
    if (!indexContextProperty.isValid()) {
        qWarning() << "Tumbler: attempting to access attached property on item without an \"index\" property";
        return;
    }

    index = indexContextProperty.toInt();

    QQuickItem *parentItem = delegateItem;
    while ((parentItem = parentItem->parentItem())) {
        if ((tumbler = qobject_cast<QQuickTumbler*>(parentItem)))
            break;
    }
}

void QQuickTumblerAttachedPrivate::calculateDisplacement()
{
    const qreal previousDisplacement = displacement;
    displacement = 0;

    if (!tumbler) {
        // Can happen if the attached properties are accessed on the wrong type of item or the tumbler was destroyed.
        // We don't want to emit the change signal though, as this could cause warnings about Tumbler.tumbler being null.
        return;
    }

    // Can happen if there is no ListView or PathView within the contentItem.
    QQuickTumblerPrivate *tumblerPrivate = QQuickTumblerPrivate::get(tumbler);
    if (!tumblerPrivate->viewContentItem) {
        emitIfDisplacementChanged(previousDisplacement, displacement);
        return;
    }

    // The attached property gets created before our count is updated, so just cheat here
    // to avoid having to listen to count changes.
    const int count = tumblerPrivate->view->property("count").toInt();
    // This can happen in tests, so it may happen in normal usage too.
    if (count == 0) {
        emitIfDisplacementChanged(previousDisplacement, displacement);
        return;
    }

    if (tumblerPrivate->viewContentItemType == QQuickTumblerPrivate::PathViewContentItem) {
        const qreal offset = tumblerPrivate->viewOffset;

        displacement = count > 1 ? count - index - offset : 0;
        // Don't add 1 if count <= visibleItemCount
        const int visibleItems = tumbler->visibleItemCount();
        const int halfVisibleItems = visibleItems / 2 + (visibleItems < count ? 1 : 0);
        if (displacement > halfVisibleItems)
            displacement -= count;
        else if (displacement < -halfVisibleItems)
            displacement += count;
    } else {
        const qreal contentY = tumblerPrivate->viewContentY;
        const qreal delegateH = delegateHeight(tumbler);
        const qreal preferredHighlightBegin = tumblerPrivate->view->property("preferredHighlightBegin").toReal();
        const qreal itemY = qobject_cast<QQuickItem*>(parent)->y();
        qreal currentItemY = 0;
        auto currentItem = tumblerPrivate->view->property("currentItem").value<QQuickItem*>();
        if (currentItem)
            currentItemY = currentItem->y();
        // Start from the y position of the current item.
        const qreal topOfCurrentItemInViewport = currentItemY - contentY;
        // Then, calculate the distance between it and the preferredHighlightBegin.
        const qreal relativePositionToPreferredHighlightBegin = topOfCurrentItemInViewport - preferredHighlightBegin;
        // Next, calculate the distance between us and the current item.
        const qreal distanceFromCurrentItem = currentItemY - itemY;
        const qreal displacementInPixels = distanceFromCurrentItem - relativePositionToPreferredHighlightBegin;
        // Convert it from pixels to a floating point index.
        displacement = displacementInPixels / delegateH;
    }

    emitIfDisplacementChanged(previousDisplacement, displacement);
}

void QQuickTumblerAttachedPrivate::emitIfDisplacementChanged(qreal oldDisplacement, qreal newDisplacement)
{
    Q_Q(QQuickTumblerAttached);
    if (newDisplacement != oldDisplacement)
        emit q->displacementChanged();
}

QQuickTumblerAttached::QQuickTumblerAttached(QObject *parent)
    : QObject(*(new QQuickTumblerAttachedPrivate), parent)
{
    Q_D(QQuickTumblerAttached);
    QQuickItem *delegateItem = qobject_cast<QQuickItem *>(parent);
    if (delegateItem)
        d->init(delegateItem);
    else if (parent)
        qmlWarning(parent) << "Tumbler: attached properties of Tumbler must be accessed through a delegate item";

    if (d->tumbler) {
        // When the Tumbler is completed, wrapChanged() is emitted to let QQuickTumblerView
        // know that it can create the view. The view itself might instantiate delegates
        // that use attached properties. At this point, setupViewData() hasn't been called yet
        // (it's called on the next line in componentComplete()), so we call it here so that
        // we have access to the view.
        QQuickTumblerPrivate *tumblerPrivate = QQuickTumblerPrivate::get(d->tumbler);
        tumblerPrivate->setupViewData(tumblerPrivate->contentItem);

        if (delegateItem->parentItem() == tumblerPrivate->viewContentItem) {
            // This item belongs to the "new" view, meaning that the tumbler's contentItem
            // was probably assigned declaratively. If they're not equal, calling
            // calculateDisplacement() would use the old contentItem data, which is bad.
            d->calculateDisplacement();
        }
    }
}

/*!
    \qmlattachedproperty Tumbler QtQuick.Controls::Tumbler::tumbler
    \readonly

    This attached property holds the tumbler. The property can be attached to
    a tumbler delegate. The value is \c null if the item is not a tumbler delegate.
*/
QQuickTumbler *QQuickTumblerAttached::tumbler() const
{
    Q_D(const QQuickTumblerAttached);
    return d->tumbler;
}

/*!
    \qmlattachedproperty real QtQuick.Controls::Tumbler::displacement
    \readonly

    This attached property holds a value from \c {-visibleItemCount / 2} to
    \c {visibleItemCount / 2}, which represents how far away this item is from
    being the current item, with \c 0 being completely current.

    For example, the item below will be 40% opaque when it is not the current item,
    and transition to 100% opacity when it becomes the current item:

    \code
    delegate: Text {
        text: modelData
        opacity: 0.4 + Math.max(0, 1 - Math.abs(Tumbler.displacement)) * 0.6
    }
    \endcode
*/
qreal QQuickTumblerAttached::displacement() const
{
    Q_D(const QQuickTumblerAttached);
    return d->displacement;
}

QT_END_NAMESPACE

#include "moc_qquicktumbler_p.cpp"
