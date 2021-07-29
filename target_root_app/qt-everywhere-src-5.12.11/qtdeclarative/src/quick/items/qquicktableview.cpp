/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquicktableview_p.h"
#include "qquicktableview_p_p.h"

#include <QtCore/qtimer.h>
#include <QtCore/qdir.h>
#include <QtQml/private/qqmldelegatemodel_p.h>
#include <QtQml/private/qqmldelegatemodel_p_p.h>
#include <QtQml/private/qqmlincubator_p.h>
#include <QtQml/private/qqmlchangeset_p.h>
#include <QtQml/qqmlinfo.h>

#include <QtQuick/private/qquickflickable_p_p.h>
#include <QtQuick/private/qquickitemviewfxitem_p_p.h>

/*!
    \qmltype TableView
    \instantiates QQuickTableView
    \inqmlmodule QtQuick
    \ingroup qtquick-views
    \inherits Flickable
    \brief Provides a table view of items provided by the model.

    A TableView has a \l model that defines the data to be displayed, and a
    \l delegate that defines how the data should be displayed.

    TableView inherits \l Flickable. This means that while the model can have
    any number of rows and columns, only a subsection of the table is usually
    visible inside the viewport. As soon as you flick, new rows and columns
    enter the viewport, while old ones exit and are removed from the viewport.
    The rows and columns that move out are reused for building the rows and columns
    that move into the viewport. As such, the TableView support models of any
    size without affecting performance.

    A TableView displays data from models created from built-in QML types
    such as ListModel and XmlListModel, which populates the first column only
    in a TableView. To create models with multiple columns, create a model in
    C++ that inherits QAbstractItemModel, and expose it to QML.

    \section1 Example Usage

    The following example shows how to create a model from C++ with multiple
    columns:

    \snippet qml/tableview/tablemodel.cpp 0

    And then how to use it from QML:

    \snippet qml/tableview/tablemodel.qml 0

    \section1 Reusing items

    TableView recycles delegate items by default, instead of instantiating from
    the \l delegate whenever new rows and columns are flicked into view. This
    can give a huge performance boost, depending on the complexity of the
    delegate.

    When an item is flicked out, it moves to the \e{reuse pool}, which is an
    internal cache of unused items. When this happens, the \l TableView::pooled
    signal is emitted to inform the item about it. Likewise, when the item is
    moved back from the pool, the \l TableView::reused signal is emitted.

    Any item properties that come from the model are updated when the
    item is reused. This includes \c index, \c row, and \c column, but also
    any model roles.

    \note Avoid storing any state inside a delegate. If you do, reset it
    manually on receiving the \l TableView::reused signal.

    If an item has timers or animations, consider pausing them on receiving
    the \l TableView::pooled signal. That way you avoid using the CPU resources
    for items that are not visible. Likewise, if an item has resources that
    cannot be reused, they could be freed up.

    If you don't want to reuse items or if the \l delegate cannot support it,
    you can set the \l reuseItems property to \c false.

    \note While an item is in the pool, it might still be alive and respond
    to connected signals and bindings.

    The following example shows a delegate that animates a spinning rectangle. When
    it is pooled, the animation is temporarily paused:

    \snippet qml/tableview/reusabledelegate.qml 0

    \section1 Row heights and column widths

    When a new column is flicked into view, TableView will determine its width
    by calling the \l columnWidthProvider function. TableView itself will never
    store row height or column width, as it's designed to support large models
    containing any number of rows and columns. Instead, it will ask the
    application whenever it needs to know.

    TableView uses the largest \c implicitWidth among the items as the column
    width, unless the \l columnWidthProvider property is explicitly set. Once
    the column width is found, all other items in the same column are resized
    to this width, even if new items that are flicked in later have larger
    \c implicitWidth. Setting an explicit \c width on an item is ignored and
    overwritten.

    \note The calculated width of a column is discarded when it is flicked out
    of the viewport, and is recalculated if the column is flicked back in. The
    calculation is always based on the items that are visible when the column
    is flicked in. This means that it can end up different each time, depending
    on which row you're at when the column enters. You should therefore have the
    same \c implicitWidth for all items in a column, or set
    \l columnWidthProvider. The same logic applies for the row height
    calculation.

    If you change the values that a \l rowHeightProvider or a
    \l columnWidthProvider return for rows and columns inside the viewport, you
    must call \l forceLayout. This informs TableView that it needs to use the
    provider functions again to recalculate and update the layout.

    \note The size of a row or column should be a whole number to avoid
    sub-pixel alignment of items.

    The following example shows how to set a simple \c columnWidthProvider
    together with a timer that modifies the values the function returns. When
    the array is modified, \l forceLayout is called to let the changes
    take effect:

    \snippet qml/tableview/tableviewwithprovider.qml 0

    \section1 Overlays and underlays

    Tableview inherits \l Flickable. And when new items are instantiated from the
    delegate, it will parent them to the \l{Flickable::}{contentItem}
    with a \c z value equal to \c 1. You can add your own items inside the
    Tableview, as child items of the Flickable. By controlling their \c z
    value, you can make them be on top of or underneath the table items.

    Here is an example that shows how to add some text on top of the table, that
    moves together with the table as you flick:

    \snippet qml/tableview/tableviewwithheader.qml 0
*/

/*!
    \qmlproperty int QtQuick::TableView::rows

    This property holds the number of rows in the table. This is
    equal to the number of rows in the model.

    This property is read only.
*/

/*!
    \qmlproperty int QtQuick::TableView::columns

    This property holds the number of columns in the table. This is
    equal to the number of columns in the model. If the model is
    a list, columns will be 1.

    This property is read only.
*/

/*!
    \qmlproperty real QtQuick::TableView::rowSpacing

    This property holds the spacing between the rows.

    The default value is 0.
*/

/*!
    \qmlproperty real QtQuick::TableView::columnSpacing

    This property holds the spacing between the columns.

    The default value is 0.
*/

/*!
    \qmlproperty var QtQuick::TableView::rowHeightProvider

    This property can hold a function that returns the row height for each row
    in the model. When assigned, it will be called whenever TableView needs to
    know the height of a specific row. The function takes one argument, \c row,
    for which the TableView needs to know the height.

    \note The height of a row must always be greater than \c 0.

    \sa columnWidthProvider, {Row heights and column widths}
*/

/*!
    \qmlproperty var QtQuick::TableView::columnWidthProvider

    This property can hold a function that returns the column width for each
    column in the model. When assigned, it is called whenever TableView needs
    to know the width of a specific column. The function takes one argument,
    \c column, for which the TableView needs to know the width.

    \note The width of a column must always be greater than \c 0.

    \sa rowHeightProvider, {Row heights and column widths}
*/

/*!
    \qmlproperty model QtQuick::TableView::model
    This property holds the model that provides data for the table.

    The model provides the set of data that is used to create the items
    in the view. Models can be created directly in QML using \l ListModel,
    \l XmlListModel or \l ObjectModel, or provided by a custom C++ model
    class. If it is a C++ model, it must be a subclass of \l QAbstractItemModel
    or a simple list.

    \sa {qml-data-models}{Data Models}
*/

/*!
    \qmlproperty Component QtQuick::TableView::delegate

    The delegate provides a template defining each cell item instantiated by the
    view. The model index is exposed as an accessible \c index property. The same
    applies to \c row and \c column. Properties of the model are also available
    depending upon the type of \l {qml-data-models}{Data Model}.

    A delegate should specify its size using \l [QML]{Item::implicitWidth}{implicitWidth} and
    \l [QML]{Item::implicitHeight}{implicitHeight}.
    The TableView lays out the items based on that information. Explicit width or
    height settings are ignored and overwritten.

    \note Delegates are instantiated as needed and may be destroyed at any time.
    They are also reused if the \l reuseItems property is set to \c true. You
    should therefore avoid storing state information in the delegates.

    \sa {Row heights and column widths}, {Reusing items}
*/

/*!
    \qmlproperty bool QtQuick::TableView::reuseItems

    This property holds whether or not items instantiated from the \l delegate
    should be reused. If set to \c false, any currently pooled items
    are destroyed.

    \sa {Reusing items}, TableView::pooled, TableView::reused
*/

/*!
    \qmlproperty real QtQuick::TableView::contentWidth

    This property holds the width of the \l view, which is also
    the width of the table (including margins). As a TableView cannot
    always know the exact width of the table without loading all columns
    in the model, the \c contentWidth is usually an estimated width based on
    the columns it has seen so far. This estimate is recalculated whenever
    new columns are flicked into view, which means that the content width
    can change dynamically.

    If you know up front what the width of the table will be, assign a value
    to \c contentWidth explicitly, to avoid unnecessary calculations and
    updates to the TableView.

    \sa contentHeight
*/

/*!
    \qmlproperty real QtQuick::TableView::contentHeight

    This property holds the height of the \l view, which is also
    the height of the table (including margins). As a TableView cannot
    always know the exact height of the table without loading all rows
    in the model, the \c contentHeight is usually an estimated height
    based on the rows it has seen so far. This estimate is recalculated
    whenever new rows are flicked into view, which means that the content height
    can change dynamically.

    If you know up front what the height of the table will be, assign a
    value to \c contentHeight explicitly, to avoid unnecessary calculations and
    updates to the TableView.

    \sa contentWidth
*/

