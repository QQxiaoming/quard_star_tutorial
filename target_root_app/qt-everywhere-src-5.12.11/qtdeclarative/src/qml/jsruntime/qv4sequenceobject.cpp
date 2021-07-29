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

#include <QtQml/qqml.h>

#include "qv4sequenceobject_p.h"

#include <private/qv4functionobject_p.h>
#include <private/qv4arrayobject_p.h>
#include <private/qqmlengine_p.h>
#include <private/qv4scopedvalue_p.h>
#include <private/qv4jscall_p.h>
#include "qv4runtime_p.h"
#include "qv4objectiterator_p.h"
#include <private/qqmlvaluetypewrapper_p.h>
#include <private/qqmlmodelindexvaluetype_p.h>
#include <QtCore/qabstractitemmodel.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

using namespace QV4;

// helper function to generate valid warnings if errors occur during sequence operations.
static void generateWarning(QV4::ExecutionEngine *v4, const QString& description)
{
    QQmlEngine *engine = v4->qmlEngine();
    if (!engine)
        return;
    QQmlError retn;
    retn.setDescription(description);

    QV4::CppStackFrame *stackFrame = v4->currentStackFrame;

    retn.setLine(stackFrame->lineNumber());
    retn.setUrl(QUrl(stackFrame->source()));
    QQmlEnginePrivate::warning(engine, retn);
}

//  F(elementType, elementTypeName, sequenceType, defaultValue)
#define FOREACH_QML_SEQUENCE_TYPE(F) \
    F(int, IntVector, QVector<int>, 0) \
    F(qreal, RealVector, QVector<qreal>, 0.0) \
    F(bool, BoolVector, QVector<bool>, false) \
    F(int, IntStdVector, std::vector<int>, 0) \
    F(qreal, RealStdVector, std::vector<qreal>, 0.0) \
    F(bool, BoolStdVector, std::vector<bool>, false) \
    F(int, Int, QList<int>, 0) \
    F(qreal, Real, QList<qreal>, 0.0) \
    F(bool, Bool, QList<bool>, false) \
    F(QString, String, QList<QString>, QString()) \
    F(QString, QString, QStringList, QString()) \
    F(QString, StringVector, QVector<QString>, QString()) \
    F(QString, StringStdVector, std::vector<QString>, QString()) \
    F(QUrl, Url, QList<QUrl>, QUrl()) \
    F(QUrl, UrlVector, QVector<QUrl>, QUrl()) \
    F(QUrl, UrlStdVector, std::vector<QUrl>, QUrl()) \
    F(QModelIndex, QModelIndex, QModelIndexList, QModelIndex()) \
    F(QModelIndex, QModelIndexVector, QVector<QModelIndex>, QModelIndex()) \
    F(QModelIndex, QModelIndexStdVector, std::vector<QModelIndex>, QModelIndex()) \
    F(QItemSelectionRange, QItemSelectionRange, QItemSelection, QItemSelectionRange())

static QV4::ReturnedValue convertElementToValue(QV4::ExecutionEngine *engine, const QString &element)
{
    return engine->newString(element)->asReturnedValue();
}

static QV4::ReturnedValue convertElementToValue(QV4::ExecutionEngine *, int element)
{
    return QV4::Encode(element);
}

static QV4::ReturnedValue convertElementToValue(QV4::ExecutionEngine *engine, const QUrl &element)
{
    return engine->newString(element.toString())->asReturnedValue();
}

static QV4::ReturnedValue convertElementToValue(QV4::ExecutionEngine *engine, const QModelIndex &element)
{
    const QMetaObject *vtmo = QQmlValueTypeFactory::metaObjectForMetaType(QMetaType::QModelIndex);
    return QV4::QQmlValueTypeWrapper::create(engine, QVariant(element), vtmo, QMetaType::QModelIndex);
}

static QV4::ReturnedValue convertElementToValue(QV4::ExecutionEngine *engine, const QItemSelectionRange &element)
{
    int metaTypeId = qMetaTypeId<QItemSelectionRange>();
    const QMetaObject *vtmo = QQmlValueTypeFactory::metaObjectForMetaType(metaTypeId);
    return QV4::QQmlValueTypeWrapper::create(engine, QVariant::fromValue(element), vtmo, metaTypeId);
}

