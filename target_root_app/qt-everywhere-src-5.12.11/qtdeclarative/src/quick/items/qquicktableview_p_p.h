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

#ifndef QQUICKTABLEVIEW_P_P_H
#define QQUICKTABLEVIEW_P_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qquicktableview_p.h"

#include <QtCore/qtimer.h>
#include <QtQml/private/qqmltableinstancemodel_p.h>
#include <QtQml/private/qqmlincubator_p.h>
#include <QtQml/private/qqmlchangeset_p.h>
#include <QtQml/qqmlinfo.h>

#include <QtQuick/private/qquickflickable_p_p.h>
#include <QtQuick/private/qquickitemviewfxitem_p_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcTableViewDelegateLifecycle)

static const qreal kDefaultRowHeight = 50;
static const qreal kDefaultColumnWidth = 50;

class FxTableItem;

class Q_QML_AUTOTEST_EXPORT QQuickTableViewPrivate : public QQuickFlickablePrivate
{
    Q_DECLARE_PUBLIC(QQuickTableView)

public:
    class TableEdgeLoadRequest
    {
        // Whenever we need to load new rows or columns in the
        // table, we fill out a TableEdgeLoadRequest.
        // TableEdgeLoadRequest is just a struct that keeps track
        // of which cells that needs to be loaded, and which cell
        // the table is currently loading. The loading itself is
        // done by QQuickTableView.

    public:
        void begin(const QPoint &cell, const QPointF &pos, QQmlIncubator::IncubationMode incubationMode)
        {
            Q_ASSERT(!active);
            active = true;
            tableEdge = Qt::Edge(0);
            tableCells = QLine(cell, cell);
            mode = incubationMode;
            cellCount = 1;
            currentIndex = 0;
            startPos = pos;
            qCDebug(lcTableViewDelegateLifecycle()) << "begin top-left:" << toString();
        }

        void begin(const QLine cellsToLoad, Qt::Edge edgeToLoad, QQmlIncubator::IncubationMode incubationMode)
        {
            Q_ASSERT(!active);
            active = true;
            tableEdge = edgeToLoad;
            tableCells = cellsToLoad;
            mode = incubationMode;
            cellCount = tableCells.x2() - tableCells.x1() + tableCells.y2() - tableCells.y1() + 1;
            currentIndex = 0;
            qCDebug(lcTableViewDelegateLifecycle()) << "begin:" << toString();
        }

        inline void markAsDone() { active = false; }
        inline bool isActive() { return active; }

        inline QPoint firstCell() { return tableCells.p1(); }
        inline QPoint lastCell() { return tableCells.p2(); }
        inline QPoint currentCell() { return cellAt(currentIndex); }
        inline QPoint previousCell() { return cellAt(currentIndex - 1); }

        inline bool atBeginning() { return currentIndex == 0; }
        inline bool hasCurrentCell() { return currentIndex < cellCount; }
        inline void moveToNextCell() { ++currentIndex; }

        inline Qt::Edge edge() { return tableEdge; }
        inline QQmlIncubator::IncubationMode incubationMode() { return mode; }

        inline QPointF startPosition() { return startPos; }

        QString toString()
        {
            QString str;
            QDebug dbg(&str);
            dbg.nospace() << "TableSectionLoadRequest(" << "edge:"
                << tableEdge << " cells:" << tableCells << " incubation:";

            switch (mode) {
            case QQmlIncubator::Asynchronous:
                dbg << "Asynchronous";
                break;
            case QQmlIncubator::AsynchronousIfNested:
                dbg << "AsynchronousIfNested";
                break;
            case QQmlIncubator::Synchronous:
                dbg << "Synchronous";
                break;
            }

            return str;
        }

    private:
        Qt::Edge tableEdge = Qt::Edge(0);
        QLine tableCells;
        int currentIndex = 0;
        int cellCount = 0;
        bool active = false;
        QQmlIncubator::IncubationMode mode = QQmlIncubator::AsynchronousIfNested;
        QPointF startPos;

        QPoint cellAt(int index)
        {
            int x = tableCells.p1().x() + (tableCells.dx() ? index : 0);
            int y = tableCells.p1().y() + (tableCells.dy() ? index : 0);
            return QPoint(x, y);
        }
    };