/*!
    \qmlmethod QtQuick::TableView::forceLayout

    Responding to changes in the model are batched so that they are handled
    only once per frame. This means the TableView delays showing any changes
    while a script is being run. The same is also true when changing
    properties such as \l rowSpacing or \l {Item::anchors.leftMargin}{leftMargin}.

    This method forces the TableView to immediately update the layout so
    that any recent changes take effect.

    Calling this function re-evaluates the size and position of each visible
    row and column. This is needed if the functions assigned to
    \l rowHeightProvider or \l columnWidthProvider return different values than
    what is already assigned.
*/

/*!
    \qmlattachedproperty TableView QtQuick::TableView::view

    This attached property holds the view that manages the delegate instance.
    It is attached to each instance of the delegate.
*/

/*!
    \qmlattachedsignal QtQuick::TableView::pooled

    This signal is emitted after an item has been added to the reuse
    pool. You can use it to pause ongoing timers or animations inside
    the item, or free up resources that cannot be reused.

    This signal is emitted only if the \l reuseItems property is \c true.

    \sa {Reusing items}, reuseItems, reused
*/

/*!
    \qmlattachedsignal QtQuick::TableView::reused

    This signal is emitted after an item has been reused. At this point, the
    item has been taken out of the pool and placed inside the content view,
    and the model properties such as index, row, and column have been updated.

    Other properties that are not provided by the model does not change when an item
    is reused. You should avoid storing any state inside a delegate, but if you do,
    manually reset that state on receiving this signal.

    This signal is emitted when the item is reused, and not the first time the
    item is created.

    This signal is emitted only if the \l reuseItems property is \c true.

    \sa {Reusing items}, reuseItems, pooled
*/

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcTableViewDelegateLifecycle, "qt.quick.tableview.lifecycle")

#define Q_TABLEVIEW_UNREACHABLE(output) { dumpTable(); qWarning() << "output:" << output; Q_UNREACHABLE(); }
#define Q_TABLEVIEW_ASSERT(cond, output) Q_ASSERT((cond) || [&](){ dumpTable(); qWarning() << "output:" << output; return false;}())

static const Qt::Edge allTableEdges[] = { Qt::LeftEdge, Qt::RightEdge, Qt::TopEdge, Qt::BottomEdge };

static QLine rectangleEdge(const QRect &rect, Qt::Edge tableEdge)
{
    switch (tableEdge) {
    case Qt::LeftEdge:
        return QLine(rect.topLeft(), rect.bottomLeft());
    case Qt::RightEdge:
        return QLine(rect.topRight(), rect.bottomRight());
    case Qt::TopEdge:
        return QLine(rect.topLeft(), rect.topRight());
    case Qt::BottomEdge:
        return QLine(rect.bottomLeft(), rect.bottomRight());
    }
    return QLine();
}

static QRect expandedRect(const QRect &rect, Qt::Edge edge, int increment)
{
    switch (edge) {
    case Qt::LeftEdge:
        return rect.adjusted(-increment, 0, 0, 0);
    case Qt::RightEdge:
        return rect.adjusted(0, 0, increment, 0);
    case Qt::TopEdge:
        return rect.adjusted(0, -increment, 0, 0);
    case Qt::BottomEdge:
        return rect.adjusted(0, 0, 0, increment);
    }
    return QRect();
}

const QPoint QQuickTableViewPrivate::kLeft = QPoint(-1, 0);
const QPoint QQuickTableViewPrivate::kRight = QPoint(1, 0);
const QPoint QQuickTableViewPrivate::kUp = QPoint(0, -1);
const QPoint QQuickTableViewPrivate::kDown = QPoint(0, 1);

QQuickTableViewPrivate::QQuickTableViewPrivate()
    : QQuickFlickablePrivate()
{
}

QQuickTableViewPrivate::~QQuickTableViewPrivate()
{
    releaseLoadedItems(QQmlTableInstanceModel::NotReusable);
    if (tableModel)
        delete tableModel;
}

QString QQuickTableViewPrivate::tableLayoutToString() const
{
    return QString(QLatin1String("table cells: (%1,%2) -> (%3,%4), item count: %5, table rect: %6,%7 x %8,%9"))
            .arg(loadedTable.topLeft().x()).arg(loadedTable.topLeft().y())
            .arg(loadedTable.bottomRight().x()).arg(loadedTable.bottomRight().y())
            .arg(loadedItems.count())
            .arg(loadedTableOuterRect.x())
            .arg(loadedTableOuterRect.y())
            .arg(loadedTableOuterRect.width())
            .arg(loadedTableOuterRect.height());
}

void QQuickTableViewPrivate::dumpTable() const
{
    auto listCopy = loadedItems.values();
    std::stable_sort(listCopy.begin(), listCopy.end(),
        [](const FxTableItem *lhs, const FxTableItem *rhs)
        { return lhs->index < rhs->index; });

    qWarning() << QStringLiteral("******* TABLE DUMP *******");
    for (int i = 0; i < listCopy.count(); ++i)
        qWarning() << static_cast<FxTableItem *>(listCopy.at(i))->cell;
    qWarning() << tableLayoutToString();

    const QString filename = QStringLiteral("QQuickTableView_dumptable_capture.png");
    const QString path = QDir::current().absoluteFilePath(filename);
    if (q_func()->window() && q_func()->window()->grabWindow().save(path))
        qWarning() << "Window capture saved to:" << path;
}

QQuickTableViewAttached *QQuickTableViewPrivate::getAttachedObject(const QObject *object) const
{
    QObject *attachedObject = qmlAttachedPropertiesObject<QQuickTableView>(object);
    return static_cast<QQuickTableViewAttached *>(attachedObject);
}

int QQuickTableViewPrivate::modelIndexAtCell(const QPoint &cell) const
{
    int availableRows = tableSize.height();
    int modelIndex = cell.y() + (cell.x() * availableRows);
    Q_TABLEVIEW_ASSERT(modelIndex < model->count(),
        "modelIndex:" << modelIndex << "cell:" << cell << "count:" << model->count());
    return modelIndex;
}

QPoint QQuickTableViewPrivate::cellAtModelIndex(int modelIndex) const
{
    int availableRows = tableSize.height();
    Q_TABLEVIEW_ASSERT(availableRows > 0, availableRows);
    int column = int(modelIndex / availableRows);
    int row = modelIndex % availableRows;
    return QPoint(column, row);
}

void QQuickTableViewPrivate::updateContentWidth()
{
    Q_Q(QQuickTableView);

    if (explicitContentWidth.isValid()) {
        // Don't calculate contentWidth when it
        // was set explicitly by the application.
        return;
    }

    const qreal thresholdBeforeAdjust = 0.1;
    int currentRightColumn = loadedTable.right();

    if (currentRightColumn > contentSizeBenchMarkPoint.x()) {
        contentSizeBenchMarkPoint.setX(currentRightColumn);

        const qreal spacing = currentRightColumn * cellSpacing.width();
        qreal currentWidth = loadedTableOuterRect.right();
        const qreal averageCellWidth = (currentWidth - spacing) / (currentRightColumn + 1);
        qreal estimatedWidth = (tableSize.width() * (averageCellWidth + cellSpacing.width())) - cellSpacing.width();

        if (currentRightColumn >= tableSize.width() - 1) {
            // We are at the last column, and can set the exact width
            if (!qFuzzyCompare(currentWidth, q->implicitWidth()))
                q->QQuickFlickable::setContentWidth(currentWidth);
        } else if (currentWidth >= q->implicitWidth()) {
            // We are at the estimated width, but there are still more columns
            q->QQuickFlickable::setContentWidth(estimatedWidth);
        } else {
            // Only set a new width if the new estimate is substantially different
            qreal diff = 1 - (estimatedWidth / q->implicitWidth());
            if (qAbs(diff) > thresholdBeforeAdjust)
                q->QQuickFlickable::setContentWidth(estimatedWidth);
        }
    }
}

void QQuickTableViewPrivate::updateContentHeight()
{
    Q_Q(QQuickTableView);

    if (explicitContentHeight.isValid()) {
        // Don't calculate contentHeight when it
        // was set explicitly by the application.
        return;
    }

    const qreal thresholdBeforeAdjust = 0.1;
    int currentBottomRow = loadedTable.bottom();

    if (currentBottomRow > contentSizeBenchMarkPoint.y()) {
        contentSizeBenchMarkPoint.setY(currentBottomRow);

        const qreal spacing = currentBottomRow * cellSpacing.height();
        qreal currentHeight = loadedTableOuterRect.bottom();
        const qreal averageCellHeight = (currentHeight - spacing) / (currentBottomRow + 1);
        qreal estimatedHeight = (tableSize.height() * (averageCellHeight + cellSpacing.height())) - cellSpacing.height();

        if (currentBottomRow >= tableSize.height() - 1) {
            // We are at the last row, and can set the exact height
            if (!qFuzzyCompare(currentHeight, q->implicitHeight()))
                q->QQuickFlickable::setContentHeight(currentHeight);
        } else if (currentHeight >= q->implicitHeight()) {
            // We are at the estimated height, but there are still more rows
            q->QQuickFlickable::setContentHeight(estimatedHeight);
        } else {
            // Only set a new height if the new estimate is substantially different
            qreal diff = 1 - (estimatedHeight / q->implicitHeight());
            if (qAbs(diff) > thresholdBeforeAdjust)
                q->QQuickFlickable::setContentHeight(estimatedHeight);
        }
    }
}

