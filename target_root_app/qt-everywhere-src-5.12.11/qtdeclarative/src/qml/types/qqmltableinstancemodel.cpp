/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qqmltableinstancemodel_p.h"
#include "qqmldelegatecomponent_p.h"

#include <QtCore/QTimer>

#include <QtQml/private/qqmlincubator_p.h>
#include <QtQml/private/qqmlchangeset_p.h>
#include <QtQml/private/qqmlcomponent_p.h>

QT_BEGIN_NAMESPACE

const char* kModelItemTag = "_tableinstancemodel_modelItem";

bool QQmlTableInstanceModel::isDoneIncubating(QQmlDelegateModelItem *modelItem)
{
    if (!modelItem->incubationTask)
        return true;

    const auto status = modelItem->incubationTask->status();
    return (status == QQmlIncubator::Ready) || (status == QQmlIncubator::Error);
}

void QQmlTableInstanceModel::deleteModelItemLater(QQmlDelegateModelItem *modelItem)
{
    Q_ASSERT(modelItem);

    delete modelItem->object;
    modelItem->object = nullptr;

    if (modelItem->contextData) {
        modelItem->contextData->invalidate();
        Q_ASSERT(modelItem->contextData->refCount == 1);
        modelItem->contextData = nullptr;
    }

    modelItem->deleteLater();
}

QQmlTableInstanceModel::QQmlTableInstanceModel(QQmlContext *qmlContext, QObject *parent)
    : QQmlInstanceModel(*(new QObjectPrivate()), parent)
    , m_qmlContext(qmlContext)
    , m_metaType(new QQmlDelegateModelItemMetaType(m_qmlContext->engine()->handle(), nullptr, QStringList()))
{
}

void QQmlTableInstanceModel::useImportVersion(int minorVersion)
{
    m_adaptorModel.useImportVersion(minorVersion);
}

QQmlTableInstanceModel::~QQmlTableInstanceModel()
{
    for (const auto modelItem : m_modelItems) {
        // No item in m_modelItems should be referenced at this point. The view
        // should release all its items before it deletes this model. Only model items
        // that are still being incubated should be left for us to delete.
        Q_ASSERT(modelItem->objectRef == 0);
        Q_ASSERT(modelItem->incubationTask);
        // Check that we are not being deleted while we're
        // in the process of e.g emitting a created signal.
        Q_ASSERT(modelItem->scriptRef == 0);

        if (modelItem->object) {
            delete modelItem->object;
            modelItem->object = nullptr;
            modelItem->contextData->invalidate();
            modelItem->contextData = nullptr;
        }
    }

    deleteAllFinishedIncubationTasks();
    qDeleteAll(m_modelItems);
    drainReusableItemsPool(0);
}

QQmlComponent *QQmlTableInstanceModel::resolveDelegate(int index)
{
    if (m_delegateChooser) {
        const int row = m_adaptorModel.rowAt(index);
        const int column = m_adaptorModel.columnAt(index);
        QQmlComponent *delegate = nullptr;
        QQmlAbstractDelegateComponent *chooser = m_delegateChooser;
        do {
            delegate = chooser->delegate(&m_adaptorModel, row, column);
            chooser = qobject_cast<QQmlAbstractDelegateComponent *>(delegate);
        } while (chooser);
        return delegate;
    }

    return m_delegate;
}

QQmlDelegateModelItem *QQmlTableInstanceModel::resolveModelItem(int index)
{
    // Check if an item for the given index is already loaded and ready
    QQmlDelegateModelItem *modelItem = m_modelItems.value(index, nullptr);
    if (modelItem)
        return modelItem;

    QQmlComponent *delegate = resolveDelegate(index);
    if (!delegate)
        return nullptr;

    // Check if the pool contains an item that can be reused
    modelItem = takeFromReusableItemsPool(delegate);
    if (modelItem) {
        reuseItem(modelItem, index);
        m_modelItems.insert(index, modelItem);
        return modelItem;
    }

    // Create a new item from scratch
    modelItem = m_adaptorModel.createItem(m_metaType, index);
    if (modelItem) {
        modelItem->delegate = delegate;
        m_modelItems.insert(index, modelItem);
        return modelItem;
    }

    qWarning() << Q_FUNC_INFO << "failed creating a model item for index: " << index;
    return nullptr;
}

