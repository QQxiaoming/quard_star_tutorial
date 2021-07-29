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

#include "handler_p.h"
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DAnimation/private/loadanimationclipjob_p.h>
#include <Qt3DAnimation/private/findrunningclipanimatorsjob_p.h>
#include <Qt3DAnimation/private/updatepropertymapjob_p.h>
#include <Qt3DAnimation/private/evaluateclipanimatorjob_p.h>
#include <Qt3DAnimation/private/buildblendtreesjob_p.h>
#include <Qt3DAnimation/private/evaluateblendclipanimatorjob_p.h>
#include <Qt3DAnimation/private/animationlogging_p.h>
#include <Qt3DAnimation/private/buildblendtreesjob_p.h>
#include <Qt3DAnimation/private/evaluateblendclipanimatorjob_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

Handler::Handler()
    : m_animationClipLoaderManager(new AnimationClipLoaderManager)
    , m_clockManager(new ClockManager)
    , m_clipAnimatorManager(new ClipAnimatorManager)
    , m_blendedClipAnimatorManager(new BlendedClipAnimatorManager)
    , m_channelMappingManager(new ChannelMappingManager)
    , m_channelMapperManager(new ChannelMapperManager)
    , m_clipBlendNodeManager(new ClipBlendNodeManager)
    , m_skeletonManager(new SkeletonManager)
    , m_loadAnimationClipJob(new LoadAnimationClipJob)
    , m_findRunningClipAnimatorsJob(new FindRunningClipAnimatorsJob)
    , m_updatePropertyMapJob(new UpdatePropertyMapJob)
    , m_buildBlendTreesJob(new BuildBlendTreesJob)
    , m_simulationTime(0)
{
    m_loadAnimationClipJob->setHandler(this);
    m_findRunningClipAnimatorsJob->setHandler(this);
    m_buildBlendTreesJob->setHandler(this);
    m_updatePropertyMapJob->setHandler(this);
}

Handler::~Handler()
{
}

void Handler::setDirty(DirtyFlag flag, Qt3DCore::QNodeId nodeId)
{
    switch (flag) {
    case AnimationClipDirty: {
        QMutexLocker lock(&m_mutex);
        const auto handle = m_animationClipLoaderManager->lookupHandle(nodeId);
        m_dirtyAnimationClips.push_back(handle);
        break;
    }

    case ChannelMappingsDirty: {
        QMutexLocker lock(&m_mutex);
        const auto handle = m_channelMapperManager->lookupHandle(nodeId);
        m_dirtyChannelMappers.push_back(handle);
        break;
    }

    case ClipAnimatorDirty: {
        QMutexLocker lock(&m_mutex);
        const auto handle = m_clipAnimatorManager->lookupHandle(nodeId);
        m_dirtyClipAnimators.push_back(handle);
        break;
    }

    case ClipAnimatorMapDirty: {
        QMutexLocker lock(&m_mutex);
        const auto handle = m_clipAnimatorManager->lookupHandle(nodeId);
        m_dirtyClipAnimatorMaps.push_back(handle);
        break;
    }

    case BlendedClipAnimatorDirty: {
        QMutexLocker lock(&m_mutex);
        const HBlendedClipAnimator handle = m_blendedClipAnimatorManager->lookupHandle(nodeId);
        m_dirtyBlendedAnimators.push_back(handle);
        break;
    }
    }
}

void Handler::setClipAnimatorRunning(const HClipAnimator &handle, bool running)
{
    // Add clip to running set if not already present
    if (running && !m_runningClipAnimators.contains(handle)) {
        m_runningClipAnimators.push_back(handle);
        ClipAnimator *clipAnimator = m_clipAnimatorManager->data(handle);
        if (clipAnimator)
            clipAnimator->setStartTime(m_simulationTime);
    }

    // If being marked as not running, remove from set of running clips
    if (!running) {
        const auto it = std::find_if(m_runningClipAnimators.begin(),
                                     m_runningClipAnimators.end(),
                                     [handle](const HClipAnimator &h) { return h == handle; });
        if (it != m_runningClipAnimators.end())
            m_runningClipAnimators.erase(it);
    }
}