void QQuickTableViewPrivate::enforceTableAtOrigin()
{
    // Gaps before the first row/column can happen if rows/columns
    // changes size while flicking e.g because of spacing changes or
    // changes to a column maxWidth/row maxHeight. Check for this, and
    // move the whole table rect accordingly.
    bool layoutNeeded = false;
    const qreal flickMargin = 50;

    if (loadedTable.x() == 0 && loadedTableOuterRect.x() > 0) {
        // The table is at the beginning, but not at the edge of the
        // content view. So move the table to origin.
        loadedTableOuterRect.moveLeft(0);
        layoutNeeded = true;
    } else if (loadedTableOuterRect.x() < 0) {
        // The table is outside the beginning of the content view. Move
        // the whole table inside, and make some room for flicking.
        loadedTableOuterRect.moveLeft(loadedTable.x() == 0 ? 0 : flickMargin);
        layoutNeeded = true;
    }

    if (loadedTable.y() == 0 && loadedTableOuterRect.y() > 0) {
        loadedTableOuterRect.moveTop(0);
        layoutNeeded = true;
    } else if (loadedTableOuterRect.y() < 0) {
        loadedTableOuterRect.moveTop(loadedTable.y() == 0 ? 0 : flickMargin);
        layoutNeeded = true;
    }

    if (layoutNeeded) {
        qCDebug(lcTableViewDelegateLifecycle);
        relayoutTableItems();
    }
}

void QQuickTableViewPrivate::updateAverageEdgeSize()
{
    int bottomCell = loadedTable.bottom();
    int rightCell = loadedTable.right();
    qreal accRowSpacing = bottomCell * cellSpacing.height();
    qreal accColumnSpacing = rightCell * cellSpacing.width();
    averageEdgeSize.setHeight((loadedTableOuterRect.bottom() - accRowSpacing) / (bottomCell + 1));
    averageEdgeSize.setWidth((loadedTableOuterRect.right() - accColumnSpacing) / (rightCell + 1));
}

void QQuickTableViewPrivate::syncLoadedTableRectFromLoadedTable()
{
    QRectF topLeftRect = loadedTableItem(loadedTable.topLeft())->geometry();
    QRectF bottomRightRect = loadedTableItem(loadedTable.bottomRight())->geometry();
    loadedTableOuterRect = topLeftRect.united(bottomRightRect);
    loadedTableInnerRect = QRectF(topLeftRect.bottomRight(), bottomRightRect.topLeft());
}

void QQuickTableViewPrivate::syncLoadedTableFromLoadRequest()
{
    if (loadRequest.edge() == Qt::Edge(0)) {
        // No edge means we're loading the top-left item
        loadedTable = QRect(loadRequest.firstCell(), loadRequest.lastCell());
        return;
    }

    switch (loadRequest.edge()) {
    case Qt::LeftEdge:
    case Qt::TopEdge:
        loadedTable.setTopLeft(loadRequest.firstCell());
        break;
    case Qt::RightEdge:
    case Qt::BottomEdge:
        loadedTable.setBottomRight(loadRequest.lastCell());
        break;
    }
}

FxTableItem *QQuickTableViewPrivate::itemNextTo(const FxTableItem *fxTableItem, const QPoint &direction) const
{
    return loadedTableItem(fxTableItem->cell + direction);
}

FxTableItem *QQuickTableViewPrivate::loadedTableItem(const QPoint &cell) const
{
    const int modelIndex = modelIndexAtCell(cell);
    Q_TABLEVIEW_ASSERT(loadedItems.contains(modelIndex), modelIndex << cell);
    return loadedItems.value(modelIndex);
}

FxTableItem *QQuickTableViewPrivate::createFxTableItem(const QPoint &cell, QQmlIncubator::IncubationMode incubationMode)
{
    Q_Q(QQuickTableView);

    bool ownItem = false;
    int modelIndex = modelIndexAtCell(cell);

    QObject* object = model->object(modelIndex, incubationMode);
    if (!object) {
        if (model->incubationStatus(modelIndex) == QQmlIncubator::Loading) {
            // Item is incubating. Return nullptr for now, and let the table call this
            // function again once we get a callback to itemCreatedCallback().
            return nullptr;
        }

        qWarning() << "TableView: failed loading index:" << modelIndex;
        object = new QQuickItem();
        ownItem = true;
    }

    QQuickItem *item = qmlobject_cast<QQuickItem*>(object);
    if (!item) {
        // The model could not provide an QQuickItem for the
        // given index, so we create a placeholder instead.
        qWarning() << "TableView: delegate is not an item:" << modelIndex;
        model->release(object);
        item = new QQuickItem();
        ownItem = true;
    } else {
        QQuickAnchors *anchors = QQuickItemPrivate::get(item)->_anchors;
        if (anchors && anchors->activeDirections())
            qmlWarning(item) << "TableView: detected anchors on delegate with index: " << modelIndex
                             << ". Use implicitWidth and implicitHeight instead.";
    }

    if (ownItem) {
        // Parent item is normally set early on from initItemCallback (to
        // allow bindings to the parent property). But if we created the item
        // within this function, we need to set it explicit.
        item->setImplicitWidth(kDefaultColumnWidth);
        item->setImplicitHeight(kDefaultRowHeight);
        item->setParentItem(q->contentItem());
    }
    Q_TABLEVIEW_ASSERT(item->parentItem() == q->contentItem(), item->parentItem());

    FxTableItem *fxTableItem = new FxTableItem(item, q, ownItem);
    fxTableItem->setVisible(false);
    fxTableItem->cell = cell;
    fxTableItem->index = modelIndex;
    return fxTableItem;
}

FxTableItem *QQuickTableViewPrivate::loadFxTableItem(const QPoint &cell, QQmlIncubator::IncubationMode incubationMode)
{
#ifdef QT_DEBUG
    // Since TableView needs to work flawlessly when e.g incubating inside an async
    // loader, being able to override all loading to async while debugging can be helpful.
    static const bool forcedAsync = forcedIncubationMode == QLatin1String("async");
    if (forcedAsync)
        incubationMode = QQmlIncubator::Asynchronous;
#endif

    // Note that even if incubation mode is asynchronous, the item might
    // be ready immediately since the model has a cache of items.
    QBoolBlocker guard(blockItemCreatedCallback);
    auto item = createFxTableItem(cell, incubationMode);
    qCDebug(lcTableViewDelegateLifecycle) << cell << "ready?" << bool(item);
    return item;
}

void QQuickTableViewPrivate::releaseLoadedItems(QQmlTableInstanceModel::ReusableFlag reusableFlag) {
    // Make a copy and clear the list of items first to avoid destroyed
    // items being accessed during the loop (QTBUG-61294)
    auto const tmpList = loadedItems;
    loadedItems.clear();
    for (FxTableItem *item : tmpList)
        releaseItem(item, reusableFlag);
}

void QQuickTableViewPrivate::releaseItem(FxTableItem *fxTableItem, QQmlTableInstanceModel::ReusableFlag reusableFlag)
{
    Q_Q(QQuickTableView);
    auto item = fxTableItem->item;
    Q_TABLEVIEW_ASSERT(item, fxTableItem->index);

    if (fxTableItem->ownItem) {
        delete item;
    } else {
        // Only QQmlTableInstanceModel supports reusing items
        auto releaseFlag = tableModel ?
                    tableModel->release(item, reusableFlag) :
                    model->release(item);

        if (releaseFlag != QQmlInstanceModel::Destroyed) {
            // When items are not destroyed, it typically means that the
            // item is reused, or that the model is an ObjectModel. If
            // so, we just hide the item instead.
            fxTableItem->setVisible(false);

            // If the item (or a descendant) has focus, remove it, so
            // that the item doesn't enter with focus when it's reused.
            if (QQuickWindow *window = item->window()) {
                const auto focusItem = qobject_cast<QQuickItem *>(window->focusObject());
                if (focusItem) {
                    const bool hasFocus = item == focusItem || item->isAncestorOf(focusItem);
                    if (hasFocus) {
                        const auto focusChild = QQuickItemPrivate::get(q)->subFocusItem;
                        QQuickWindowPrivate::get(window)->clearFocusInScope(q, focusChild, Qt::OtherFocusReason);
                    }
                }
            }
        }
    }

    delete fxTableItem;
}

void QQuickTableViewPrivate::unloadItem(const QPoint &cell)
{
    const int modelIndex = modelIndexAtCell(cell);
    Q_TABLEVIEW_ASSERT(loadedItems.contains(modelIndex), modelIndex << cell);
    releaseItem(loadedItems.take(modelIndex), reusableFlag);
}

void QQuickTableViewPrivate::unloadItems(const QLine &items)
{
    qCDebug(lcTableViewDelegateLifecycle) << items;

    if (items.dx()) {
        int y = items.p1().y();
        for (int x = items.p1().x(); x <= items.p2().x(); ++x)
            unloadItem(QPoint(x, y));
    } else {
        int x = items.p1().x();
        for (int y = items.p1().y(); y <= items.p2().y(); ++y)
            unloadItem(QPoint(x, y));
    }
}

