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

#include "buildblendtreesjob_p.h"
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DAnimation/private/clipblendnodevisitor_p.h>
#include <Qt3DAnimation/private/clipblendnode_p.h>
#include <Qt3DAnimation/private/clipblendvalue_p.h>
#include <Qt3DAnimation/private/lerpclipblend_p.h>
#include <Qt3DAnimation/private/job_common_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

BuildBlendTreesJob::BuildBlendTreesJob()
    : Qt3DCore::QAspectJob()
{
    SET_JOB_RUN_STAT_TYPE(this, JobTypes::BuildBlendTree, 0);
}

void BuildBlendTreesJob::setBlendedClipAnimators(const QVector<HBlendedClipAnimator> &blendedClipAnimatorHandles)
{
    m_blendedClipAnimatorHandles = blendedClipAnimatorHandles;
    BlendedClipAnimatorManager *blendedClipAnimatorManager = m_handler->blendedClipAnimatorManager();
    BlendedClipAnimator *blendedClipAnimator = nullptr;
    for (const auto &blendedClipAnimatorHandle : qAsConst(m_blendedClipAnimatorHandles)) {
        blendedClipAnimator = blendedClipAnimatorManager->data(blendedClipAnimatorHandle);
        Q_ASSERT(blendedClipAnimator);
    }
}

// Note this job is run once for all stopped blended animators that have been marked dirty
// We assume that the structure of blend node tree does not change once a BlendClipAnimator has been set to running
void BuildBlendTreesJob::run()
{
    for (const HBlendedClipAnimator &blendedClipAnimatorHandle : qAsConst(m_blendedClipAnimatorHandles)) {
        // Retrieve BlendTree node
        BlendedClipAnimator *blendClipAnimator = m_handler->blendedClipAnimatorManager()->data(blendedClipAnimatorHandle);
        Q_ASSERT(blendClipAnimator);


        const bool canRun = blendClipAnimator->canRun();
        const bool running = blendClipAnimator->isRunning();
        const bool seeking = blendClipAnimator->isSeeking();
        m_handler->setBlendedClipAnimatorRunning(blendedClipAnimatorHandle, canRun && (seeking || running));

        if (!canRun && !(seeking || running))
            continue;

        // Build the format for clip results that should be used by nodes in the blend
        // tree when used with this animator
        const ChannelMapper *mapper = m_handler->channelMapperManager()->lookupResource(blendClipAnimator->mapperId());
        Q_ASSERT(mapper);
        const QVector<ChannelNameAndType> channelNamesAndTypes
                = buildRequiredChannelsAndTypes(m_handler, mapper);
        const QVector<ComponentIndices> channelComponentIndices
                = assignChannelComponentIndices(channelNamesAndTypes);

        // Find the leaf value nodes of the blend tree and for each of them
        // create a set of format indices that can later be used to map the
        // raw ClipResults resulting from evaluating an animation clip to the
        // layout used by the blend tree for this animator
        QVector<QBitArray> blendTreeChannelMask;
        const QVector<Qt3DCore::QNodeId> valueNodeIds
                = gatherValueNodesToEvaluate(m_handler, blendClipAnimator->blendTreeRootId());

        // Store the clip value nodes to avoid further lookups below.
        // TODO: Refactor this next block into a function in animationutils.cpp that takes
        // a QVector<QClipBlendValue*> as input.
        QVector<ClipBlendValue *> valueNodes;
        valueNodes.reserve(valueNodeIds.size());
        for (const auto valueNodeId : valueNodeIds) {
            ClipBlendValue *valueNode
                    = static_cast<ClipBlendValue *>(m_handler->clipBlendNodeManager()->lookupNode(valueNodeId));
            Q_ASSERT(valueNode);
            valueNodes.push_back(valueNode);

            const Qt3DCore::QNodeId clipId = valueNode->clipId();
            AnimationClip *clip = m_handler->animationClipLoaderManager()->lookupResource(clipId);
            Q_ASSERT(clip);

            const ClipFormat format = generateClipFormatIndices(channelNamesAndTypes,
                                                                channelComponentIndices,
                                                                clip);
            valueNode->setClipFormat(blendClipAnimator->peerId(), format);

            // this BlendClipAnimator needs to be notified when the clip has been loaded
            clip->addDependingBlendedClipAnimator(blendClipAnimator->peerId());

            // Combine the masks from each source clip to see which channels should be
            // evaluated and result in a mappingData being created. If any contributing clip
            // supports a channel, that will produce a channel mapping. Clips with those channels
            // missing will use default values when blending:
            //
            //  Default scale = (1, 1, 1)
            //  Default rotation = (1, 0, 0, 0)
            //  Default translation = (0, 0, 0)
            //  Default joint transforms should be taken from the skeleton initial pose
            //
            // Everything else has all components set to 0. If user wants something else, they
            // should provide a clip with a single keyframe at the desired default value.
            if (blendTreeChannelMask.isEmpty()) {
                // Initialize the blend tree mask from the mask of the first clip
                blendTreeChannelMask = format.sourceClipMask;
            } else {
                // We allow through a channel if any source clip in the tree has
                // data for that channel. Clips without data for a channel will
                // have default values substituted when evaluating the blend tree.
                int channelIndex = 0;
                for (const auto &channelMask : qAsConst(format.sourceClipMask))
                    blendTreeChannelMask[channelIndex++] |= channelMask;
            }
        }

        // Now that we know the overall blend tree mask, go back and compare this to
        // the masks from each of the value nodes. If the overall mask requires a
        // channel but the value node does not provide it, we need to store default
        // values to use for that channel so that the blending evaluation works as
        // expected.
        for (const auto valueNode : valueNodes) {
            ClipFormat &f = valueNode->clipFormat(blendClipAnimator->peerId());

            const int channelCount = blendTreeChannelMask.size();
            for (int i = 0; i < channelCount; ++i) {
                if (blendTreeChannelMask[i] == f.sourceClipMask[i])
                    continue; // Masks match, nothing to do

                // If we get to here then we need to obtain a default value
                // for this channel and store it for later application to any
                // missing components of this channel.
                const QVector<float> defaultValue = defaultValueForChannel(m_handler,
                                                                           f.namesAndTypes[i]);

                // Find the indices where we later need to inject these default
                // values and store them in the format.
                const ComponentIndices &componentIndices = f.formattedComponentIndices[i];
                Q_ASSERT(componentIndices.size() == defaultValue.size());
                for (int j = 0; j < defaultValue.size(); ++j)
                    f.defaultComponentValues.push_back({componentIndices[j], defaultValue[j]});
            }
        }

        // Finally, build the mapping data vector for this blended clip animator. This
        // gets used during the final stage of evaluation when sending the property changes
        // out to the targets of the animation. We do the costly work once up front.
        const QVector<Qt3DCore::QNodeId> channelMappingIds = mapper->mappingIds();
        QVector<ChannelMapping *> channelMappings;
        channelMappings.reserve(channelMappingIds.size());
        for (const auto mappingId : channelMappingIds) {
            ChannelMapping *mapping = m_handler->channelMappingManager()->lookupResource(mappingId);
            Q_ASSERT(mapping);
            channelMappings.push_back(mapping);
        }

        const QVector<MappingData> mappingDataVec
                = buildPropertyMappings(channelMappings,
                                        channelNamesAndTypes,
                                        channelComponentIndices,
                                        blendTreeChannelMask);
        blendClipAnimator->setMappingData(mappingDataVec);
    }
}

} // Animation
} // Qt3DAnimation

QT_END_NAMESPACE
