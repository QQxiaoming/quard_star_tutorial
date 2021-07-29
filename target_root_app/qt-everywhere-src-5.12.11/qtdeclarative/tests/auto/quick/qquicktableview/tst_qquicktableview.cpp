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
#include <QtQuick/private/qquicktableview_p.h>
#include <QtQuick/private/qquicktableview_p_p.h>
#include <QtQuick/private/qquickloader_p.h>

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlexpression.h>
#include <QtQml/qqmlincubator.h>
#include <QtQml/private/qqmlobjectmodel_p.h>
#include <QtQml/private/qqmllistmodel_p.h>

#include "testmodel.h"

#include "../../shared/util.h"
#include "../shared/viewtestutil.h"
#include "../shared/visualtestutil.h"

using namespace QQuickViewTestUtil;
using namespace QQuickVisualTestUtil;

static const char* kTableViewPropName = "tableView";
static const char* kDelegateObjectName = "tableViewDelegate";
static const char *kDelegatesCreatedCountProp = "delegatesCreatedCount";
static const char *kModelDataBindingProp = "modelDataBinding";

Q_DECLARE_METATYPE(QMarginsF);

#define DECLARE_TABLEVIEW_VARIABLES \
    auto tableView = view->rootObject()->property(kTableViewPropName).value<QQuickTableView *>(); \
    QVERIFY(tableView); \
    auto tableViewPrivate = QQuickTableViewPrivate::get(tableView); \
    Q_UNUSED(tableViewPrivate)

#define LOAD_TABLEVIEW(fileName) \
    view->setSource(testFileUrl(fileName)); \
    view->show(); \
    QVERIFY(QTest::qWaitForWindowActive(view)); \
    DECLARE_TABLEVIEW_VARIABLES

#define LOAD_TABLEVIEW_ASYNC(fileName) \
    view->setSource(testFileUrl("asyncloader.qml")); \
    view->show(); \
    QVERIFY(QTest::qWaitForWindowActive(view)); \
    auto loader = view->rootObject()->property("loader").value<QQuickLoader *>(); \
    loader->setSource(QUrl::fromLocalFile("data/" fileName)); \
    QTRY_VERIFY(loader->item()); \
    QCOMPARE(loader->status(), QQuickLoader::Status::Ready); \
    DECLARE_TABLEVIEW_VARIABLES

#define WAIT_UNTIL_POLISHED \
    QVERIFY(tableViewPrivate->polishScheduled); \
    QTRY_VERIFY(!tableViewPrivate->polishScheduled)

class tst_QQuickTableView : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_QQuickTableView();

    QQuickTableViewAttached *getAttachedObject(const QObject *object) const;
    QPoint getContextRowAndColumn(const QQuickItem *item) const;

private:
    QQuickView *view = nullptr;

private slots:
    void initTestCase() override;
    void cleanupTestCase();

    void setAndGetModel_data();
    void setAndGetModel();
    void emptyModel_data();
    void emptyModel();
    void checkPreload_data();
    void checkPreload();
    void checkZeroSizedDelegate();
    void checkImplicitSizeDelegate();
    void checkColumnWidthWithoutProvider();
    void checkDelegateWithAnchors();
    void checkColumnWidthProvider();
    void checkColumnWidthProviderInvalidReturnValues();
    void checkColumnWidthProviderNotCallable();
    void checkRowHeightWithoutProvider();
    void checkRowHeightProvider();
    void checkRowHeightProviderInvalidReturnValues();
    void checkRowHeightProviderNotCallable();
    void checkForceLayoutFunction();
    void checkContentWidthAndHeight();
    void checkPageFlicking();
    void checkExplicitContentWidthAndHeight();
    void checkContentXY();
    void noDelegate();
    void changeDelegateDuringUpdate();
    void changeModelDuringUpdate();
    void countDelegateItems_data();
    void countDelegateItems();
    void checkLayoutOfEqualSizedDelegateItems_data();
    void checkLayoutOfEqualSizedDelegateItems();
    void checkFocusRemoved_data();
    void checkFocusRemoved();
    void fillTableViewButNothingMore_data();
    void fillTableViewButNothingMore();
    void checkInitialAttachedProperties_data();
    void checkInitialAttachedProperties();
    void checkSpacingValues();
    void checkDelegateParent();
    void flick_data();
    void flick();
    void flickOvershoot_data();
    void flickOvershoot();
    void checkRowColumnCount();
    void modelSignals();
    void checkModelSignalsUpdateLayout();
    void dataChangedSignal();
    void checkThatPoolIsDrainedWhenReuseIsFalse();
    void checkIfDelegatesAreReused_data();
    void checkIfDelegatesAreReused();
    void checkIfDelegatesAreReusedAsymmetricTableSize();
    void checkContextProperties_data();
    void checkContextProperties();
    void checkContextPropertiesQQmlListProperyModel_data();
    void checkContextPropertiesQQmlListProperyModel();
    void checkRowAndColumnChangedButNotIndex();
    void checkChangingModelFromDelegate();
    void checkRebuildViewportOnly();
    void useDelegateChooserWithoutDefault();
    void checkTableviewInsideAsyncLoader();
    void checkThatRevisionedPropertiesCannotBeUsedInOldImports();
};

tst_QQuickTableView::tst_QQuickTableView()
{
}

void tst_QQuickTableView::initTestCase()
{
    QQmlDataTest::initTestCase();
    qmlRegisterType<TestModel>("TestModel", 0, 1, "TestModel");
    view = createView();
}

void tst_QQuickTableView::cleanupTestCase()
{
    delete view;
}

QQuickTableViewAttached *tst_QQuickTableView::getAttachedObject(const QObject *object) const
{
    QObject *attachedObject = qmlAttachedPropertiesObject<QQuickTableView>(object);
    return static_cast<QQuickTableViewAttached *>(attachedObject);
}

QPoint tst_QQuickTableView::getContextRowAndColumn(const QQuickItem *item) const
{
    const auto context = qmlContext(item);
    const int row = context->contextProperty("row").toInt();
    const int column = context->contextProperty("column").toInt();
    return QPoint(column, row);
}

void tst_QQuickTableView::setAndGetModel_data()
{
    QTest::addColumn<QVariant>("model");

    QTest::newRow("QAIM 1x1") << TestModelAsVariant(1, 1);
    QTest::newRow("Number model 1") << QVariant::fromValue(1);
    QTest::newRow("QStringList 1") << QVariant::fromValue(QStringList() << "one");
}

void tst_QQuickTableView::setAndGetModel()
{
    // Test that we can set and get different kind of models
    QFETCH(QVariant, model);
    LOAD_TABLEVIEW("plaintableview.qml");

    tableView->setModel(model);
    QCOMPARE(model, tableView->model());
}

void tst_QQuickTableView::emptyModel_data()
{
    QTest::addColumn<QVariant>("model");

    QTest::newRow("QAIM") << TestModelAsVariant(0, 0);
    QTest::newRow("Number model") << QVariant::fromValue(0);
    QTest::newRow("QStringList") << QVariant::fromValue(QStringList());
}

void tst_QQuickTableView::emptyModel()
{
    // Check that if we assign an empty model to
    // TableView, no delegate items will be created.
    QFETCH(QVariant, model);
    LOAD_TABLEVIEW("plaintableview.qml");

    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableViewPrivate->loadedItems.count(), 0);
}

void tst_QQuickTableView::checkPreload_data()
{
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("reuse") << true;
    QTest::newRow("not reuse") << false;
}

void tst_QQuickTableView::checkPreload()
{
    // Check that the reuse pool is filled up with one extra row and
    // column (pluss corner) after rebuilding the table, but only if we reuse items.
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("plaintableview.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    tableView->setReuseItems(reuseItems);

    WAIT_UNTIL_POLISHED;

    if (reuseItems) {
        QSize visibleTableSize = tableViewPrivate->loadedTable.size();
        int expectedPoolSize = visibleTableSize.height() + visibleTableSize.width() + 1;
        QCOMPARE(tableViewPrivate->tableModel->poolSize(), expectedPoolSize);
    } else {
        QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);
    }
}

