/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qqmlopenmetaobject_p.h"
#include <private/qqmlpropertycache_p.h>
#include <private/qqmldata_p.h>
#include <private/qmetaobjectbuilder_p.h>
#include <private/qv8engine_p.h>
#include <qqmlengine.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE


class QQmlOpenMetaObjectTypePrivate
{
public:
    QQmlOpenMetaObjectTypePrivate() : mem(nullptr), cache(nullptr), engine(nullptr) {}

    void init(const QMetaObject *metaObj);

    int propertyOffset;
    int signalOffset;
    QHash<QByteArray, int> names;
    QMetaObjectBuilder mob;
    QMetaObject *mem;
    QQmlPropertyCache *cache;
    QQmlEngine *engine;
    QSet<QQmlOpenMetaObject*> referers;
};

QQmlOpenMetaObjectType::QQmlOpenMetaObjectType(const QMetaObject *base, QQmlEngine *engine)
    : QQmlCleanup(engine), d(new QQmlOpenMetaObjectTypePrivate)
{
    d->engine = engine;
    d->init(base);
}

QQmlOpenMetaObjectType::~QQmlOpenMetaObjectType()
{
    if (d->mem)
        free(d->mem);
    if (d->cache)
        d->cache->release();
    delete d;
}

void QQmlOpenMetaObjectType::clear()
{
    d->engine = nullptr;
}

int QQmlOpenMetaObjectType::propertyOffset() const
{
    return d->propertyOffset;
}

int QQmlOpenMetaObjectType::signalOffset() const
{
    return d->signalOffset;
}

int QQmlOpenMetaObjectType::propertyCount() const
{
    return d->names.count();
}

QByteArray QQmlOpenMetaObjectType::propertyName(int idx) const
{
    Q_ASSERT(idx >= 0 && idx < d->names.count());

    return d->mob.property(idx).name();
}

QMetaObject *QQmlOpenMetaObjectType::metaObject() const
{
    return d->mem;
}

void QQmlOpenMetaObjectType::createProperties(const QVector<QByteArray> &names)
{
    for (int i = 0; i < names.count(); ++i) {
        const QByteArray &name = names.at(i);
        const int id = d->mob.propertyCount();
        d->mob.addSignal("__" + QByteArray::number(id) + "()");
        QMetaPropertyBuilder build = d->mob.addProperty(name, "QVariant", id);
        propertyCreated(id, build);
        d->names.insert(name, id);
    }
    free(d->mem);
    d->mem = d->mob.toMetaObject();
    QSet<QQmlOpenMetaObject*>::iterator it = d->referers.begin();
    while (it != d->referers.end()) {
        QQmlOpenMetaObject *omo = *it;
        *static_cast<QMetaObject *>(omo) = *d->mem;
        if (d->cache)
            d->cache->update(omo);
        ++it;
    }
}

int QQmlOpenMetaObjectType::createProperty(const QByteArray &name)
{
    int id = d->mob.propertyCount();
    d->mob.addSignal("__" + QByteArray::number(id) + "()");
    QMetaPropertyBuilder build = d->mob.addProperty(name, "QVariant", id);
    propertyCreated(id, build);
    free(d->mem);
    d->mem = d->mob.toMetaObject();
    d->names.insert(name, id);
    QSet<QQmlOpenMetaObject*>::iterator it = d->referers.begin();
    while (it != d->referers.end()) {
        QQmlOpenMetaObject *omo = *it;
        *static_cast<QMetaObject *>(omo) = *d->mem;
        if (d->cache)
            d->cache->update(omo);
        ++it;
    }

    return d->propertyOffset + id;
}

void QQmlOpenMetaObjectType::propertyCreated(int id, QMetaPropertyBuilder &builder)
{
    if (d->referers.count())
        (*d->referers.begin())->propertyCreated(id, builder);
}

void QQmlOpenMetaObjectTypePrivate::init(const QMetaObject *metaObj)
{
    if (!mem) {
        mob.setSuperClass(metaObj);
        mob.setClassName(metaObj->className());
        mob.setFlags(QMetaObjectBuilder::DynamicMetaObject);

        mem = mob.toMetaObject();

        propertyOffset = mem->propertyOffset();
        signalOffset = mem->methodOffset();
    }
}

//----------------------------------------------------------------------------

