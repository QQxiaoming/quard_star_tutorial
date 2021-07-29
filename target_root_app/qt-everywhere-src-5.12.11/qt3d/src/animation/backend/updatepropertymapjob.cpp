/****************************************************************************
**
** Copyright (C) 2019 Klaralvdalens Datakonsult AB (KDAB).
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

#include "updatepropertymapjob_p.h"

#include "findrunningclipanimatorsjob_p.h"
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DAnimation/private/animationlogging_p.h>
#include <Qt3DAnimation/private/animationutils_p.h>
#include <Qt3DAnimation/private/job_common_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

UpdatePropertyMapJob::UpdatePropertyMapJob()
    : Qt3DCore::QAspectJob()
{
    SET_JOB_RUN_STAT_TYPE(this, JobTypes::UpdatePropertyMap, 0)
}

void UpdatePropertyMapJob::setHandler(Handler *handler)
{
    m_handler = handler;
}

void UpdatePropertyMapJob::setDirtyClipAnimators(const QVector<HClipAnimator> &clipAnimators)
{
    m_clipAnimatorHandles = clipAnimators;
}

void UpdatePropertyMapJob::run()
{
    Q_ASSERT(m_handler);

    const auto &runningClipAnimators = m_handler->runningClipAnimators();
    auto *clipAnimatorManager = m_handler->clipAnimatorManager();
    for (const auto &clipAnimatorHandle : qAsConst(m_clipAnimatorHandles)) {

        ClipAnimator *clipAnimator = clipAnimatorManager->data(clipAnimatorHandle);
        Q_ASSERT(clipAnimator);
        if (!clipAnimator->isEnabled())
            continue;

        // Check the clipAnimator is running
        // If its not running, no sense in updating the mapping
        if (std::find(std::begin(runningClipAnimators), std::end(runningClipAnimators), clipAnimatorHandle) == std::end(runningClipAnimators))
            continue;

        // The clip animator needs to know how to map fcurve values through to properties on QNodes.
        // Now we know this animator can run, build the mapping table. Even though this could be
        // done a little simpler in the non-blended case, we follow the same code path as the
        // blended clip animator for consistency and ease of maintenance.
        const ChannelMapper *mapper = m_handler->channelMapperManager()->lookupResource(clipAnimator->mapperId());
        Q_ASSERT(mapper);
        const QVector<ChannelMapping *> channelMappings = mapper->mappings();

        const QVector<ChannelNameAndType> channelNamesAndTypes
                = buildRequiredChannelsAndTypes(m_handler, mapper);
        const QVector<ComponentIndices> channelComponentIndices
                = assignChannelComponentIndices(channelNamesAndTypes);

        const AnimationClip *clip = m_handler->animationClipLoaderManager()->lookupResource(clipAnimator->clipId());
        Q_ASSERT(clip);
        const ClipFormat format = generateClipFormatIndices(channelNamesAndTypes,
                                                            channelComponentIndices,
                                                            clip);
        clipAnimator->setClipFormat(format);

        const QVector<MappingData> mappingData = buildPropertyMappings(channelMappings,
                                                                       channelNamesAndTypes,
                                                                       format.formattedComponentIndices,
                                                                       format.sourceClipMask);
        clipAnimator->setMappingData(mappingData);
    }
}

} //  Animation
} //  Qt3DAnimation

QT_END_NAMESPACE