void tst_QQuickTableView::checkZeroSizedDelegate()
{
    // Check that if we assign a delegate with empty width and height, we
    // fall back to use kDefaultColumnWidth and kDefaultRowHeight as
    // column/row sizes.
    LOAD_TABLEVIEW("plaintableview.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    view->rootObject()->setProperty("delegateWidth", 0);
    view->rootObject()->setProperty("delegateHeight", 0);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*implicit"));

    WAIT_UNTIL_POLISHED;

    auto items = tableViewPrivate->loadedItems;
    QVERIFY(!items.isEmpty());

    for (auto fxItem : tableViewPrivate->loadedItems) {
        auto item = fxItem->item;
        QCOMPARE(item->width(), kDefaultColumnWidth);
        QCOMPARE(item->height(), kDefaultRowHeight);
    }
}

void tst_QQuickTableView::checkImplicitSizeDelegate()
{
    // Check that we can set the size of delegate items using
    // implicit width/height, instead of forcing the user to
    // create an attached object by using implicitWidth/Height.
    LOAD_TABLEVIEW("tableviewimplicitsize.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    auto items = tableViewPrivate->loadedItems;
    QVERIFY(!items.isEmpty());

    for (auto fxItem : tableViewPrivate->loadedItems) {
        auto item = fxItem->item;
        QCOMPARE(item->width(), 90);
        QCOMPARE(item->height(), 60);
    }
}

void tst_QQuickTableView::checkColumnWidthWithoutProvider()
{
    // Checks that a function isn't assigned to the columnWidthProvider property
    // and that the column width is then equal to sizeHintForColumn.
    LOAD_TABLEVIEW("alternatingrowheightcolumnwidth.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);
    QVERIFY(tableView->columnWidthProvider().isUndefined());

    WAIT_UNTIL_POLISHED;

    QRect table = tableViewPrivate->loadedTable;
    for (int column = table.left(); column <= table.right(); ++column) {
        const qreal expectedColumnWidth = tableViewPrivate->sizeHintForColumn(column);
        for (int row = table.top(); row <= table.bottom(); ++row) {
            const auto item = tableViewPrivate->loadedTableItem(QPoint(column, row))->item;
            QCOMPARE(item->width(), expectedColumnWidth);
        }
    }
}

void tst_QQuickTableView::checkDelegateWithAnchors()
{
    // Checks that we issue a warning if the delegate has anchors
    LOAD_TABLEVIEW("delegatewithanchors.qml");

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*anchors"));

    auto model = TestModelAsVariant(1, 1);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;
}

void tst_QQuickTableView::checkColumnWidthProvider()
{
    // Check that you can assign a function to the columnWidthProvider property, and
    // that it's used to control (and override) the width of the columns.
    LOAD_TABLEVIEW("userowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);
    QVERIFY(tableView->columnWidthProvider().isCallable());

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems) {
        // expectedWidth mirrors the expected return value of the assigned javascript function
        qreal expectedWidth = fxItem->cell.x() + 10;
        QCOMPARE(fxItem->item->width(), expectedWidth);
    }
}

void tst_QQuickTableView::checkColumnWidthProviderInvalidReturnValues()
{
    // Check that we fall back to use default columns widths, if you
    // assign a function to columnWidthProvider that returns invalid values.
    LOAD_TABLEVIEW("usefaultyrowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Provider.*valid"));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), kDefaultColumnWidth);
}

void tst_QQuickTableView::checkColumnWidthProviderNotCallable()
{
    // Check that we fall back to use default columns widths, if you
    // assign something to columnWidthProvider that is not callable.
    LOAD_TABLEVIEW("usefaultyrowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);
    tableView->setRowHeightProvider(QJSValue());
    tableView->setColumnWidthProvider(QJSValue(10));

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".Provider.*function"));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), kDefaultColumnWidth);
}

void tst_QQuickTableView::checkRowHeightWithoutProvider()
{
    // Checks that a function isn't assigned to the rowHeightProvider property
    // and that the row height is then equal to sizeHintForRow.
    LOAD_TABLEVIEW("alternatingrowheightcolumnwidth.qml");

    auto model = TestModelAsVariant(10, 10);
    QVERIFY(tableView->rowHeightProvider().isUndefined());

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    QRect table = tableViewPrivate->loadedTable;
    for (int row = table.top(); row <= table.bottom(); ++row) {
        const qreal expectedRowHeight = tableViewPrivate->sizeHintForRow(row);
        for (int column = table.left(); column <= table.right(); ++column) {
            const auto item = tableViewPrivate->loadedTableItem(QPoint(column, row))->item;
            QCOMPARE(item->height(), expectedRowHeight);
        }
    }
}

void tst_QQuickTableView::checkRowHeightProvider()
{
    // Check that you can assign a function to the columnWidthProvider property, and
    // that it's used to control (and override) the width of the columns.
    LOAD_TABLEVIEW("userowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);
    QVERIFY(tableView->rowHeightProvider().isCallable());

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems) {
        // expectedWidth mirrors the expected return value of the assigned javascript function
        qreal expectedHeight = fxItem->cell.y() + 10;
        QCOMPARE(fxItem->item->height(), expectedHeight);
    }
}

void tst_QQuickTableView::checkRowHeightProviderInvalidReturnValues()
{
    // Check that we fall back to use default row heights, if you
    // assign a function to rowHeightProvider that returns invalid values.
    LOAD_TABLEVIEW("usefaultyrowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Provider.*valid"));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->height(), kDefaultRowHeight);
}

void tst_QQuickTableView::checkRowHeightProviderNotCallable()
{
    // Check that we fall back to use default row heights, if you
    // assign something to rowHeightProvider that is not callable.
    LOAD_TABLEVIEW("usefaultyrowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    tableView->setColumnWidthProvider(QJSValue());
    tableView->setRowHeightProvider(QJSValue(10));

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Provider.*function"));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->height(), kDefaultRowHeight);
}

void tst_QQuickTableView::checkForceLayoutFunction()
{
    // When we set the 'columnWidths' property in the test file, the
    // columnWidthProvider should return other values than it did during
    // start-up. Check that this takes effect after a call to the 'forceLayout()' function.
    LOAD_TABLEVIEW("forcelayout.qml");

    const char *propertyName = "columnWidths";
    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Check that the initial column widths are as specified in the QML file
    const qreal initialColumnWidth = view->rootObject()->property(propertyName).toReal();
    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), initialColumnWidth);

    // Change the return value from the columnWidthProvider to something else
    const qreal newColumnWidth = 100;
    view->rootObject()->setProperty(propertyName, newColumnWidth);
    tableView->forceLayout();
    // We don't have to polish; The re-layout happens immediately

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), newColumnWidth);
}

void tst_QQuickTableView::checkContentWidthAndHeight()
{
    // Check that contentWidth/Height reports the correct size of the the
    // table, based on knowledge of the rows and columns that has been loaded.
    LOAD_TABLEVIEW("contentwidthheight.qml");

    // Vertical and horizontal properties should be mirrored, so we only have
    // to do the calculations once, and use them for both axis, below.
    QCOMPARE(tableView->width(), tableView->height());
    QCOMPARE(tableView->rowSpacing(), tableView->columnSpacing());

    const int tableSize = 100;
    const int cellSizeSmall = 100;
    const int cellSizeLarge = 200;
    const int spacing = 1;
    const int smallCellCount = 20;
    const int largeCellCount = tableSize - smallCellCount;
    const qreal accumulatedSpacing = ((tableSize - 1) * spacing);
    auto model = TestModelAsVariant(tableSize, tableSize);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    const qreal expectedSizeInit = (tableSize * cellSizeSmall) + ((tableSize - 1) * spacing);
    QCOMPARE(tableView->contentWidth(), expectedSizeInit);
    QCOMPARE(tableView->contentHeight(), expectedSizeInit);

    // Flick in 5 more rows and columns, but not so far that we start loading in
    // the ones that are bigger. Loading in more rows and columns of the same
    // size as the initial ones should not change the first prediction.
    qreal flickTo = ((cellSizeSmall + spacing) * 5);
    tableView->setContentX(flickTo);
    tableView->setContentY(flickTo);

    QCOMPARE(tableView->contentWidth(), expectedSizeInit);
    QCOMPARE(tableView->contentHeight(), expectedSizeInit);

    // Flick to row and column 20 (smallCellCount), since there the row and
    // column sizes increases with 100. Check that TableView then adjusts
    // contentWidth and contentHeight accordingly.
    flickTo = ((cellSizeSmall + spacing) * smallCellCount) - spacing;
    tableView->setContentX(flickTo);
    tableView->setContentY(flickTo);

    // Since we move the viewport more than a page, tableview
    // will jump to the new position and do a rebuild.
    QVERIFY(tableViewPrivate->polishScheduled);
    QVERIFY(tableViewPrivate->rebuildScheduled);
    WAIT_UNTIL_POLISHED;

    const int largeSizeCellCountInView = qCeil(tableView->width() / cellSizeLarge);
    const int columnCount = smallCellCount + largeSizeCellCountInView;
    QCOMPARE(tableViewPrivate->loadedTable.left(), smallCellCount);
    QCOMPARE(tableViewPrivate->loadedTable.right(), columnCount - 1);

    const qreal firstHalfLength = smallCellCount * cellSizeSmall;
    const qreal secondHalfOneScreenLength = largeSizeCellCountInView * cellSizeLarge;
    const qreal lengthAfterFlick = firstHalfLength + secondHalfOneScreenLength;

    const qreal averageCellSize = lengthAfterFlick / columnCount;
    const qreal expectedSizeHalf = (tableSize * averageCellSize) + accumulatedSpacing;

    QCOMPARE(tableView->contentWidth(), expectedSizeHalf);
    QCOMPARE(tableView->contentHeight(), expectedSizeHalf);

    // Flick to the end (row/column 100, and overshoot a bit), and
    // check that we then end up with the exact content width/height.
    const qreal secondHalfLength = largeCellCount * cellSizeLarge;
    const qreal expectedFullSize = (firstHalfLength + secondHalfLength) + accumulatedSpacing;

    // If we flick more than one page at a time, tableview will jump to the new
    // position and rebuild the table without loading the edges in-between. Which
    // row and column that ends up as new top-left is then based on a prediction, and
    // therefore unreliable. To avoid this to happen (which will also affect the
    // reported size of the table), we flick to the end position in smaller chuncks.
    QVERIFY(!tableViewPrivate->polishScheduled);
    QVERIFY(!tableViewPrivate->rebuildScheduled);
    int pages = qCeil((expectedFullSize - tableView->contentX()) / tableView->width());
    for (int i = 0; i < pages; i++) {
        tableView->setContentX(tableView->contentX() + tableView->width() - 1);
        tableView->setContentY(tableView->contentY() + tableView->height() - 1);
        QVERIFY(!tableViewPrivate->rebuildScheduled);
    }

    QCOMPARE(tableView->contentWidth(), expectedFullSize);
    QCOMPARE(tableView->contentHeight(), expectedFullSize);

    // Flick back to start. Since we know the actual table
    // size, contentWidth/Height shouldn't change.
    tableView->setContentX(0);
    tableView->setContentY(0);

    QCOMPARE(tableView->contentWidth(), expectedFullSize);
    QCOMPARE(tableView->contentHeight(), expectedFullSize);
}