static QV4::ReturnedValue convertElementToValue(QV4::ExecutionEngine *, qreal element)
{
    return QV4::Encode(element);
}

static QV4::ReturnedValue convertElementToValue(QV4::ExecutionEngine *, bool element)
{
    return QV4::Encode(element);
}

static QString convertElementToString(const QString &element)
{
    return element;
}

static QString convertElementToString(int element)
{
    return QString::number(element);
}

static QString convertElementToString(const QUrl &element)
{
    return element.toString();
}

static QString convertElementToString(const QModelIndex &element)
{
    return reinterpret_cast<const QQmlModelIndexValueType *>(&element)->toString();
}

static QString convertElementToString(const QItemSelectionRange &element)
{
    return reinterpret_cast<const QQmlItemSelectionRangeValueType *>(&element)->toString();
}

static QString convertElementToString(qreal element)
{
    QString qstr;
    RuntimeHelpers::numberToString(&qstr, element, 10);
    return qstr;
}

static QString convertElementToString(bool element)
{
    if (element)
        return QStringLiteral("true");
    else
        return QStringLiteral("false");
}

template <typename ElementType> ElementType convertValueToElement(const Value &value);

template <> QString convertValueToElement(const Value &value)
{
    return value.toQString();
}

template <> int convertValueToElement(const Value &value)
{
    return value.toInt32();
}

template <> QUrl convertValueToElement(const Value &value)
{
    return QUrl(value.toQString());
}

template <> QModelIndex convertValueToElement(const Value &value)
{
    const QQmlValueTypeWrapper *v = value.as<QQmlValueTypeWrapper>();
    if (v)
        return v->toVariant().toModelIndex();
    return QModelIndex();
}

template <> QItemSelectionRange convertValueToElement(const Value &value)
{
    const QQmlValueTypeWrapper *v = value.as<QQmlValueTypeWrapper>();
    if (v)
        return v->toVariant().value<QItemSelectionRange>();
    return QItemSelectionRange();
}

template <> qreal convertValueToElement(const Value &value)
{
    return value.toNumber();
}

template <> bool convertValueToElement(const Value &value)
{
    return value.toBoolean();
}

namespace QV4 {

template <typename Container> struct QQmlSequence;

namespace Heap {

template <typename Container>
struct QQmlSequence : Object {
    void init(const Container &container);
    void init(QObject *object, int propertyIndex, bool readOnly);
    void destroy() {
        delete container;
        object.destroy();
        Object::destroy();
    }

    mutable Container *container;
    QQmlQPointer<QObject> object;
    int propertyIndex;
    bool isReference : 1;
    bool isReadOnly : 1;
};

}

template <typename Container>
struct QQmlSequence : public QV4::Object
{
    V4_OBJECT2(QQmlSequence<Container>, QV4::Object)
    Q_MANAGED_TYPE(QmlSequence)
    V4_PROTOTYPE(sequencePrototype)
    V4_NEEDS_DESTROY
public:

    void init()
    {
        defineAccessorProperty(QStringLiteral("length"), method_get_length, method_set_length);
    }

    QV4::ReturnedValue containerGetIndexed(uint index, bool *hasProperty) const
    {
        /* Qt containers have int (rather than uint) allowable indexes. */
        if (index > INT_MAX) {
            generateWarning(engine(), QLatin1String("Index out of range during indexed get"));
            if (hasProperty)
                *hasProperty = false;
            return Encode::undefined();
        }
        if (d()->isReference) {
            if (!d()->object) {
                if (hasProperty)
                    *hasProperty = false;
                return Encode::undefined();
            }
            loadReference();
        }
        if (index < size_t(d()->container->size())) {
            if (hasProperty)
                *hasProperty = true;
            return convertElementToValue(engine(), qAsConst(*(d()->container))[index]);
        }
        if (hasProperty)
            *hasProperty = false;
        return Encode::undefined();
    }