QObject *QQmlTableInstanceModel::object(int index, QQmlIncubator::IncubationMode incubationMode)
{
    Q_ASSERT(m_delegate);
    Q_ASSERT(index >= 0 && index < m_adaptorModel.count());
    Q_ASSERT(m_qmlContext && m_qmlContext->isValid());

    QQmlDelegateModelItem *modelItem = resolveModelItem(index);
    if (!modelItem)
        return nullptr;

    if (modelItem->object) {
        // The model item has already been incubated. So
        // just bump the ref-count and return it.
        modelItem->referenceObject();
        return modelItem->object;
    }

    // The object is not ready, and needs to be incubated
    incubateModelItem(modelItem, incubationMode);
    if (!isDoneIncubating(modelItem))
        return nullptr;

    // Incubation is done, so the task should be removed
    Q_ASSERT(!modelItem->incubationTask);

    if (!modelItem->object) {
        // The object was incubated synchronously (otherwise we would return above). But since
        // we have no object, the incubation must have failed. And when we have no object, there
        // should be no object references either. And there should also not be any internal script
        // refs at this point. So we delete the model item.
        Q_ASSERT(!modelItem->isObjectReferenced());
        Q_ASSERT(!modelItem->isReferenced());
        m_modelItems.remove(modelItem->index);
        delete modelItem;
        return nullptr;
    }

    // Incubation was completed sync and successful
    modelItem->referenceObject();
    return modelItem->object;
}

QQmlInstanceModel::ReleaseFlags QQmlTableInstanceModel::release(QObject *object, ReusableFlag reusable)
{
    Q_ASSERT(object);
    auto modelItem = qvariant_cast<QQmlDelegateModelItem *>(object->property(kModelItemTag));
    Q_ASSERT(modelItem);

    if (!modelItem->releaseObject())
        return QQmlDelegateModel::Referenced;

    if (modelItem->isReferenced()) {
        // We still have an internal reference to this object, which means that we are told to release an
        // object while the createdItem signal for it is still on the stack. This can happen when objects
        // are e.g delivered async, and the user flicks back and forth quicker than the loading can catch
        // up with. The view might then find that the object is no longer visible and should be released.
        // We detect this case in incubatorStatusChanged(), and delete it there instead. But from the callers
        // point of view, it should consider it destroyed.
        return QQmlDelegateModel::Destroyed;
    }

    // The item is not referenced by anyone
    m_modelItems.remove(modelItem->index);

    if (reusable == Reusable) {
        insertIntoReusableItemsPool(modelItem);
        return QQmlInstanceModel::Referenced;
    }

    // The item is not reused or referenced by anyone, so just delete it
    modelItem->destroyObject();
    emit destroyingItem(object);

    delete modelItem;
    return QQmlInstanceModel::Destroyed;
}

void QQmlTableInstanceModel::cancel(int index)
{
    auto modelItem = m_modelItems.value(index);
    Q_ASSERT(modelItem);

    // Since the view expects the item to be incubating, there should be
    // an incubation task. And since the incubation is not done, no-one
    // should yet have received, and therfore hold a reference to, the object.
    Q_ASSERT(modelItem->incubationTask);
    Q_ASSERT(!modelItem->isObjectReferenced());

    m_modelItems.remove(index);

    if (modelItem->object)
        delete modelItem->object;

    // modelItem->incubationTask will be deleted from the modelItems destructor
    delete modelItem;
}