bool QQuickTableViewPrivate::canLoadTableEdge(Qt::Edge tableEdge, const QRectF fillRect) const
{
    switch (tableEdge) {
    case Qt::LeftEdge:
        if (loadedTable.topLeft().x() == 0)
            return false;
        return loadedTableOuterRect.left() > fillRect.left() + cellSpacing.width();
    case Qt::RightEdge:
        if (loadedTable.bottomRight().x() >= tableSize.width() - 1)
            return false;
        return loadedTableOuterRect.right() < fillRect.right() - cellSpacing.width();
    case Qt::TopEdge:
        if (loadedTable.topLeft().y() == 0)
            return false;
        return loadedTableOuterRect.top() > fillRect.top() + cellSpacing.height();
    case Qt::BottomEdge:
        if (loadedTable.bottomRight().y() >= tableSize.height() - 1)
            return false;
        return loadedTableOuterRect.bottom() < fillRect.bottom() - cellSpacing.height();
    }

    return false;
}

bool QQuickTableViewPrivate::canUnloadTableEdge(Qt::Edge tableEdge, const QRectF fillRect) const
{
    // Note: if there is only one row or column left, we cannot unload, since
    // they are needed as anchor point for further layouting.
    switch (tableEdge) {
    case Qt::LeftEdge:
        if (loadedTable.width() <= 1)
            return false;
        return loadedTableInnerRect.left() <= fillRect.left();
    case Qt::RightEdge:
        if (loadedTable.width() <= 1)
            return false;
        return loadedTableInnerRect.right() >= fillRect.right();
    case Qt::TopEdge:
        if (loadedTable.height() <= 1)
            return false;
        return loadedTableInnerRect.top() <= fillRect.top();
    case Qt::BottomEdge:
        if (loadedTable.height() <= 1)
            return false;
        return loadedTableInnerRect.bottom() >= fillRect.bottom();
    }
    Q_TABLEVIEW_UNREACHABLE(tableEdge);
    return false;
}

Qt::Edge QQuickTableViewPrivate::nextEdgeToLoad(const QRectF rect)
{
    for (Qt::Edge edge : allTableEdges) {
        if (canLoadTableEdge(edge, rect))
            return edge;
    }
    return Qt::Edge(0);
}

Qt::Edge QQuickTableViewPrivate::nextEdgeToUnload(const QRectF rect)
{
    for (Qt::Edge edge : allTableEdges) {
        if (canUnloadTableEdge(edge, rect))
            return edge;
    }
    return Qt::Edge(0);
}

qreal QQuickTableViewPrivate::cellWidth(const QPoint& cell)
{
    // Using an items width directly is not an option, since we change
    // it during layout (which would also cause problems when recycling items).
    auto const cellItem = loadedTableItem(cell)->item;
    return cellItem->implicitWidth();
}

qreal QQuickTableViewPrivate::cellHeight(const QPoint& cell)
{
    // Using an items height directly is not an option, since we change
    // it during layout (which would also cause problems when recycling items).
    auto const cellItem = loadedTableItem(cell)->item;
    return cellItem->implicitHeight();
}

qreal QQuickTableViewPrivate::sizeHintForColumn(int column)
{
    // Find the widest cell in the column, and return its width
    qreal columnWidth = 0;
    for (int row = loadedTable.top(); row <= loadedTable.bottom(); ++row)
        columnWidth = qMax(columnWidth, cellWidth(QPoint(column, row)));

    return columnWidth;
}

qreal QQuickTableViewPrivate::sizeHintForRow(int row)
{
    // Find the highest cell in the row, and return its height
    qreal rowHeight = 0;
    for (int column = loadedTable.left(); column <= loadedTable.right(); ++column)
        rowHeight = qMax(rowHeight, cellHeight(QPoint(column, row)));

    return rowHeight;
}

void QQuickTableViewPrivate::calculateTableSize()
{
    // tableSize is the same as row and column count, and will always
    // be the same as the number of rows and columns in the model.
    Q_Q(QQuickTableView);
    QSize prevTableSize = tableSize;

    if (tableModel)
        tableSize = QSize(tableModel->columns(), tableModel->rows());
    else if (model)
        tableSize = QSize(1, model->count());
    else
        tableSize = QSize(0, 0);

    if (prevTableSize.width() != tableSize.width())
        emit q->columnsChanged();
    if (prevTableSize.height() != tableSize.height())
        emit q->rowsChanged();
}

qreal QQuickTableViewPrivate::resolveColumnWidth(int column)
{
    Q_TABLEVIEW_ASSERT(column >= loadedTable.left() && column <= loadedTable.right(), column);
    qreal columnWidth = -1;

    if (!columnWidthProvider.isUndefined()) {
        if (columnWidthProvider.isCallable()) {
            auto const columnAsArgument = QJSValueList() << QJSValue(column);
            columnWidth = columnWidthProvider.call(columnAsArgument).toNumber();
            if (qIsNaN(columnWidth) || columnWidth <= 0) {
                // The column width needs to be greater than 0, otherwise we never reach the edge
                // while loading/refilling columns. This would cause the application to hang.
                if (!layoutWarningIssued) {
                    layoutWarningIssued = true;
                    qmlWarning(q_func()) << "columnWidthProvider did not return a valid width for column: " << column;
                }
                columnWidth = kDefaultColumnWidth;
            }
        } else {
            if (!layoutWarningIssued) {
                layoutWarningIssued = true;
                qmlWarning(q_func()) << "columnWidthProvider doesn't contain a function";
            }
            columnWidth = kDefaultColumnWidth;
        }
    } else {
        // If columnWidthProvider is left unspecified, we just iterate over the currently visible items in
        // the column. The downside of doing that, is that the column width will then only be based
        // on the implicit width of the currently loaded items (which can be different depending on
        // which row you're at when the column is flicked in). The upshot is that you don't have to
        // bother setting columnWidthProvider for small tables, or if the implicit width doesn't vary.
        columnWidth = sizeHintForColumn(column);
        if (qIsNaN(columnWidth) || columnWidth <= 0) {
            // The column width needs to be greater than 0, otherwise we never reach the edge
            // while loading/refilling columns. This would cause the application to hang.
            if (!layoutWarningIssued) {
                layoutWarningIssued = true;
                qmlWarning(q_func()) << "the delegate's implicitWidth needs to be greater than zero";
            }
            columnWidth = kDefaultColumnWidth;
        }
    }

    return columnWidth;
}

qreal QQuickTableViewPrivate::resolveRowHeight(int row)
{
    Q_TABLEVIEW_ASSERT(row >= loadedTable.top() && row <= loadedTable.bottom(), row);
    qreal rowHeight = -1;

    if (!rowHeightProvider.isUndefined()) {
        if (rowHeightProvider.isCallable()) {
            auto const rowAsArgument = QJSValueList() << QJSValue(row);
            rowHeight = rowHeightProvider.call(rowAsArgument).toNumber();
            if (qIsNaN(rowHeight) || rowHeight <= 0) {
                // The row height needs to be greater than 0, otherwise we never reach the edge
                // while loading/refilling rows. This would cause the application to hang.
                if (!layoutWarningIssued) {
                    layoutWarningIssued = true;
                    qmlWarning(q_func()) << "rowHeightProvider did not return a valid height for row: " << row;
                }
                rowHeight = kDefaultRowHeight;
            }
        } else {
            if (!layoutWarningIssued) {
                layoutWarningIssued = true;
                qmlWarning(q_func()) << "rowHeightProvider doesn't contain a function";
            }
            rowHeight = kDefaultRowHeight;
        }
    } else {
        // If rowHeightProvider is left unspecified, we just iterate over the currently visible items in
        // the row. The downside of doing that, is that the row height will then only be based
        // on the implicit height of the currently loaded items (which can be different depending on
        // which column you're at when the row is flicked in). The upshot is that you don't have to
        // bother setting rowHeightProvider for small tables, or if the implicit height doesn't vary.
        rowHeight = sizeHintForRow(row);
        if (qIsNaN(rowHeight) || rowHeight <= 0) {
            if (!layoutWarningIssued) {
                layoutWarningIssued = true;
                qmlWarning(q_func()) << "the delegate's implicitHeight needs to be greater than zero";
            }
            rowHeight = kDefaultRowHeight;
        }
    }

    return rowHeight;
}

void QQuickTableViewPrivate::relayoutTable()
{
    relayoutTableItems();
    syncLoadedTableRectFromLoadedTable();
    enforceTableAtOrigin();
    contentSizeBenchMarkPoint = QPoint(-1, -1);
    updateContentWidth();
    updateContentHeight();
    // Return back to updatePolish to loadAndUnloadVisibleEdges()
    // since the re-layout might have caused some edges to be pushed
    // out, while others might have been pushed in.
}

void QQuickTableViewPrivate::relayoutTableItems()
{
    qCDebug(lcTableViewDelegateLifecycle);
    columnRowPositionsInvalid = false;

    qreal nextColumnX = loadedTableOuterRect.x();
    qreal nextRowY = loadedTableOuterRect.y();

    for (int column = loadedTable.left(); column <= loadedTable.right(); ++column) {
        // Adjust the geometry of all cells in the current column
        const qreal width = resolveColumnWidth(column);

        for (int row = loadedTable.top(); row <= loadedTable.bottom(); ++row) {
            auto item = loadedTableItem(QPoint(column, row));
            QRectF geometry = item->geometry();
            geometry.moveLeft(nextColumnX);
            geometry.setWidth(width);
            item->setGeometry(geometry);
        }

        nextColumnX += width + cellSpacing.width();
    }

    for (int row = loadedTable.top(); row <= loadedTable.bottom(); ++row) {
        // Adjust the geometry of all cells in the current row
        const qreal height = resolveRowHeight(row);

        for (int column = loadedTable.left(); column <= loadedTable.right(); ++column) {
            auto item = loadedTableItem(QPoint(column, row));
            QRectF geometry = item->geometry();
            geometry.moveTop(nextRowY);
            geometry.setHeight(height);
            item->setGeometry(geometry);
        }

        nextRowY += height + cellSpacing.height();
    }

    if (Q_UNLIKELY(lcTableViewDelegateLifecycle().isDebugEnabled())) {
        for (int column = loadedTable.left(); column <= loadedTable.right(); ++column) {
            for (int row = loadedTable.top(); row <= loadedTable.bottom(); ++row) {
                QPoint cell = QPoint(column, row);
                qCDebug(lcTableViewDelegateLifecycle()) << "relayout item:" << cell << loadedTableItem(cell)->geometry();
            }
        }
    }
}