    bool containerPutIndexed(uint index, const QV4::Value &value)
    {
        if (internalClass()->engine->hasException)
            return false;

        /* Qt containers have int (rather than uint) allowable indexes. */
        if (index > INT_MAX) {
            generateWarning(engine(), QLatin1String("Index out of range during indexed set"));
            return false;
        }

        if (d()->isReadOnly)
            return false;

        if (d()->isReference) {
            if (!d()->object)
                return false;
            loadReference();
        }

        size_t count = size_t(d()->container->size());

        typename Container::value_type element = convertValueToElement<typename Container::value_type>(value);

        if (index == count) {
            d()->container->push_back(element);
        } else if (index < count) {
            (*d()->container)[index] = element;
        } else {
            /* according to ECMA262r3 we need to insert */
            /* the value at the given index, increasing length to index+1. */
            d()->container->reserve(index + 1);
            while (index > count++) {
                d()->container->push_back(typename Container::value_type());
            }
            d()->container->push_back(element);
        }

        if (d()->isReference)
            storeReference();
        return true;
    }

    QV4::PropertyAttributes containerQueryIndexed(uint index) const
    {
        /* Qt containers have int (rather than uint) allowable indexes. */
        if (index > INT_MAX) {
            generateWarning(engine(), QLatin1String("Index out of range during indexed query"));
            return QV4::Attr_Invalid;
        }
        if (d()->isReference) {
            if (!d()->object)
                return QV4::Attr_Invalid;
            loadReference();
        }
        return (index < size_t(d()->container->size())) ? QV4::Attr_Data : QV4::Attr_Invalid;
    }

    struct OwnPropertyKeyIterator : ObjectOwnPropertyKeyIterator
    {
        ~OwnPropertyKeyIterator() override = default;
        PropertyKey next(const Object *o, Property *pd = nullptr, PropertyAttributes *attrs = nullptr) override
        {
            const QQmlSequence *s = static_cast<const QQmlSequence *>(o);

            if (s->d()->isReference) {
                if (!s->d()->object)
                    return ObjectOwnPropertyKeyIterator::next(o, pd, attrs);
                s->loadReference();
            }

            if (arrayIndex < static_cast<uint>(s->d()->container->size())) {
                uint index = arrayIndex;
                ++arrayIndex;
                if (attrs)
                    *attrs = QV4::Attr_Data;
                if (pd)
                    pd->value = convertElementToValue(s->engine(), s->d()->container->at(index));
                return PropertyKey::fromArrayIndex(index);
            }

            return ObjectOwnPropertyKeyIterator::next(o, pd, attrs);
        }
    };

    static OwnPropertyKeyIterator *containerOwnPropertyKeys(const Object *m, Value *target)
    {
        *target = *m;
        return new OwnPropertyKeyIterator;
    }

    bool containerDeleteIndexedProperty(uint index)
    {
        /* Qt containers have int (rather than uint) allowable indexes. */
        if (index > INT_MAX)
            return false;
        if (d()->isReadOnly)
            return false;
        if (d()->isReference) {
            if (!d()->object)
                return false;
            loadReference();
        }

        if (index >= size_t(d()->container->size()))
            return false;

        /* according to ECMA262r3 it should be Undefined, */
        /* but we cannot, so we insert a default-value instead. */
        (*d()->container)[index] = typename Container::value_type();

        if (d()->isReference)
            storeReference();

        return true;
    }

    bool containerIsEqualTo(Managed *other)
    {
        if (!other)
            return false;
        QQmlSequence<Container> *otherSequence = other->as<QQmlSequence<Container> >();
        if (!otherSequence)
            return false;
        if (d()->isReference && otherSequence->d()->isReference) {
            return d()->object == otherSequence->d()->object && d()->propertyIndex == otherSequence->d()->propertyIndex;
        } else if (!d()->isReference && !otherSequence->d()->isReference) {
            return this == otherSequence;
        }
        return false;
    }

    struct DefaultCompareFunctor
    {
        bool operator()(typename Container::value_type lhs, typename Container::value_type rhs)
        {
            return convertElementToString(lhs) < convertElementToString(rhs);
        }
    };