void Handler::setBlendedClipAnimatorRunning(const HBlendedClipAnimator &handle, bool running)
{
    // Add clip to running set if not already present
    if (running && !m_runningBlendedClipAnimators.contains(handle)) {
        m_runningBlendedClipAnimators.push_back(handle);
        BlendedClipAnimator *blendedClipAnimator = m_blendedClipAnimatorManager->data(handle);
        if (blendedClipAnimator)
            blendedClipAnimator->setStartTime(m_simulationTime);
    }

    // If being marked as not running, remove from set of running clips
    if (!running) {
        const auto it = std::find_if(m_runningBlendedClipAnimators.begin(),
                                     m_runningBlendedClipAnimators.end(),
                                     [handle](const HBlendedClipAnimator &h) { return h == handle; });
        if (it != m_runningBlendedClipAnimators.end())
            m_runningBlendedClipAnimators.erase(it);
    }
}

// The vectors may get outdated when the application removes/deletes an
// animator component in the meantime. Recognize this. This should be
// relatively infrequent so in most cases the vectors will not change at all.
void Handler::cleanupHandleList(QVector<HAnimationClip> *clips)
{
    for (auto it = clips->begin(); it != clips->end(); ) {
        if (!m_animationClipLoaderManager->data(*it))
            it = clips->erase(it);
        else
            ++it;
    }
}

void Handler::cleanupHandleList(QVector<HClipAnimator> *animators)
{
    for (auto it = animators->begin(); it != animators->end(); ) {
        if (!m_clipAnimatorManager->data(*it))
            it = animators->erase(it);
        else
            ++it;
    }
}

void Handler::cleanupHandleList(QVector<HBlendedClipAnimator> *animators)
{
    for (auto it = animators->begin(); it != animators->end(); ) {
        if (!m_blendedClipAnimatorManager->data(*it))
            it = animators->erase(it);
        else
            ++it;
    }
}