void QQuickTableViewPrivate::layoutVerticalEdge(Qt::Edge tableEdge)
{
    int column = (tableEdge == Qt::LeftEdge) ? loadedTable.left() : loadedTable.right();
    QPoint neighbourDirection = (tableEdge == Qt::LeftEdge) ? kRight : kLeft;
    qreal width = resolveColumnWidth(column);

    for (int row = loadedTable.top(); row <= loadedTable.bottom(); ++row) {
        auto fxTableItem = loadedTableItem(QPoint(column, row));
        auto const neighbourItem = itemNextTo(fxTableItem, neighbourDirection);

        QRectF geometry = fxTableItem->geometry();
        geometry.setWidth(width);
        geometry.setHeight(neighbourItem->geometry().height());
        qreal left = tableEdge == Qt::LeftEdge ?
                    neighbourItem->geometry().left() - cellSpacing.width() - geometry.width() :
                    neighbourItem->geometry().right() + cellSpacing.width();

        geometry.moveLeft(left);
        geometry.moveTop(neighbourItem->geometry().top());

        fxTableItem->setGeometry(geometry);
        fxTableItem->setVisible(true);

        qCDebug(lcTableViewDelegateLifecycle()) << "layout item:" << QPoint(column, row) << fxTableItem->geometry();
    }
}

void QQuickTableViewPrivate::layoutHorizontalEdge(Qt::Edge tableEdge)
{
    int row = (tableEdge == Qt::TopEdge) ? loadedTable.top() : loadedTable.bottom();
    QPoint neighbourDirection = (tableEdge == Qt::TopEdge) ? kDown : kUp;
    qreal height = resolveRowHeight(row);

    for (int column = loadedTable.left(); column <= loadedTable.right(); ++column) {
        auto fxTableItem = loadedTableItem(QPoint(column, row));
        auto const neighbourItem = itemNextTo(fxTableItem, neighbourDirection);

        QRectF geometry = fxTableItem->geometry();
        geometry.setWidth(neighbourItem->geometry().width());
        geometry.setHeight(height);
        qreal top = tableEdge == Qt::TopEdge ?
                    neighbourItem->geometry().top() - cellSpacing.height() - geometry.height() :
                    neighbourItem->geometry().bottom() + cellSpacing.height();

        geometry.moveTop(top);
        geometry.moveLeft(neighbourItem->geometry().left());

        fxTableItem->setGeometry(geometry);
        fxTableItem->setVisible(true);

        qCDebug(lcTableViewDelegateLifecycle()) << "layout item:" << QPoint(column, row) << fxTableItem->geometry();
    }
}

void QQuickTableViewPrivate::layoutTopLeftItem()
{
    const QPoint cell = loadRequest.firstCell();
    auto topLeftItem = loadedTableItem(cell);
    auto item = topLeftItem->item;

    item->setPosition(loadRequest.startPosition());
    item->setSize(QSizeF(resolveColumnWidth(cell.x()), resolveRowHeight(cell.y())));
    topLeftItem->setVisible(true);
    qCDebug(lcTableViewDelegateLifecycle) << "geometry:" << topLeftItem->geometry();
}

void QQuickTableViewPrivate::layoutTableEdgeFromLoadRequest()
{
    if (loadRequest.edge() == Qt::Edge(0)) {
        // No edge means we're loading the top-left item
        layoutTopLeftItem();
        return;
    }

    switch (loadRequest.edge()) {
    case Qt::LeftEdge:
    case Qt::RightEdge:
        layoutVerticalEdge(loadRequest.edge());
        break;
    case Qt::TopEdge:
    case Qt::BottomEdge:
        layoutHorizontalEdge(loadRequest.edge());
        break;
    }
}

void QQuickTableViewPrivate::cancelLoadRequest()
{
    loadRequest.markAsDone();
    model->cancel(modelIndexAtCell(loadRequest.currentCell()));

    if (rebuildScheduled) {
        // No reason to rollback already loaded edge items
        // since we anyway are about to reload all items.
        return;
    }

    if (loadRequest.atBeginning()) {
        // No items have yet been loaded, so nothing to unload
        return;
    }

    QLine rollbackItems;
    rollbackItems.setP1(loadRequest.firstCell());
    rollbackItems.setP2(loadRequest.previousCell());
    qCDebug(lcTableViewDelegateLifecycle()) << "rollback:" << rollbackItems << tableLayoutToString();
    unloadItems(rollbackItems);
}

void QQuickTableViewPrivate::processLoadRequest()
{
    Q_TABLEVIEW_ASSERT(loadRequest.isActive(), "");

    while (loadRequest.hasCurrentCell()) {
        QPoint cell = loadRequest.currentCell();
        FxTableItem *fxTableItem = loadFxTableItem(cell, loadRequest.incubationMode());

        if (!fxTableItem) {
            // Requested item is not yet ready. Just leave, and wait for this
            // function to be called again when the item is ready.
            return;
        }

        loadedItems.insert(modelIndexAtCell(cell), fxTableItem);
        loadRequest.moveToNextCell();
    }

    qCDebug(lcTableViewDelegateLifecycle()) << "all items loaded!";

    syncLoadedTableFromLoadRequest();
    layoutTableEdgeFromLoadRequest();
    syncLoadedTableRectFromLoadedTable();

    if (rebuildState == RebuildState::Done) {
        enforceTableAtOrigin();
        updateContentWidth();
        updateContentHeight();
        drainReusePoolAfterLoadRequest();
    }

    loadRequest.markAsDone();

    qCDebug(lcTableViewDelegateLifecycle()) << "request completed! Table:" << tableLayoutToString();
}

void QQuickTableViewPrivate::processRebuildTable()
{
    moveToNextRebuildState();

    if (rebuildState == RebuildState::LoadInitalTable) {
        beginRebuildTable();
        if (!moveToNextRebuildState())
            return;
    }

    if (rebuildState == RebuildState::VerifyTable) {
        if (loadedItems.isEmpty()) {
            qCDebug(lcTableViewDelegateLifecycle()) << "no items loaded, meaning empty model or no delegate";
            rebuildState = RebuildState::Done;
            return;
        }
        if (!moveToNextRebuildState())
            return;
    }

    if (rebuildState == RebuildState::LayoutTable) {
        layoutAfterLoadingInitialTable();
        if (!moveToNextRebuildState())
            return;
    }

    if (rebuildState == RebuildState::LoadAndUnloadAfterLayout) {
        loadAndUnloadVisibleEdges();
        if (!moveToNextRebuildState())
            return;
    }

    const bool preload = (rebuildOptions & RebuildOption::All
                          && reusableFlag == QQmlTableInstanceModel::Reusable);

    if (rebuildState == RebuildState::PreloadColumns) {
        if (preload && loadedTable.right() < tableSize.width() - 1)
            loadEdge(Qt::RightEdge, QQmlIncubator::AsynchronousIfNested);
        if (!moveToNextRebuildState())
            return;
    }

    if (rebuildState == RebuildState::PreloadRows) {
        if (preload && loadedTable.bottom() < tableSize.height() - 1)
            loadEdge(Qt::BottomEdge, QQmlIncubator::AsynchronousIfNested);
        if (!moveToNextRebuildState())
            return;
    }

    if (rebuildState == RebuildState::MovePreloadedItemsToPool) {
        while (Qt::Edge edge = nextEdgeToUnload(viewportRect))
            unloadEdge(edge);
        if (!moveToNextRebuildState())
            return;
    }

    Q_TABLEVIEW_ASSERT(rebuildState == RebuildState::Done, int(rebuildState));
}

bool QQuickTableViewPrivate::moveToNextRebuildState()
{
    if (loadRequest.isActive()) {
        // Items are still loading async, which means
        // that the current state is not yet done.
        return false;
    }
    rebuildState = RebuildState(int(rebuildState) + 1);
    qCDebug(lcTableViewDelegateLifecycle()) << int(rebuildState);
    return true;
}

