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

#include "qskeletonloader.h"
#include "qskeletonloader_p.h"
#include <Qt3DCore/qjoint.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qskeletoncreatedchange_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

QSkeletonLoaderPrivate::QSkeletonLoaderPrivate()
    : QAbstractSkeletonPrivate()
    , m_source()
    , m_createJoints(false)
    , m_status(QSkeletonLoader::NotReady)
    , m_rootJoint(nullptr)
{
    m_type = QSkeletonCreatedChangeBase::SkeletonLoader;
}

void QSkeletonLoaderPrivate::setStatus(QSkeletonLoader::Status status)
{
    Q_Q(QSkeletonLoader);
    if (status != m_status) {
        m_status = status;
        const bool blocked = q->blockNotifications(true);
        emit q->statusChanged(m_status);
        q->blockNotifications(blocked);
    }
}

/*!
    \qmltype SkeletonLoader
    \inqmlmodule Qt3D.Core
    \inherits AbstractSkeleton
    \instantiates Qt3DCore::QSkeletonLoader
    \since 5.10
    \brief Used to load a skeleton of joints from file.

    Use SkeletonLoader if you wish to load a whole skeleton from file rather
    than creating the joints yourself using Skeleton and Joints. Creating a
    skeleton and binding the vertices of a mesh to the skeleton is most easily
    performed in a 3D digital content creation tool such as Blender. The
    resulting skeleton and mesh can then be exported in a suitable format such
    as glTF 2 for consumption by Qt 3D.
*/

/*!
    \qmlproperty url SkeletonLoader::source

    Holds the source url from which to load the skeleton.
*/

/*!
    \qmlproperty SkeletonLoader.Status SkeletonLoader::status

    Holds the current status of skeleton loading.
*/

/*!
    \class Qt3DCore::QSkeletonLoader
    \inmodule Qt3DCore
    \inherits Qt3DCore::QAbstractSkeleton
    \since 5.10
    \brief Used to load a skeleton of joints from file.

    Use SkeletonLoader if you wish to load a whole skeleton from file rather
    than creating the joints yourself using Skeleton and Joints. Creating a
    skeleton and binding the vertices of a mesh to the skeleton is most easily
    performed in a 3D digital content creation tool such as Blender. The
    resulting skeleton and mesh can then be exported in a suitable format such
    as glTF 2 for consumption by Qt 3D.
*/

/*!
    \enum QSkeletonLoader::Status

    This enum identifies the status of skeleton.

    \value NotReady              The skeleton has not been loaded yet
    \value Ready                 The skeleton was successfully loaded
    \value Error                 An error occurred while loading the skeleton
*/
/*!
    \property Qt3DCore::QSkeletonLoader::createJointsEnabled

    \brief A boolean to indicate whether createJoints is enabled or not.
*/
/*!
    Constructs a new QSkeletonLoader with \a parent.
*/
QSkeletonLoader::QSkeletonLoader(Qt3DCore::QNode *parent)
    : QAbstractSkeleton(*new QSkeletonLoaderPrivate, parent)
{
}

/*!
    Constructs a new QSkeletonLoader with \a parent and sets the \a source.
*/
QSkeletonLoader::QSkeletonLoader(const QUrl &source, QNode *parent)
    : QAbstractSkeleton(*new QSkeletonLoaderPrivate, parent)
{
    setSource(source);
}

/*! \internal */
QSkeletonLoader::QSkeletonLoader(QSkeletonLoaderPrivate &dd, Qt3DCore::QNode *parent)
    : QAbstractSkeleton(dd, parent)
{
}

/*! \internal */
QSkeletonLoader::~QSkeletonLoader()
{
}

/*!
    \property Qt3DCore::QSkeletonLoader::source

    Holds the source url from which to load the skeleton.
*/
QUrl QSkeletonLoader::source() const
{
    Q_D(const QSkeletonLoader);
    return d->m_source;
}

/*!
    \property Qt3DCore::QSkeletonLoader::status

    Holds the current status of skeleton loading.
*/
QSkeletonLoader::Status QSkeletonLoader::status() const
{
    Q_D(const QSkeletonLoader);
    return d->m_status;
}

/*!
    Returns a boolean indicating whether CreateJoints
    is enabled or not.
    The default value is \c false.
*/
bool QSkeletonLoader::isCreateJointsEnabled() const
{
    Q_D(const QSkeletonLoader);
    return d->m_createJoints;
}
/*!
    Returns the root joint of the hierarchy of joints forming the skeleton.
*/
Qt3DCore::QJoint *QSkeletonLoader::rootJoint() const
{
    Q_D(const QSkeletonLoader);
    return d->m_rootJoint;
}

void QSkeletonLoader::setSource(const QUrl &source)
{
    Q_D(QSkeletonLoader);
    if (d->m_source == source)
        return;

    d->m_source = source;
    emit sourceChanged(source);
}

void QSkeletonLoader::setCreateJointsEnabled(bool createJoints)
{
    Q_D(QSkeletonLoader);
    if (d->m_createJoints == createJoints)
        return;

    d->m_createJoints = createJoints;
    emit createJointsEnabledChanged(createJoints);
}

void QSkeletonLoader::setRootJoint(QJoint *rootJoint)
{
    Q_D(QSkeletonLoader);
    if (rootJoint == d->m_rootJoint)
        return;

    if (d->m_rootJoint)
        d->unregisterDestructionHelper(d->m_rootJoint);

    if (rootJoint && !rootJoint->parent())
        rootJoint->setParent(this);

    d->m_rootJoint = rootJoint;

    // Ensures proper bookkeeping
    if (d->m_rootJoint)
        d->registerDestructionHelper(d->m_rootJoint, &QSkeletonLoader::setRootJoint, d->m_rootJoint);

    emit rootJointChanged(d->m_rootJoint);
}

/*! \internal */
void QSkeletonLoader::sceneChangeEvent(const QSceneChangePtr &change)
{
    Q_D(QSkeletonLoader);
    if (change->type() == Qt3DCore::PropertyUpdated) {
        auto propertyChange = qSharedPointerCast<QStaticPropertyUpdatedChangeBase>(change);
        if (propertyChange->propertyName() == QByteArrayLiteral("status")) {
            const auto e = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(change);
            d->setStatus(static_cast<QSkeletonLoader::Status>(e->value().toInt()));
        } else if (propertyChange->propertyName() == QByteArrayLiteral("rootJoint")) {
            auto typedChange = qSharedPointerCast<QJointChange>(propertyChange);
            auto rootJoint = std::move(typedChange->data);
            setRootJoint(rootJoint.release());
        }
    }
    QAbstractSkeleton::sceneChangeEvent(change);
}

/*! \internal */
Qt3DCore::QNodeCreatedChangeBasePtr QSkeletonLoader::createNodeCreationChange() const
{
    auto creationChange = QSkeletonCreatedChangePtr<QSkeletonLoaderData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QSkeletonLoader);
    data.source = d->m_source;
    data.createJoints = d->m_createJoints;
    return creationChange;
}

} // namespace Qt3DCore

QT_END_NAMESPACE