void QQmlTableInstanceModel::insertIntoReusableItemsPool(QQmlDelegateModelItem *modelItem)
{
    // Currently, the only way for a view to reuse items is to call QQmlTableInstanceModel::release()
    // with the second argument explicitly set to QQmlTableInstanceModel::Reusable. If the released
    // item is no longer referenced, it will be added to the pool. Reusing of items can be specified
    // per item, in case certain items cannot be recycled.
    // A QQmlDelegateModelItem knows which delegate its object was created from. So when we are
    // about to create a new item, we first check if the pool contains an item based on the same
    // delegate from before. If so, we take it out of the pool (instead of creating a new item), and
    // update all its context-, and attached properties.
    // When a view is recycling items, it should call QQmlTableInstanceModel::drainReusableItemsPool()
    // regularly. As there is currently no logic to 'hibernate' items in the pool, they are only
    // meant to rest there for a short while, ideally only from the time e.g a row is unloaded
    // on one side of the view, and until a new row is loaded on the opposite side. In-between
    // this time, the application will see the item as fully functional and 'alive' (just not
    // visible on screen). Since this time is supposed to be short, we don't take any action to
    // notify the application about it, since we don't want to trigger any bindings that can
    // disturb performance.
    // A recommended time for calling drainReusableItemsPool() is each time a view has finished
    // loading e.g a new row or column. If there are more items in the pool after that, it means
    // that the view most likely doesn't need them anytime soon. Those items should be destroyed to
    // not consume resources.
    // Depending on if a view is a list or a table, it can sometimes be performant to keep
    // items in the pool for a bit longer than one "row out/row in" cycle. E.g for a table, if the
    // number of visible rows in a view is much larger than the number of visible columns.
    // In that case, if you flick out a row, and then flick in a column, you would throw away a lot
    // of items in the pool if completely draining it. The reason is that unloading a row places more
    // items in the pool than what ends up being recycled when loading a new column. And then, when you
    // next flick in a new row, you would need to load all those drained items again from scratch. For
    // that reason, you can specify a maxPoolTime to the drainReusableItemsPool() that allows you to keep
    // items in the pool for a bit longer, effectively keeping more items in circulation.
    // A recommended maxPoolTime would be equal to the number of dimenstions in the view, which
    // means 1 for a list view and 2 for a table view. If you specify 0, all items will be drained.
    Q_ASSERT(!modelItem->incubationTask);
    Q_ASSERT(!modelItem->isObjectReferenced());
    Q_ASSERT(!modelItem->isReferenced());
    Q_ASSERT(modelItem->object);

    modelItem->poolTime = 0;
    m_reusableItemsPool.append(modelItem);
    emit itemPooled(modelItem->index, modelItem->object);
}

QQmlDelegateModelItem *QQmlTableInstanceModel::takeFromReusableItemsPool(const QQmlComponent *delegate)
{
    // Find the oldest item in the pool that was made from the same delegate as
    // the given argument, remove it from the pool, and return it.
    if (m_reusableItemsPool.isEmpty())
        return nullptr;

    for (auto it = m_reusableItemsPool.begin(); it != m_reusableItemsPool.end(); ++it) {
        if ((*it)->delegate != delegate)
            continue;
        auto modelItem = *it;
        m_reusableItemsPool.erase(it);
        return modelItem;
    }

    return nullptr;
}

void QQmlTableInstanceModel::drainReusableItemsPool(int maxPoolTime)
{
    // Rather than releasing all pooled items upon a call to this function, each
    // item has a poolTime. The poolTime specifies for how many loading cycles an item
    // has been resting in the pool. And for each invocation of this function, poolTime
    // will increase. If poolTime is equal to, or exceeds, maxPoolTime, it will be removed
    // from the pool and released. This way, the view can tweak a bit for how long
    // items should stay in "circulation", even if they are not recycled right away.
    for (auto it = m_reusableItemsPool.begin(); it != m_reusableItemsPool.end();) {
        auto modelItem = *it;
        modelItem->poolTime++;
        if (modelItem->poolTime <= maxPoolTime) {
            ++it;
        } else {
            it = m_reusableItemsPool.erase(it);
            release(modelItem->object, NotReusable);
        }
    }
}