    struct CompareFunctor
    {
        CompareFunctor(QV4::ExecutionEngine *v4, const QV4::Value &compareFn)
            : m_v4(v4), m_compareFn(&compareFn)
        {}

        bool operator()(typename Container::value_type lhs, typename Container::value_type rhs)
        {
            QV4::Scope scope(m_v4);
            ScopedFunctionObject compare(scope, m_compareFn);
            if (!compare)
                return m_v4->throwTypeError();
            Value *argv = scope.alloc(2);
            argv[0] = convertElementToValue(m_v4, lhs);
            argv[1] = convertElementToValue(m_v4, rhs);
            QV4::ScopedValue result(scope, compare->call(m_v4->globalObject, argv, 2));
            if (scope.engine->hasException)
                return false;
            return result->toNumber() < 0;
        }

    private:
        QV4::ExecutionEngine *m_v4;
        const QV4::Value *m_compareFn;
    };

    bool sort(const FunctionObject *f, const Value *, const Value *argv, int argc)
    {
        if (d()->isReadOnly)
            return false;
        if (d()->isReference) {
            if (!d()->object)
                return false;
            loadReference();
        }

        if (argc == 1 && argv[0].as<FunctionObject>()) {
            CompareFunctor cf(f->engine(), argv[0]);
            std::sort(d()->container->begin(), d()->container->end(), cf);
        } else {
            DefaultCompareFunctor cf;
            std::sort(d()->container->begin(), d()->container->end(), cf);
        }

        if (d()->isReference)
            storeReference();

        return true;
    }

    static QV4::ReturnedValue method_get_length(const FunctionObject *b, const Value *thisObject, const Value *, int)
    {
        QV4::Scope scope(b);
        QV4::Scoped<QQmlSequence<Container>> This(scope, thisObject->as<QQmlSequence<Container> >());
        if (!This)
            THROW_TYPE_ERROR();

        if (This->d()->isReference) {
            if (!This->d()->object)
                RETURN_RESULT(Encode(0));
            This->loadReference();
        }
        RETURN_RESULT(Encode(qint32(This->d()->container->size())));
    }

