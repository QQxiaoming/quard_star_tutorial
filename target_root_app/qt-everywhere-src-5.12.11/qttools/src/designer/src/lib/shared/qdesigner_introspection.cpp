/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#include "qdesigner_introspection_p.h"

#include <QtCore/qmetaobject.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

// Qt Implementation
static QStringList byteArrayListToStringList(const QByteArrayList &l)
{
    if (l.empty())
        return QStringList();
    QStringList rc;
    for (const QByteArray &b : l)
        rc += QString::fromUtf8(b);
    return rc;
}

static inline QString charToQString(const char *c)
{
    if (!c)
        return QString();
    return QString::fromUtf8(c);
}

namespace  {
    // ------- QDesignerMetaEnum
    class QDesignerMetaEnum : public QDesignerMetaEnumInterface {
    public:
        QDesignerMetaEnum(const QMetaEnum &qEnum);
        bool isFlag() const override                        { return m_enum.isFlag(); }
        QString key(int index) const override               { return charToQString(m_enum.key(index)); }
        int keyCount() const override                       { return m_enum.keyCount(); }
        int keyToValue(const QString &key) const override   { return m_enum.keyToValue(key.toUtf8()); }
        int keysToValue(const QString &keys) const override { return m_enum.keysToValue(keys.toUtf8()); }
        QString name() const override                       { return m_name; }
        QString scope() const override                      { return m_scope; }
        QString separator() const override;
        int value(int index) const override                 { return m_enum.value(index); }
        QString valueToKey(int value) const override        { return charToQString(m_enum.valueToKey(value)); }
        QString valueToKeys(int value) const override       { return charToQString(m_enum.valueToKeys(value)); }

    private:
        const QMetaEnum m_enum;
        const QString m_name;
        const QString m_scope;
    };

    QDesignerMetaEnum::QDesignerMetaEnum(const QMetaEnum &qEnum) :
        m_enum(qEnum),
        m_name(charToQString(m_enum.name())),
        m_scope(charToQString(m_enum.scope()))
    {
    }

    QString QDesignerMetaEnum::separator() const
    {
        static const QString rc = QStringLiteral("::");
        return rc;
    }

    // ------- QDesignerMetaProperty
    class QDesignerMetaProperty : public QDesignerMetaPropertyInterface {
    public:
        QDesignerMetaProperty(const QMetaProperty &property);
        ~QDesignerMetaProperty() override;

        const QDesignerMetaEnumInterface *enumerator() const override { return m_enumerator; }

        Kind kind() const override { return m_kind; }

        AccessFlags accessFlags() const override { return m_access; }
        Attributes attributes(const QObject *object = 0) const override;

        QVariant::Type type() const override { return m_property.type(); }
        QString name() const override        { return m_name; }
        QString typeName() const override    { return m_typeName; }
        int userType() const override        { return m_property.userType(); }
        bool hasSetter() const override      { return m_property.hasStdCppSet(); }

        QVariant read(const QObject *object) const override { return m_property.read(object); }
        bool reset(QObject *object) const  override { return m_property.reset(object); }
        bool write(QObject *object, const QVariant &value) const override { return m_property.write(object, value); }

    private:
        const QMetaProperty m_property;
        const QString m_name;
        const QString m_typeName;
        Kind m_kind;
        AccessFlags m_access;
        Attributes m_defaultAttributes;
        QDesignerMetaEnumInterface *m_enumerator;
    };

    QDesignerMetaProperty::QDesignerMetaProperty(const QMetaProperty &property) :
        m_property(property),
        m_name(charToQString(m_property.name())),
        m_typeName(charToQString(m_property.typeName())),
        m_kind(OtherKind),
        m_enumerator(0)
    {
        if (m_property.isFlagType() || m_property.isEnumType()) {
            const QMetaEnum metaEnum = m_property.enumerator();
            Q_ASSERT(metaEnum.isValid());
            m_enumerator = new QDesignerMetaEnum(metaEnum);
        }
        // kind
        if (m_property.isFlagType())
            m_kind = FlagKind;
        else
            if (m_property.isEnumType())
                m_kind = EnumKind;
        // flags and attributes
        if (m_property.isReadable())
            m_access |= ReadAccess;
        if (m_property.isWritable())
            m_access |= WriteAccess;
        if (m_property.isResettable())
            m_access |= ResetAccess;

        if (m_property.isDesignable())
            m_defaultAttributes |= DesignableAttribute;
        if (m_property.isScriptable())
            m_defaultAttributes |= ScriptableAttribute;
        if (m_property.isStored())
            m_defaultAttributes |= StoredAttribute;
        if (m_property.isUser())
            m_defaultAttributes |= UserAttribute;
    }

