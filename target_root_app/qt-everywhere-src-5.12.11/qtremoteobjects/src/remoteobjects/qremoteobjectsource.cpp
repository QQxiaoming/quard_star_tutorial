/****************************************************************************
**
** Copyright (C) 2017 Ford Motor Company
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
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

#include "qremoteobjectsource.h"
#include "qremoteobjectsource_p.h"
#include "qremoteobjectnode.h"
#include "qremoteobjectdynamicreplica.h"

#include "qconnectionfactories_p.h"
#include "qremoteobjectsourceio_p.h"
#include "qremoteobjectabstractitemmodeladapter_p.h"

#include <QtCore/qmetaobject.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qabstractitemmodel.h>

#include <algorithm>
#include <iterator>

QT_BEGIN_NAMESPACE

using namespace QRemoteObjectPackets;
using namespace QRemoteObjectStringLiterals;

const int QRemoteObjectSourceBase::qobjectPropertyOffset = QObject::staticMetaObject.propertyCount();
const int QRemoteObjectSourceBase::qobjectMethodOffset = QObject::staticMetaObject.methodCount();
static const QByteArray s_classinfoRemoteobjectSignature(QCLASSINFO_REMOTEOBJECT_SIGNATURE);

QByteArray QtPrivate::qtro_classinfo_signature(const QMetaObject *metaObject)
{
    if (!metaObject)
        return QByteArray{};

    for (int i = metaObject->classInfoOffset(); i < metaObject->classInfoCount(); ++i) {
        auto ci = metaObject->classInfo(i);
        if (s_classinfoRemoteobjectSignature == ci.name())
            return ci.value();
    }
    return QByteArray{};
}

QRemoteObjectSourceBase::QRemoteObjectSourceBase(QObject *obj, Private *d, const SourceApiMap *api,
                                                 QObject *adapter)
    : QObject(obj),
      m_object(obj),
      m_adapter(adapter),
      m_api(api),
      d(d)
{
    if (!obj) {
        qCWarning(QT_REMOTEOBJECT) << "QRemoteObjectSourceBase: Cannot replicate a NULL object" << m_api->name();
        return;
    }

    setConnections();

    const int nChildren = api->m_models.count() + api->m_subclasses.count();
    if (nChildren > 0) {
        QVector<int> roles;
        const int numProperties = api->propertyCount();
        int modelIndex = 0, subclassIndex = 0;
        for (int i = 0; i < numProperties; ++i) {
            if (api->isAdapterProperty(i))
                continue;
            const int index = api->sourcePropertyIndex(i);
            const auto property = m_object->metaObject()->property(index);
            if (QMetaType::typeFlags(property.userType()).testFlag(QMetaType::PointerToQObject)) {
                auto propertyMeta = QMetaType::metaObjectForType(property.userType());
                QObject *child = property.read(m_object).value<QObject *>();
                if (propertyMeta->inherits(&QAbstractItemModel::staticMetaObject)) {
                    const auto modelInfo = api->m_models.at(modelIndex++);
                    QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(child);
                    QAbstractItemAdapterSourceAPI<QAbstractItemModel, QAbstractItemModelSourceAdapter> *modelApi =
                        new QAbstractItemAdapterSourceAPI<QAbstractItemModel, QAbstractItemModelSourceAdapter>(modelInfo.name);
                    if (!model)
                        m_children.insert(i, new QRemoteObjectSource(nullptr, d, modelApi, nullptr));
                    else {
                        roles.clear();
                        const auto knownRoles = model->roleNames();
                        for (auto role : modelInfo.roles.split('|')) {
                            if (role.isEmpty())
                                continue;
                            const int roleIndex = knownRoles.key(role, -1);
                            if (roleIndex == -1) {
                                qCWarning(QT_REMOTEOBJECT) << "Invalid role" << role << "for model" << model->metaObject()->className();
                                qCWarning(QT_REMOTEOBJECT) << "  known roles:" << knownRoles;
                            } else
                                roles << roleIndex;
                        }
                        auto adapter = new QAbstractItemModelSourceAdapter(model, nullptr,
                                                                           roles.isEmpty() ? knownRoles.keys().toVector() : roles);
                        m_children.insert(i, new QRemoteObjectSource(model, d, modelApi, adapter));
                    }
                } else {
                    const auto classApi = api->m_subclasses.at(subclassIndex++);
                    m_children.insert(i, new QRemoteObjectSource(child, d, classApi, nullptr));
                }
            }
        }
    }
}

QRemoteObjectSource::QRemoteObjectSource(QObject *obj, Private *d, const SourceApiMap *api, QObject *adapter)
    : QRemoteObjectSourceBase(obj, d, api, adapter)
    , m_name(api->typeName() == QLatin1String("QAbstractItemModelAdapter") ? MODEL().arg(api->name()) : CLASS().arg(api->name()))
{
    if (obj)
        d->m_sourceIo->registerSource(this);
}

QRemoteObjectRootSource::QRemoteObjectRootSource(QObject *obj, const SourceApiMap *api,
                                                 QObject *adapter, QRemoteObjectSourceIo *sourceIo)
    : QRemoteObjectSourceBase(obj, new Private(sourceIo, this), api, adapter)
    , m_name(api->name())
{
    d->m_sourceIo->registerSource(this);
}

QRemoteObjectSourceBase::~QRemoteObjectSourceBase()
{
    delete m_api;
}

void QRemoteObjectSourceBase::setConnections()
{
    const QMetaObject *meta = m_object->metaObject();
    for (int idx = 0; idx < m_api->signalCount(); ++idx) {
        const int sourceIndex = m_api->sourceSignalIndex(idx);

        // This basically connects the parent Signals (note, all dynamic properties have onChange
        //notifications, thus signals) to us.  Normally each Signal is mapped to a unique index,
        //but since we are forwarding them all, we keep the offset constant.
        //
        //We know no one will inherit from this class, so no need to worry about indices from
        //derived classes.
        const auto target = m_api->isAdapterSignal(idx) ? m_adapter : m_object;
        if (!QMetaObject::connect(target, sourceIndex, this, QRemoteObjectSource::qobjectMethodOffset+idx, Qt::DirectConnection, 0)) {
            qCWarning(QT_REMOTEOBJECT) << "QRemoteObjectSourceBase: QMetaObject::connect returned false. Unable to connect.";
            return;
        }

        qCDebug(QT_REMOTEOBJECT) << "Connection made" << idx << sourceIndex
                                 << (m_api->isAdapterSignal(idx)
                                     ? m_adapter->metaObject()->method(sourceIndex).name()
                                     : meta->method(sourceIndex).name());
    }
}

void QRemoteObjectSourceBase::resetObject(QObject *newObject)
{
    if (m_object)
        m_object->disconnect(this);
    if (m_adapter) {
        m_adapter->disconnect(this);
        delete m_adapter;
        m_adapter = nullptr;
    }
    // We need some dynamic replica specific code here, in case an object had null sub-classes that
    // have been replaced with real objects.  In this case, the ApiMap could be wrong and need updating.
    if (newObject && qobject_cast<QRemoteObjectDynamicReplica *>(newObject) && m_api->isDynamic()) {
        auto api = static_cast<const DynamicApiMap*>(m_api);
        if (api->m_properties[0] == 0) { // 0 is an index into QObject itself, so this isn't a valid QtRO index
            const auto rep = qobject_cast<QRemoteObjectDynamicReplica *>(newObject);
            auto tmp = m_api;
            m_api = new DynamicApiMap(newObject, rep->metaObject(), api->m_name, QLatin1String(rep->metaObject()->className()));
            qCDebug(QT_REMOTEOBJECT) << "  Reset m_api for" << api->m_name << "using new metaObject:" << rep->metaObject()->className();
            delete tmp;
        }
    }

    m_object = newObject;
    auto model = qobject_cast<QAbstractItemModel *>(newObject);
    if (model) {
        d->m_sourceIo->registerSource(this);
        m_adapter = new QAbstractItemModelSourceAdapter(model, nullptr, model->roleNames().keys().toVector());
    }

    setParent(newObject);
    if (newObject)
        setConnections();

    const int nChildren = m_api->m_models.count() + m_api->m_subclasses.count();
    if (nChildren == 0)
        return;

    if (!newObject) {
        for (auto child : m_children)
            child->resetObject(nullptr);
        return;
    }

    for (int i : m_children.keys()) {
        const int index = m_api->sourcePropertyIndex(i);
        const auto property = m_object->metaObject()->property(index);
        QObject *child = property.read(m_object).value<QObject *>();
        m_children[i]->resetObject(child);
    }
}

QRemoteObjectSource::~QRemoteObjectSource()
{
    for (auto it : m_children) {
        // We used QPointers for m_children because we don't control the lifetime of child QObjects
        // Since the this/source QObject's parent is the referenced QObject, it could have already
        // been deleted
        delete it;
    }
}

QRemoteObjectRootSource::~QRemoteObjectRootSource()
{
    for (auto it : m_children) {
        // We used QPointers for m_children because we don't control the lifetime of child QObjects
        // Since the this/source QObject's parent is the referenced QObject, it could have already
        // been deleted
        delete it;
    }
    d->m_sourceIo->unregisterSource(this);
    Q_FOREACH (IoDeviceBase *io, d->m_listeners) {
        removeListener(io, true);
    }
    delete d;
}

QVariantList* QRemoteObjectSourceBase::marshalArgs(int index, void **a)
{
    QVariantList &list = m_marshalledArgs;
    int N = m_api->signalParameterCount(index);
    if (N == 1 && QMetaType::typeFlags(m_api->signalParameterType(index, 0)).testFlag(QMetaType::PointerToQObject))
        N = 0; // Don't try to send pointers, the will be handle by QRO_
    if (list.size() < N)
        list.reserve(N);
    const int minFill = std::min(list.size(), N);
    for (int i = 0; i < minFill; ++i) {
        const int type = m_api->signalParameterType(index, i);
        if (type == QMetaType::QVariant)
            list[i] = *reinterpret_cast<QVariant *>(a[i + 1]);
        else
            list[i] = QVariant(type, a[i + 1]);
    }
    for (int i = list.size(); i < N; ++i) {
        const int type = m_api->signalParameterType(index, i);
        if (type == QMetaType::QVariant)
            list << *reinterpret_cast<QVariant *>(a[i + 1]);
        else
            list << QVariant(type, a[i + 1]);
    }
    for (int i = N; i < list.size(); ++i)
        list.removeLast();
    return &m_marshalledArgs;
}

bool QRemoteObjectSourceBase::invoke(QMetaObject::Call c, int index, const QVariantList &args, QVariant* returnValue)
{
    int status = -1;
    int flags = 0;
    bool forAdapter = (c == QMetaObject::InvokeMetaMethod ? m_api->isAdapterMethod(index) : m_api->isAdapterProperty(index));
    int resolvedIndex = (c == QMetaObject::InvokeMetaMethod ? m_api->sourceMethodIndex(index) : m_api->sourcePropertyIndex(index));
    if (resolvedIndex < 0)
        return false;
    QVarLengthArray<void*, 10> param(args.size() + 1);

    if (c == QMetaObject::InvokeMetaMethod) {
        QMetaMethod method;
        if (!forAdapter)
            method = parent()->metaObject()->method(resolvedIndex);

        if (returnValue) {
            if (!forAdapter && method.isValid() && method.returnType() == QMetaType::QVariant)
                param[0] = const_cast<void*>(reinterpret_cast<const void*>(returnValue));
            else
                param[0] = returnValue->data();
        } else {
            param[0] = nullptr;
        }

        auto argument = [&](int i) -> void * {
            if ((forAdapter && m_api->methodParameterType(index, i) == QMetaType::QVariant) ||
                    (method.isValid() && method.parameterType(i) == QMetaType::QVariant)) {
                return const_cast<void*>(reinterpret_cast<const void*>(&args.at(i)));
            }
            return const_cast<void*>(args.at(i).data());
        };

        for (int i = 0; i < args.size(); ++i) {
            param[i + 1] = argument(i);
        }
    } else if (c == QMetaObject::WriteProperty || c == QMetaObject::ReadProperty) {
        bool isQVariant = !forAdapter && parent()->metaObject()->property(resolvedIndex).userType() == QMetaType::QVariant;
        for (int i = 0; i < args.size(); ++i) {
            if (isQVariant)
                param[i] = const_cast<void*>(reinterpret_cast<const void*>(&args.at(i)));
            else
                param[i] = const_cast<void*>(args.at(i).data());
        }
        if (c == QMetaObject::WriteProperty) {
            Q_ASSERT(param.size() == 2); // for return-value and setter value
            // check QMetaProperty::write for an explanation of these
            param.append(&status);
            param.append(&flags);
        }
    } else {
        // Better safe than sorry
        return false;
    }
    int r = -1;
    if (forAdapter)
        r = m_adapter->qt_metacall(c, resolvedIndex, param.data());
    else
        r = parent()->qt_metacall(c, resolvedIndex, param.data());
    return r == -1 && status == -1;
}

void QRemoteObjectSourceBase::handleMetaCall(int index, QMetaObject::Call call, void **a)
{
    if (d->m_listeners.empty())
        return;

    int propertyIndex = m_api->propertyIndexFromSignal(index);
    if (propertyIndex >= 0) {
        const int internalIndex = m_api->propertyRawIndexFromSignal(index);
        const auto target = m_api->isAdapterProperty(internalIndex) ? m_adapter : m_object;
        const QMetaProperty mp = target->metaObject()->property(propertyIndex);
        qCDebug(QT_REMOTEOBJECT) << "Sending Invoke Property" << (m_api->isAdapterSignal(internalIndex) ? "via adapter" : "") << internalIndex << propertyIndex << mp.name() << mp.read(target);

        serializePropertyChangePacket(this, index);
        d->m_packet.baseAddress = d->m_packet.size;
        propertyIndex = internalIndex;
    }

    qCDebug(QT_REMOTEOBJECT) << "# Listeners" << d->m_listeners.length();
    qCDebug(QT_REMOTEOBJECT) << "Invoke args:" << m_object
                             << (call == 0 ? QLatin1String("InvokeMetaMethod") : QStringLiteral("Non-invoked call: %d").arg(call))
                             << m_api->signalSignature(index) << *marshalArgs(index, a);

    serializeInvokePacket(d->m_packet, name(), call, index, *marshalArgs(index, a), -1, propertyIndex);
    d->m_packet.baseAddress = 0;

    Q_FOREACH (IoDeviceBase *io, d->m_listeners)
        io->write(d->m_packet.array, d->m_packet.size);
}

void QRemoteObjectRootSource::addListener(IoDeviceBase *io, bool dynamic)
{
    d->m_listeners.append(io);
    d->isDynamic = d->isDynamic || dynamic;

    if (dynamic) {
        d->sentTypes.clear();
        serializeInitDynamicPacket(d->m_packet, this);
        io->write(d->m_packet.array, d->m_packet.size);
    } else {
        serializeInitPacket(d->m_packet, this);
        io->write(d->m_packet.array, d->m_packet.size);
    }
}

int QRemoteObjectRootSource::removeListener(IoDeviceBase *io, bool shouldSendRemove)
{
    d->m_listeners.removeAll(io);
    if (shouldSendRemove)
    {
        serializeRemoveObjectPacket(d->m_packet, m_api->name());
        io->write(d->m_packet.array, d->m_packet.size);
    }
    return d->m_listeners.length();
}

int QRemoteObjectSourceBase::qt_metacall(QMetaObject::Call call, int methodId, void **a)
{
    methodId = QObject::qt_metacall(call, methodId, a);
    if (methodId < 0)
        return methodId;

    if (call == QMetaObject::InvokeMetaMethod)
        handleMetaCall(methodId, call, a);

    return -1;
}

DynamicApiMap::DynamicApiMap(QObject *object, const QMetaObject *metaObject, const QString &name, const QString &typeName)
    : m_name(name),
      m_typeName(typeName),
      m_metaObject(metaObject),
      m_cachedMetamethodIndex(-1)
{
    m_enumOffset = metaObject->enumeratorOffset();
    m_enumCount = metaObject->enumeratorCount() - m_enumOffset;

    const int propCount = metaObject->propertyCount();
    const int propOffset = metaObject->propertyOffset();
    m_properties.reserve(propCount-propOffset);
    int i = 0;
    for (i = propOffset; i < propCount; ++i) {
        const QMetaProperty property = metaObject->property(i);
        if (QMetaType::typeFlags(property.userType()).testFlag(QMetaType::PointerToQObject)) {
            auto propertyMeta = QMetaType::metaObjectForType(property.userType());
            QObject *child = property.read(object).value<QObject *>();
            if (propertyMeta->inherits(&QAbstractItemModel::staticMetaObject)) {
                const QByteArray name = QByteArray::fromRawData(property.name(),
                                                                qstrlen(property.name()));
                const QByteArray infoName = name.toUpper() + QByteArrayLiteral("_ROLES");
                const int infoIndex = metaObject->indexOfClassInfo(infoName.constData());
                QByteArray roleInfo;
                if (infoIndex >= 0) {
                    auto ci = metaObject->classInfo(infoIndex);
                    roleInfo = QByteArray::fromRawData(ci.value(), qstrlen(ci.value()));
                }
                m_models << ModelInfo({qobject_cast<QAbstractItemModel *>(child),
                                       QString::fromLatin1(property.name()),
                                       roleInfo});
            } else {
                const QMetaObject *meta = child ? child->metaObject() : propertyMeta;
                QString typeName = QtRemoteObjects::getTypeNameAndMetaobjectFromClassInfo(meta);
                if (typeName.isNull()) {
                    typeName = QString::fromLatin1(propertyMeta->className());
                    // TODO better way to ensure we have consistent typenames between source/replicas?
                    if (typeName.endsWith(QStringLiteral("Source")))
                        typeName.chop(6);
                }

                m_subclasses << new DynamicApiMap(child, meta, QString::fromLatin1(property.name()), typeName);
            }
        }
        m_properties << i;
        const int notifyIndex = metaObject->property(i).notifySignalIndex();
        if (notifyIndex != -1) {
            m_signals << notifyIndex;
            m_propertyAssociatedWithSignal.append(i-propOffset);
            //The starting values of _signals will be the notify signals
            //So if we are processing _signal with index i, api->sourcePropertyIndex(_propertyAssociatedWithSignal.at(i))
            //will be the property that changed.  This is only valid if i < _propertyAssociatedWithSignal.size().
        }
    }
    const int methodCount = metaObject->methodCount();
    const int methodOffset = metaObject->methodOffset();
    for (i = methodOffset; i < methodCount; ++i) {
        const QMetaMethod mm = metaObject->method(i);
        const QMetaMethod::MethodType m = mm.methodType();
        if (m == QMetaMethod::Signal) {
            if (m_signals.indexOf(i) >= 0) //Already added as a property notifier
                continue;
            m_signals << i;
        } else if (m == QMetaMethod::Slot || m == QMetaMethod::Method)
            m_methods << i;
    }

    m_objectSignature = QtPrivate::qtro_classinfo_signature(metaObject);
}

QList<QByteArray> DynamicApiMap::signalParameterNames(int index) const
{
    const int objectIndex = m_signals.at(index);
    checkCache(objectIndex);
    return m_cachedMetamethod.parameterNames();
}

int DynamicApiMap::parameterCount(int objectIndex) const
{
    checkCache(objectIndex);
    return m_cachedMetamethod.parameterCount();
}

int DynamicApiMap::parameterType(int objectIndex, int paramIndex) const
{
    checkCache(objectIndex);
    return m_cachedMetamethod.parameterType(paramIndex);
}

const QByteArray DynamicApiMap::signature(int objectIndex) const
{
    checkCache(objectIndex);
    return m_cachedMetamethod.methodSignature();
}

QMetaMethod::MethodType DynamicApiMap::methodType(int index) const
{
    const int objectIndex = m_methods.at(index);
    checkCache(objectIndex);
    return m_cachedMetamethod.methodType();
}

const QByteArray DynamicApiMap::typeName(int index) const
{
    const int objectIndex = m_methods.at(index);
    checkCache(objectIndex);
    return m_cachedMetamethod.typeName();
}

QList<QByteArray> DynamicApiMap::methodParameterNames(int index) const
{
    const int objectIndex = m_methods.at(index);
    checkCache(objectIndex);
    return m_cachedMetamethod.parameterNames();
}

QT_END_NAMESPACE
