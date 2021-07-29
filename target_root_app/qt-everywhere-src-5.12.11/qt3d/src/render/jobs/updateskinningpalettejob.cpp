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

#include "updateskinningpalettejob_p.h"
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/handle_types_p.h>
#include <Qt3DRender/private/job_common_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
namespace Render {

UpdateSkinningPaletteJob::UpdateSkinningPaletteJob()
    : Qt3DCore::QAspectJob()
    , m_nodeManagers(nullptr)
    , m_root()
{
    SET_JOB_RUN_STAT_TYPE(this, JobTypes::UpdateSkinningPalette, 0);
}

UpdateSkinningPaletteJob::~UpdateSkinningPaletteJob()
{
}

void UpdateSkinningPaletteJob::run()
{
    auto armatureManager = m_nodeManagers->armatureManager();
    if (armatureManager->count() == 0)
        return;

    // TODO: Decompose this job across several jobs, say one per skeleton so
    // that it can be done in parallel

    // Update the local pose transforms of JointInfo's in Skeletons from
    // the set of dirty joints.
    for (const auto &jointHandle : qAsConst(m_dirtyJoints)) {
        Joint *joint = m_nodeManagers->jointManager()->data(jointHandle);
        Q_ASSERT(joint);
        Skeleton *skeleton = m_nodeManagers->skeletonManager()->data(joint->owningSkeleton());
        Q_ASSERT(skeleton);
        if (skeleton->isEnabled() && joint->isEnabled())
            skeleton->setLocalPose(jointHandle, joint->localPose());
    }

    // Find all the armature components and update their skinning palettes
    QVector<HArmature> dirtyArmatures;
    findDirtyArmatures(m_root, dirtyArmatures);

    // Update the skeleton for each dirty armature
    auto skeletonManager = m_nodeManagers->skeletonManager();
    for (const auto &armatureHandle : qAsConst(dirtyArmatures)) {
        auto armature = armatureManager->data(armatureHandle);
        Q_ASSERT(armature);

        auto skeletonId = armature->skeletonId();
        auto skeleton = skeletonManager->lookupResource(skeletonId);
        Q_ASSERT(skeleton);

        const QVector<QMatrix4x4> skinningPalette = skeleton->calculateSkinningMatrixPalette();
        armature->skinningPaletteUniform().setData(skinningPalette);
    }
}

void UpdateSkinningPaletteJob::findDirtyArmatures(Entity *entity,
                                                  QVector<HArmature> &armatures) const
{
    // Just return all enabled armatures found on entities for now
    // TODO: Be smarter about limiting which armatures we update. For e.g. only
    // those with skeletons that have changed and only those that are within view
    // of one or more renderviews.
    const auto armatureHandle = entity->componentHandle<Armature>();
    if (!armatureHandle.isNull() && !armatures.contains(armatureHandle))
        armatures.push_back(armatureHandle);

    const auto children = entity->children();
    for (const auto child : children)
        findDirtyArmatures(child, armatures);
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
