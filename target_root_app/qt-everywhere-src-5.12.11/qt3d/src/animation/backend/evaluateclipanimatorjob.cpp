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

#include "evaluateclipanimatorjob_p.h"
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DAnimation/private/animationlogging_p.h>
#include <Qt3DAnimation/private/animationutils_p.h>
#include <Qt3DAnimation/private/job_common_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

EvaluateClipAnimatorJob::EvaluateClipAnimatorJob()
    : Qt3DCore::QAspectJob()
{
    SET_JOB_RUN_STAT_TYPE(this, JobTypes::EvaluateClipAnimator, 0);
}

void EvaluateClipAnimatorJob::run()
{
    Q_ASSERT(m_handler);

    ClipAnimator *clipAnimator = m_handler->clipAnimatorManager()->data(m_clipAnimatorHandle);
    Q_ASSERT(clipAnimator);
    const bool running = clipAnimator->isRunning();
    const bool seeking = clipAnimator->isSeeking();
    if (!running && !seeking) {
        m_handler->setClipAnimatorRunning(m_clipAnimatorHandle, false);
        return;
    }

    const qint64 globalTimeNS = m_handler->simulationTime();

    Clock *clock = m_handler->clockManager()->lookupResource(clipAnimator->clockId());

    // Evaluate the fcurves
    AnimationClip *clip = m_handler->animationClipLoaderManager()->lookupResource(clipAnimator->clipId());
    Q_ASSERT(clip);

    const qint64 nsSincePreviousFrame = seeking ? toNsecs(clip->duration() * clipAnimator->normalizedLocalTime())
                                                : clipAnimator->nsSincePreviousFrame(globalTimeNS);

    // Prepare for evaluation (convert global time to local time ....)
    const AnimatorEvaluationData animatorEvaluationData = evaluationDataForAnimator(clipAnimator,
                                                                                    clock,
                                                                                    nsSincePreviousFrame);

    const ClipEvaluationData preEvaluationDataForClip = evaluationDataForClip(clip, animatorEvaluationData);
    const ClipResults rawClipResults = evaluateClipAtPhase(clip, preEvaluationDataForClip.normalizedLocalTime);

    // Reformat the clip results into the layout used by this animator/blend tree
    const ClipFormat clipFormat = clipAnimator->clipFormat();
    ClipResults formattedClipResults = formatClipResults(rawClipResults, clipFormat.sourceClipIndices);

    if (preEvaluationDataForClip.isFinalFrame)
        clipAnimator->setRunning(false);

    clipAnimator->setCurrentLoop(preEvaluationDataForClip.currentLoop);
    clipAnimator->setLastGlobalTimeNS(globalTimeNS);
    clipAnimator->setLastLocalTime(preEvaluationDataForClip.localTime);
    clipAnimator->setLastNormalizedLocalTime(preEvaluationDataForClip.normalizedLocalTime);
    clipAnimator->setNormalizedLocalTime(-1.0f); // Re-set to something invalid.

    // Prepare property changes (if finalFrame it also prepares the change for the running property for the frontend)
    const QVector<Qt3DCore::QSceneChangePtr> changes = preparePropertyChanges(clipAnimator->peerId(),
                                                                              clipAnimator->mappingData(),
                                                                              formattedClipResults,
                                                                              preEvaluationDataForClip.isFinalFrame,
                                                                              preEvaluationDataForClip.normalizedLocalTime);

    // Send the property changes
    clipAnimator->sendPropertyChanges(changes);

    // Trigger callbacks either on this thread or by notifying the gui thread.
    const QVector<AnimationCallbackAndValue> callbacks = prepareCallbacks(clipAnimator->mappingData(),
                                                                          formattedClipResults);
    clipAnimator->sendCallbacks(callbacks);
}

} // namespace Animation
} // namespace Qt3DAnimation

QT_END_NAMESPACE