void tst_QQuickTableView::checkPageFlicking()
{
    // Check that we rebuild the table instead of refilling edges, if the viewport moves
    // more than a page (the size of TableView).
    LOAD_TABLEVIEW("plaintableview.qml");

    const int cellWidth = 100;
    const int cellHeight = 50;
    auto model = TestModelAsVariant(10000, 10000);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Sanity check startup table
    QRect tableRect = tableViewPrivate->loadedTable;
    QCOMPARE(tableRect.x(), 0);
    QCOMPARE(tableRect.y(), 0);
    QCOMPARE(tableRect.width(), tableView->width() / cellWidth);
    QCOMPARE(tableRect.height(), tableView->height() / cellHeight);

    // Since all cells have the same size, the average row/column
    // size found by TableView should be exactly equal to this.
    QCOMPARE(tableViewPrivate->averageEdgeSize.width(), cellWidth);
    QCOMPARE(tableViewPrivate->averageEdgeSize.height(), cellHeight);

    QVERIFY(!tableViewPrivate->rebuildScheduled);
    QCOMPARE(tableViewPrivate->scheduledRebuildOptions, QQuickTableViewPrivate::RebuildOption::None);

    // Flick 5000 columns to the right, and check that this triggers a
    // rebuild, and that we end up at the expected top-left.
    const int flickToColumn = 5000;
    const qreal columnSpacing = tableView->columnSpacing();
    const qreal flickToColumnInPixels = ((cellWidth + columnSpacing) * flickToColumn) - columnSpacing;
    tableView->setContentX(flickToColumnInPixels);

    QVERIFY(tableViewPrivate->rebuildScheduled);
    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::ViewportOnly);
    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftColumn);
    QVERIFY(!(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftRow));

    WAIT_UNTIL_POLISHED;

    tableRect = tableViewPrivate->loadedTable;
    QCOMPARE(tableRect.x(), flickToColumn);
    QCOMPARE(tableRect.y(), 0);
    QCOMPARE(tableRect.width(), tableView->width() / cellWidth);
    QCOMPARE(tableRect.height(), tableView->height() / cellHeight);

    // Flick 5000 rows down as well. Since flicking down should only calculate a new row (but
    // keep the current column), we deliberatly change the average width to check that it's
    // actually ignored by the rebuild, and that the column stays the same.
    tableViewPrivate->averageEdgeSize.rwidth() /= 2;

    const int flickToRow = 5000;
    const qreal rowSpacing = tableView->rowSpacing();
    const qreal flickToRowInPixels = ((cellHeight + rowSpacing) * flickToRow) - rowSpacing;
    tableView->setContentY(flickToRowInPixels);

    QVERIFY(tableViewPrivate->rebuildScheduled);
    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::ViewportOnly);
    QVERIFY(!(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftColumn));
    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftRow);

    WAIT_UNTIL_POLISHED;

    tableRect = tableViewPrivate->loadedTable;
    QCOMPARE(tableRect.x(), flickToRow);
    QCOMPARE(tableRect.y(), flickToColumn);
    QCOMPARE(tableRect.width(), tableView->width() / cellWidth);
    QCOMPARE(tableRect.height(), tableView->height() / cellHeight);
}

void tst_QQuickTableView::checkExplicitContentWidthAndHeight()
{
    // Check that you can set a custom contentWidth/Height, and that
    // TableView doesn't override it while loading more rows and columns.
    LOAD_TABLEVIEW("contentwidthheight.qml");

    tableView->setContentWidth(1000);
    tableView->setContentHeight(1000);
    QCOMPARE(tableView->contentWidth(), 1000);
    QCOMPARE(tableView->contentHeight(), 1000);

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;

    // Flick somewhere. It should not affect the contentWidth/Height
    tableView->setContentX(500);
    tableView->setContentY(500);
    QCOMPARE(tableView->contentWidth(), 1000);
    QCOMPARE(tableView->contentHeight(), 1000);
}

void tst_QQuickTableView::checkContentXY()
{
    // Check that you can bind contentX and contentY to
    // e.g show the center of the table at start-up
    LOAD_TABLEVIEW("setcontentpos.qml");

    auto model = TestModelAsVariant(10, 10);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->width(), 400);
    QCOMPARE(tableView->height(), 400);
    QCOMPARE(tableView->contentWidth(), 1000);
    QCOMPARE(tableView->contentHeight(), 1000);

    // Check that the content item is positioned according
    // to the binding in the QML file (which will set the
    // viewport to be at the center of the table).
    const qreal expectedXY = (tableView->contentWidth() - tableView->width()) / 2;
    QCOMPARE(tableView->contentX(), expectedXY);
    QCOMPARE(tableView->contentY(), expectedXY);

    // Check that we end up at the correct top-left cell:
    const qreal delegateWidth = tableViewPrivate->loadedItems.values().first()->item->width();
    const int expectedCellXY = qCeil(expectedXY / delegateWidth);
    QCOMPARE(tableViewPrivate->loadedTable.left(), expectedCellXY);
    QCOMPARE(tableViewPrivate->loadedTable.top(), expectedCellXY);
}

void tst_QQuickTableView::noDelegate()
{
    // Check that you can skip setting a delegate without
    // it causing any problems (like crashing or asserting).
    // And then set a delegate, and do a quick check that the
    // view gets populated as expected.
    LOAD_TABLEVIEW("plaintableview.qml");

    const int rows = 5;
    const int columns = 5;
    auto model = TestModelAsVariant(columns, rows);
    tableView->setModel(model);

    // Start with no delegate, and check
    // that we end up with no items in the table.
    tableView->setDelegate(nullptr);

    WAIT_UNTIL_POLISHED;

    auto items = tableViewPrivate->loadedItems;
    QVERIFY(items.isEmpty());

    // Set a delegate, and check that we end
    // up with the expected number of items.
    auto delegate = view->rootObject()->property("delegate").value<QQmlComponent *>();
    QVERIFY(delegate);
    tableView->setDelegate(delegate);

    WAIT_UNTIL_POLISHED;

    items = tableViewPrivate->loadedItems;
    QCOMPARE(items.count(), rows * columns);

    // And then unset the delegate again, and check
    // that we end up with no items.
    tableView->setDelegate(nullptr);

    WAIT_UNTIL_POLISHED;

    items = tableViewPrivate->loadedItems;
    QVERIFY(items.isEmpty());
}

void tst_QQuickTableView::changeDelegateDuringUpdate()
{
    // Check that you can change the delegate (set it to null)
    // while the TableView is busy loading the table.
    LOAD_TABLEVIEW("changemodelordelegateduringupdate.qml");

    auto model = TestModelAsVariant(1, 1);
    tableView->setModel(model);
    view->rootObject()->setProperty("changeDelegate", true);

    WAIT_UNTIL_POLISHED;

    // We should no longer have a delegate, and no
    // items should therefore be loaded.
    QCOMPARE(tableView->delegate(), nullptr);
    QCOMPARE(tableViewPrivate->loadedItems.size(), 0);

    // Even if the delegate is missing, we still report
    // the correct size of the model
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 1);
};

void tst_QQuickTableView::changeModelDuringUpdate()
{
    // Check that you can change the model (set it to null)
    // while the TableView is buzy loading the table.
    LOAD_TABLEVIEW("changemodelordelegateduringupdate.qml");

    auto model = TestModelAsVariant(1, 1);
    tableView->setModel(model);
    view->rootObject()->setProperty("changeModel", true);

    WAIT_UNTIL_POLISHED;

    // We should no longer have a model, and the no
    // items should therefore be loaded.
    QVERIFY(tableView->model().isNull());
    QCOMPARE(tableViewPrivate->loadedItems.size(), 0);

    // The empty model has no rows or columns
    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 0);
};

void tst_QQuickTableView::countDelegateItems_data()
{
    QTest::addColumn<QVariant>("model");
    QTest::addColumn<int>("count");

    QTest::newRow("QAIM 1x1") << TestModelAsVariant(1, 1) << 1;
    QTest::newRow("QAIM 2x1") << TestModelAsVariant(2, 1) << 2;
    QTest::newRow("QAIM 1x2") << TestModelAsVariant(1, 2) << 2;
    QTest::newRow("QAIM 2x2") << TestModelAsVariant(2, 2) << 4;
    QTest::newRow("QAIM 4x4") << TestModelAsVariant(4, 4) << 16;

    QTest::newRow("Number model 1") << QVariant::fromValue(1) << 1;
    QTest::newRow("Number model 4") << QVariant::fromValue(4) << 4;

    QTest::newRow("QStringList 1") << QVariant::fromValue(QStringList() << "one") << 1;
    QTest::newRow("QStringList 4") << QVariant::fromValue(QStringList() << "one" << "two" << "three" << "four") << 4;
}