void QQuickTableViewPrivate::beginRebuildTable()
{
    if (loadRequest.isActive())
        cancelLoadRequest();

    calculateTableSize();

    QPoint topLeft;
    QPointF topLeftPos;

    if (rebuildOptions & RebuildOption::All) {
        qCDebug(lcTableViewDelegateLifecycle()) << "RebuildOption::All";
        releaseLoadedItems(QQmlTableInstanceModel::NotReusable);
    } else if (rebuildOptions & RebuildOption::ViewportOnly) {
        qCDebug(lcTableViewDelegateLifecycle()) << "RebuildOption::ViewportOnly";
        releaseLoadedItems(reusableFlag);

        if (rebuildOptions & RebuildOption::CalculateNewTopLeftRow) {
            const int newRow = int(viewportRect.y() / (averageEdgeSize.height() + cellSpacing.height()));
            topLeft.ry() = qBound(0, newRow, tableSize.height() - 1);
            topLeftPos.ry() = topLeft.y() * (averageEdgeSize.height() + cellSpacing.height());
        } else {
            topLeft.ry() = qBound(0, loadedTable.topLeft().y(), tableSize.height() - 1);
            topLeftPos.ry() = loadedTableOuterRect.topLeft().y();
        }
        if (rebuildOptions & RebuildOption::CalculateNewTopLeftColumn) {
            const int newColumn = int(viewportRect.x() / (averageEdgeSize.width() + cellSpacing.width()));
            topLeft.rx() = qBound(0, newColumn, tableSize.width() - 1);
            topLeftPos.rx() = topLeft.x() * (averageEdgeSize.width() + cellSpacing.width());
        } else {
            topLeft.rx() = qBound(0, loadedTable.topLeft().x(), tableSize.width() - 1);
            topLeftPos.rx() = loadedTableOuterRect.topLeft().x();
        }
    } else {
        Q_TABLEVIEW_UNREACHABLE(rebuildOptions);
    }

    loadedTable = QRect();
    loadedTableOuterRect = QRect();
    loadedTableInnerRect = QRect();
    contentSizeBenchMarkPoint = QPoint(-1, -1);
    columnRowPositionsInvalid = false;

    loadInitialTopLeftItem(topLeft, topLeftPos);
    loadAndUnloadVisibleEdges();
}

void QQuickTableViewPrivate::layoutAfterLoadingInitialTable()
{
    if (rowHeightProvider.isUndefined() || columnWidthProvider.isUndefined()) {
        // Since we don't have both size providers, we need to calculate the
        // size of each row and column based on the size of the delegate items.
        // This couldn't be done while we were loading the initial rows and
        // columns, since during the process, we didn't have all the items
        // available yet for the calculation. So we do it now.
        relayoutTable();
    }

    updateAverageEdgeSize();
    updateContentWidth();
    updateContentHeight();
}

void QQuickTableViewPrivate::loadInitialTopLeftItem(const QPoint &cell, const QPointF &pos)
{
    Q_TABLEVIEW_ASSERT(loadedItems.isEmpty(), "");

    if (tableSize.isEmpty())
        return;

    if (model->count() == 0)
        return;

    if (tableModel && !tableModel->delegate())
        return;

    // Load top-left item. After loaded, loadItemsInsideRect() will take
    // care of filling out the rest of the table.
    loadRequest.begin(cell, pos, QQmlIncubator::AsynchronousIfNested);
    processLoadRequest();
}

void QQuickTableViewPrivate::unloadEdge(Qt::Edge edge)
{
    unloadItems(rectangleEdge(loadedTable, edge));
    loadedTable = expandedRect(loadedTable, edge, -1);
    syncLoadedTableRectFromLoadedTable();
    qCDebug(lcTableViewDelegateLifecycle) << tableLayoutToString();
}

void QQuickTableViewPrivate::loadEdge(Qt::Edge edge, QQmlIncubator::IncubationMode incubationMode)
{
    QLine cellsToLoad = rectangleEdge(expandedRect(loadedTable, edge, 1), edge);
    loadRequest.begin(cellsToLoad, edge, incubationMode);
    processLoadRequest();
}

void QQuickTableViewPrivate::loadAndUnloadVisibleEdges()
{
    // Unload table edges that have been moved outside the visible part of the
    // table (including buffer area), and load new edges that has been moved inside.
    // Note: an important point is that we always keep the table rectangular
    // and without holes to reduce complexity (we never leave the table in
    // a half-loaded state, or keep track of multiple patches).
    // We load only one edge (row or column) at a time. This is especially
    // important when loading into the buffer, since we need to be able to
    // cancel the buffering quickly if the user starts to flick, and then
    // focus all further loading on the edges that are flicked into view.

    if (loadRequest.isActive()) {
        // Don't start loading more edges while we're
        // already waiting for another one to load.
        return;
    }

    if (loadedItems.isEmpty()) {
        // We need at least the top-left item to be loaded before we can
        // start loading edges around it. Not having a top-left item at
        // this point means that the model is empty (or no delegate).
        return;
    }

    bool tableModified;

    do {
        tableModified = false;

        if (Qt::Edge edge = nextEdgeToUnload(viewportRect)) {
            tableModified = true;
            unloadEdge(edge);
        }

        if (Qt::Edge edge = nextEdgeToLoad(viewportRect)) {
            tableModified = true;
            loadEdge(edge, QQmlIncubator::AsynchronousIfNested);
            if (loadRequest.isActive())
                return;
        }
    } while (tableModified);

}

void QQuickTableViewPrivate::drainReusePoolAfterLoadRequest()
{
    Q_Q(QQuickTableView);

    if (reusableFlag == QQmlTableInstanceModel::NotReusable || !tableModel)
        return;

    if (!qFuzzyIsNull(q->verticalOvershoot()) || !qFuzzyIsNull(q->horizontalOvershoot())) {
        // Don't drain while we're overshooting, since this will fill up the
        // pool, but we expect to reuse them all once the content item moves back.
        return;
    }

    // When loading edges, we don't want to drain the reuse pool too aggressively. Normally,
    // all the items in the pool are reused rapidly as the content view is flicked around
    // anyway. Even if the table is temporarily flicked to a section that contains fewer
    // cells than what used to be (e.g if the flicked-in rows are taller than average), it
    // still makes sense to keep all the items in circulation; Chances are, that soon enough,
    // thinner rows are flicked back in again (meaning that we can fit more items into the
    // view). But at the same time, if a delegate chooser is in use, the pool might contain
    // items created from different delegates. And some of those delegates might be used only
    // occasionally. So to avoid situations where an item ends up in the pool for too long, we
    // call drain after each load request, but with a sufficiently large pool time. (If an item
    // in the pool has a large pool time, it means that it hasn't been reused for an equal
    // amount of load cycles, and should be released).
    //
    // We calculate an appropriate pool time by figuring out what the minimum time must be to
    // not disturb frequently reused items. Since the number of items in a row might be higher
    // than in a column (or vice versa), the minimum pool time should take into account that
    // you might be flicking out a single row (filling up the pool), before you continue
    // flicking in several new columns (taking them out again, but now in smaller chunks). This
    // will increase the number of load cycles items are kept in the pool (poolTime), but still,
    // we shouldn't release them, as they are still being reused frequently.
    // To get a flexible maxValue (that e.g tolerates rows and columns being flicked
    // in with varying sizes, causing some items not to be resued immediately), we multiply the
    // value by 2. Note that we also add an extra +1 to the column count, because the number of
    // visible columns will fluctuate between +1/-1 while flicking.
    const int w = loadedTable.width();
    const int h = loadedTable.height();
    const int minTime = int(std::ceil(w > h ? qreal(w + 1) / h : qreal(h + 1) / w));
    const int maxTime = minTime * 2;
    tableModel->drainReusableItemsPool(maxTime);
}

void QQuickTableViewPrivate::scheduleRebuildTable(RebuildOptions options) {
    if (!q_func()->isComponentComplete()) {
        // We'll rebuild the table once complete anyway
        return;
    }

    rebuildScheduled = true;
    scheduledRebuildOptions |= options;
    q_func()->polish();
}

void QQuickTableViewPrivate::invalidateColumnRowPositions() {
    columnRowPositionsInvalid = true;
    q_func()->polish();
}

void QQuickTableViewPrivate::updatePolish()
{
    // Whenever something changes, e.g viewport moves, spacing is set to a
    // new value, model changes etc, this function will end up being called. Here
    // we check what needs to be done, and load/unload cells accordingly.

    Q_TABLEVIEW_ASSERT(!polishing, "recursive updatePolish() calls are not allowed!");
    QBoolBlocker polishGuard(polishing, true);

    if (loadRequest.isActive()) {
        // We're currently loading items async to build a new edge in the table. We see the loading
        // as an atomic operation, which means that we don't continue doing anything else until all
        // items have been received and laid out. Note that updatePolish is then called once more
        // after the loadRequest has completed to handle anything that might have occurred in-between.
        return;
    }

    if (rebuildState != RebuildState::Done) {
        processRebuildTable();
        return;
    }

    syncWithPendingChanges();

    if (rebuildState == RebuildState::Begin) {
        processRebuildTable();
        return;
    }

    if (loadedItems.isEmpty())
        return;

    if (columnRowPositionsInvalid)
        relayoutTable();

    loadAndUnloadVisibleEdges();
}

void QQuickTableViewPrivate::fixup(QQuickFlickablePrivate::AxisData &data, qreal minExtent, qreal maxExtent)
{
    if (rebuildScheduled || rebuildState != RebuildState::Done)
        return;

    QQuickFlickablePrivate::fixup(data, minExtent, maxExtent);
}

int QQuickTableViewPrivate::resolveImportVersion()
{
    const auto data = QQmlData::get(q_func());
    if (!data || !data->propertyCache)
        return 0;

    const auto cppMetaObject = data->propertyCache->firstCppMetaObject();
    const auto qmlTypeView = QQmlMetaType::qmlType(cppMetaObject);
    return qmlTypeView.minorVersion();
}

