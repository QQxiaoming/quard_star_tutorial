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

#include "qabstractskeleton.h"
#include "qabstractskeleton_p.h"
#include <Qt3DCore/qpropertyupdatedchange.h>

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

QAbstractSkeletonPrivate::QAbstractSkeletonPrivate()
    : Qt3DCore::QNodePrivate()
    , m_jointCount(0)
{
}

/*!
    \internal
 */
const QAbstractSkeletonPrivate *QAbstractSkeletonPrivate::get(const QAbstractSkeleton *q)
{
    return q->d_func();
}

/*!
    \qmltype AbstractSkeleton
    \inqmlmodule Qt3D.Core
    \inherits Node
    \instantiates Qt3DCore::QAbstractSkeleton
    \since 5.10
    \brief A skeleton contains the joints for a skinned mesh.

    Do not use this class directly. You should use SkeletonLoader if loading
    skeleton data from a file (most likely) or Skeleton if creating the
    skeleton and skinned mesh data yourself (mainly for people creating
    editors or tooling).
*/

/*!
    \class Qt3DCore::QAbstractSkeleton
    \inmodule Qt3DCore
    \inherits Qt3DCore::QNode
    \since 5.10
    \brief A skeleton contains the joints for a skinned mesh.

    Do not use this class directly. You should use QSkeletonLoader if loading
    skeleton data from a file (most likely) or QSkeleton if creating the
    skeleton and skinned mesh data yourself (mainly for people creating
    editors or tooling).
*/

/*! \internal */
QAbstractSkeleton::QAbstractSkeleton(QAbstractSkeletonPrivate &dd, Qt3DCore::QNode *parent)
    : Qt3DCore::QNode(dd, parent)
{
}

/*! \internal */
QAbstractSkeleton::~QAbstractSkeleton()
{
}

/*!
    \property Qt3DCore::QAbstractSkeleton::jointCount

    Holds the number of joints contained in the skeleton
*/
int QAbstractSkeleton::jointCount() const
{
    Q_D(const QAbstractSkeleton);
    return d->m_jointCount;
}

void QAbstractSkeletonPrivate::setJointCount(int jointCount)
{
    Q_Q(QAbstractSkeleton);
    if (m_jointCount == jointCount)
        return;
    m_jointCount = jointCount;
    const bool block = q->blockNotifications(true);
    emit q->jointCountChanged(jointCount);
    q->blockNotifications(block);
}

void QAbstractSkeleton::sceneChangeEvent(const QSceneChangePtr &change)
{
    Q_D(QAbstractSkeleton);
    if (change->type() == Qt3DCore::PropertyUpdated) {
        const Qt3DCore::QPropertyUpdatedChangePtr e = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(change);
        if (e->propertyName() == QByteArrayLiteral("jointCount"))
            d->setJointCount(e->value().toInt());
    }
}

} // namespace Qt3DCore

QT_END_NAMESPACE
