/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include "qchannel.h"

#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {

class QChannelPrivate
{
public:
    QVector<QChannelComponent> m_channelComponents;
    QString m_name;
    int m_jointIndex = -1;
};

/*!
    \class QChannel
    \inmodule Qt3DAnimation
    \brief Defines a channel for a QAnimationClipData.
    The animation system interpolates each channel component independently
    except in the case the QChannel is called "Rotation" (case sensitive),
    it has four QChannelComponents and the same number of keyframes for
    each QChannelComponent. In that case the interpolation will be performed
    using SLERP.
*/
QChannel::QChannel()
    : d(new QChannelPrivate)
{
}

QChannel::QChannel(const QString &name)
    : d(new QChannelPrivate)
{
    d->m_name = name;
}

QChannel::QChannel(const QChannel &rhs)
    : d(new QChannelPrivate)
{
    *d = *(rhs.d);
}

QChannel &QChannel::operator=(const QChannel &rhs)
{
    if (this != &rhs)
        *d = *(rhs.d);
    return *this;
}

QChannel::~QChannel()
{
}

void QChannel::setName(const QString &name)
{
    d->m_name = name;
}

QString QChannel::name() const
{
    return d->m_name;
}

void QChannel::setJointIndex(int jointIndex)
{
    d->m_jointIndex = jointIndex;
}

int QChannel::jointIndex() const
{
    return d->m_jointIndex;
}

int QChannel::channelComponentCount() const
{
    return d->m_channelComponents.size();
}

void QChannel::appendChannelComponent(const QChannelComponent &component)
{
    d->m_channelComponents.append(component);
}

void QChannel::insertChannelComponent(int index, const QChannelComponent &component)
{
    d->m_channelComponents.insert(index, component);
}

void QChannel::removeChannelComponent(int index)
{
    d->m_channelComponents.remove(index);
}

void QChannel::clearChannelComponents()
{
    d->m_channelComponents.clear();
}

QChannel::const_iterator QChannel::begin() const Q_DECL_NOTHROW
{
    return d->m_channelComponents.cbegin();
}

QChannel::const_iterator QChannel::end() const Q_DECL_NOTHROW
{
    return d->m_channelComponents.cend();
}

bool operator==(const QChannel &lhs, const QChannel &rhs) Q_DECL_NOTHROW
{
    return lhs.d->m_name == rhs.d->m_name && lhs.d->m_channelComponents == rhs.d->m_channelComponents;
}

bool operator!=(const QChannel &lhs, const QChannel &rhs) Q_DECL_NOTHROW
{
    return lhs.d->m_name != rhs.d->m_name || lhs.d->m_channelComponents != rhs.d->m_channelComponents;
}

} // namespace Qt3DAnimation

QT_END_NAMESPACE