    static QV4::ReturnedValue method_set_length(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
    {
        QV4::Scope scope(f);
        QV4::Scoped<QQmlSequence<Container>> This(scope, thisObject->as<QQmlSequence<Container> >());
        if (!This)
            THROW_TYPE_ERROR();

        quint32 newLength = argc ? argv[0].toUInt32() : 0;
        /* Qt containers have int (rather than uint) allowable indexes. */
        if (newLength > INT_MAX) {
            generateWarning(scope.engine, QLatin1String("Index out of range during length set"));
            RETURN_UNDEFINED();
        }

        if (This->d()->isReadOnly)
            THROW_TYPE_ERROR();

        /* Read the sequence from the QObject property if we're a reference */
        if (This->d()->isReference) {
            if (!This->d()->object)
                RETURN_UNDEFINED();
            This->loadReference();
        }
        /* Determine whether we need to modify the sequence */
        quint32 newCount = static_cast<quint32>(newLength);
        quint32 count = static_cast<quint32>(This->d()->container->size());
        if (newCount == count) {
            RETURN_UNDEFINED();
        } else if (newCount > count) {
            /* according to ECMA262r3 we need to insert */
            /* undefined values increasing length to newLength. */
            /* We cannot, so we insert default-values instead. */
            This->d()->container->reserve(newCount);
            while (newCount > count++) {
                This->d()->container->push_back(typename Container::value_type());
            }
        } else {
            /* according to ECMA262r3 we need to remove */
            /* elements until the sequence is the required length. */
            if (newCount < count) {
                This->d()->container->erase(This->d()->container->begin() + newCount, This->d()->container->end());
            }
        }
        /* write back if required. */
        if (This->d()->isReference) {
            /* write back.  already checked that object is non-null, so skip that check here. */
            This->storeReference();
        }
        RETURN_UNDEFINED();
    }

    QVariant toVariant() const
    { return QVariant::fromValue<Container>(*d()->container); }

    static QVariant toVariant(QV4::ArrayObject *array)
    {
        QV4::Scope scope(array->engine());
        Container result;
        quint32 length = array->getLength();
        QV4::ScopedValue v(scope);
        for (quint32 i = 0; i < length; ++i)
            result.push_back(convertValueToElement<typename Container::value_type>((v = array->get(i))));
        return QVariant::fromValue(result);
    }

    void* getRawContainerPtr() const
    { return d()->container; }

    void loadReference() const
    {
        Q_ASSERT(d()->object);
        Q_ASSERT(d()->isReference);
        void *a[] = { d()->container, nullptr };
        QMetaObject::metacall(d()->object, QMetaObject::ReadProperty, d()->propertyIndex, a);
    }

    void storeReference()
    {
        Q_ASSERT(d()->object);
        Q_ASSERT(d()->isReference);
        int status = -1;
        QQmlPropertyData::WriteFlags flags = QQmlPropertyData::DontRemoveBinding;
        void *a[] = { d()->container, nullptr, &status, &flags };
        QMetaObject::metacall(d()->object, QMetaObject::WriteProperty, d()->propertyIndex, a);
    }

    static QV4::ReturnedValue virtualGet(const QV4::Managed *that, PropertyKey id, const Value *receiver, bool *hasProperty)
    {
        if (!id.isArrayIndex())
            return Object::virtualGet(that, id, receiver, hasProperty);
        return static_cast<const QQmlSequence<Container> *>(that)->containerGetIndexed(id.asArrayIndex(), hasProperty);
    }
    static bool virtualPut(Managed *that, PropertyKey id, const QV4::Value &value, Value *receiver)
    {
        if (id.isArrayIndex())
            return static_cast<QQmlSequence<Container> *>(that)->containerPutIndexed(id.asArrayIndex(), value);
        return Object::virtualPut(that, id, value, receiver);
    }
    static QV4::PropertyAttributes queryIndexed(const QV4::Managed *that, uint index)
    { return static_cast<const QQmlSequence<Container> *>(that)->containerQueryIndexed(index); }
    static bool virtualDeleteProperty(QV4::Managed *that, PropertyKey id)
    {
        if (id.isArrayIndex()) {
            uint index = id.asArrayIndex();
            return static_cast<QQmlSequence<Container> *>(that)->containerDeleteIndexedProperty(index);
        }
        return Object::virtualDeleteProperty(that, id);
    }
    static bool virtualIsEqualTo(Managed *that, Managed *other)
    { return static_cast<QQmlSequence<Container> *>(that)->containerIsEqualTo(other); }
    static QV4::OwnPropertyKeyIterator *virtualOwnPropertyKeys(const Object *m, Value *target)
    { return static_cast<const QQmlSequence<Container> *>(m)->containerOwnPropertyKeys(m, target);}

};


template <typename Container>
void Heap::QQmlSequence<Container>::init(const Container &container)
{
    Object::init();
    this->container = new Container(container);
    propertyIndex = -1;
    isReference = false;
    isReadOnly = false;
    object.init();

    QV4::Scope scope(internalClass->engine);
    QV4::Scoped<QV4::QQmlSequence<Container> > o(scope, this);
    o->setArrayType(Heap::ArrayData::Custom);
    o->init();
}

template <typename Container>
void Heap::QQmlSequence<Container>::init(QObject *object, int propertyIndex, bool readOnly)
{
    Object::init();
    this->container = new Container;
    this->propertyIndex = propertyIndex;
    isReference = true;
    this->isReadOnly = readOnly;
    this->object.init(object);
    QV4::Scope scope(internalClass->engine);
    QV4::Scoped<QV4::QQmlSequence<Container> > o(scope, this);
    o->setArrayType(Heap::ArrayData::Custom);
    o->loadReference();
    o->init();
}

}

namespace QV4 {

typedef QQmlSequence<QVector<int> > QQmlIntVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlIntVectorList);
typedef QQmlSequence<QVector<qreal> > QQmlRealVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlRealVectorList);
typedef QQmlSequence<QVector<bool> > QQmlBoolVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlBoolVectorList);
typedef QQmlSequence<std::vector<int> > QQmlIntStdVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlIntStdVectorList);
typedef QQmlSequence<std::vector<qreal> > QQmlRealStdVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlRealStdVectorList);
typedef QQmlSequence<std::vector<bool> > QQmlBoolStdVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlBoolStdVectorList);
typedef QQmlSequence<QStringList> QQmlQStringList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlQStringList);
typedef QQmlSequence<QList<QString> > QQmlStringList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlStringList);
typedef QQmlSequence<QVector<QString> > QQmlStringVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlStringVectorList);
typedef QQmlSequence<std::vector<QString> > QQmlStringStdVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlStringStdVectorList);
typedef QQmlSequence<QList<int> > QQmlIntList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlIntList);
typedef QQmlSequence<QList<QUrl> > QQmlUrlList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlUrlList);
typedef QQmlSequence<QVector<QUrl> > QQmlUrlVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlUrlVectorList);
typedef QQmlSequence<std::vector<QUrl> > QQmlUrlStdVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlUrlStdVectorList);
typedef QQmlSequence<QModelIndexList> QQmlQModelIndexList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlQModelIndexList);
typedef QQmlSequence<QVector<QModelIndex> > QQmlQModelIndexVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlQModelIndexVectorList);
typedef QQmlSequence<std::vector<QModelIndex> > QQmlQModelIndexStdVectorList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlQModelIndexStdVectorList);
typedef QQmlSequence<QItemSelection> QQmlQItemSelectionRangeList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlQItemSelectionRangeList);
typedef QQmlSequence<QList<bool> > QQmlBoolList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlBoolList);
typedef QQmlSequence<QList<qreal> > QQmlRealList;
DEFINE_OBJECT_TEMPLATE_VTABLE(QQmlRealList);

}