    enum class RebuildState {
        Begin = 0,
        LoadInitalTable,
        VerifyTable,
        LayoutTable,
        LoadAndUnloadAfterLayout,
        PreloadColumns,
        PreloadRows,
        MovePreloadedItemsToPool,
        Done
    };

    enum class RebuildOption {
        None = 0,
        ViewportOnly = 0x1,
        CalculateNewTopLeftRow = 0x2,
        CalculateNewTopLeftColumn = 0x4,
        All = 0x8,
    };
    Q_DECLARE_FLAGS(RebuildOptions, RebuildOption)

public:
    QQuickTableViewPrivate();
    ~QQuickTableViewPrivate() override;

    static inline QQuickTableViewPrivate *get(QQuickTableView *q) { return q->d_func(); }

    void updatePolish() override;
    void fixup(AxisData &data, qreal minExtent, qreal maxExtent) override;

public:
    QHash<int, FxTableItem *> loadedItems;

    // model, tableModel and modelVariant all point to the same model. modelVariant
    // is the model assigned by the user. And tableModel is the wrapper model we create
    // around it. But if the model is an instance model directly, we cannot wrap it, so
    // we need a pointer for that case as well.
    QQmlInstanceModel* model = nullptr;
    QPointer<QQmlTableInstanceModel> tableModel = nullptr;
    QVariant modelVariant;

    // When the applications assignes a new model or delegate to the view, we keep them
    // around until we're ready to take them into use (syncWithPendingChanges).
    QVariant assignedModel = QVariant(int(0));
    QQmlComponent *assignedDelegate = nullptr;

    // loadedTable describes the table cells that are currently loaded (from top left
    // row/column to bottom right row/column). loadedTableOuterRect describes the actual
    // pixels that those cells cover, and is matched agains the viewport to determine when
    // we need to fill up with more rows/columns. loadedTableInnerRect describes the pixels
    // that the loaded table covers if you remove one row/column on each side of the table, and
    // is used to determine rows/columns that are no longer visible and can be unloaded.
    QRect loadedTable;
    QRectF loadedTableOuterRect;
    QRectF loadedTableInnerRect;

    QRectF viewportRect = QRectF(0, 0, -1, -1);

    QSize tableSize;

    RebuildState rebuildState = RebuildState::Done;
    RebuildOptions rebuildOptions = RebuildOption::All;
    RebuildOptions scheduledRebuildOptions = RebuildOption::All;

    TableEdgeLoadRequest loadRequest;

    QPoint contentSizeBenchMarkPoint = QPoint(-1, -1);
    QSizeF cellSpacing = QSizeF(0, 0);

    QQmlTableInstanceModel::ReusableFlag reusableFlag = QQmlTableInstanceModel::Reusable;

    bool blockItemCreatedCallback = false;
    bool columnRowPositionsInvalid = false;
    bool layoutWarningIssued = false;
    bool polishing = false;
    bool rebuildScheduled = true;

    QJSValue rowHeightProvider;
    QJSValue columnWidthProvider;

    // TableView uses contentWidth/height to report the size of the table (this
    // will e.g make scrollbars written for Flickable work out of the box). This
    // value is continuously calculated, and will change/improve as more columns
    // are loaded into view. At the same time, we want to open up for the
    // possibility that the application can set the content width explicitly, in
    // case it knows what the exact width should be from the start. We therefore
    // override the contentWidth/height properties from QQuickFlickable, to be able
    // to implement this combined behavior. This also lets us lazy build the table
    // if the application needs to know the content size early on.
    QQmlNullableValue<qreal> explicitContentWidth;
    QQmlNullableValue<qreal> explicitContentHeight;

    QSizeF averageEdgeSize;

    const static QPoint kLeft;
    const static QPoint kRight;
    const static QPoint kUp;
    const static QPoint kDown;

#ifdef QT_DEBUG
    QString forcedIncubationMode = qEnvironmentVariable("QT_TABLEVIEW_INCUBATION_MODE");
#endif

public:
    QQuickTableViewAttached *getAttachedObject(const QObject *object) const;

    int modelIndexAtCell(const QPoint &cell) const;
    QPoint cellAtModelIndex(int modelIndex) const;

    qreal sizeHintForColumn(int column);
    qreal sizeHintForRow(int row);
    void calculateTableSize();

