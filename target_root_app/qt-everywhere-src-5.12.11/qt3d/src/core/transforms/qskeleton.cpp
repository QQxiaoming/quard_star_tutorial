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

#include "qskeleton.h"
#include "qskeleton_p.h"
#include <Qt3DCore/qjoint.h>
#include <Qt3DCore/private/qskeletoncreatedchange_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

QSkeletonPrivate::QSkeletonPrivate()
    : QAbstractSkeletonPrivate()
    , m_rootJoint(nullptr)
{
    m_type = QSkeletonCreatedChangeBase::Skeleton;
}

/*!
    \qmltype Skeleton
    \inqmlmodule Qt3D.Core
    \inherits AbstractSkeleton
    \instantiates Qt3DCore::QSkeleton
    \since 5.10
    \brief Holds the data for a skeleton to be used with skinned meshes.

    Use Skeleton if you wish to manually create the joints of a skeleton for
    use with skinned meshes. This is mainly of use to people creating editors,
    tooling, or dynamic skeletons. It is more common that a Qt 3D application
    would simply consume an existing skeleton and skinned mesh as created in
    a digital content creation tool such as Blender. For this use case, please
    see SkeletonLoader.
*/

/*!
    \qmlproperty Joint Skeleton::rootJoint

    Holds the root joint of the hierarchy of joints forming the skeleton.
*/

/*!
    \class Qt3DCore::QSkeleton
    \inmodule Qt3DCore
    \inherits Qt3DCore::QAbstractSkeleton
    \since 5.10
    \brief Holds the data for a skeleton to be used with skinned meshes.

    Use QSkeleton if you wish to manually create the joints of a skeleton for
    use with skinned meshes. This is mainly of use to people creating editors,
    tooling, or dynamic skeletons. It is more common that a Qt 3D application
    would simply consume an existing skeleton and skinned mesh as created in
    a digital content creation tool such as Blender. For this use case, please
    see QSkeletonLoader.
*/

/*!
    Constructs a new QSkeleton with \a parent.
*/
QSkeleton::QSkeleton(Qt3DCore::QNode *parent)
    : QAbstractSkeleton(*new QSkeletonPrivate, parent)
{
}

/*! \internal */
QSkeleton::~QSkeleton()
{
}

/*!
    \property Qt3DCore::QSkeleton::rootJoint

    Holds the root joint of the hierarchy of joints forming the skeleton.
*/
Qt3DCore::QJoint *QSkeleton::rootJoint() const
{
    Q_D(const QSkeleton);
    return d->m_rootJoint;
}

void QSkeleton::setRootJoint(Qt3DCore::QJoint *rootJoint)
{
    Q_D(QSkeleton);
    if (d->m_rootJoint != rootJoint) {
        if (d->m_rootJoint)
            d->unregisterDestructionHelper(d->m_rootJoint);

        // We need to add it as a child of the current node if it has been declared inline
        // Or not previously added as a child of the current node so that
        // 1) The backend gets notified about it's creation
        // 2) When the current node is destroyed, it gets destroyed as well
        if (rootJoint && !rootJoint->parent())
            rootJoint->setParent(this);
        d->m_rootJoint = rootJoint;

        // Ensures proper bookkeeping
        if (d->m_rootJoint)
            d->registerDestructionHelper(d->m_rootJoint, &QSkeleton::setRootJoint, d->m_rootJoint);

        emit rootJointChanged(rootJoint);
    }
}

/*! \internal */
Qt3DCore::QNodeCreatedChangeBasePtr QSkeleton::createNodeCreationChange() const
{
    auto creationChange = QSkeletonCreatedChangePtr<QSkeletonData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QSkeleton);
    data.rootJointId = qIdForNode(d->m_rootJoint);
    return creationChange;
}

} // namespace Qt3DCore

QT_END_NAMESPACE