void tst_QQuickTableView::countDelegateItems()
{
    // Assign different models of various sizes, and check that the number of
    // delegate items in the view matches the size of the model. Note that for
    // this test to be valid, all items must be within the visible area of the view.
    QFETCH(QVariant, model);
    QFETCH(int, count);
    LOAD_TABLEVIEW("plaintableview.qml");

    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;

    // Check that tableview internals contain the expected number of items
    auto const items = tableViewPrivate->loadedItems;
    QCOMPARE(items.count(), count);

    // Check that this also matches the items found in the view
    auto foundItems = findItems<QQuickItem>(tableView, kDelegateObjectName);
    QCOMPARE(foundItems.count(), count);
}

void tst_QQuickTableView::checkLayoutOfEqualSizedDelegateItems_data()
{
    QTest::addColumn<QVariant>("model");
    QTest::addColumn<QSize>("tableSize");
    QTest::addColumn<QSizeF>("spacing");
    QTest::addColumn<QMarginsF>("margins");

    // Check spacing together with different table setups
    QTest::newRow("QAIM 1x1 1,1") << TestModelAsVariant(1, 1) << QSize(1, 1) << QSizeF(1, 1) << QMarginsF(0, 0, 0, 0);
    QTest::newRow("QAIM 5x5 0,0") << TestModelAsVariant(5, 5) << QSize(5, 5) << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0);
    QTest::newRow("QAIM 5x5 1,0") << TestModelAsVariant(5, 5) << QSize(5, 5) << QSizeF(1, 0) << QMarginsF(0, 0, 0, 0);
    QTest::newRow("QAIM 5x5 0,1") << TestModelAsVariant(5, 5) << QSize(5, 5) << QSizeF(0, 1) << QMarginsF(0, 0, 0, 0);

    // Check spacing together with margins
    QTest::newRow("QAIM 1x1 1,1 5555") << TestModelAsVariant(1, 1) << QSize(1, 1) << QSizeF(1, 1) << QMarginsF(5, 5, 5, 5);
    QTest::newRow("QAIM 4x4 0,0 3333") << TestModelAsVariant(4, 4) << QSize(4, 4) << QSizeF(0, 0) << QMarginsF(3, 3, 3, 3);
    QTest::newRow("QAIM 4x4 2,2 1234") << TestModelAsVariant(4, 4) << QSize(4, 4) << QSizeF(2, 2) << QMarginsF(1, 2, 3, 4);

    // Check "list" models
    QTest::newRow("NumberModel 1x4, 0000") << QVariant::fromValue(4) << QSize(1, 4) << QSizeF(1, 1) << QMarginsF(0, 0, 0, 0);
    QTest::newRow("QStringList 1x4, 0,0 1111") << QVariant::fromValue(QStringList() << "one" << "two" << "three" << "four")
                                               << QSize(1, 4) << QSizeF(0, 0) << QMarginsF(1, 1, 1, 1);
}

void tst_QQuickTableView::checkLayoutOfEqualSizedDelegateItems()
{
    // Check that the geometry of the delegate items are correct
    QFETCH(QVariant, model);
    QFETCH(QSize, tableSize);
    QFETCH(QSizeF, spacing);
    QFETCH(QMarginsF, margins);
    LOAD_TABLEVIEW("plaintableview.qml");

    const qreal expectedItemWidth = 100;
    const qreal expectedItemHeight = 50;
    const int expectedItemCount = tableSize.width() * tableSize.height();

    tableView->setModel(model);
    tableView->setRowSpacing(spacing.height());
    tableView->setColumnSpacing(spacing.width());

    // Setting margins on Flickable should not affect the layout of the
    // delegate items, since the margins is "transparent" to the TableView.
    tableView->setLeftMargin(margins.left());
    tableView->setTopMargin(margins.top());
    tableView->setRightMargin(margins.right());
    tableView->setBottomMargin(margins.bottom());

    WAIT_UNTIL_POLISHED;

    auto const items = tableViewPrivate->loadedItems;
    QVERIFY(!items.isEmpty());

    for (int i = 0; i < expectedItemCount; ++i) {
        const QQuickItem *item = items[i]->item;
        QVERIFY(item);
        QCOMPARE(item->parentItem(), tableView->contentItem());

        const QPoint cell = getContextRowAndColumn(item);
        qreal expectedX = cell.x() * (expectedItemWidth + spacing.width());
        qreal expectedY = cell.y() * (expectedItemHeight + spacing.height());
        QCOMPARE(item->x(), expectedX);
        QCOMPARE(item->y(), expectedY);
        QCOMPARE(item->z(), 1);
        QCOMPARE(item->width(), expectedItemWidth);
        QCOMPARE(item->height(), expectedItemHeight);
    }
}

void tst_QQuickTableView::checkFocusRemoved_data()
{
    QTest::addColumn<QString>("focusedItemProp");

    QTest::newRow("delegate root") << QStringLiteral("delegateRoot");
    QTest::newRow("delegate child") << QStringLiteral("delegateChild");
}

void tst_QQuickTableView::checkFocusRemoved()
{
    // Check that we clear the focus of a delegate item when
    // a child of the delegate item has focus, and the cell is
    // flicked out of view.
    QFETCH(QString, focusedItemProp);
    LOAD_TABLEVIEW("tableviewfocus.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    auto const item = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item;
    auto const focusedItem = qvariant_cast<QQuickItem *>(item->property(focusedItemProp.toUtf8().data()));
    QVERIFY(focusedItem);
    QCOMPARE(tableView->hasActiveFocus(), false);
    QCOMPARE(focusedItem->hasActiveFocus(), false);

    focusedItem->forceActiveFocus();
    QCOMPARE(tableView->hasActiveFocus(), true);
    QCOMPARE(focusedItem->hasActiveFocus(), true);

    // Flick the focused cell out, and check that none of the
    // items in the table has focus (which means that the reused
    // item lost focus when it was flicked out). But the tableview
    // itself will maintain active focus.
    tableView->setContentX(500);
    QCOMPARE(tableView->hasActiveFocus(), true);
    for (auto fxItem : tableViewPrivate->loadedItems) {
        auto const focusedItem2 = qvariant_cast<QQuickItem *>(fxItem->item->property(focusedItemProp.toUtf8().data()));
        QCOMPARE(focusedItem2->hasActiveFocus(), false);
    }
}

void tst_QQuickTableView::fillTableViewButNothingMore_data()
{
    QTest::addColumn<QSizeF>("spacing");

    QTest::newRow("0 0,0 0") << QSizeF(0, 0);
    QTest::newRow("0 10,10 0") << QSizeF(10, 10);
    QTest::newRow("100 10,10 0") << QSizeF(10, 10);
    QTest::newRow("0 0,0 100") << QSizeF(0, 0);
    QTest::newRow("0 10,10 100") << QSizeF(10, 10);
    QTest::newRow("100 10,10 100") << QSizeF(10, 10);
}

void tst_QQuickTableView::fillTableViewButNothingMore()
{
    // Check that we end up filling the whole visible part of
    // the tableview with cells, but nothing more.
    QFETCH(QSizeF, spacing);
    LOAD_TABLEVIEW("plaintableview.qml");

    const int rows = 100;
    const int columns = 100;
    auto model = TestModelAsVariant(rows, columns);

    tableView->setModel(model);
    tableView->setRowSpacing(spacing.height());
    tableView->setColumnSpacing(spacing.width());

    WAIT_UNTIL_POLISHED;

    auto const topLeftFxItem = tableViewPrivate->loadedTableItem(QPoint(0, 0));
    auto const topLeftItem = topLeftFxItem->item;

    auto const bottomRightFxItem = tableViewPrivate->loadedTableItem(tableViewPrivate->loadedTable.bottomRight());
    auto const bottomRightItem = bottomRightFxItem->item;
    const QPoint bottomRightCell = getContextRowAndColumn(bottomRightItem.data());

    // Check that the right-most item is overlapping the right edge of the view
    QVERIFY(bottomRightItem->x() < tableView->width());
    QVERIFY(bottomRightItem->x() + bottomRightItem->width() >= tableView->width() - spacing.width());

    // Check that the actual number of columns matches what we expect
    qreal cellWidth = bottomRightItem->width() + spacing.width();
    int expectedColumns = qCeil(tableView->width() / cellWidth);
    int actualColumns = bottomRightCell.x() + 1;
    QCOMPARE(actualColumns, expectedColumns);

    // Check that the bottom-most item is overlapping the bottom edge of the view
    QVERIFY(bottomRightItem->y() < tableView->height());
    QVERIFY(bottomRightItem->y() + bottomRightItem->height() >= tableView->height() - spacing.height());

    // Check that the actual number of rows matches what we expect
    qreal cellHeight = bottomRightItem->height() + spacing.height();
    int expectedRows = qCeil(tableView->height() / cellHeight);
    int actualRows = bottomRightCell.y() + 1;
    QCOMPARE(actualRows, expectedRows);
}

void tst_QQuickTableView::checkInitialAttachedProperties_data()
{
    QTest::addColumn<QVariant>("model");

    QTest::newRow("QAIM") << TestModelAsVariant(4, 4);
    QTest::newRow("Number model") << QVariant::fromValue(4);
    QTest::newRow("QStringList") << QVariant::fromValue(QStringList() << "0" << "1" << "2" << "3");
}

void tst_QQuickTableView::checkInitialAttachedProperties()
{
    // Check that the context and attached properties inside
    // the delegate items are what we expect at start-up.
    QFETCH(QVariant, model);
    LOAD_TABLEVIEW("plaintableview.qml");

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems) {
        const int index = fxItem->index;
        const auto item = fxItem->item;
        const auto context = qmlContext(item.data());
        const QPoint cell = tableViewPrivate->cellAtModelIndex(index);
        const int contextIndex = context->contextProperty("index").toInt();
        const QPoint contextCell = getContextRowAndColumn(item.data());
        const QString contextModelData = context->contextProperty("modelData").toString();

        QCOMPARE(contextCell.y(), cell.y());
        QCOMPARE(contextCell.x(), cell.x());
        QCOMPARE(contextIndex, index);
        QCOMPARE(contextModelData, QStringLiteral("%1").arg(cell.y()));
        QCOMPARE(getAttachedObject(item)->view(), tableView);
    }
}

