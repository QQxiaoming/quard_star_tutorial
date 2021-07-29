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

#include "qcallbackmapping.h"
#include "qcallbackmapping_p.h"

#include <Qt3DAnimation/private/qchannelmappingcreatedchange_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>

#include <QtCore/qmetaobject.h>
#include <QtCore/QMetaProperty>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {

QCallbackMappingPrivate::QCallbackMappingPrivate()
    : QAbstractChannelMappingPrivate()
    , m_channelName()
    , m_type(static_cast<int>(QVariant::Invalid))
    , m_callback(nullptr)
    , m_callbackFlags(0)
{
    m_mappingType = QChannelMappingCreatedChangeBase::CallbackMapping;
}

/*!
    \class QCallbackMapping
    \inherits Qt3DCore::QAbstractChannelMapping
    \inmodule Qt3DAnimation
    \brief Allows to map the channels within the clip onto an invocation
           of a callback object.
*/

QCallbackMapping::QCallbackMapping(Qt3DCore::QNode *parent)
    : QAbstractChannelMapping(*new QCallbackMappingPrivate, parent)
{
}

QCallbackMapping::QCallbackMapping(QCallbackMappingPrivate &dd, Qt3DCore::QNode *parent)
    : QAbstractChannelMapping(dd, parent)
{
}

QCallbackMapping::~QCallbackMapping()
{
}

QString QCallbackMapping::channelName() const
{
    Q_D(const QCallbackMapping);
    return d->m_channelName;
}

QAnimationCallback *QCallbackMapping::callback() const
{
    Q_D(const QCallbackMapping);
    return d->m_callback;
}

void QCallbackMapping::setChannelName(const QString &channelName)
{
    Q_D(QCallbackMapping);
    if (d->m_channelName == channelName)
        return;

    d->m_channelName = channelName;
    emit channelNameChanged(channelName);
}

/*!
    Associates a \a callback object with this channel mapping.

    Such mappings do not have to have a target object and property name. When
    the \a callback object is set, every change in the animated value will lead
    to invoking the callback's
    \l {Qt3DAnimation::QAnimationCallback::valueChanged}{valueChanged} function either
    on the gui/main thread, or directly on one of the thread pool's worker
    thread. This is controlled by \a flags.

    \a type specifies the type (for example, QVariant::Vector3D,
    QVariant::Color, or QMetaType::Float) of the animated value. When animating
    node properties this does not need to be provided separately, however it
    becomes important to supply this when there is only a callback.

    \note A mapping can be associated both with a node property and a
    callback. It is important however that \a type matches the type of the
    property in this case. Note also that for properties of type QVariant (for
    example, QParameter::value), the \a type is the type of the value stored in
    the QVariant.

    \note The \a callback pointer is expected to stay valid while any
    associated animators are running.
 */
void QCallbackMapping::setCallback(int type, QAnimationCallback *callback, QAnimationCallback::Flags flags)
{
    Q_D(QCallbackMapping);
    if (d->m_type != type) {
        d->m_type = type;
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(id());
        e->setPropertyName("type");
        e->setValue(QVariant(d->m_type));
        notifyObservers(e);
    }
    if (d->m_callback != callback) {
        d->m_callback = callback;
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(id());
        e->setPropertyName("callback");
        e->setValue(QVariant::fromValue(static_cast<void *>(d->m_callback)));
        notifyObservers(e);
    }
    if (d->m_callbackFlags != flags) {
        d->m_callbackFlags = flags;
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(id());
        e->setPropertyName("callbackFlags");
        e->setValue(QVariant::fromValue(int(d->m_callbackFlags)));
        notifyObservers(e);
    }
}

Qt3DCore::QNodeCreatedChangeBasePtr QCallbackMapping::createNodeCreationChange() const
{
    auto creationChange = QChannelMappingCreatedChangePtr<QCallbackMappingData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QCallbackMapping);
    data.channelName = d->m_channelName;
    data.type = d->m_type;
    data.callback = d->m_callback;
    data.callbackFlags = d->m_callbackFlags;
    return creationChange;
}

} // namespace Qt3DAnimation

QT_END_NAMESPACE