QVector<Qt3DCore::QAspectJobPtr> Handler::jobsToExecute(qint64 time)
{
    // Store the simulation time so we can mark the start time of
    // animators which will allow us to calculate the local time of
    // animation clips.
    m_simulationTime = time;

    QVector<Qt3DCore::QAspectJobPtr> jobs;

    QMutexLocker lock(&m_mutex);

    // If there are any dirty animation clips that need loading,
    // queue up a job for them
    const bool hasLoadAnimationClipJob = !m_dirtyAnimationClips.isEmpty();
    if (hasLoadAnimationClipJob) {
        qCDebug(HandlerLogic) << "Added LoadAnimationClipJob";
        cleanupHandleList(&m_dirtyAnimationClips);
        m_loadAnimationClipJob->addDirtyAnimationClips(m_dirtyAnimationClips);
        jobs.push_back(m_loadAnimationClipJob);
        m_dirtyAnimationClips.clear();
    }

    // If there are dirty clip animators, find the set that are able to
    // run. I.e. are marked as running and have animation clips and
    // channel mappings

    const bool hasFindRunningClipAnimatorsJob = !m_dirtyClipAnimators.isEmpty();
    if (hasFindRunningClipAnimatorsJob) {
        qCDebug(HandlerLogic) << "Added FindRunningClipAnimatorsJob";
        m_findRunningClipAnimatorsJob->removeDependency(QWeakPointer<Qt3DCore::QAspectJob>());
        cleanupHandleList(&m_dirtyClipAnimators);
        m_findRunningClipAnimatorsJob->setDirtyClipAnimators(m_dirtyClipAnimators);
        jobs.push_back(m_findRunningClipAnimatorsJob);
        if (hasLoadAnimationClipJob)
            m_findRunningClipAnimatorsJob->addDependency(m_loadAnimationClipJob);
        m_dirtyClipAnimators.clear();
    }

    const bool hasUpdatePropertyMapJob = !m_dirtyClipAnimatorMaps.isEmpty();
    if (hasUpdatePropertyMapJob) {
        qCDebug(HandlerLogic) << "Added UpdatePropertyMapJob";
        cleanupHandleList(&m_dirtyClipAnimatorMaps);
        m_updatePropertyMapJob->setDirtyClipAnimators(m_dirtyClipAnimatorMaps);
        m_updatePropertyMapJob->removeDependency(QWeakPointer<Qt3DCore::QAspectJob>());
        jobs.push_back(m_updatePropertyMapJob);
        if (hasFindRunningClipAnimatorsJob)
            m_updatePropertyMapJob->addDependency(m_findRunningClipAnimatorsJob);
        m_dirtyClipAnimatorMaps.clear();
    }

    // Rebuild blending trees if a blend tree is dirty
    const bool hasBuildBlendTreesJob = !m_dirtyBlendedAnimators.isEmpty();
    if (hasBuildBlendTreesJob) {
        const QVector<HBlendedClipAnimator> dirtyBlendedAnimators = std::move(m_dirtyBlendedAnimators);
        m_buildBlendTreesJob->setBlendedClipAnimators(dirtyBlendedAnimators);
        jobs.push_back(m_buildBlendTreesJob);
    }

    // TODO: Parallelise the animator evaluation and property building at a finer level

    // If there are any running ClipAnimators, evaluate them for the current
    // time and send property changes
    cleanupHandleList(&m_runningClipAnimators);
    if (!m_runningClipAnimators.isEmpty()) {
        qCDebug(HandlerLogic) << "Added EvaluateClipAnimatorJobs";

        // Ensure we have a job per clip animator
        const int oldSize = m_evaluateClipAnimatorJobs.size();
        const int newSize = m_runningClipAnimators.size();
        if (oldSize < newSize) {
            m_evaluateClipAnimatorJobs.resize(newSize);
            for (int i = oldSize; i < newSize; ++i) {
                m_evaluateClipAnimatorJobs[i] = QSharedPointer<EvaluateClipAnimatorJob>::create();
                m_evaluateClipAnimatorJobs[i]->setHandler(this);
            }
        }

        // Set each job up with an animator to process and set dependencies
        for (int i = 0; i < newSize; ++i) {
            m_evaluateClipAnimatorJobs[i]->setClipAnimator(m_runningClipAnimators[i]);
            m_evaluateClipAnimatorJobs[i]->removeDependency(QWeakPointer<Qt3DCore::QAspectJob>());
            if (hasLoadAnimationClipJob &&
                    !m_evaluateClipAnimatorJobs[i]->dependencies().contains(m_loadAnimationClipJob))
                m_evaluateClipAnimatorJobs[i]->addDependency(m_loadAnimationClipJob);

            if (hasFindRunningClipAnimatorsJob &&
                    !m_evaluateClipAnimatorJobs[i]->dependencies().contains(m_findRunningClipAnimatorsJob))
                m_evaluateClipAnimatorJobs[i]->addDependency(m_findRunningClipAnimatorsJob);

            if (hasUpdatePropertyMapJob &&
                    !m_evaluateClipAnimatorJobs[i]->dependencies().contains(m_updatePropertyMapJob))
                m_evaluateClipAnimatorJobs[i]->addDependency(m_updatePropertyMapJob);
            jobs.push_back(m_evaluateClipAnimatorJobs[i]);
        }
    }

    // BlendClipAnimator execution
    cleanupHandleList(&m_runningBlendedClipAnimators);
    if (!m_runningBlendedClipAnimators.isEmpty()) {
        // Ensure we have a job per clip animator
        const int oldSize = m_evaluateBlendClipAnimatorJobs.size();
        const int newSize = m_runningBlendedClipAnimators.size();
        if (oldSize < newSize) {
            m_evaluateBlendClipAnimatorJobs.resize(newSize);
            for (int i = oldSize; i < newSize; ++i) {
                m_evaluateBlendClipAnimatorJobs[i] = QSharedPointer<EvaluateBlendClipAnimatorJob>::create();
                m_evaluateBlendClipAnimatorJobs[i]->setHandler(this);
            }
        }

        // Set each job up with an animator to process and set dependencies
        for (int i = 0; i < newSize; ++i) {
            m_evaluateBlendClipAnimatorJobs[i]->setBlendClipAnimator(m_runningBlendedClipAnimators[i]);
            m_evaluateBlendClipAnimatorJobs[i]->removeDependency(QWeakPointer<Qt3DCore::QAspectJob>());
            if (hasLoadAnimationClipJob)
                m_evaluateBlendClipAnimatorJobs[i]->addDependency(m_loadAnimationClipJob);
            if (hasBuildBlendTreesJob)
                m_evaluateBlendClipAnimatorJobs[i]->addDependency(m_buildBlendTreesJob);
            jobs.push_back(m_evaluateBlendClipAnimatorJobs[i]);
        }
    }

    return jobs;
}

} // namespace Animation
} // namespace Qt3DAnimation

QT_END_NAMESPACE