void tst_QQuickTableView::checkSpacingValues()
{
    LOAD_TABLEVIEW("tableviewdefaultspacing.qml");

    int rowCount = 9;
    int columnCount = 9;
    int delegateWidth = 15;
    int delegateHeight = 10;
    auto model = TestModelAsVariant(rowCount, columnCount);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Default spacing : 0
    QCOMPARE(tableView->rowSpacing(), 0);
    QCOMPARE(tableView->columnSpacing(), 0);

    tableView->polish();
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->contentWidth(), columnCount * (delegateWidth + tableView->columnSpacing()) - tableView->columnSpacing());
    QCOMPARE(tableView->contentHeight(), rowCount * (delegateHeight + tableView->rowSpacing()) - tableView->rowSpacing());

    // Valid spacing assignment
    tableView->setRowSpacing(42);
    tableView->setColumnSpacing(12);
    QCOMPARE(tableView->rowSpacing(), 42);
    QCOMPARE(tableView->columnSpacing(), 12);

    tableView->polish();
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->contentWidth(), columnCount * (delegateWidth + tableView->columnSpacing()) - tableView->columnSpacing());
    QCOMPARE(tableView->contentHeight(), rowCount * (delegateHeight + tableView->rowSpacing()) - tableView->rowSpacing());

    // Invalid assignments (should ignore)
    tableView->setRowSpacing(-1);
    tableView->setColumnSpacing(-5);
    tableView->setRowSpacing(INFINITY);
    tableView->setColumnSpacing(INFINITY);
    tableView->setRowSpacing(NAN);
    tableView->setColumnSpacing(NAN);
    QCOMPARE(tableView->rowSpacing(), 42);
    QCOMPARE(tableView->columnSpacing(), 12);
}

void tst_QQuickTableView::checkDelegateParent()
{
    // Check that TableView sets the delegate parent before
    // bindings are evaluated, so that the app can bind to it.
    LOAD_TABLEVIEW("plaintableview.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    QVERIFY(view->rootObject()->property("delegateParentSetBeforeCompleted").toBool());
}

void tst_QQuickTableView::flick_data()
{
    QTest::addColumn<QSizeF>("spacing");
    QTest::addColumn<QMarginsF>("margins");
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("s:0 m:0 reuse") << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0) << true;
    QTest::newRow("s:5 m:0 reuse") << QSizeF(5, 5) << QMarginsF(0, 0, 0, 0) << true;
    QTest::newRow("s:0 m:20 reuse") << QSizeF(0, 0) << QMarginsF(20, 20, 20, 20) << true;
    QTest::newRow("s:5 m:20 reuse") << QSizeF(5, 5) << QMarginsF(20, 20, 20, 20) << true;
    QTest::newRow("s:0 m:0") << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0) << false;
    QTest::newRow("s:5 m:0") << QSizeF(5, 5) << QMarginsF(0, 0, 0, 0) << false;
    QTest::newRow("s:0 m:20") << QSizeF(0, 0) << QMarginsF(20, 20, 20, 20) << false;
    QTest::newRow("s:5 m:20") << QSizeF(5, 5) << QMarginsF(20, 20, 20, 20) << false;
}

void tst_QQuickTableView::flick()
{
    // Check that if we end up with the correct start and end column/row as we flick around
    // with different table configurations.
    QFETCH(QSizeF, spacing);
    QFETCH(QMarginsF, margins);
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("plaintableview.qml");

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const int visualColumnCount = 4;
    const int visualRowCount = 4;
    const qreal cellWidth = delegateWidth + spacing.width();
    const qreal cellHeight = delegateHeight + spacing.height();
    auto model = TestModelAsVariant(100, 100);

    tableView->setModel(model);
    tableView->setRowSpacing(spacing.height());
    tableView->setColumnSpacing(spacing.width());
    tableView->setLeftMargin(margins.left());
    tableView->setTopMargin(margins.top());
    tableView->setRightMargin(margins.right());
    tableView->setBottomMargin(margins.bottom());
    tableView->setReuseItems(reuseItems);
    tableView->setWidth(margins.left() + (visualColumnCount * cellWidth) - spacing.width());
    tableView->setHeight(margins.top() + (visualRowCount * cellHeight) - spacing.height());

    WAIT_UNTIL_POLISHED;

    // Check the "simple" case if the cells never lands egde-to-edge with the viewport. For
    // that case we only accept that visible row/columns are loaded.
    qreal flickValues[] = {0.5, 1.5, 4.5, 20.5, 10.5, 3.5, 1.5, 0.5};

    for (qreal cellsToFlick : flickValues) {
        // Flick to the beginning of the cell
        tableView->setContentX(cellsToFlick * cellWidth);
        tableView->setContentY(cellsToFlick * cellHeight);
        tableView->polish();

        WAIT_UNTIL_POLISHED;

        const QRect loadedTable = tableViewPrivate->loadedTable;

        const int expectedTableLeft = cellsToFlick - int((margins.left() + spacing.width()) / cellWidth);
        const int expectedTableTop = cellsToFlick - int((margins.top() + spacing.height()) / cellHeight);

        QCOMPARE(loadedTable.left(), expectedTableLeft);
        QCOMPARE(loadedTable.right(), expectedTableLeft + visualColumnCount);
        QCOMPARE(loadedTable.top(), expectedTableTop);
        QCOMPARE(loadedTable.bottom(), expectedTableTop + visualRowCount);
    }
}

void tst_QQuickTableView::flickOvershoot_data()
{
    QTest::addColumn<QSizeF>("spacing");
    QTest::addColumn<QMarginsF>("margins");
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("s:0 m:0 reuse") << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0) << true;
    QTest::newRow("s:5 m:0 reuse") << QSizeF(5, 5) << QMarginsF(0, 0, 0, 0) << true;
    QTest::newRow("s:0 m:20 reuse") << QSizeF(0, 0) << QMarginsF(20, 20, 20, 20) << true;
    QTest::newRow("s:5 m:20 reuse") << QSizeF(5, 5) << QMarginsF(20, 20, 20, 20) << true;
    QTest::newRow("s:0 m:0") << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0) << false;
    QTest::newRow("s:5 m:0") << QSizeF(5, 5) << QMarginsF(0, 0, 0, 0) << false;
    QTest::newRow("s:0 m:20") << QSizeF(0, 0) << QMarginsF(20, 20, 20, 20) << false;
    QTest::newRow("s:5 m:20") << QSizeF(5, 5) << QMarginsF(20, 20, 20, 20) << false;
}