    QDesignerMetaProperty::~QDesignerMetaProperty()
    {
        delete m_enumerator;
    }

    QDesignerMetaProperty::Attributes QDesignerMetaProperty::attributes(const QObject *object) const
    {
        if (!object)
            return m_defaultAttributes;
        Attributes rc;
        if (m_property.isDesignable(object))
            rc |= DesignableAttribute;
        if (m_property.isScriptable(object))
            rc |= ScriptableAttribute;
        if (m_property.isStored(object))
            rc |= StoredAttribute;
        if (m_property.isUser(object))
            rc |= UserAttribute;
        return rc;
    }

    // -------------- QDesignerMetaMethod

    class QDesignerMetaMethod : public QDesignerMetaMethodInterface {
    public:
        QDesignerMetaMethod(const QMetaMethod &method);

        Access access() const override               { return m_access; }
        MethodType methodType() const override       { return m_methodType; }
        QStringList parameterNames() const override  { return m_parameterNames; }
        QStringList parameterTypes() const override  { return m_parameterTypes; }
        QString signature() const override           { return m_signature; }
        QString normalizedSignature() const override { return m_normalizedSignature; }
        QString tag() const  override                { return m_tag; }
        QString typeName() const override            { return m_typeName; }

    private:
        Access m_access;
        MethodType m_methodType;
        const QStringList m_parameterNames;
        const QStringList m_parameterTypes;
        const QString m_signature;
        const QString m_normalizedSignature;
        const QString m_tag;
        const QString m_typeName;
    };

    QDesignerMetaMethod::QDesignerMetaMethod(const QMetaMethod &method) :
       m_parameterNames(byteArrayListToStringList(method.parameterNames())),
       m_parameterTypes(byteArrayListToStringList(method.parameterTypes())),
       m_signature(QString::fromLatin1(method.methodSignature())),
       m_normalizedSignature(QString::fromLatin1(QMetaObject::normalizedSignature(method.methodSignature().constData()))),
       m_tag(charToQString(method.tag())),
       m_typeName(charToQString(method.typeName()))
    {
        switch (method.access()) {
        case QMetaMethod::Public:
            m_access = Public;
            break;
        case QMetaMethod::Protected:
            m_access = Protected;
            break;
        case QMetaMethod::Private:
            m_access = Private;
            break;

        }
        switch (method.methodType()) {
        case QMetaMethod::Constructor:
            m_methodType = Constructor;
            break;
        case QMetaMethod::Method:
            m_methodType = Method;
            break;
        case QMetaMethod::Signal:
            m_methodType = Signal;
            break;

        case QMetaMethod::Slot:
            m_methodType = Slot;
            break;
        }
    }

    // ------------- QDesignerMetaObject
    class QDesignerMetaObject : public QDesignerMetaObjectInterface {
    public:
        QDesignerMetaObject(const qdesigner_internal::QDesignerIntrospection *introspection, const QMetaObject *metaObject);
        ~QDesignerMetaObject() override;

        QString className() const override { return m_className; }
        const QDesignerMetaEnumInterface *enumerator(int index) const  override
        { return m_enumerators[index]; }
        int enumeratorCount() const override { return m_enumerators.size(); }
        int enumeratorOffset() const override { return m_metaObject->enumeratorOffset(); }