void QQuickTableViewPrivate::createWrapperModel()
{
    Q_Q(QQuickTableView);
    // When the assigned model is not an instance model, we create a wrapper
    // model (QQmlTableInstanceModel) that keeps a pointer to both the
    // assigned model and the assigned delegate. This model will give us a
    // common interface to any kind of model (js arrays, QAIM, number etc), and
    // help us create delegate instances.
    tableModel = new QQmlTableInstanceModel(qmlContext(q));
    tableModel->useImportVersion(resolveImportVersion());
    model = tableModel;
}

void QQuickTableViewPrivate::itemCreatedCallback(int modelIndex, QObject*)
{
    if (blockItemCreatedCallback)
        return;

    qCDebug(lcTableViewDelegateLifecycle) << "item done loading:"
        << cellAtModelIndex(modelIndex);

    // Since the item we waited for has finished incubating, we can
    // continue with the load request. processLoadRequest will
    // ask the model for the requested item once more, which will be
    // quick since the model has cached it.
    processLoadRequest();
    loadAndUnloadVisibleEdges();
    updatePolish();
}

void QQuickTableViewPrivate::initItemCallback(int modelIndex, QObject *object)
{
    Q_UNUSED(modelIndex);
    Q_Q(QQuickTableView);

    if (auto item = qmlobject_cast<QQuickItem*>(object)) {
        item->setParentItem(q->contentItem());
        item->setZ(1);
    }

    if (auto attached = getAttachedObject(object))
        attached->setView(q);
}

void QQuickTableViewPrivate::itemPooledCallback(int modelIndex, QObject *object)
{
    Q_UNUSED(modelIndex);

    if (auto attached = getAttachedObject(object))
        emit attached->pooled();
}

void QQuickTableViewPrivate::itemReusedCallback(int modelIndex, QObject *object)
{
    Q_UNUSED(modelIndex);

    if (auto attached = getAttachedObject(object))
        emit attached->reused();
}

void QQuickTableViewPrivate::syncWithPendingChanges()
{
    // The application can change properties like the model or the delegate while
    // we're e.g in the middle of e.g loading a new row. Since this will lead to
    // unpredicted behavior, and possibly a crash, we need to postpone taking
    // such assignments into effect until we're in a state that allows it.
    Q_Q(QQuickTableView);
    viewportRect = QRectF(q->contentX(), q->contentY(), q->width(), q->height());
    syncRebuildOptions();
    syncModel();
    syncDelegate();
}

void QQuickTableViewPrivate::syncRebuildOptions()
{
    if (!rebuildScheduled)
        return;

    rebuildState = RebuildState::Begin;
    rebuildOptions = scheduledRebuildOptions;
    scheduledRebuildOptions = RebuildOption::None;
    rebuildScheduled = false;
}

void QQuickTableViewPrivate::syncDelegate()
{
    if (tableModel && assignedDelegate == tableModel->delegate())
        return;

    if (!tableModel)
        createWrapperModel();

    tableModel->setDelegate(assignedDelegate);
}

void QQuickTableViewPrivate::syncModel()
{
    if (modelVariant == assignedModel)
        return;

    if (model)
        disconnectFromModel();

    modelVariant = assignedModel;
    QVariant effectiveModelVariant = modelVariant;
    if (effectiveModelVariant.userType() == qMetaTypeId<QJSValue>())
        effectiveModelVariant = effectiveModelVariant.value<QJSValue>().toVariant();

    const auto instanceModel = qobject_cast<QQmlInstanceModel *>(qvariant_cast<QObject*>(effectiveModelVariant));

    if (instanceModel) {
        if (tableModel) {
            delete tableModel;
            tableModel = nullptr;
        }
        model = instanceModel;
    } else {
        if (!tableModel)
            createWrapperModel();
        tableModel->setModel(effectiveModelVariant);
    }

    connectToModel();
}

void QQuickTableViewPrivate::connectToModel()
{
    Q_TABLEVIEW_ASSERT(model, "");

    QObjectPrivate::connect(model, &QQmlInstanceModel::createdItem, this, &QQuickTableViewPrivate::itemCreatedCallback);
    QObjectPrivate::connect(model, &QQmlInstanceModel::initItem, this, &QQuickTableViewPrivate::initItemCallback);

    if (tableModel) {
        const auto tm = tableModel.data();
        QObjectPrivate::connect(tm, &QQmlTableInstanceModel::itemPooled, this, &QQuickTableViewPrivate::itemPooledCallback);
        QObjectPrivate::connect(tm, &QQmlTableInstanceModel::itemReused, this, &QQuickTableViewPrivate::itemReusedCallback);
    }

    if (auto const aim = model->abstractItemModel()) {
        // When the model exposes a QAIM, we connect to it directly. This means that if the current model is
        // a QQmlDelegateModel, we just ignore all the change sets it emits. In most cases, the model will instead
        // be our own QQmlTableInstanceModel, which doesn't bother creating change sets at all. For models that are
        // not based on QAIM (like QQmlObjectModel, QQmlListModel, javascript arrays etc), there is currently no way
        // to modify the model at runtime without also re-setting the model on the view.
        connect(aim, &QAbstractItemModel::rowsMoved, this, &QQuickTableViewPrivate::rowsMovedCallback);
        connect(aim, &QAbstractItemModel::columnsMoved, this, &QQuickTableViewPrivate::columnsMovedCallback);
        connect(aim, &QAbstractItemModel::rowsInserted, this, &QQuickTableViewPrivate::rowsInsertedCallback);
        connect(aim, &QAbstractItemModel::rowsRemoved, this, &QQuickTableViewPrivate::rowsRemovedCallback);
        connect(aim, &QAbstractItemModel::columnsInserted, this, &QQuickTableViewPrivate::columnsInsertedCallback);
        connect(aim, &QAbstractItemModel::columnsRemoved, this, &QQuickTableViewPrivate::columnsRemovedCallback);
        connect(aim, &QAbstractItemModel::modelReset, this, &QQuickTableViewPrivate::modelResetCallback);
        connect(aim, &QAbstractItemModel::layoutChanged, this, &QQuickTableViewPrivate::layoutChangedCallback);
    } else {
        QObjectPrivate::connect(model, &QQmlInstanceModel::modelUpdated, this, &QQuickTableViewPrivate::modelUpdated);
    }
}

void QQuickTableViewPrivate::disconnectFromModel()
{
    Q_TABLEVIEW_ASSERT(model, "");

    QObjectPrivate::disconnect(model, &QQmlInstanceModel::createdItem, this, &QQuickTableViewPrivate::itemCreatedCallback);
    QObjectPrivate::disconnect(model, &QQmlInstanceModel::initItem, this, &QQuickTableViewPrivate::initItemCallback);

    if (tableModel) {
        const auto tm = tableModel.data();
        QObjectPrivate::disconnect(tm, &QQmlTableInstanceModel::itemPooled, this, &QQuickTableViewPrivate::itemPooledCallback);
        QObjectPrivate::disconnect(tm, &QQmlTableInstanceModel::itemReused, this, &QQuickTableViewPrivate::itemReusedCallback);
    }

    if (auto const aim = model->abstractItemModel()) {
        disconnect(aim, &QAbstractItemModel::rowsMoved, this, &QQuickTableViewPrivate::rowsMovedCallback);
        disconnect(aim, &QAbstractItemModel::columnsMoved, this, &QQuickTableViewPrivate::columnsMovedCallback);
        disconnect(aim, &QAbstractItemModel::rowsInserted, this, &QQuickTableViewPrivate::rowsInsertedCallback);
        disconnect(aim, &QAbstractItemModel::rowsRemoved, this, &QQuickTableViewPrivate::rowsRemovedCallback);
        disconnect(aim, &QAbstractItemModel::columnsInserted, this, &QQuickTableViewPrivate::columnsInsertedCallback);
        disconnect(aim, &QAbstractItemModel::columnsRemoved, this, &QQuickTableViewPrivate::columnsRemovedCallback);
        disconnect(aim, &QAbstractItemModel::modelReset, this, &QQuickTableViewPrivate::modelResetCallback);
        disconnect(aim, &QAbstractItemModel::layoutChanged, this, &QQuickTableViewPrivate::layoutChangedCallback);
    } else {
        QObjectPrivate::disconnect(model, &QQmlInstanceModel::modelUpdated, this, &QQuickTableViewPrivate::modelUpdated);
    }
}

void QQuickTableViewPrivate::modelUpdated(const QQmlChangeSet &changeSet, bool reset)
{
    Q_UNUSED(changeSet);
    Q_UNUSED(reset);

    Q_TABLEVIEW_ASSERT(!model->abstractItemModel(), "");
    scheduleRebuildTable(RebuildOption::ViewportOnly);
}

void QQuickTableViewPrivate::rowsMovedCallback(const QModelIndex &parent, int, int, const QModelIndex &, int )
{
    if (parent != QModelIndex())
        return;

    scheduleRebuildTable(RebuildOption::ViewportOnly);
}

void QQuickTableViewPrivate::columnsMovedCallback(const QModelIndex &parent, int, int, const QModelIndex &, int)
{
    if (parent != QModelIndex())
        return;

    scheduleRebuildTable(RebuildOption::ViewportOnly);
}

void QQuickTableViewPrivate::rowsInsertedCallback(const QModelIndex &parent, int, int)
{
    if (parent != QModelIndex())
        return;

    scheduleRebuildTable(RebuildOption::ViewportOnly);
}

void QQuickTableViewPrivate::rowsRemovedCallback(const QModelIndex &parent, int, int)
{
    if (parent != QModelIndex())
        return;

    scheduleRebuildTable(RebuildOption::ViewportOnly);
}