#define REGISTER_QML_SEQUENCE_METATYPE(unused, unused2, SequenceType, unused3) qRegisterMetaType<SequenceType>(#SequenceType);
void SequencePrototype::init()
{
    FOREACH_QML_SEQUENCE_TYPE(REGISTER_QML_SEQUENCE_METATYPE)
    defineDefaultProperty(QStringLiteral("sort"), method_sort, 1);
    defineDefaultProperty(engine()->id_valueOf(), method_valueOf, 0);
}
#undef REGISTER_QML_SEQUENCE_METATYPE

ReturnedValue SequencePrototype::method_valueOf(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    return Encode(thisObject->toString(f->engine()));
}

ReturnedValue SequencePrototype::method_sort(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    QV4::ScopedObject o(scope, thisObject);
    if (!o || !o->isListType())
        THROW_TYPE_ERROR();

    if (argc >= 2)
        return o.asReturnedValue();

#define CALL_SORT(SequenceElementType, SequenceElementTypeName, SequenceType, DefaultValue) \
        if (QQml##SequenceElementTypeName##List *s = o->as<QQml##SequenceElementTypeName##List>()) { \
            if (!s->sort(b, thisObject, argv, argc)) \
                THROW_TYPE_ERROR(); \
        } else

        FOREACH_QML_SEQUENCE_TYPE(CALL_SORT)

#undef CALL_SORT
        {}
    return o.asReturnedValue();
}

#define IS_SEQUENCE(unused1, unused2, SequenceType, unused3) \
    if (sequenceTypeId == qMetaTypeId<SequenceType>()) { \
        return true; \
    } else

bool SequencePrototype::isSequenceType(int sequenceTypeId)
{
    FOREACH_QML_SEQUENCE_TYPE(IS_SEQUENCE) { /* else */ return false; }
}
#undef IS_SEQUENCE