void tst_QQuickTableView::flickOvershoot()
{
    // Flick the table completely out and then in again, and see
    // that we still contains the expected rows/columns
    // Note that TableView always keeps top-left item loaded, even
    // when everything is flicked out of view.
    QFETCH(QSizeF, spacing);
    QFETCH(QMarginsF, margins);
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("plaintableview.qml");

    const int rowCount = 5;
    const int columnCount = 5;
    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const qreal cellWidth = delegateWidth + spacing.width();
    const qreal cellHeight = delegateHeight + spacing.height();
    const qreal tableWidth = margins.left() + margins.right() + (cellWidth * columnCount) - spacing.width();
    const qreal tableHeight = margins.top() + margins.bottom() + (cellHeight * rowCount) - spacing.height();
    const int outsideMargin = 10;
    auto model = TestModelAsVariant(rowCount, columnCount);

    tableView->setModel(model);
    tableView->setRowSpacing(spacing.height());
    tableView->setColumnSpacing(spacing.width());
    tableView->setLeftMargin(margins.left());
    tableView->setTopMargin(margins.top());
    tableView->setRightMargin(margins.right());
    tableView->setBottomMargin(margins.bottom());
    tableView->setReuseItems(reuseItems);
    tableView->setWidth(tableWidth - margins.right() - cellWidth / 2);
    tableView->setHeight(tableHeight - margins.bottom() - cellHeight / 2);

    WAIT_UNTIL_POLISHED;

    // Flick table out of view left
    tableView->setContentX(-tableView->width() - outsideMargin);
    tableView->setContentY(0);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedTable.left(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.right(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.top(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.bottom(), rowCount - 1);

    // Flick table out of view right
    tableView->setContentX(tableWidth + outsideMargin);
    tableView->setContentY(0);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedTable.left(), columnCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.right(), columnCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.top(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.bottom(), rowCount - 1);

    // Flick table out of view on top
    tableView->setContentX(0);
    tableView->setContentY(-tableView->height() - outsideMargin);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedTable.left(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.right(), columnCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.top(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.bottom(), 0);

    // Flick table out of view at the bottom
    tableView->setContentX(0);
    tableView->setContentY(tableHeight + outsideMargin);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedTable.left(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.right(), columnCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.top(), rowCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.bottom(), rowCount - 1);

    // Flick table out of view left and top at the same time
    tableView->setContentX(-tableView->width() - outsideMargin);
    tableView->setContentY(-tableView->height() - outsideMargin);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedTable.left(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.right(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.top(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.bottom(), 0);

    // Flick table back to origo
    tableView->setContentX(0);
    tableView->setContentY(0);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedTable.left(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.right(), columnCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.top(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.bottom(), rowCount - 1);

    // Flick table out of view right and bottom at the same time
    tableView->setContentX(tableWidth + outsideMargin);
    tableView->setContentY(tableHeight + outsideMargin);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedTable.left(), columnCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.right(), columnCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.top(), rowCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.bottom(), rowCount - 1);

    // Flick table back to origo
    tableView->setContentX(0);
    tableView->setContentY(0);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedTable.left(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.right(), columnCount - 1);
    QCOMPARE(tableViewPrivate->loadedTable.top(), 0);
    QCOMPARE(tableViewPrivate->loadedTable.bottom(), rowCount - 1);
}

void tst_QQuickTableView::checkRowColumnCount()
{
    // If we flick several columns (rows) at the same time, check that we don't
    // end up with loading more delegate items into memory than necessary. We
    // should free up columns as we go before loading new ones.
    LOAD_TABLEVIEW("countingtableview.qml");

    const char *maxDelegateCountProp = "maxDelegateCount";
    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    auto model = TestModelAsVariant(100, 100);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // We expect that the number of created items after start-up should match
    //the size of the visible table, pluss one extra preloaded row and column.
    const QSize visibleTableSize = tableViewPrivate->loadedTable.size();
    const int qmlCountAfterInit = view->rootObject()->property(maxDelegateCountProp).toInt();
    const int expectedCount = (visibleTableSize.width() + 1) * (visibleTableSize.height() + 1);
    QCOMPARE(qmlCountAfterInit, expectedCount);

    // This test will keep track of the maximum number of delegate items TableView
    // had to show at any point while flicking (in countingtableview.qml). Because
    // of the geometries chosen for TableView and the delegate, only complete columns
    // will be shown at start-up.
    const QRect loadedTable = tableViewPrivate->loadedTable;
    QVERIFY(loadedTable.height() > loadedTable.width());
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.width(), tableView->width());
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.height(), tableView->height());

    // Flick half an item to the left+up, to force one extra column and row to load before we
    // start. By doing so, we end up showing the maximum number of rows and columns that will
    // ever be shown in the view. This will make things less complicated below, when checking
    // how many items that end up visible while flicking.
    tableView->setContentX(delegateWidth / 2);
    tableView->setContentY(delegateHeight / 2);
    const int qmlCountAfterFirstFlick = view->rootObject()->property(maxDelegateCountProp).toInt();

    // Flick a long distance right
    tableView->setContentX(tableView->width() * 2);

    const int qmlCountAfterLongFlick = view->rootObject()->property(maxDelegateCountProp).toInt();
    QCOMPARE(qmlCountAfterLongFlick, qmlCountAfterFirstFlick);

    // Flick a long distance down
    tableView->setContentX(tableView->height() * 2);

    const int qmlCountAfterDownFlick = view->rootObject()->property(maxDelegateCountProp).toInt();
    QCOMPARE(qmlCountAfterDownFlick, qmlCountAfterFirstFlick);

    // Flick a long distance left
    tableView->setContentX(0);

    const int qmlCountAfterLeftFlick = view->rootObject()->property(maxDelegateCountProp).toInt();
    QCOMPARE(qmlCountAfterLeftFlick, qmlCountAfterFirstFlick);

    // Flick a long distance up
    tableView->setContentY(0);

    const int qmlCountAfterUpFlick = view->rootObject()->property(maxDelegateCountProp).toInt();
    QCOMPARE(qmlCountAfterUpFlick, qmlCountAfterFirstFlick);
}

void tst_QQuickTableView::modelSignals()
{
    LOAD_TABLEVIEW("plaintableview.qml");

    TestModel model(1, 1);
    tableView->setModel(QVariant::fromValue(&model));
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 1);

    QVERIFY(model.insertRows(0, 1));
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 2);
    QCOMPARE(tableView->columns(), 1);

    QVERIFY(model.removeRows(1, 1));
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 1);

    model.insertColumns(1, 1);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 2);

    model.removeColumns(1, 1);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 1);

    model.setRowCount(10);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 10);
    QCOMPARE(tableView->columns(), 1);

    model.setColumnCount(10);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 10);
    QCOMPARE(tableView->columns(), 10);

    model.setRowCount(0);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 10);

    model.setColumnCount(1);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 1);

    model.setRowCount(10);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 10);
    QCOMPARE(tableView->columns(), 1);

    model.setColumnCount(10);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 10);
    QCOMPARE(tableView->columns(), 10);

    model.clear();
    model.setColumnCount(1);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 1);
}

void tst_QQuickTableView::checkModelSignalsUpdateLayout()
{
    // Check that if the model rearranges rows and emit the
    // 'layoutChanged' signal, TableView will be updated correctly.
    LOAD_TABLEVIEW("plaintableview.qml");

    TestModel model(0, 1);
    tableView->setModel(QVariant::fromValue(&model));
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 1);

    QString modelRow1Text = QStringLiteral("firstRow");
    QString modelRow2Text = QStringLiteral("secondRow");
    model.insertRow(0);
    model.insertRow(0);
    model.setModelData(QPoint(0, 0), QSize(1, 1), modelRow1Text);
    model.setModelData(QPoint(0, 1), QSize(1, 1), modelRow2Text);
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->rows(), 2);
    QCOMPARE(tableView->columns(), 1);

    QString delegate1text = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item->property("modelDataBinding").toString();
    QString delegate2text = tableViewPrivate->loadedTableItem(QPoint(0, 1))->item->property("modelDataBinding").toString();
    QCOMPARE(delegate1text, modelRow1Text);
    QCOMPARE(delegate2text, modelRow2Text);

    model.swapRows(0, 1);
    WAIT_UNTIL_POLISHED;

    delegate1text = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item->property("modelDataBinding").toString();
    delegate2text = tableViewPrivate->loadedTableItem(QPoint(0, 1))->item->property("modelDataBinding").toString();
    QCOMPARE(delegate1text, modelRow2Text);
    QCOMPARE(delegate2text, modelRow1Text);
}

void tst_QQuickTableView::dataChangedSignal()
{
    // Check that bindings to the model inside a delegate gets updated
    // when the model item they bind to changes.
    LOAD_TABLEVIEW("plaintableview.qml");

    const QString prefix(QStringLiteral("changed"));

    TestModel model(10, 10);
    tableView->setModel(QVariant::fromValue(&model));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems) {
        const auto item = tableViewPrivate->loadedTableItem(fxItem->cell)->item;
        const QString modelDataBindingProperty = item->property(kModelDataBindingProp).toString();
        QString expectedModelData = QString::number(fxItem->cell.y());
        QCOMPARE(modelDataBindingProperty, expectedModelData);
    }

    // Change one cell in the model
    model.setModelData(QPoint(0, 0), QSize(1, 1), prefix);

    for (auto fxItem : tableViewPrivate->loadedItems) {
        const QPoint cell = fxItem->cell;
        const auto modelIndex = model.index(cell.y(), cell.x());
        QString expectedModelData = model.data(modelIndex, Qt::DisplayRole).toString();

        const auto item = tableViewPrivate->loadedTableItem(fxItem->cell)->item;
        const QString modelDataBindingProperty = item->property(kModelDataBindingProp).toString();

        QCOMPARE(modelDataBindingProperty, expectedModelData);
    }

    // Change four cells in one go
    model.setModelData(QPoint(1, 0), QSize(2, 2), prefix);

    for (auto fxItem : tableViewPrivate->loadedItems) {
        const QPoint cell = fxItem->cell;
        const auto modelIndex = model.index(cell.y(), cell.x());
        QString expectedModelData = model.data(modelIndex, Qt::DisplayRole).toString();

        const auto item = tableViewPrivate->loadedTableItem(fxItem->cell)->item;
        const QString modelDataBindingProperty = item->property(kModelDataBindingProp).toString();

        QCOMPARE(modelDataBindingProperty, expectedModelData);
    }
}