void QQuickTableViewPrivate::columnsInsertedCallback(const QModelIndex &parent, int, int)
{
    if (parent != QModelIndex())
        return;

    scheduleRebuildTable(RebuildOption::ViewportOnly);
}

void QQuickTableViewPrivate::columnsRemovedCallback(const QModelIndex &parent, int, int)
{
    if (parent != QModelIndex())
        return;

    scheduleRebuildTable(RebuildOption::ViewportOnly);
}

void QQuickTableViewPrivate::layoutChangedCallback(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(parents);
    Q_UNUSED(hint);

    scheduleRebuildTable(RebuildOption::ViewportOnly);
}

void QQuickTableViewPrivate::modelResetCallback()
{
    scheduleRebuildTable(RebuildOption::All);
}

QQuickTableView::QQuickTableView(QQuickItem *parent)
    : QQuickFlickable(*(new QQuickTableViewPrivate), parent)
{
    setFlag(QQuickItem::ItemIsFocusScope);
}

int QQuickTableView::rows() const
{
    return d_func()->tableSize.height();
}

int QQuickTableView::columns() const
{
    return d_func()->tableSize.width();
}

qreal QQuickTableView::rowSpacing() const
{
    return d_func()->cellSpacing.height();
}

void QQuickTableView::setRowSpacing(qreal spacing)
{
    Q_D(QQuickTableView);
    if (qt_is_nan(spacing) || !qt_is_finite(spacing) || spacing < 0)
        return;
    if (qFuzzyCompare(d->cellSpacing.height(), spacing))
        return;

    d->cellSpacing.setHeight(spacing);
    d->invalidateColumnRowPositions();
    emit rowSpacingChanged();
}

qreal QQuickTableView::columnSpacing() const
{
    return d_func()->cellSpacing.width();
}

void QQuickTableView::setColumnSpacing(qreal spacing)
{
    Q_D(QQuickTableView);
    if (qt_is_nan(spacing) || !qt_is_finite(spacing) || spacing < 0)
        return;
    if (qFuzzyCompare(d->cellSpacing.width(), spacing))
        return;

    d->cellSpacing.setWidth(spacing);
    d->invalidateColumnRowPositions();
    emit columnSpacingChanged();
}

QJSValue QQuickTableView::rowHeightProvider() const
{
    return d_func()->rowHeightProvider;
}

void QQuickTableView::setRowHeightProvider(QJSValue provider)
{
    Q_D(QQuickTableView);
    if (provider.strictlyEquals(d->rowHeightProvider))
        return;

    d->rowHeightProvider = provider;
    d->scheduleRebuildTable(QQuickTableViewPrivate::RebuildOption::ViewportOnly);
    emit rowHeightProviderChanged();
}

QJSValue QQuickTableView::columnWidthProvider() const
{
    return d_func()->columnWidthProvider;
}

void QQuickTableView::setColumnWidthProvider(QJSValue provider)
{
    Q_D(QQuickTableView);
    if (provider.strictlyEquals(d->columnWidthProvider))
        return;

    d->columnWidthProvider = provider;
    d->scheduleRebuildTable(QQuickTableViewPrivate::RebuildOption::ViewportOnly);
    emit columnWidthProviderChanged();
}

QVariant QQuickTableView::model() const
{
    return d_func()->assignedModel;
}

void QQuickTableView::setModel(const QVariant &newModel)
{
    Q_D(QQuickTableView);
    if (newModel == d->assignedModel)
        return;

    d->assignedModel = newModel;
    d->scheduleRebuildTable(QQuickTableViewPrivate::RebuildOption::All);
    emit modelChanged();
}

QQmlComponent *QQuickTableView::delegate() const
{
    return d_func()->assignedDelegate;
}

void QQuickTableView::setDelegate(QQmlComponent *newDelegate)
{
    Q_D(QQuickTableView);
    if (newDelegate == d->assignedDelegate)
        return;

    d->assignedDelegate = newDelegate;
    d->scheduleRebuildTable(QQuickTableViewPrivate::RebuildOption::All);

    emit delegateChanged();
}

bool QQuickTableView::reuseItems() const
{
    return bool(d_func()->reusableFlag == QQmlTableInstanceModel::Reusable);
}

void QQuickTableView::setReuseItems(bool reuse)
{
    Q_D(QQuickTableView);
    if (reuseItems() == reuse)
        return;

    d->reusableFlag = reuse ? QQmlTableInstanceModel::Reusable : QQmlTableInstanceModel::NotReusable;

    if (!reuse && d->tableModel) {
        // When we're told to not reuse items, we
        // immediately, as documented, drain the pool.
        d->tableModel->drainReusableItemsPool(0);
    }

    emit reuseItemsChanged();
}

void QQuickTableView::setContentWidth(qreal width)
{
    Q_D(QQuickTableView);
    d->explicitContentWidth = width;
    QQuickFlickable::setContentWidth(width);
}

void QQuickTableView::setContentHeight(qreal height)
{
    Q_D(QQuickTableView);
    d->explicitContentHeight = height;
    QQuickFlickable::setContentHeight(height);
}

void QQuickTableView::forceLayout()
{
    Q_D(QQuickTableView);
    d->columnRowPositionsInvalid = true;

    if (d->polishing) {
        qWarning() << "TableView::forceLayout(): Cannot do an immediate re-layout during an ongoing layout!";
        polish();
        return;
    }

    d->updatePolish();
}

QQuickTableViewAttached *QQuickTableView::qmlAttachedProperties(QObject *obj)
{
    return new QQuickTableViewAttached(obj);
}

void QQuickTableView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickTableView);
    QQuickFlickable::geometryChanged(newGeometry, oldGeometry);

    if (d->tableModel) {
        // When the view changes size, we force the pool to
        // shrink by releasing all pooled items.
        d->tableModel->drainReusableItemsPool(0);
    }

    polish();
}

void QQuickTableView::viewportMoved(Qt::Orientations orientation)
{
    Q_D(QQuickTableView);
    QQuickFlickable::viewportMoved(orientation);

    QQuickTableViewPrivate::RebuildOptions options = QQuickTableViewPrivate::RebuildOption::None;

    // Check the viewport moved more than one page vertically
    if (!d->viewportRect.intersects(QRectF(d->viewportRect.x(), contentY(), 1, height())))
        options |= QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftRow;
    // Check the viewport moved more than one page horizontally
    if (!d->viewportRect.intersects(QRectF(contentX(), d->viewportRect.y(), width(), 1)))
        options |= QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftColumn;

    if (options) {
        // When the viewport has moved more than one page vertically or horizontally, we switch
        // strategy from refilling edges around the current table to instead rebuild the table
        // from scratch inside the new viewport. This will greatly improve performance when flicking
        // a long distance in one go, which can easily happen when dragging on scrollbars.
        options |= QQuickTableViewPrivate::RebuildOption::ViewportOnly;
        d->scheduleRebuildTable(options);
    }

    if (d->rebuildScheduled) {
        // No reason to do anything, since we're about to rebuild the whole table anyway.
        // Besides, calling updatePolish, which will start the rebuild, can easily cause
        // binding loops to happen since we usually end up modifying the geometry of the
        // viewport (contentItem) as well.
        return;
    }

    // Calling polish() will schedule a polish event. But while the user is flicking, several
    // mouse events will be handled before we get an updatePolish() call. And the updatePolish()
    // call will only see the last mouse position. This results in a stuttering flick experience
    // (especially on windows). We improve on this by calling updatePolish() directly. But this
    // has the pitfall that we open up for recursive callbacks. E.g while inside updatePolish(), we
    // load/unload items, and emit signals. The application can listen to those signals and set a
    // new contentX/Y on the flickable. So we need to guard for this, to avoid unexpected behavior.
    if (d->polishing)
        polish();
    else
        d->updatePolish();
}

void QQuickTableViewPrivate::_q_componentFinalized()
{
    // Now that all bindings are evaluated, and we know
    // our final geometery, we can build the table.
    qCDebug(lcTableViewDelegateLifecycle);
    updatePolish();
}

void QQuickTableViewPrivate::registerCallbackWhenBindingsAreEvaluated()
{
    // componentComplete() is called on us after all static values have been assigned, but
    // before bindings to any anchestors has been evaluated. Especially this means that
    // if our size is bound to the parents size, it will still be empty at that point.
    // And we cannot build the table without knowing our own size. We could wait until we
    // got the first updatePolish() callback, but at that time, any asynchronous loaders that we
    // might be inside have already finished loading, which means that we would load all
    // the delegate items synchronously instead of asynchronously. We therefore add a componentFinalized
    // function that gets called after all the bindings we rely on has been evaluated.
    // When receiving this call, we load the delegate items (and build the table).
    Q_Q(QQuickTableView);
    QQmlEnginePrivate *engPriv = QQmlEnginePrivate::get(qmlEngine(q));
    static int finalizedIdx = -1;
    if (finalizedIdx < 0)
        finalizedIdx = q->metaObject()->indexOfSlot("_q_componentFinalized()");
    engPriv->registerFinalizeCallback(q, finalizedIdx);
}

void QQuickTableView::componentComplete()
{
    QQuickFlickable::componentComplete();
    d_func()->registerCallbackWhenBindingsAreEvaluated();
}

#include "moc_qquicktableview_p.cpp"

QT_END_NAMESPACE