        int indexOfEnumerator(const QString &name) const override
        { return m_metaObject->indexOfEnumerator(name.toUtf8()); }
        int indexOfMethod(const QString &method) const override
        { return m_metaObject->indexOfMethod(method.toUtf8()); }
        int indexOfProperty(const QString &name) const override
        { return m_metaObject->indexOfProperty(name.toUtf8()); }
        int indexOfSignal(const QString &signal) const override
        { return m_metaObject->indexOfSignal(signal.toUtf8());  }
        int indexOfSlot(const QString &slot) const override
        { return m_metaObject->indexOfSlot(slot.toUtf8()); }

        const QDesignerMetaMethodInterface *method(int index) const override
        { return m_methods[index]; }
        int methodCount() const override { return m_methods.size(); }
        int methodOffset() const override { return m_metaObject->methodOffset(); }

        const QDesignerMetaPropertyInterface *property(int index) const override
        { return m_properties[index]; }
        int propertyCount() const override { return m_properties.size(); }
        int propertyOffset() const override { return m_metaObject->propertyOffset(); }

        const QDesignerMetaObjectInterface *superClass() const override;
        const QDesignerMetaPropertyInterface *userProperty() const override
        { return m_userProperty; }

    private:
        const QString m_className;
        const qdesigner_internal::QDesignerIntrospection *m_introspection;
        const QMetaObject *m_metaObject;

        typedef QVector<QDesignerMetaEnumInterface *> Enumerators;
        Enumerators m_enumerators;

        typedef QVector<QDesignerMetaMethodInterface *> Methods;
        Methods m_methods;

        typedef QVector<QDesignerMetaPropertyInterface *> Properties;
        Properties m_properties;

        QDesignerMetaPropertyInterface *m_userProperty;
    };

    QDesignerMetaObject::QDesignerMetaObject(const qdesigner_internal::QDesignerIntrospection *introspection, const QMetaObject *metaObject) :
       m_className(charToQString(metaObject->className())),
       m_introspection(introspection),
       m_metaObject(metaObject),
       m_userProperty(0)
    {
        const int numEnumerators = metaObject->enumeratorCount();
        m_enumerators.reserve(numEnumerators);
        for (int i = 0; i < numEnumerators; i++)
            m_enumerators.push_back(new QDesignerMetaEnum(metaObject->enumerator(i)));
        const int numMethods = metaObject->methodCount();
        m_methods.reserve(numMethods);
        for (int i = 0; i < numMethods; i++)
            m_methods.push_back(new QDesignerMetaMethod(metaObject->method(i)));

        const int numProperties = metaObject->propertyCount();
        m_properties.reserve(numProperties);
        for (int i = 0; i < numProperties; i++)
            m_properties.push_back(new QDesignerMetaProperty(metaObject->property(i)));

        const QMetaProperty userProperty = metaObject->userProperty();
        if (userProperty.isValid())
            m_userProperty = new QDesignerMetaProperty(userProperty);
    }

    QDesignerMetaObject::~QDesignerMetaObject()
    {
        qDeleteAll(m_enumerators);
        qDeleteAll(m_methods);
        qDeleteAll(m_properties);
        delete m_userProperty;
    }

    const QDesignerMetaObjectInterface *QDesignerMetaObject::superClass() const
    {
        const QMetaObject *qSuperClass = m_metaObject->superClass();
        if (!qSuperClass)
            return 0;
        return m_introspection->metaObjectForQMetaObject(qSuperClass);
    }

}

namespace qdesigner_internal {

    QDesignerIntrospection::QDesignerIntrospection() = default;

    QDesignerIntrospection::~QDesignerIntrospection()
    {
        qDeleteAll(m_metaObjectMap.values());
    }

    const QDesignerMetaObjectInterface* QDesignerIntrospection::metaObject(const QObject *object) const
    {
        return metaObjectForQMetaObject(object->metaObject());
    }

    const QDesignerMetaObjectInterface* QDesignerIntrospection::metaObjectForQMetaObject(const QMetaObject *metaObject) const
    {
        MetaObjectMap::iterator it = m_metaObjectMap.find(metaObject);
        if (it == m_metaObjectMap.end())
            it = m_metaObjectMap.insert(metaObject, new QDesignerMetaObject(this, metaObject));
        return it.value();
    }
}

QT_END_NAMESPACE
