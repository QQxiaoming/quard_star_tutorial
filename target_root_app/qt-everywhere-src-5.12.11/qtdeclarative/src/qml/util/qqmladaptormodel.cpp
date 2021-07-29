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

#include "qqmladaptormodel_p.h"

#include <private/qqmldelegatemodel_p_p.h>
#include <private/qmetaobjectbuilder_p.h>
#include <private/qqmlproperty_p.h>
#include <private/qv8engine_p.h>

#include <private/qv4value_p.h>
#include <private/qv4functionobject_p.h>

QT_BEGIN_NAMESPACE

class QQmlAdaptorModelEngineData : public QV8Engine::Deletable
{
public:
    QQmlAdaptorModelEngineData(QV4::ExecutionEngine *v4);
    ~QQmlAdaptorModelEngineData();

    QV4::ExecutionEngine *v4;
    QV4::PersistentValue listItemProto;
};

V4_DEFINE_EXTENSION(QQmlAdaptorModelEngineData, engineData)

static QV4::ReturnedValue get_index(const QV4::FunctionObject *f, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(f);
    QV4::Scoped<QQmlDelegateModelItemObject> o(scope, thisObject->as<QQmlDelegateModelItemObject>());
    if (!o)
        RETURN_RESULT(scope.engine->throwTypeError(QStringLiteral("Not a valid DelegateModel object")));

    RETURN_RESULT(QV4::Encode(o->d()->item->index));
}

template <typename T, typename M> static void setModelDataType(QMetaObjectBuilder *builder, M *metaType)
{
    builder->setFlags(QMetaObjectBuilder::DynamicMetaObject);
    builder->setClassName(T::staticMetaObject.className());
    builder->setSuperClass(&T::staticMetaObject);
    metaType->propertyOffset = T::staticMetaObject.propertyCount();
    metaType->signalOffset = T::staticMetaObject.methodCount();
}

static void addProperty(QMetaObjectBuilder *builder, int propertyId, const QByteArray &propertyName, const QByteArray &propertyType)
{
    builder->addSignal("__" + QByteArray::number(propertyId) + "()");
    QMetaPropertyBuilder property = builder->addProperty(
            propertyName, propertyType, propertyId);
    property.setWritable(true);
}

class VDMModelDelegateDataType;

class QQmlDMCachedModelData : public QQmlDelegateModelItem
{
public:
    QQmlDMCachedModelData(
            QQmlDelegateModelItemMetaType *metaType,
            VDMModelDelegateDataType *dataType,
            int index, int row, int column);

    int metaCall(QMetaObject::Call call, int id, void **arguments);

    virtual QVariant value(int role) const = 0;
    virtual void setValue(int role, const QVariant &value) = 0;

    void setValue(const QString &role, const QVariant &value) override;
    bool resolveIndex(const QQmlAdaptorModel &model, int idx) override;

    static QV4::ReturnedValue get_property(const QV4::FunctionObject *, const QV4::Value *thisObject, const QV4::Value *argv, int argc);
    static QV4::ReturnedValue set_property(const QV4::FunctionObject *, const QV4::Value *thisObject, const QV4::Value *argv, int argc);

    VDMModelDelegateDataType *type;
    QVector<QVariant> cachedData;
};