    qreal resolveColumnWidth(int column);
    qreal resolveRowHeight(int row);

    void relayoutTable();
    void relayoutTableItems();

    void layoutVerticalEdge(Qt::Edge tableEdge);
    void layoutHorizontalEdge(Qt::Edge tableEdge);
    void layoutTopLeftItem();
    void layoutTableEdgeFromLoadRequest();

    void updateContentWidth();
    void updateContentHeight();
    void updateAverageEdgeSize();

    void enforceTableAtOrigin();
    void syncLoadedTableRectFromLoadedTable();
    void syncLoadedTableFromLoadRequest();

    bool canLoadTableEdge(Qt::Edge tableEdge, const QRectF fillRect) const;
    bool canUnloadTableEdge(Qt::Edge tableEdge, const QRectF fillRect) const;
    Qt::Edge nextEdgeToLoad(const QRectF rect);
    Qt::Edge nextEdgeToUnload(const QRectF rect);

    qreal cellWidth(const QPoint &cell);
    qreal cellHeight(const QPoint &cell);

    FxTableItem *loadedTableItem(const QPoint &cell) const;
    FxTableItem *itemNextTo(const FxTableItem *fxTableItem, const QPoint &direction) const;
    FxTableItem *createFxTableItem(const QPoint &cell, QQmlIncubator::IncubationMode incubationMode);
    FxTableItem *loadFxTableItem(const QPoint &cell, QQmlIncubator::IncubationMode incubationMode);

    void releaseItem(FxTableItem *fxTableItem, QQmlTableInstanceModel::ReusableFlag reusableFlag);
    void releaseLoadedItems(QQmlTableInstanceModel::ReusableFlag reusableFlag);

    void unloadItem(const QPoint &cell);
    void unloadItems(const QLine &items);

    void loadInitialTopLeftItem(const QPoint &cell, const QPointF &pos);
    void loadEdge(Qt::Edge edge, QQmlIncubator::IncubationMode incubationMode);
    void unloadEdge(Qt::Edge edge);
    void loadAndUnloadVisibleEdges();
    void drainReusePoolAfterLoadRequest();
    void cancelLoadRequest();
    void processLoadRequest();

    void processRebuildTable();
    bool moveToNextRebuildState();
    void beginRebuildTable();
    void layoutAfterLoadingInitialTable();

    void scheduleRebuildTable(QQuickTableViewPrivate::RebuildOptions options);
    void invalidateColumnRowPositions();

    int resolveImportVersion();
    void createWrapperModel();

    void initItemCallback(int modelIndex, QObject *item);
    void itemCreatedCallback(int modelIndex, QObject *object);
    void itemPooledCallback(int modelIndex, QObject *object);
    void itemReusedCallback(int modelIndex, QObject *object);
    void modelUpdated(const QQmlChangeSet &changeSet, bool reset);

    inline void syncWithPendingChanges();
    inline void syncDelegate();
    inline void syncModel();
    inline void syncRebuildOptions();

    void connectToModel();
    void disconnectFromModel();

    void rowsMovedCallback(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    void columnsMovedCallback(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int column);
    void rowsInsertedCallback(const QModelIndex &parent, int begin, int end);
    void rowsRemovedCallback(const QModelIndex &parent, int begin, int end);
    void columnsInsertedCallback(const QModelIndex &parent, int begin, int end);
    void columnsRemovedCallback(const QModelIndex &parent, int begin, int end);
    void layoutChangedCallback(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
    void modelResetCallback();

    void _q_componentFinalized();
    void registerCallbackWhenBindingsAreEvaluated();

    inline QString tableLayoutToString() const;
    void dumpTable() const;
};

class FxTableItem : public QQuickItemViewFxItem
{
public:
    FxTableItem(QQuickItem *item, QQuickTableView *table, bool own)
        : QQuickItemViewFxItem(item, own, QQuickTableViewPrivate::get(table))
    {
    }

    qreal position() const override { return 0; }
    qreal endPosition() const override { return 0; }
    qreal size() const override { return 0; }
    qreal sectionSize() const override { return 0; }
    bool contains(qreal, qreal) const override { return false; }

    QPoint cell;
};

QT_END_NAMESPACE

#endif