#define NEW_REFERENCE_SEQUENCE(ElementType, ElementTypeName, SequenceType, unused) \
    if (sequenceType == qMetaTypeId<SequenceType>()) { \
        QV4::ScopedObject obj(scope, engine->memoryManager->allocate<QQml##ElementTypeName##List>(object, propertyIndex, readOnly)); \
        return obj.asReturnedValue(); \
    } else

ReturnedValue SequencePrototype::newSequence(QV4::ExecutionEngine *engine, int sequenceType, QObject *object, int propertyIndex, bool readOnly, bool *succeeded)
{
    QV4::Scope scope(engine);
    // This function is called when the property is a QObject Q_PROPERTY of
    // the given sequence type.  Internally we store a typed-sequence
    // (as well as object ptr + property index for updated-read and write-back)
    // and so access/mutate avoids variant conversion.
    *succeeded = true;
    FOREACH_QML_SEQUENCE_TYPE(NEW_REFERENCE_SEQUENCE) { /* else */ *succeeded = false; return QV4::Encode::undefined(); }
}
#undef NEW_REFERENCE_SEQUENCE

#define NEW_COPY_SEQUENCE(ElementType, ElementTypeName, SequenceType, unused) \
    if (sequenceType == qMetaTypeId<SequenceType>()) { \
        QV4::ScopedObject obj(scope, engine->memoryManager->allocate<QQml##ElementTypeName##List>(v.value<SequenceType >())); \
        return obj.asReturnedValue(); \
    } else

ReturnedValue SequencePrototype::fromVariant(QV4::ExecutionEngine *engine, const QVariant& v, bool *succeeded)
{
    QV4::Scope scope(engine);
    // This function is called when assigning a sequence value to a normal JS var
    // in a JS block.  Internally, we store a sequence of the specified type.
    // Access and mutation is extremely fast since it will not need to modify any
    // QObject property.
    int sequenceType = v.userType();
    *succeeded = true;
    FOREACH_QML_SEQUENCE_TYPE(NEW_COPY_SEQUENCE) { /* else */ *succeeded = false; return QV4::Encode::undefined(); }
}
#undef NEW_COPY_SEQUENCE

#define SEQUENCE_TO_VARIANT(ElementType, ElementTypeName, SequenceType, unused) \
    if (QQml##ElementTypeName##List *list = object->as<QQml##ElementTypeName##List>()) \
        return list->toVariant(); \
    else

QVariant SequencePrototype::toVariant(Object *object)
{
    Q_ASSERT(object->isListType());
    FOREACH_QML_SEQUENCE_TYPE(SEQUENCE_TO_VARIANT) { /* else */ return QVariant(); }
}

#undef SEQUENCE_TO_VARIANT
#define SEQUENCE_TO_VARIANT(ElementType, ElementTypeName, SequenceType, unused) \
    if (typeHint == qMetaTypeId<SequenceType>()) { \
        return QQml##ElementTypeName##List::toVariant(a); \
    } else

QVariant SequencePrototype::toVariant(const QV4::Value &array, int typeHint, bool *succeeded)
{
    *succeeded = true;

    if (!array.as<ArrayObject>()) {
        *succeeded = false;
        return QVariant();
    }
    QV4::Scope scope(array.as<Object>()->engine());
    QV4::ScopedArrayObject a(scope, array);

    FOREACH_QML_SEQUENCE_TYPE(SEQUENCE_TO_VARIANT) { /* else */ *succeeded = false; return QVariant(); }
}

#undef SEQUENCE_TO_VARIANT

#define SEQUENCE_GET_RAWCONTAINERPTR(ElementType, ElementTypeName, SequenceType, unused) \
    if (const QQml##ElementTypeName##List *list = [&]() -> const QQml##ElementTypeName##List* \
        { if (typeHint == qMetaTypeId<SequenceType>()) return object->as<QQml##ElementTypeName##List>(); return nullptr;}()) \
        return list->getRawContainerPtr(); \
    else

void* SequencePrototype::getRawContainerPtr(const Object *object, int typeHint)
{
    FOREACH_QML_SEQUENCE_TYPE(SEQUENCE_GET_RAWCONTAINERPTR) { /* else */ return nullptr; }
}

#undef SEQUENCE_GET_RAWCONTAINERPTR

#define MAP_META_TYPE(ElementType, ElementTypeName, SequenceType, unused) \
    if (object->as<QQml##ElementTypeName##List>()) { \
        return qMetaTypeId<SequenceType>(); \
    } else

int SequencePrototype::metaTypeForSequence(const QV4::Object *object)
{
    FOREACH_QML_SEQUENCE_TYPE(MAP_META_TYPE)
    /*else*/ {
        return -1;
    }
}

#undef MAP_META_TYPE

QT_END_NAMESPACE
