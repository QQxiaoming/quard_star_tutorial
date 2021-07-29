/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include "qchannelmapping.h"
#include "qchannelmapping_p.h"

#include <Qt3DAnimation/private/qchannelmappingcreatedchange_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>

#include <QtCore/qmetaobject.h>
#include <QtCore/QMetaProperty>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {

namespace {

template<typename T>
int componentCountForValue(const T &)
{
    return 0;
}

template<>
int componentCountForValue<QVector<float>>(const QVector<float> &v)
{
    return v.size();
}

template<>
int componentCountForValue<QVariantList>(const QVariantList &v)
{
    return v.size();
}


int componentCountForType(int type, const QVariant &value)
{
    const int vectorOfFloatTypeId = qMetaTypeId<QVector<float>>();

    if (type == vectorOfFloatTypeId)
        return componentCountForValue<QVector<float>>(value.value<QVector<float>>());

    switch (type) {
    case QMetaType::Float:
    case QVariant::Double:
        return 1;

    case QVariant::Vector2D:
        return 2;

    case QVariant::Vector3D:
    case QVariant::Color:
        return 3;

    case QVariant::Vector4D:
    case QVariant::Quaternion:
        return 4;

    case QVariant::List:
        return componentCountForValue<QVariantList>(value.toList());

    default:
        qWarning() << "Unhandled animation type";
        return 0;
    }
}

} // anonymous

QChannelMappingPrivate::QChannelMappingPrivate()
    : QAbstractChannelMappingPrivate()
    , m_channelName()
    , m_target(nullptr)
    , m_property()
    , m_propertyName(nullptr)
    , m_type(static_cast<int>(QVariant::Invalid))
    , m_componentCount(0)
{
    m_mappingType = QChannelMappingCreatedChangeBase::ChannelMapping;
}

/*!
    \internal

    Find the type of the property specified on the target node
 */
void QChannelMappingPrivate::updatePropertyNameTypeAndComponentCount()
{
    int type;
    int componentCount = 0;
    const char *propertyName = nullptr;

    if (!m_target || m_property.isNull()) {
         type = QVariant::Invalid;
    } else {
        const QMetaObject *mo = m_target->metaObject();
        const int propertyIndex = mo->indexOfProperty(m_property.toLocal8Bit());
        QMetaProperty mp = mo->property(propertyIndex);
        propertyName = mp.name();
        type = mp.userType();
        const QVariant currentValue = m_target->property(mp.name());
        if (type == QMetaType::QVariant) {
            if (currentValue.isValid()) {
                type = currentValue.userType();
            } else {
                qWarning("QChannelMapping: Attempted to target QVariant property with no value set. "
                         "Set a value first in order to be able to determine the type.");
            }
        }
        componentCount = componentCountForType(type, currentValue);
    }

    if (m_type != type) {
        m_type = type;

        // Send update to the backend
        Q_Q(QChannelMapping);
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(q->id());
        e->setPropertyName("type");
        e->setValue(QVariant(m_type));
        notifyObservers(e);
    }

    if (m_componentCount != componentCount) {
        m_componentCount = componentCount;

        // Send update to the backend
        Q_Q(QChannelMapping);
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(q->id());
        e->setPropertyName("componentCount");
        e->setValue(QVariant(m_componentCount));
        notifyObservers(e);
    }

    if (qstrcmp(m_propertyName, propertyName) != 0) {
        m_propertyName = propertyName;

        // Send update to the backend
        Q_Q(QChannelMapping);
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(q->id());
        e->setPropertyName("propertyName");
        e->setValue(QVariant::fromValue(const_cast<void *>(static_cast<const void *>(m_propertyName))));
        notifyObservers(e);
    }
}
/*!
    \class QChannelMapping
    \inherits Qt3DCore::QNode
    \inmodule Qt3DAnimation
    \brief Allows to map the channels within the clip onto properties of
           objects in the application.

*/

QChannelMapping::QChannelMapping(Qt3DCore::QNode *parent)
    : QAbstractChannelMapping(*new QChannelMappingPrivate, parent)
{
}

QChannelMapping::QChannelMapping(QChannelMappingPrivate &dd, Qt3DCore::QNode *parent)
    : QAbstractChannelMapping(dd, parent)
{
}

QChannelMapping::~QChannelMapping()
{
}

QString QChannelMapping::channelName() const
{
    Q_D(const QChannelMapping);
    return d->m_channelName;
}

Qt3DCore::QNode *QChannelMapping::target() const
{
    Q_D(const QChannelMapping);
    return d->m_target;
}

QString QChannelMapping::property() const
{
    Q_D(const QChannelMapping);
    return d->m_property;
}

void QChannelMapping::setChannelName(const QString &channelName)
{
    Q_D(QChannelMapping);
    if (d->m_channelName == channelName)
        return;

    d->m_channelName = channelName;
    emit channelNameChanged(channelName);
}

void QChannelMapping::setTarget(Qt3DCore::QNode *target)
{
    Q_D(QChannelMapping);
    if (d->m_target == target)
        return;

    if (d->m_target)
        d->unregisterDestructionHelper(d->m_target);

    if (target && !target->parent())
        target->setParent(this);
    d->m_target = target;

    // Ensures proper bookkeeping
    if (d->m_target)
        d->registerDestructionHelper(d->m_target, &QChannelMapping::setTarget, d->m_target);

    emit targetChanged(target);
    d->updatePropertyNameTypeAndComponentCount();
}

void QChannelMapping::setProperty(const QString &property)
{
    Q_D(QChannelMapping);
    if (d->m_property == property)
        return;

    d->m_property = property;

    // The backend uses propertyName instead of property
    const bool blocked = blockNotifications(true);
    emit propertyChanged(property);
    blockNotifications(blocked);

    d->updatePropertyNameTypeAndComponentCount();
}

Qt3DCore::QNodeCreatedChangeBasePtr QChannelMapping::createNodeCreationChange() const
{
    auto creationChange = QChannelMappingCreatedChangePtr<QChannelMappingData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QChannelMapping);
    data.channelName = d->m_channelName;
    data.targetId = Qt3DCore::qIdForNode(d->m_target);
    data.type = d->m_type;
    data.componentCount = d->m_componentCount;
    data.propertyName = d->m_propertyName;
    return creationChange;
}

} // namespace Qt3DAnimation

QT_END_NAMESPACE