void tst_QQuickTableView::checkThatPoolIsDrainedWhenReuseIsFalse()
{
    // Check that the reuse pool is drained
    // immediately when setting reuseItems to false.
    LOAD_TABLEVIEW("countingtableview.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // The pool should now contain preloaded items
    QVERIFY(tableViewPrivate->tableModel->poolSize() > 0);
    tableView->setReuseItems(false);
    // The pool should now be empty
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);
}

void tst_QQuickTableView::checkIfDelegatesAreReused_data()
{
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("reuse = true") << true;
    QTest::newRow("reuse = false") << false;
}

void tst_QQuickTableView::checkIfDelegatesAreReused()
{
    // Check that we end up reusing delegate items while flicking if
    // TableView has reuseItems set to true, but otherwise not.
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("countingtableview.qml");

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const int pageFlickCount = 4;

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    tableView->setReuseItems(reuseItems);

    WAIT_UNTIL_POLISHED;

    // Flick half an item to the side, to force one extra row and column to load before we start.
    // This will make things less complicated below, when checking how many times the items
    // have been reused (all items will then report the same number).
    tableView->setContentX(delegateWidth / 2);
    tableView->setContentY(delegateHeight / 2);
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);

    // Some items have already been pooled and reused after we moved the content view, because
    // we preload one extra row and column at start-up. So reset the count-properties back to 0
    // before we continue.
    for (auto fxItem : tableViewPrivate->loadedItems) {
        fxItem->item->setProperty("pooledCount", 0);
        fxItem->item->setProperty("reusedCount", 0);
    }

    const int visibleColumnCount = tableViewPrivate->loadedTable.width();
    const int visibleRowCount = tableViewPrivate->loadedTable.height();
    const int delegateCountAfterInit = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();

    for (int column = 1; column <= (visibleColumnCount * pageFlickCount); ++column) {
        // Flick columns to the left (and add one pixel to ensure the left column is completely out)
        tableView->setContentX((delegateWidth * column) + 1);
        // Check that the number of delegate items created so far is what we expect.
        const int delegatesCreatedCount = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();
        int expectedCount = delegateCountAfterInit + (reuseItems ? 0 : visibleRowCount * column);
        QCOMPARE(delegatesCreatedCount, expectedCount);
    }

    // Check that each delegate item has been reused as many times
    // as we have flicked pages (if reuse is enabled).
    for (auto fxItem : tableViewPrivate->loadedItems) {
        int pooledCount = fxItem->item->property("pooledCount").toInt();
        int reusedCount = fxItem->item->property("reusedCount").toInt();
        if (reuseItems) {
            QCOMPARE(pooledCount, pageFlickCount);
            QCOMPARE(reusedCount, pageFlickCount);
        } else {
            QCOMPARE(pooledCount, 0);
            QCOMPARE(reusedCount, 0);
        }
    }
}

void tst_QQuickTableView::checkIfDelegatesAreReusedAsymmetricTableSize()
{
    // Check that we end up reusing all delegate items while flicking, also if the table contain
    // more columns than rows. In that case, if we flick out a whole row, we'll move a lot of
    // items into the pool. And if we then start flicking in columns, we'll only reuse a few of
    // them for each column. Still, we don't want the pool to release the superfluous items after
    // each load, since they are still in circulation and will be needed once we flick in a new
    // row at the end of the test.
    LOAD_TABLEVIEW("countingtableview.qml");

    const int columnCount = 20;
    const int rowCount = 2;
    const qreal delegateWidth = tableView->width() / columnCount;
    const qreal delegateHeight = (tableView->height() / rowCount) + 10;

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    // Let the height of each row be much bigger than the width of each column.
    view->rootObject()->setProperty("delegateWidth", delegateWidth);
    view->rootObject()->setProperty("delegateHeight", delegateHeight);

    WAIT_UNTIL_POLISHED;

    auto initialTopLeftItem = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item;
    QVERIFY(initialTopLeftItem);
    int pooledCount = initialTopLeftItem->property("pooledCount").toInt();
    int reusedCount = initialTopLeftItem->property("reusedCount").toInt();
    QCOMPARE(pooledCount, 0);
    QCOMPARE(reusedCount, 0);

    // Flick half an item left+down, to force one extra row and column to load. By doing
    // so, we force the maximum number of rows and columns to show before we start the test.
    // This will make things less complicated below, when checking how many
    // times the items have been reused (all items will then report the same number).
    tableView->setContentX(delegateWidth * 0.5);
    tableView->setContentY(delegateHeight * 0.5);

    // Since we have flicked half a delegate to the left, the number of visible
    // columns is now one more than the column count were when we started the test.
    const int visibleColumnCount = tableViewPrivate->loadedTable.width();
    QCOMPARE(visibleColumnCount, columnCount + 1);

    // We expect no items to have been pooled so far
    pooledCount = initialTopLeftItem->property("pooledCount").toInt();
    reusedCount = initialTopLeftItem->property("reusedCount").toInt();
    QCOMPARE(pooledCount, 0);
    QCOMPARE(reusedCount, 0);
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);

    // Flick one row out of view. This will move one whole row of items into the
    // pool without reusing them, since no new row is exposed at the bottom.
    tableView->setContentY(delegateHeight + 1);
    pooledCount = initialTopLeftItem->property("pooledCount").toInt();
    reusedCount = initialTopLeftItem->property("reusedCount").toInt();
    QCOMPARE(pooledCount, 1);
    QCOMPARE(reusedCount, 0);
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), visibleColumnCount);

    const int delegateCountAfterInit = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();

    // Start flicking in a lot of columns, and check that the created count stays the same
    for (int column = 1; column <= 10; ++column) {
        tableView->setContentX((delegateWidth * column) + 10);
        const int delegatesCreatedCount = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();
        // Since we reuse items while flicking, the created count should stay the same
        QCOMPARE(delegatesCreatedCount, delegateCountAfterInit);
        // Since we flick out just as many columns as we flick in, the pool size should stay the same
        QCOMPARE(tableViewPrivate->tableModel->poolSize(), visibleColumnCount);
    }

    // Finally, flick one row back into view (but without flicking so far that we push the third
    // row out and into the pool). The pool should still contain the exact amount of items that
    // we had after we flicked the first row out. And this should be exactly the amount of items
    // needed to load the row back again. And this also means that the pool count should then return
    // back to 0.
    tableView->setContentY(delegateHeight - 1);
    const int delegatesCreatedCount = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();
    QCOMPARE(delegatesCreatedCount, delegateCountAfterInit);
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);
}

void tst_QQuickTableView::checkContextProperties_data()
{
    QTest::addColumn<QVariant>("model");
    QTest::addColumn<bool>("reuseItems");

    auto stringList = QStringList();
    for (int i = 0; i < 100; ++i)
        stringList.append(QString::number(i));

    QTest::newRow("QAIM, reuse=false") << TestModelAsVariant(100, 100) << false;
    QTest::newRow("QAIM, reuse=true") << TestModelAsVariant(100, 100) << true;
    QTest::newRow("Number model, reuse=false") << QVariant::fromValue(100) << false;
    QTest::newRow("Number model, reuse=true") << QVariant::fromValue(100) << true;
    QTest::newRow("QStringList, reuse=false") << QVariant::fromValue(stringList) << false;
    QTest::newRow("QStringList, reuse=true") << QVariant::fromValue(stringList) << true;
}

void tst_QQuickTableView::checkContextProperties()
{
    // Check that the context properties of the delegate items
    // are what we expect while flicking, with or without item recycling.
    QFETCH(QVariant, model);
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("countingtableview.qml");

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const int rowCount = 100;
    const int pageFlickCount = 3;

    tableView->setModel(model);
    tableView->setReuseItems(reuseItems);

    WAIT_UNTIL_POLISHED;

    const int visibleRowCount = qMin(tableView->rows(), qCeil(tableView->height() / delegateHeight));
    const int visibleColumnCount = qMin(tableView->columns(), qCeil(tableView->width() / delegateWidth));

    for (int row = 1; row <= (visibleRowCount * pageFlickCount); ++row) {
        // Flick rows up
        tableView->setContentY((delegateHeight * row) + (delegateHeight / 2));
        tableView->polish();

        WAIT_UNTIL_POLISHED;

        for (int col = 0; col < visibleColumnCount; ++col) {
            const auto item = tableViewPrivate->loadedTableItem(QPoint(col, row))->item;
            const auto context = qmlContext(item.data());
            const int contextIndex = context->contextProperty("index").toInt();
            const int contextRow = context->contextProperty("row").toInt();
            const int contextColumn = context->contextProperty("column").toInt();
            const QString contextModelData = context->contextProperty("modelData").toString();

            QCOMPARE(contextIndex, row + (col * rowCount));
            QCOMPARE(contextRow, row);
            QCOMPARE(contextColumn, col);
            QCOMPARE(contextModelData, QStringLiteral("%1").arg(row));
        }
    }
}

void tst_QQuickTableView::checkContextPropertiesQQmlListProperyModel_data()
{
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("reuse=false") << false;
    QTest::newRow("reuse=true") << true;
}