void QQmlTableInstanceModel::reuseItem(QQmlDelegateModelItem *item, int newModelIndex)
{
    // Update the context properties index, row and column on
    // the delegate item, and inform the application about it.
    const int newRow = m_adaptorModel.rowAt(newModelIndex);
    const int newColumn = m_adaptorModel.columnAt(newModelIndex);
    item->setModelIndex(newModelIndex, newRow, newColumn);

    // Notify the application that all 'dynamic'/role-based context data has
    // changed as well (their getter function will use the updated index).
    auto const itemAsList = QList<QQmlDelegateModelItem *>() << item;
    auto const updateAllRoles = QVector<int>();
    m_adaptorModel.notify(itemAsList, newModelIndex, 1, updateAllRoles);

    // Inform the view that the item is recycled. This will typically result
    // in the view updating its own attached delegate item properties.
    emit itemReused(newModelIndex, item->object);
}

void QQmlTableInstanceModel::incubateModelItem(QQmlDelegateModelItem *modelItem, QQmlIncubator::IncubationMode incubationMode)
{
    // Guard the model item temporarily so that it's not deleted from
    // incubatorStatusChanged(), in case the incubation is done synchronously.
    modelItem->scriptRef++;

    if (modelItem->incubationTask) {
        // We're already incubating the model item from a previous request. If the previous call requested
        // the item async, but the current request needs it sync, we need to force-complete the incubation.
        const bool sync = (incubationMode == QQmlIncubator::Synchronous || incubationMode == QQmlIncubator::AsynchronousIfNested);
        if (sync && modelItem->incubationTask->incubationMode() == QQmlIncubator::Asynchronous)
            modelItem->incubationTask->forceCompletion();
    } else {
        modelItem->incubationTask = new QQmlTableInstanceModelIncubationTask(this, modelItem, incubationMode);

        QQmlContextData *ctxt = new QQmlContextData;
        QQmlContext *creationContext = modelItem->delegate->creationContext();
        ctxt->setParent(QQmlContextData::get(creationContext  ? creationContext : m_qmlContext.data()));
        ctxt->contextObject = modelItem;
        modelItem->contextData = ctxt;

        QQmlComponentPrivate::get(modelItem->delegate)->incubateObject(
                    modelItem->incubationTask,
                    modelItem->delegate,
                    m_qmlContext->engine(),
                    ctxt,
                    QQmlContextData::get(m_qmlContext));
    }

    // Remove the temporary guard
    modelItem->scriptRef--;
}

void QQmlTableInstanceModel::incubatorStatusChanged(QQmlTableInstanceModelIncubationTask *incubationTask, QQmlIncubator::Status status)
{
    QQmlDelegateModelItem *modelItem = incubationTask->modelItemToIncubate;
    Q_ASSERT(modelItem->incubationTask);

    modelItem->incubationTask = nullptr;
    incubationTask->modelItemToIncubate = nullptr;

    if (status == QQmlIncubator::Ready) {
        // Tag the incubated object with the model item for easy retrieval upon release etc.
        modelItem->object->setProperty(kModelItemTag, QVariant::fromValue(modelItem));

        // Emit that the item has been created. What normally happens next is that the view
        // upon receiving the signal asks for the model item once more. And since the item is
        // now in the map, it will be returned directly.
        Q_ASSERT(modelItem->object);
        modelItem->scriptRef++;
        emit createdItem(modelItem->index, modelItem->object);
        modelItem->scriptRef--;
    } else if (status == QQmlIncubator::Error) {
        qWarning() << "Error incubating delegate:" << incubationTask->errors();
    }

    if (!modelItem->isReferenced() && !modelItem->isObjectReferenced()) {
        // We have no internal reference to the model item, and the view has no
        // reference to the incubated object. So just delete the model item.
        // Note that being here means that the object was incubated _async_
        // (otherwise modelItem->isReferenced() would be true).
        m_modelItems.remove(modelItem->index);

        if (modelItem->object) {
            modelItem->scriptRef++;
            emit destroyingItem(modelItem->object);
            modelItem->scriptRef--;
            Q_ASSERT(!modelItem->isReferenced());
        }

        deleteModelItemLater(modelItem);
    }

    deleteIncubationTaskLater(incubationTask);
}