class VDMModelDelegateDataType
        : public QQmlRefCount
        , public QQmlAdaptorModel::Accessors
        , public QAbstractDynamicMetaObject
{
public:
    VDMModelDelegateDataType(QQmlAdaptorModel *model)
        : model(model)
        , propertyOffset(0)
        , signalOffset(0)
        , hasModelData(false)
    {
    }

    bool notify(
            const QQmlAdaptorModel &,
            const QList<QQmlDelegateModelItem *> &items,
            int index,
            int count,
            const QVector<int> &roles) const override
    {
        bool changed = roles.isEmpty() && !watchedRoles.isEmpty();
        if (!changed && !watchedRoles.isEmpty() && watchedRoleIds.isEmpty()) {
            QList<int> roleIds;
            for (const QByteArray &r : watchedRoles) {
                QHash<QByteArray, int>::const_iterator it = roleNames.find(r);
                if (it != roleNames.end())
                    roleIds << it.value();
            }
            const_cast<VDMModelDelegateDataType *>(this)->watchedRoleIds = roleIds;
        }

        QVector<int> signalIndexes;
        for (int i = 0; i < roles.count(); ++i) {
            const int role = roles.at(i);
            if (!changed && watchedRoleIds.contains(role))
                changed = true;

            int propertyId = propertyRoles.indexOf(role);
            if (propertyId != -1)
                signalIndexes.append(propertyId + signalOffset);
        }
        if (roles.isEmpty()) {
            const int propertyRolesCount = propertyRoles.count();
            signalIndexes.reserve(propertyRolesCount);
            for (int propertyId = 0; propertyId < propertyRolesCount; ++propertyId)
                signalIndexes.append(propertyId + signalOffset);
        }

        QVarLengthArray<QQmlGuard<QQmlDelegateModelItem>> guardedItems;
        for (const auto item : items)
            guardedItems.append(item);

        for (const auto &item : qAsConst(guardedItems)) {
            if (item.isNull())
                continue;

            const int idx = item->modelIndex();
            if (idx >= index && idx < index + count) {
                for (int i = 0; i < signalIndexes.count(); ++i)
                    QMetaObject::activate(item, signalIndexes.at(i), nullptr);
            }
        }
        return changed;
    }

    void replaceWatchedRoles(
            QQmlAdaptorModel &,
            const QList<QByteArray> &oldRoles,
            const QList<QByteArray> &newRoles) const override
    {
        VDMModelDelegateDataType *dataType = const_cast<VDMModelDelegateDataType *>(this);

        dataType->watchedRoleIds.clear();
        for (const QByteArray &oldRole : oldRoles)
            dataType->watchedRoles.removeOne(oldRole);
        dataType->watchedRoles += newRoles;
    }

    static QV4::ReturnedValue get_hasModelChildren(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
    {
        QV4::Scope scope(b);
        QV4::Scoped<QQmlDelegateModelItemObject> o(scope, thisObject->as<QQmlDelegateModelItemObject>());
        if (!o)
            RETURN_RESULT(scope.engine->throwTypeError(QStringLiteral("Not a valid DelegateModel object")));

        const QQmlAdaptorModel *const model = static_cast<QQmlDMCachedModelData *>(o->d()->item)->type->model;
        if (o->d()->item->index >= 0 && *model) {
            const QAbstractItemModel * const aim = model->aim();
            RETURN_RESULT(QV4::Encode(aim->hasChildren(aim->index(o->d()->item->index, 0, model->rootIndex))));
        } else {
            RETURN_RESULT(QV4::Encode(false));
        }
    }


    void initializeConstructor(QQmlAdaptorModelEngineData *const data)
    {
        QV4::ExecutionEngine *v4 = data->v4;
        QV4::Scope scope(v4);
        QV4::ScopedObject proto(scope, v4->newObject());
        proto->defineAccessorProperty(QStringLiteral("index"), get_index, nullptr);
        proto->defineAccessorProperty(QStringLiteral("hasModelChildren"), get_hasModelChildren, nullptr);
        QV4::ScopedProperty p(scope);

        typedef QHash<QByteArray, int>::const_iterator iterator;
        for (iterator it = roleNames.constBegin(), end = roleNames.constEnd(); it != end; ++it) {
            const int propertyId = propertyRoles.indexOf(it.value());
            const QByteArray &propertyName = it.key();

            QV4::ScopedString name(scope, v4->newString(QString::fromUtf8(propertyName)));
            QV4::ExecutionContext *global = v4->rootContext();
            QV4::ScopedFunctionObject g(scope, v4->memoryManager->allocate<QV4::IndexedBuiltinFunction>(global, propertyId, QQmlDMCachedModelData::get_property));
            QV4::ScopedFunctionObject s(scope, v4->memoryManager->allocate<QV4::IndexedBuiltinFunction>(global, propertyId, QQmlDMCachedModelData::set_property));
            p->setGetter(g);
            p->setSetter(s);
            proto->insertMember(name, p, QV4::Attr_Accessor|QV4::Attr_NotEnumerable|QV4::Attr_NotConfigurable);
        }
        prototype.set(v4, proto);
    }

    // QAbstractDynamicMetaObject

    void objectDestroyed(QObject *) override
    {
        release();
    }

    int metaCall(QObject *object, QMetaObject::Call call, int id, void **arguments) override
    {
        return static_cast<QQmlDMCachedModelData *>(object)->metaCall(call, id, arguments);
    }

    QV4::PersistentValue prototype;
    QList<int> propertyRoles;
    QList<int> watchedRoleIds;
    QList<QByteArray> watchedRoles;
    QHash<QByteArray, int> roleNames;
    QQmlAdaptorModel *model;
    int propertyOffset;
    int signalOffset;
    bool hasModelData;
};

QQmlDMCachedModelData::QQmlDMCachedModelData(QQmlDelegateModelItemMetaType *metaType, VDMModelDelegateDataType *dataType, int index, int row, int column)
    : QQmlDelegateModelItem(metaType, dataType, index, row, column)
    , type(dataType)
{
    if (index == -1)
        cachedData.resize(type->hasModelData ? 1 : type->propertyRoles.count());

    QObjectPrivate::get(this)->metaObject = type;

    type->addref();
}

int QQmlDMCachedModelData::metaCall(QMetaObject::Call call, int id, void **arguments)
{
    if (call == QMetaObject::ReadProperty && id >= type->propertyOffset) {
        const int propertyIndex = id - type->propertyOffset;
        if (index == -1) {
            if (!cachedData.isEmpty()) {
                *static_cast<QVariant *>(arguments[0]) = cachedData.at(
                    type->hasModelData ? 0 : propertyIndex);
            }
        } else  if (*type->model) {
            *static_cast<QVariant *>(arguments[0]) = value(type->propertyRoles.at(propertyIndex));
        }
        return -1;
    } else if (call == QMetaObject::WriteProperty && id >= type->propertyOffset) {
        const int propertyIndex = id - type->propertyOffset;
        if (index == -1) {
            const QMetaObject *meta = metaObject();
            if (cachedData.count() > 1) {
                cachedData[propertyIndex] = *static_cast<QVariant *>(arguments[0]);
                QMetaObject::activate(this, meta, propertyIndex, nullptr);
            } else if (cachedData.count() == 1) {
                cachedData[0] = *static_cast<QVariant *>(arguments[0]);
                QMetaObject::activate(this, meta, 0, nullptr);
                QMetaObject::activate(this, meta, 1, nullptr);
            }
        } else if (*type->model) {
            setValue(type->propertyRoles.at(propertyIndex), *static_cast<QVariant *>(arguments[0]));
        }
        return -1;
    } else {
        return qt_metacall(call, id, arguments);
    }
}

void QQmlDMCachedModelData::setValue(const QString &role, const QVariant &value)
{
    QHash<QByteArray, int>::iterator it = type->roleNames.find(role.toUtf8());
    if (it != type->roleNames.end()) {
        for (int i = 0; i < type->propertyRoles.count(); ++i) {
            if (type->propertyRoles.at(i) == *it) {
                cachedData[i] = value;
                return;
            }
        }
    }
}

bool QQmlDMCachedModelData::resolveIndex(const QQmlAdaptorModel &adaptorModel, int idx)
{
    if (index == -1) {
        Q_ASSERT(idx >= 0);
        cachedData.clear();
        setModelIndex(idx, adaptorModel.rowAt(idx), adaptorModel.columnAt(idx));
        const QMetaObject *meta = metaObject();
        const int propertyCount = type->propertyRoles.count();
        for (int i = 0; i < propertyCount; ++i)
            QMetaObject::activate(this, meta, i, nullptr);
        return true;
    } else {
        return false;
    }
}

QV4::ReturnedValue QQmlDMCachedModelData::get_property(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
{
    QV4::Scope scope(b);
    QV4::Scoped<QQmlDelegateModelItemObject> o(scope, thisObject->as<QQmlDelegateModelItemObject>());
    if (!o)
        return scope.engine->throwTypeError(QStringLiteral("Not a valid DelegateModel object"));

    uint propertyId = static_cast<const QV4::IndexedBuiltinFunction *>(b)->d()->index;

    QQmlDMCachedModelData *modelData = static_cast<QQmlDMCachedModelData *>(o->d()->item);
    if (o->d()->item->index == -1) {
        if (!modelData->cachedData.isEmpty()) {
            return scope.engine->fromVariant(
                    modelData->cachedData.at(modelData->type->hasModelData ? 0 : propertyId));
        }
    } else if (*modelData->type->model) {
        return scope.engine->fromVariant(
                modelData->value(modelData->type->propertyRoles.at(propertyId)));
    }
    return QV4::Encode::undefined();
}

QV4::ReturnedValue QQmlDMCachedModelData::set_property(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::Scoped<QQmlDelegateModelItemObject> o(scope, thisObject->as<QQmlDelegateModelItemObject>());
    if (!o)
        return scope.engine->throwTypeError(QStringLiteral("Not a valid DelegateModel object"));
    if (!argc)
        return scope.engine->throwTypeError();

    uint propertyId = static_cast<const QV4::IndexedBuiltinFunction *>(b)->d()->index;

    if (o->d()->item->index == -1) {
        QQmlDMCachedModelData *modelData = static_cast<QQmlDMCachedModelData *>(o->d()->item);
        if (!modelData->cachedData.isEmpty()) {
            if (modelData->cachedData.count() > 1) {
                modelData->cachedData[propertyId] = scope.engine->toVariant(argv[0], QVariant::Invalid);
                QMetaObject::activate(o->d()->item, o->d()->item->metaObject(), propertyId, nullptr);
            } else if (modelData->cachedData.count() == 1) {
                modelData->cachedData[0] = scope.engine->toVariant(argv[0], QVariant::Invalid);
                QMetaObject::activate(o->d()->item, o->d()->item->metaObject(), 0, nullptr);
                QMetaObject::activate(o->d()->item, o->d()->item->metaObject(), 1, nullptr);
            }
        }
    }
    return QV4::Encode::undefined();
}

//-----------------------------------------------------------------
// QAbstractItemModel
//-----------------------------------------------------------------

class QQmlDMAbstractItemModelData : public QQmlDMCachedModelData
{
    Q_OBJECT
    Q_PROPERTY(bool hasModelChildren READ hasModelChildren CONSTANT)

public:
    QQmlDMAbstractItemModelData(
            QQmlDelegateModelItemMetaType *metaType,
            VDMModelDelegateDataType *dataType,
            int index, int row, int column)
        : QQmlDMCachedModelData(metaType, dataType, index, row, column)
    {
    }

    bool hasModelChildren() const
    {
        if (index >= 0 && *type->model) {
            const QAbstractItemModel * const model = type->model->aim();
            return model->hasChildren(model->index(row, column, type->model->rootIndex));
        } else {
            return false;
        }
    }

    QVariant value(int role) const override
    {
        return type->model->aim()->index(row, column, type->model->rootIndex).data(role);
    }

    void setValue(int role, const QVariant &value) override
    {
        type->model->aim()->setData(
                type->model->aim()->index(row, column, type->model->rootIndex), value, role);
    }

    QV4::ReturnedValue get() override
    {
        if (type->prototype.isUndefined()) {
            QQmlAdaptorModelEngineData * const data = engineData(v4);
            type->initializeConstructor(data);
        }
        QV4::Scope scope(v4);
        QV4::ScopedObject proto(scope, type->prototype.value());
        QV4::ScopedObject o(scope, proto->engine()->memoryManager->allocate<QQmlDelegateModelItemObject>(this));
        o->setPrototypeOf(proto);
        ++scriptRef;
        return o.asReturnedValue();
    }
};

class VDMAbstractItemModelDataType : public VDMModelDelegateDataType
{
public:
    VDMAbstractItemModelDataType(QQmlAdaptorModel *model)
        : VDMModelDelegateDataType(model)
    {
    }

    int rowCount(const QQmlAdaptorModel &model) const override
    {
        return model.aim()->rowCount(model.rootIndex);
    }

    int columnCount(const QQmlAdaptorModel &model) const override
    {
        return model.aim()->columnCount(model.rootIndex);
    }

    void cleanup(QQmlAdaptorModel &) const override
    {
        const_cast<VDMAbstractItemModelDataType *>(this)->release();
    }

    QVariant value(const QQmlAdaptorModel &model, int index, const QString &role) const override
    {
        QHash<QByteArray, int>::const_iterator it = roleNames.find(role.toUtf8());
        if (it != roleNames.end()) {
            return model.aim()->index(model.rowAt(index), model.columnAt(index), model.rootIndex).data(*it);
        } else if (role == QLatin1String("hasModelChildren")) {
            return QVariant(model.aim()->hasChildren(model.aim()->index(model.rowAt(index), model.columnAt(index), model.rootIndex)));
        } else {
            return QVariant();
        }
    }

    QVariant parentModelIndex(const QQmlAdaptorModel &model) const override
    {
        return model
                ? QVariant::fromValue(model.aim()->parent(model.rootIndex))
                : QVariant();
    }

    QVariant modelIndex(const QQmlAdaptorModel &model, int index) const override
    {
        return model
                ? QVariant::fromValue(model.aim()->index(model.rowAt(index), model.columnAt(index), model.rootIndex))
                : QVariant();
    }

    bool canFetchMore(const QQmlAdaptorModel &model) const override
    {
        return model && model.aim()->canFetchMore(model.rootIndex);
    }

    void fetchMore(QQmlAdaptorModel &model) const override
    {
        if (model)
            model.aim()->fetchMore(model.rootIndex);
    }

    QQmlDelegateModelItem *createItem(
            QQmlAdaptorModel &model,
            QQmlDelegateModelItemMetaType *metaType,
            int index, int row, int column) const override
    {
        VDMAbstractItemModelDataType *dataType = const_cast<VDMAbstractItemModelDataType *>(this);
        if (!metaObject)
            dataType->initializeMetaType(model);
        return new QQmlDMAbstractItemModelData(metaType, dataType, index, row, column);
    }

    void initializeMetaType(QQmlAdaptorModel &model)
    {
        QMetaObjectBuilder builder;
        setModelDataType<QQmlDMAbstractItemModelData>(&builder, this);

        const QByteArray propertyType = QByteArrayLiteral("QVariant");
        const QHash<int, QByteArray> names = model.aim()->roleNames();
        for (QHash<int, QByteArray>::const_iterator it = names.begin(), cend = names.end(); it != cend; ++it) {
            const int propertyId = propertyRoles.count();
            propertyRoles.append(it.key());
            roleNames.insert(it.value(), it.key());
            addProperty(&builder, propertyId, it.value(), propertyType);
        }
        if (propertyRoles.count() == 1) {
            hasModelData = true;
            const int role = names.begin().key();
            const QByteArray propertyName = QByteArrayLiteral("modelData");

            propertyRoles.append(role);
            roleNames.insert(propertyName, role);
            addProperty(&builder, 1, propertyName, propertyType);
        }

        metaObject.reset(builder.toMetaObject());
        *static_cast<QMetaObject *>(this) = *metaObject;
        propertyCache.adopt(new QQmlPropertyCache(metaObject.data(), model.modelItemRevision));
    }
};

//-----------------------------------------------------------------
// QQmlListAccessor
//-----------------------------------------------------------------

class QQmlDMListAccessorData : public QQmlDelegateModelItem
{
    Q_OBJECT
    Q_PROPERTY(QVariant modelData READ modelData WRITE setModelData NOTIFY modelDataChanged)
public:
    QQmlDMListAccessorData(QQmlDelegateModelItemMetaType *metaType,
                           QQmlAdaptorModel::Accessors *accessor,
                           int index, int row, int column, const QVariant &value)
        : QQmlDelegateModelItem(metaType, accessor, index, row, column)
        , cachedData(value)
    {
    }

    QVariant modelData() const
    {
        return cachedData;
    }

    void setModelData(const QVariant &data)
    {
        if (data == cachedData)
            return;

        cachedData = data;
        emit modelDataChanged();
    }

    static QV4::ReturnedValue get_modelData(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *, int)
    {
        QV4::ExecutionEngine *v4 = b->engine();
        const QQmlDelegateModelItemObject *o = thisObject->as<QQmlDelegateModelItemObject>();
        if (!o)
            return v4->throwTypeError(QStringLiteral("Not a valid DelegateModel object"));

        return v4->fromVariant(static_cast<QQmlDMListAccessorData *>(o->d()->item)->cachedData);
    }

    static QV4::ReturnedValue set_modelData(const QV4::FunctionObject *b, const QV4::Value *thisObject, const QV4::Value *argv, int argc)
    {
        QV4::ExecutionEngine *v4 = b->engine();
        const QQmlDelegateModelItemObject *o = thisObject->as<QQmlDelegateModelItemObject>();
        if (!o)
            return v4->throwTypeError(QStringLiteral("Not a valid DelegateModel object"));
        if (!argc)
            return v4->throwTypeError();

        static_cast<QQmlDMListAccessorData *>(o->d()->item)->setModelData(v4->toVariant(argv[0], QVariant::Invalid));
        return QV4::Encode::undefined();
    }

    QV4::ReturnedValue get() override
    {
        QQmlAdaptorModelEngineData *data = engineData(v4);
        QV4::Scope scope(v4);
        QV4::ScopedObject o(scope, v4->memoryManager->allocate<QQmlDelegateModelItemObject>(this));
        QV4::ScopedObject p(scope, data->listItemProto.value());
        o->setPrototypeOf(p);
        ++scriptRef;
        return o.asReturnedValue();
    }

    void setValue(const QString &role, const QVariant &value) override
    {
        if (role == QLatin1String("modelData"))
            cachedData = value;
    }

    bool resolveIndex(const QQmlAdaptorModel &model, int idx) override
    {
        if (index == -1) {
            index = idx;
            cachedData = model.list.at(idx);
            emit modelIndexChanged();
            emit modelDataChanged();
            return true;
        } else {
            return false;
        }
    }


Q_SIGNALS:
    void modelDataChanged();

private:
    QVariant cachedData;
};


class VDMListDelegateDataType : public QQmlRefCount, public QQmlAdaptorModel::Accessors
{
public:
    VDMListDelegateDataType()
        : QQmlRefCount()
        , QQmlAdaptorModel::Accessors()
    {}

    void cleanup(QQmlAdaptorModel &) const override
    {
        const_cast<VDMListDelegateDataType *>(this)->release();
    }

    int rowCount(const QQmlAdaptorModel &model) const override
    {
        return model.list.count();
    }

    int columnCount(const QQmlAdaptorModel &) const override
    {
        return 1;
    }

    QVariant value(const QQmlAdaptorModel &model, int index, const QString &role) const override
    {
        return role == QLatin1String("modelData")
                ? model.list.at(index)
                : QVariant();
    }

    QQmlDelegateModelItem *createItem(
            QQmlAdaptorModel &model,
            QQmlDelegateModelItemMetaType *metaType,
            int index, int row, int column) const override
    {
        VDMListDelegateDataType *dataType = const_cast<VDMListDelegateDataType *>(this);
        if (!propertyCache) {
            dataType->propertyCache.adopt(new QQmlPropertyCache(
                        &QQmlDMListAccessorData::staticMetaObject, model.modelItemRevision));
        }

        return new QQmlDMListAccessorData(
                metaType,
                dataType,
                index, row, column,
                index >= 0 && index < model.list.count() ? model.list.at(index) : QVariant());
    }

    bool notify(const QQmlAdaptorModel &model, const QList<QQmlDelegateModelItem *> &items, int index, int count, const QVector<int> &) const override
    {
        for (auto modelItem : items) {
            const int modelItemIndex = modelItem->index;
            if (modelItemIndex < index || modelItemIndex >= index + count)
                continue;

            auto listModelItem = static_cast<QQmlDMListAccessorData *>(modelItem);
            QVariant updatedModelData = model.list.at(listModelItem->index);
            listModelItem->setModelData(updatedModelData);
        }
        return true;
    }
};

//-----------------------------------------------------------------
// QObject
//-----------------------------------------------------------------

class VDMObjectDelegateDataType;
class QQmlDMObjectData : public QQmlDelegateModelItem, public QQmlAdaptorModelProxyInterface
{
    Q_OBJECT
    Q_PROPERTY(QObject *modelData READ modelData NOTIFY modelDataChanged)
    Q_INTERFACES(QQmlAdaptorModelProxyInterface)
public:
    QQmlDMObjectData(
            QQmlDelegateModelItemMetaType *metaType,
            VDMObjectDelegateDataType *dataType,
            int index, int row, int column,
            QObject *object);

    void setModelData(QObject *modelData)
    {
        if (modelData == object)
            return;

        object = modelData;
        emit modelDataChanged();
    }

    QObject *modelData() const { return object; }
    QObject *proxiedObject() override { return object; }

    QPointer<QObject> object;

Q_SIGNALS:
    void modelDataChanged();
};

class VDMObjectDelegateDataType : public QQmlRefCount, public QQmlAdaptorModel::Accessors
{
public:
    int propertyOffset;
    int signalOffset;
    bool shared;
    QMetaObjectBuilder builder;

    VDMObjectDelegateDataType()
        : propertyOffset(0)
        , signalOffset(0)
        , shared(true)
    {
    }

    VDMObjectDelegateDataType(const VDMObjectDelegateDataType &type)
        : QQmlRefCount()
        , QQmlAdaptorModel::Accessors()
        , propertyOffset(type.propertyOffset)
        , signalOffset(type.signalOffset)
        , shared(false)
        , builder(type.metaObject.data(), QMetaObjectBuilder::Properties
                | QMetaObjectBuilder::Signals
                | QMetaObjectBuilder::SuperClass
                | QMetaObjectBuilder::ClassName)
    {
        builder.setFlags(QMetaObjectBuilder::DynamicMetaObject);
    }

    int rowCount(const QQmlAdaptorModel &model) const override
    {
        return model.list.count();
    }

    int columnCount(const QQmlAdaptorModel &) const override
    {
        return 1;
    }

    QVariant value(const QQmlAdaptorModel &model, int index, const QString &role) const override
    {
        if (QObject *object = model.list.at(index).value<QObject *>())
            return object->property(role.toUtf8());
        return QVariant();
    }

    QQmlDelegateModelItem *createItem(
            QQmlAdaptorModel &model,
            QQmlDelegateModelItemMetaType *metaType,
            int index, int row, int column) const override
    {
        VDMObjectDelegateDataType *dataType = const_cast<VDMObjectDelegateDataType *>(this);
        if (!metaObject)
            dataType->initializeMetaType(model);
        return index >= 0 && index < model.list.count()
                ? new QQmlDMObjectData(metaType, dataType, index, row, column, qvariant_cast<QObject *>(model.list.at(index)))
                : nullptr;
    }

    void initializeMetaType(QQmlAdaptorModel &model)
    {
        Q_UNUSED(model);
        setModelDataType<QQmlDMObjectData>(&builder, this);

        metaObject.reset(builder.toMetaObject());
        // Note: ATM we cannot create a shared property cache for this class, since each model
        // object can have different properties. And to make those properties available to the
        // delegate, QQmlDMObjectData makes use of a QAbstractDynamicMetaObject subclass
        // (QQmlDMObjectDataMetaObject), which we cannot represent in a QQmlPropertyCache.
        // By not having a shared property cache, revisioned properties in QQmlDelegateModelItem
        // will always be available to the delegate, regardless of the import version.
    }

    void cleanup(QQmlAdaptorModel &) const override
    {
        const_cast<VDMObjectDelegateDataType *>(this)->release();
    }

    bool notify(const QQmlAdaptorModel &model, const QList<QQmlDelegateModelItem *> &items, int index, int count, const QVector<int> &) const override
    {
        for (auto modelItem : items) {
            const int modelItemIndex = modelItem->index;
            if (modelItemIndex < index || modelItemIndex >= index + count)
                continue;

            auto objectModelItem = static_cast<QQmlDMObjectData *>(modelItem);
            QObject *updatedModelData = qvariant_cast<QObject *>(model.list.at(objectModelItem->index));
            objectModelItem->setModelData(updatedModelData);
        }
        return true;
    }
};

class QQmlDMObjectDataMetaObject : public QAbstractDynamicMetaObject
{
public:
    QQmlDMObjectDataMetaObject(QQmlDMObjectData *data, VDMObjectDelegateDataType *type)
        : m_data(data)
        , m_type(type)
    {
        QObjectPrivate *op = QObjectPrivate::get(m_data);
        *static_cast<QMetaObject *>(this) = *type->metaObject;
        op->metaObject = this;
        m_type->addref();
    }

    ~QQmlDMObjectDataMetaObject()
    {
        m_type->release();
    }

    int metaCall(QObject *o, QMetaObject::Call call, int id, void **arguments) override
    {
        Q_ASSERT(o == m_data);
        Q_UNUSED(o);

        static const int objectPropertyOffset = QObject::staticMetaObject.propertyCount();
        if (id >= m_type->propertyOffset
                && (call == QMetaObject::ReadProperty
                || call == QMetaObject::WriteProperty
                || call == QMetaObject::ResetProperty)) {
            if (m_data->object)
                QMetaObject::metacall(m_data->object, call, id - m_type->propertyOffset + objectPropertyOffset, arguments);
            return -1;
        } else if (id >= m_type->signalOffset && call == QMetaObject::InvokeMetaMethod) {
            QMetaObject::activate(m_data, this, id - m_type->signalOffset, nullptr);
            return -1;
        } else {
            return m_data->qt_metacall(call, id, arguments);
        }
    }

    int createProperty(const char *name, const char *) override
    {
        if (!m_data->object)
            return -1;
        const QMetaObject *metaObject = m_data->object->metaObject();
        static const int objectPropertyOffset = QObject::staticMetaObject.propertyCount();

        const int previousPropertyCount = propertyCount() - propertyOffset();
        int propertyIndex = metaObject->indexOfProperty(name);
        if (propertyIndex == -1)
            return -1;
        if (previousPropertyCount + objectPropertyOffset == metaObject->propertyCount())
            return propertyIndex + m_type->propertyOffset - objectPropertyOffset;

        if (m_type->shared) {
            VDMObjectDelegateDataType *type = m_type;
            m_type = new VDMObjectDelegateDataType(*m_type);
            type->release();
        }

        const int previousMethodCount = methodCount();
        int notifierId = previousMethodCount - methodOffset();
        for (int propertyId = previousPropertyCount; propertyId < metaObject->propertyCount() - objectPropertyOffset; ++propertyId) {
            QMetaProperty property = metaObject->property(propertyId + objectPropertyOffset);
            QMetaPropertyBuilder propertyBuilder;
            if (property.hasNotifySignal()) {
                m_type->builder.addSignal("__" + QByteArray::number(propertyId) + "()");
                propertyBuilder = m_type->builder.addProperty(property.name(), property.typeName(), notifierId);
                ++notifierId;
            } else {
                propertyBuilder = m_type->builder.addProperty(property.name(), property.typeName());
            }
            propertyBuilder.setWritable(property.isWritable());
            propertyBuilder.setResettable(property.isResettable());
            propertyBuilder.setConstant(property.isConstant());
        }

        m_type->metaObject.reset(m_type->builder.toMetaObject());
        *static_cast<QMetaObject *>(this) = *m_type->metaObject;

        notifierId = previousMethodCount;
        for (int i = previousPropertyCount; i < metaObject->propertyCount() - objectPropertyOffset; ++i) {
            QMetaProperty property = metaObject->property(i + objectPropertyOffset);
            if (property.hasNotifySignal()) {
                QQmlPropertyPrivate::connect(
                        m_data->object, property.notifySignalIndex(), m_data, notifierId);
                ++notifierId;
            }
        }
        return propertyIndex + m_type->propertyOffset - objectPropertyOffset;
    }

    QQmlDMObjectData *m_data;
    VDMObjectDelegateDataType *m_type;
};

QQmlDMObjectData::QQmlDMObjectData(QQmlDelegateModelItemMetaType *metaType,
        VDMObjectDelegateDataType *dataType,
        int index, int row, int column,
        QObject *object)
    : QQmlDelegateModelItem(metaType, dataType, index, row, column)
    , object(object)
{
    new QQmlDMObjectDataMetaObject(this, dataType);
}

//-----------------------------------------------------------------
// QQmlAdaptorModel
//-----------------------------------------------------------------

static const QQmlAdaptorModel::Accessors qt_vdm_null_accessors;

QQmlAdaptorModel::Accessors::~Accessors()
{
}

QQmlAdaptorModel::QQmlAdaptorModel()
    : accessors(&qt_vdm_null_accessors)
{
}

QQmlAdaptorModel::~QQmlAdaptorModel()
{
    accessors->cleanup(*this);
}

void QQmlAdaptorModel::setModel(const QVariant &variant, QObject *parent, QQmlEngine *engine)
{
    accessors->cleanup(*this);

    list.setList(variant, engine);

    if (QObject *object = qvariant_cast<QObject *>(list.list())) {
        setObject(object, parent);
        if (qobject_cast<QAbstractItemModel *>(object))
            accessors = new VDMAbstractItemModelDataType(this);
        else
            accessors = new VDMObjectDelegateDataType;
    } else if (list.type() == QQmlListAccessor::ListProperty) {
        setObject(static_cast<const QQmlListReference *>(variant.constData())->object(), parent);
        accessors = new VDMObjectDelegateDataType;
    } else if (list.type() != QQmlListAccessor::Invalid
            && list.type() != QQmlListAccessor::Instance) { // Null QObject
        setObject(nullptr, parent);
        accessors = new VDMListDelegateDataType;
    } else {
        setObject(nullptr, parent);
        accessors = &qt_vdm_null_accessors;
    }
}

void QQmlAdaptorModel::invalidateModel()
{
    accessors->cleanup(*this);
    accessors = &qt_vdm_null_accessors;
    // Don't clear the model object as we still need the guard to clear the list variant if the
    // object is destroyed.
}

bool QQmlAdaptorModel::isValid() const
{
    return accessors != &qt_vdm_null_accessors;
}

int QQmlAdaptorModel::count() const
{
    return rowCount() * columnCount();
}

int QQmlAdaptorModel::rowCount() const
{
    return qMax(0, accessors->rowCount(*this));
}

int QQmlAdaptorModel::columnCount() const
{
    return qMax(0, accessors->columnCount(*this));
}

int QQmlAdaptorModel::rowAt(int index) const
{
    int count = rowCount();
    return count <= 0 ? -1 : index % count;
}

int QQmlAdaptorModel::columnAt(int index) const
{
    int count = rowCount();
    return count <= 0 ? -1 : index / count;
}

int QQmlAdaptorModel::indexAt(int row, int column) const
{
    return column * rowCount() + row;
}

void QQmlAdaptorModel::useImportVersion(int minorVersion)
{
    modelItemRevision = minorVersion;
}

void QQmlAdaptorModel::objectDestroyed(QObject *)
{
    setModel(QVariant(), nullptr, nullptr);
}

QQmlAdaptorModelEngineData::QQmlAdaptorModelEngineData(QV4::ExecutionEngine *v4)
    : v4(v4)
{
    QV4::Scope scope(v4);
    QV4::ScopedObject proto(scope, v4->newObject());
    proto->defineAccessorProperty(QStringLiteral("index"), get_index, nullptr);
    proto->defineAccessorProperty(QStringLiteral("modelData"),
                                  QQmlDMListAccessorData::get_modelData, QQmlDMListAccessorData::set_modelData);
    listItemProto.set(v4, proto);
}

QQmlAdaptorModelEngineData::~QQmlAdaptorModelEngineData()
{
}

QT_END_NAMESPACE

#include <qqmladaptormodel.moc>