class QQmlOpenMetaObjectPrivate
{
public:
    QQmlOpenMetaObjectPrivate(QQmlOpenMetaObject *_q, bool _autoCreate, QObject *obj)
        : q(_q), object(obj), autoCreate(_autoCreate) {}

    struct Property {
    private:
        QVariant m_value;
        QPointer<QObject> qobjectTracker;
    public:
        bool valueSet = false;

        QVariant value() const {
            if (QMetaType::typeFlags(m_value.userType()) & QMetaType::PointerToQObject
                && qobjectTracker.isNull())
                return QVariant::fromValue<QObject*>(nullptr);
            return m_value;
        }
        QVariant &valueRef() { return m_value; }
        void setValue(const QVariant &v) {
            m_value = v;
            valueSet = true;
            if (QMetaType::typeFlags(v.userType()) & QMetaType::PointerToQObject)
                qobjectTracker = m_value.value<QObject*>();
        }
    };

    inline void setPropertyValue(int idx, const QVariant &value) {
        if (data.count() <= idx)
            data.resize(idx + 1);
        data[idx].setValue(value);
    }

    inline Property &propertyRef(int idx) {
        if (data.count() <= idx)
            data.resize(idx + 1);
        Property &prop = data[idx];
        if (!prop.valueSet)
            prop.setValue(q->initialValue(idx));
        return prop;
    }

    inline QVariant propertyValue(int idx) {
        auto &prop = propertyRef(idx);
        return prop.value();
    }

    inline QVariant &propertyValueRef(int idx) {
        auto &prop = propertyRef(idx);
        return prop.valueRef();
    }

    inline bool hasProperty(int idx) const {
        if (idx >= data.count())
            return false;
        return data[idx].valueSet;
    }

    QQmlOpenMetaObject *q;
    QAbstractDynamicMetaObject *parent = nullptr;
    QVector<Property> data;
    QObject *object;
    QQmlRefPointer<QQmlOpenMetaObjectType> type;
    bool autoCreate;
    bool cacheProperties = false;
};

QQmlOpenMetaObject::QQmlOpenMetaObject(QObject *obj, const QMetaObject *base, bool automatic)
: d(new QQmlOpenMetaObjectPrivate(this, automatic, obj))
{
    d->type.adopt(new QQmlOpenMetaObjectType(base ? base : obj->metaObject(), nullptr));
    d->type->d->referers.insert(this);

    QObjectPrivate *op = QObjectPrivate::get(obj);
    d->parent = static_cast<QAbstractDynamicMetaObject *>(op->metaObject);
    *static_cast<QMetaObject *>(this) = *d->type->d->mem;
    op->metaObject = this;
}

QQmlOpenMetaObject::QQmlOpenMetaObject(QObject *obj, QQmlOpenMetaObjectType *type, bool automatic)
: d(new QQmlOpenMetaObjectPrivate(this, automatic, obj))
{
    d->type = type;
    d->type->d->referers.insert(this);

    QObjectPrivate *op = QObjectPrivate::get(obj);
    d->parent = static_cast<QAbstractDynamicMetaObject *>(op->metaObject);
    *static_cast<QMetaObject *>(this) = *d->type->d->mem;
    op->metaObject = this;
}

QQmlOpenMetaObject::~QQmlOpenMetaObject()
{
    if (d->parent)
        delete d->parent;
    d->type->d->referers.remove(this);
    delete d;
}

QQmlOpenMetaObjectType *QQmlOpenMetaObject::type() const
{
    return d->type.data();
}

void QQmlOpenMetaObject::emitPropertyNotification(const QByteArray &propertyName)
{
    QHash<QByteArray, int>::ConstIterator iter = d->type->d->names.constFind(propertyName);
    if (iter == d->type->d->names.constEnd())
        return;
    activate(d->object, *iter + d->type->d->signalOffset, nullptr);
}