QQmlIncubator::Status QQmlTableInstanceModel::incubationStatus(int index) {
    const auto modelItem = m_modelItems.value(index, nullptr);
    if (!modelItem)
        return QQmlIncubator::Null;

    if (modelItem->incubationTask)
        return modelItem->incubationTask->status();

    // Since we clear the incubation task when we're done
    // incubating, it means that the status is Ready.
    return QQmlIncubator::Ready;
}

void QQmlTableInstanceModel::deleteIncubationTaskLater(QQmlIncubator *incubationTask)
{
    // We often need to post-delete incubation tasks, since we cannot
    // delete them while we're in the middle of an incubation change callback.
    Q_ASSERT(!m_finishedIncubationTasks.contains(incubationTask));
    m_finishedIncubationTasks.append(incubationTask);
    if (m_finishedIncubationTasks.count() == 1)
        QTimer::singleShot(1, this, &QQmlTableInstanceModel::deleteAllFinishedIncubationTasks);
}

void QQmlTableInstanceModel::deleteAllFinishedIncubationTasks()
{
    qDeleteAll(m_finishedIncubationTasks);
    m_finishedIncubationTasks.clear();
}

QVariant QQmlTableInstanceModel::model() const
{
    return m_adaptorModel.model();
}

void QQmlTableInstanceModel::setModel(const QVariant &model)
{
    // Pooled items are still accessible/alive for the application, and
    // needs to stay in sync with the model. So we need to drain the pool
    // completely when the model changes.
    drainReusableItemsPool(0);
    if (auto const aim = abstractItemModel())
        disconnect(aim, &QAbstractItemModel::dataChanged, this, &QQmlTableInstanceModel::dataChangedCallback);
    m_adaptorModel.setModel(model, this, m_qmlContext->engine());
    if (auto const aim = abstractItemModel())
        connect(aim, &QAbstractItemModel::dataChanged, this, &QQmlTableInstanceModel::dataChangedCallback);
}

void QQmlTableInstanceModel::dataChangedCallback(const QModelIndex &begin, const QModelIndex &end, const QVector<int> &roles)
{
    // This function is called when model data has changed. In that case, we tell the adaptor model
    // to go through all the items we have created, find the ones that are affected, and notify that
    // their model data has changed. This will in turn update QML bindings inside the delegate items.
    int numberOfRowsChanged = end.row() - begin.row() + 1;
    int numberOfColumnsChanged = end.column() - begin.column() + 1;

    for (int column = 0; column < numberOfColumnsChanged; ++column) {
        const int columnIndex = begin.column() + column;
        const int rowIndex = begin.row() + (columnIndex * rows());
        m_adaptorModel.notify(m_modelItems.values(), rowIndex, numberOfRowsChanged, roles);
    }
}

QQmlComponent *QQmlTableInstanceModel::delegate() const
{
    return m_delegate;
}

void QQmlTableInstanceModel::setDelegate(QQmlComponent *delegate)
{
    if (m_delegate == delegate)
        return;

    m_delegateChooser = nullptr;
    if (delegate) {
        QQmlAbstractDelegateComponent *adc =
                qobject_cast<QQmlAbstractDelegateComponent *>(delegate);
        if (adc)
            m_delegateChooser = adc;
    }

    m_delegate = delegate;
}

const QAbstractItemModel *QQmlTableInstanceModel::abstractItemModel() const
{
    return m_adaptorModel.adaptsAim() ? m_adaptorModel.aim() : nullptr;
}

// --------------------------------------------------------

void QQmlTableInstanceModelIncubationTask::setInitialState(QObject *object)
{
    modelItemToIncubate->object = object;
    emit tableInstanceModel->initItem(modelItemToIncubate->index, object);
}

void QQmlTableInstanceModelIncubationTask::statusChanged(QQmlIncubator::Status status)
{
    if (!QQmlTableInstanceModel::isDoneIncubating(modelItemToIncubate))
        return;

    // We require the view to cancel any ongoing load
    // requests before the tableInstanceModel is destructed.
    Q_ASSERT(tableInstanceModel);

    tableInstanceModel->incubatorStatusChanged(this, status);
}

#include "moc_qqmltableinstancemodel_p.cpp"

QT_END_NAMESPACE

