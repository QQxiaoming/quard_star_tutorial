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

#include "qskeletonmapping.h"
#include "qskeletonmapping_p.h"
#include <Qt3DCore/qabstractskeleton.h>

#include <Qt3DAnimation/private/qchannelmappingcreatedchange_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {

QSkeletonMappingPrivate::QSkeletonMappingPrivate()
    : QAbstractChannelMappingPrivate()
    , m_skeleton(nullptr)
{
    m_mappingType = QChannelMappingCreatedChangeBase::SkeletonMapping;
}

QSkeletonMapping::QSkeletonMapping(Qt3DCore::QNode *parent)
    : QAbstractChannelMapping(*new QSkeletonMappingPrivate, parent)
{
}

QSkeletonMapping::QSkeletonMapping(QSkeletonMappingPrivate &dd, Qt3DCore::QNode *parent)
    : QAbstractChannelMapping(dd, parent)
{
}

QSkeletonMapping::~QSkeletonMapping()
{
}

Qt3DCore::QAbstractSkeleton *QSkeletonMapping::skeleton() const
{
    Q_D(const QSkeletonMapping);
    return d->m_skeleton;
}

void QSkeletonMapping::setSkeleton(Qt3DCore::QAbstractSkeleton *skeleton)
{
    Q_D(QSkeletonMapping);
    if (d->m_skeleton == skeleton)
        return;

    if (d->m_skeleton)
        d->unregisterDestructionHelper(d->m_skeleton);

    if (skeleton && !skeleton->parent())
        skeleton->setParent(this);
    d->m_skeleton = skeleton;

    // Ensures proper bookkeeping
    if (d->m_skeleton)
        d->registerDestructionHelper(d->m_skeleton, &QSkeletonMapping::setSkeleton, d->m_skeleton);

    emit skeletonChanged(skeleton);
}

Qt3DCore::QNodeCreatedChangeBasePtr QSkeletonMapping::createNodeCreationChange() const
{
    auto creationChange = QChannelMappingCreatedChangePtr<QSkeletonMappingData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QSkeletonMapping);
    data.skeletonId = Qt3DCore::qIdForNode(d->m_skeleton);
    return creationChange;
}

} // namespace Qt3DAnimation

QT_END_NAMESPACE