int QQmlOpenMetaObject::metaCall(QObject *o, QMetaObject::Call c, int id, void **a)
{
    Q_ASSERT(d->object == o);

    if (( c == QMetaObject::ReadProperty || c == QMetaObject::WriteProperty)
            && id >= d->type->d->propertyOffset) {
        int propId = id - d->type->d->propertyOffset;
        if (c == QMetaObject::ReadProperty) {
            propertyRead(propId);
            *reinterpret_cast<QVariant *>(a[0]) = d->propertyValue(propId);
        } else if (c == QMetaObject::WriteProperty) {
            if (propId >= d->data.count() || d->data.at(propId).value() != *reinterpret_cast<QVariant *>(a[0]))  {
                propertyWrite(propId);
                d->setPropertyValue(propId, propertyWriteValue(propId, *reinterpret_cast<QVariant *>(a[0])));
                propertyWritten(propId);
                activate(o, d->type->d->signalOffset + propId, nullptr);
            }
        }
        return -1;
    } else {
        if (d->parent)
            return d->parent->metaCall(o, c, id, a);
        else
            return o->qt_metacall(c, id, a);
    }
}

QAbstractDynamicMetaObject *QQmlOpenMetaObject::parent() const
{
    return d->parent;
}

QVariant QQmlOpenMetaObject::value(int id) const
{
    return d->propertyValue(id);
}

void QQmlOpenMetaObject::setValue(int id, const QVariant &value)
{
    d->setPropertyValue(id, propertyWriteValue(id, value));
    activate(d->object, id + d->type->d->signalOffset, nullptr);
}

QVariant QQmlOpenMetaObject::value(const QByteArray &name) const
{
    QHash<QByteArray, int>::ConstIterator iter = d->type->d->names.constFind(name);
    if (iter == d->type->d->names.cend())
        return QVariant();

    return d->propertyValue(*iter);
}

QVariant &QQmlOpenMetaObject::valueRef(const QByteArray &name)
{
    QHash<QByteArray, int>::ConstIterator iter = d->type->d->names.constFind(name);
    Q_ASSERT(iter != d->type->d->names.cend());

    return d->propertyValueRef(*iter);
}

bool QQmlOpenMetaObject::setValue(const QByteArray &name, const QVariant &val, bool force)
{
    QHash<QByteArray, int>::ConstIterator iter = d->type->d->names.constFind(name);

    int id = -1;
    if (iter == d->type->d->names.cend()) {
        id = createProperty(name.constData(), "") - d->type->d->propertyOffset;
    } else {
        id = *iter;
    }

    if (id >= 0) {
        if (!force && d->propertyValue(id) == val)
            return false;

        d->setPropertyValue(id, val);
        activate(d->object, id + d->type->d->signalOffset, nullptr);
        return true;
    }

    return false;
}

// returns true if this value has been initialized by a call to either value() or setValue()
bool QQmlOpenMetaObject::hasValue(int id) const
{
    return d->hasProperty(id);
}

void QQmlOpenMetaObject::setCached(bool c)
{
    if (c == d->cacheProperties || !d->type->d->engine)
        return;

    d->cacheProperties = c;

    QQmlData *qmldata = QQmlData::get(d->object, true);
    if (d->cacheProperties) {
        if (!d->type->d->cache)
            d->type->d->cache = new QQmlPropertyCache(this);
        qmldata->propertyCache = d->type->d->cache;
        d->type->d->cache->addref();
    } else {
        if (d->type->d->cache)
            d->type->d->cache->release();
        qmldata->propertyCache = nullptr;
    }
}


int QQmlOpenMetaObject::createProperty(const char *name, const char *)
{
    if (d->autoCreate) {
        int result = d->type->createProperty(name);

        if (QQmlData *ddata = QQmlData::get(d->object, /*create*/false)) {
            if (ddata->propertyCache) {
                ddata->propertyCache->release();
                ddata->propertyCache = nullptr;
            }
        }

        return result;
    } else
        return -1;
}

void QQmlOpenMetaObject::propertyRead(int)
{
}

void QQmlOpenMetaObject::propertyWrite(int)
{
}

QVariant QQmlOpenMetaObject::propertyWriteValue(int, const QVariant &value)
{
    return value;
}

void QQmlOpenMetaObject::propertyWritten(int)
{
}

void QQmlOpenMetaObject::propertyCreated(int, QMetaPropertyBuilder &)
{
}

QVariant QQmlOpenMetaObject::initialValue(int)
{
    return QVariant();
}

int QQmlOpenMetaObject::count() const
{
    return d->type->d->names.count();
}

QByteArray QQmlOpenMetaObject::name(int idx) const
{
    Q_ASSERT(idx >= 0 && idx < d->type->d->names.count());

    return d->type->d->mob.property(idx).name();
}

QObject *QQmlOpenMetaObject::object() const
{
    return d->object;
}

QT_END_NAMESPACE