void tst_QQuickTableView::checkContextPropertiesQQmlListProperyModel()
{
    // Check that the context properties of the delegate items
    // are what we expect while flicking, with or without item recycling.
    // This test hard-codes the model to be a QQmlListPropertyModel from
    // within the qml file.
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("qqmllistpropertymodel.qml");

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const int rowCount = 100;
    const int pageFlickCount = 3;

    tableView->setReuseItems(reuseItems);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    const int visibleRowCount = qMin(tableView->rows(), qCeil(tableView->height() / delegateHeight));
    const int visibleColumnCount = qMin(tableView->columns(), qCeil(tableView->width() / delegateWidth));

    for (int row = 1; row <= (visibleRowCount * pageFlickCount); ++row) {
        // Flick rows up
        tableView->setContentY((delegateHeight * row) + (delegateHeight / 2));
        tableView->polish();

        WAIT_UNTIL_POLISHED;

        for (int col = 0; col < visibleColumnCount; ++col) {
            const auto item = tableViewPrivate->loadedTableItem(QPoint(col, row))->item;
            const auto context = qmlContext(item.data());
            const int contextIndex = context->contextProperty("index").toInt();
            const int contextRow = context->contextProperty("row").toInt();
            const int contextColumn = context->contextProperty("column").toInt();
            const QObject *contextModelData = qvariant_cast<QObject *>(context->contextProperty("modelData"));
            const QString modelDataProperty = contextModelData->property("someCustomProperty").toString();

            QCOMPARE(contextIndex, row + (col * rowCount));
            QCOMPARE(contextRow, row);
            QCOMPARE(contextColumn, col);
            QCOMPARE(modelDataProperty, QStringLiteral("%1").arg(row));
        }
    }
}

void tst_QQuickTableView::checkRowAndColumnChangedButNotIndex()
{
    // Check that context row and column changes even if the index stays the
    // same when the item is reused. This can happen in rare cases if the item
    // is first used at e.g (row 1, col 0), but then reused at (row 0, col 1)
    // while the model has changed row count in-between.
    LOAD_TABLEVIEW("checkrowandcolumnnotchanged.qml");

    TestModel model(2, 1);
    tableView->setModel(QVariant::fromValue(&model));

    WAIT_UNTIL_POLISHED;

    model.removeRow(1);
    model.insertColumn(1);
    tableView->forceLayout();

    const auto item = tableViewPrivate->loadedTableItem(QPoint(1, 0))->item;
    const auto context = qmlContext(item.data());
    const int contextIndex = context->contextProperty("index").toInt();
    const int contextRow = context->contextProperty("row").toInt();
    const int contextColumn = context->contextProperty("column").toInt();

    QCOMPARE(contextIndex, 1);
    QCOMPARE(contextRow, 0);
    QCOMPARE(contextColumn, 1);
}

void tst_QQuickTableView::checkChangingModelFromDelegate()
{
    // Check that we don't restart a rebuild of the table
    // while we're in the middle of rebuilding it from before
    LOAD_TABLEVIEW("changemodelfromdelegate.qml");

    // Set addRowFromDelegate. This will trigger the QML code to add a new
    // row and call forceLayout(). When TableView instantiates the first
    // delegate in the new row, the Component.onCompleted handler will try to
    // add a new row. But since we're currently rebuilding, this should be
    // scheduled for later.
    view->rootObject()->setProperty("addRowFromDelegate", true);

    // We now expect two rows in the table, one more than initially
    QCOMPARE(tableViewPrivate->tableSize.height(), 2);
    QCOMPARE(tableViewPrivate->loadedTable.height(), 2);

    // And since the QML code tried to add another row as well, we
    // expect rebuildScheduled to be true, and a polish event to be pending.
    QCOMPARE(tableViewPrivate->rebuildScheduled, true);
    QCOMPARE(tableViewPrivate->polishScheduled, true);
    WAIT_UNTIL_POLISHED;

    // After handling the polish event, we expect also the third row to now be added
    QCOMPARE(tableViewPrivate->tableSize.height(), 3);
    QCOMPARE(tableViewPrivate->loadedTable.height(), 3);
}

void tst_QQuickTableView::checkRebuildViewportOnly()
{
    // Check that we only rebuild from the current top-left cell
    // when you add or remove rows and columns. There should be
    // no need to do a rebuild from scratch in such cases.
    LOAD_TABLEVIEW("countingtableview.qml");

    const char *propName = "delegatesCreatedCount";
    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;

    TestModel model(100, 100);
    tableView->setModel(QVariant::fromValue(&model));

    WAIT_UNTIL_POLISHED;

    // Flick to row/column 50, 50
    tableView->setContentX(delegateWidth * 50);
    tableView->setContentY(delegateHeight * 50);

    // Set reuse items to false, just to make it easier to
    // check the number of items created during a rebuild.
    tableView->setReuseItems(false);
    const int itemCountBeforeRebuild = tableViewPrivate->loadedItems.count();

    // Since all cells have the same size, we expect that we end up creating
    // the same amount of items that were already showing before, even after
    // adding or removing rows and columns.
    view->rootObject()->setProperty(propName, 0);
    model.insertRow(51);
    WAIT_UNTIL_POLISHED;
    int countAfterRebuild = view->rootObject()->property(propName).toInt();
    QCOMPARE(countAfterRebuild, itemCountBeforeRebuild);

    view->rootObject()->setProperty(propName, 0);
    model.removeRow(51);
    WAIT_UNTIL_POLISHED;
    countAfterRebuild = view->rootObject()->property(propName).toInt();
    QCOMPARE(countAfterRebuild, itemCountBeforeRebuild);

    view->rootObject()->setProperty(propName, 0);
    model.insertColumn(51);
    WAIT_UNTIL_POLISHED;
    countAfterRebuild = view->rootObject()->property(propName).toInt();
    QCOMPARE(countAfterRebuild, itemCountBeforeRebuild);

    view->rootObject()->setProperty(propName, 0);
    model.removeColumn(51);
    WAIT_UNTIL_POLISHED;
    countAfterRebuild = view->rootObject()->property(propName).toInt();
    QCOMPARE(countAfterRebuild, itemCountBeforeRebuild);
}

void tst_QQuickTableView::useDelegateChooserWithoutDefault()
{
    // Check that the application issues a warning (but doesn't e.g
    // crash) if the delegate chooser doesn't cover all cells
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*failed"));
    LOAD_TABLEVIEW("usechooserwithoutdefault.qml");
    auto model = TestModelAsVariant(2, 1);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;
};

void tst_QQuickTableView::checkTableviewInsideAsyncLoader()
{
    // Check that you can put a TableView inside an async Loader, and
    // that the delegate items are created before the loader is ready.
    LOAD_TABLEVIEW_ASYNC("asyncplain.qml");

    // At this point the Loader has finished
    QCOMPARE(loader->status(), QQuickLoader::Ready);

    // Check that TableView has finished building
    QCOMPARE(tableViewPrivate->rebuildScheduled, false);
    QCOMPARE(tableViewPrivate->rebuildState, QQuickTableViewPrivate::RebuildState::Done);

    // Check that all expected delegate items have been loaded
    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    int expectedColumns = qCeil(tableView->width() / delegateWidth);
    int expectedRows = qCeil(tableView->height() / delegateHeight);
    QCOMPARE(tableViewPrivate->loadedTable.width(), expectedColumns);
    QCOMPARE(tableViewPrivate->loadedTable.height(), expectedRows);

    // Check that the loader was still in a loading state while TableView was creating
    // delegate items. If we delayed creating delegate items until we got the first
    // updatePolish() callback in QQuickTableView, this would not be the case.
    auto statusWhenDelegate0_0Completed = qvariant_cast<QQuickLoader::Status>(
                loader->item()->property("statusWhenDelegate0_0Created"));
    auto statusWhenDelegate5_5Completed = qvariant_cast<QQuickLoader::Status>(
                loader->item()->property("statusWhenDelegate5_5Created"));
    QCOMPARE(statusWhenDelegate0_0Completed, QQuickLoader::Loading);
    QCOMPARE(statusWhenDelegate5_5Completed, QQuickLoader::Loading);

    // Check that TableView had a valid geometry when we started to build. If the build
    // was started too early (e.g upon QQuickTableView::componentComplete), width and
    // height would still be 0 since the bindings would not have been evaluated yet.
    qreal width = loader->item()->property("tableViewWidthWhileBuilding").toReal();
    qreal height = loader->item()->property("tableViewHeightWhileBuilding").toReal();
    QVERIFY(width > 0);
    QVERIFY(height > 0);
};

void tst_QQuickTableView::checkThatRevisionedPropertiesCannotBeUsedInOldImports()
{
    // Check that if you use a QQmlAdaptorModel together with a Repeater, the
    // revisioned context properties 'row' and 'column' are not accessible.
    LOAD_TABLEVIEW("checkmodelpropertyrevision.qml");
    const int resolvedRow = view->rootObject()->property("resolvedDelegateRow").toInt();
    const int resolvedColumn = view->rootObject()->property("resolvedDelegateColumn").toInt();
    QCOMPARE(resolvedRow, 42);
    QCOMPARE(resolvedColumn, 42);
}

QTEST_MAIN(tst_QQuickTableView)

#include "tst_qquicktableview.moc"
