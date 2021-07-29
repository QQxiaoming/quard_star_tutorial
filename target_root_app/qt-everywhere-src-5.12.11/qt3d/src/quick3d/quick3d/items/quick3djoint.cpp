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

#include "quick3djoint_p.h"
#include <Qt3DCore/qjoint.h>

QT_BEGIN_NAMESPACE

namespace Qt3DCore {
namespace Quick {

Quick3DJoint::Quick3DJoint(QObject *parent)
    : QObject(parent)
{
}

/*!
    \qmlproperty list<Joint> Qt3DCore::Joint::childJoints
    \readonly
*/
QQmlListProperty<QJoint> Quick3DJoint::childJoints()
{
    return QQmlListProperty<QJoint>(this, 0,
                                    Quick3DJoint::appendJoint,
                                    Quick3DJoint::jointCount,
                                    Quick3DJoint::jointAt,
                                    Quick3DJoint::clearJoints);
}

void Quick3DJoint::appendJoint(QQmlListProperty<QJoint> *list, QJoint *joint)
{
    Quick3DJoint *jointExtension = qobject_cast<Quick3DJoint *>(list->object);
    jointExtension->parentJoint()->addChildJoint(joint);
}

QJoint *Quick3DJoint::jointAt(QQmlListProperty<QJoint> *list, int index)
{
    Quick3DJoint *jointExtension = qobject_cast<Quick3DJoint *>(list->object);
    return jointExtension->parentJoint()->childJoints().at(index);
}

int Quick3DJoint::jointCount(QQmlListProperty<QJoint> *list)
{
    Quick3DJoint *jointExtension = qobject_cast<Quick3DJoint *>(list->object);
    return jointExtension->parentJoint()->childJoints().count();
}

void Quick3DJoint::clearJoints(QQmlListProperty<QJoint> *list)
{
    Quick3DJoint *jointExtension = qobject_cast<Quick3DJoint *>(list->object);
    const auto joints = jointExtension->parentJoint()->childJoints();
    for (QJoint *joint : joints)
        jointExtension->parentJoint()->removeChildJoint(joint);
}

} // namespace Quick
} // namespace Qt3DCore

QT_END_NAMESPACE
