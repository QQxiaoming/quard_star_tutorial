/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QTest>
#include <Qt3DAnimation/qanimationcliploader.h>
#include <Qt3DAnimation/qclipanimator.h>
#include <Qt3DAnimation/qchannelmapper.h>
#include <Qt3DAnimation/qchannelmapping.h>
#include <Qt3DAnimation/private/clipanimator_p.h>
#include <Qt3DAnimation/private/channelmapper_p.h>
#include <Qt3DAnimation/private/channelmapping_p.h>
#include <Qt3DAnimation/private/findrunningclipanimatorsjob_p.h>
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DCore/private/qnode_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qbackendnode_p.h>
#include <qbackendnodetester.h>
#include <testpostmanarbiter.h>

using namespace Qt3DAnimation::Animation;

Q_DECLARE_METATYPE(Qt3DAnimation::Animation::Handler*)
Q_DECLARE_METATYPE(QVector<Qt3DAnimation::Animation::HClipAnimator>)

class tst_FindRunningClipAnimatorsJob: public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT
public:
    ChannelMapper *createChannelMapper(Handler *handler)
    {
        auto channelMapperId = Qt3DCore::QNodeId::createId();
        ChannelMapper *channelMapper = handler->channelMapperManager()->getOrCreateResource(channelMapperId);
        setPeerId(channelMapper, channelMapperId);
        channelMapper->setHandler(handler);
        return channelMapper;
    }

    AnimationClip *createAnimationClip(Handler *handler)
    {
        auto clipId = Qt3DCore::QNodeId::createId();
        AnimationClip *clip = handler->animationClipLoaderManager()->getOrCreateResource(clipId);
        setPeerId(clip, clipId);
        clip->setHandler(handler);
        clip->setDuration(100);
        return clip;
    }

    ClipAnimator *createClipAnimator(Handler *handler,
                                     qint64 globalStartTimeNS,
                                     int loops)
    {
        auto animatorId = Qt3DCore::QNodeId::createId();
        ClipAnimator *animator = handler->clipAnimatorManager()->getOrCreateResource(animatorId);
        setPeerId(animator, animatorId);
        animator->setHandler(handler);
        animator->setStartTime(globalStartTimeNS);
        animator->setLoops(loops);
        return animator;
    }

private Q_SLOTS:
    void checkJob_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<QVector<HClipAnimator>>("dirtyClipAnimators");
        QTest::addColumn<bool>("isRunning");

        {
            Handler *handler;
            ClipAnimator *animator;
            QVector<HClipAnimator> dirtyClipAnimators;
            handler = new Handler();

            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            dirtyClipAnimators = (QVector<HClipAnimator>()
                                 << handler->clipAnimatorManager()->getOrAcquireHandle(animator->peerId()));

            animator->setRunning(true); // Has to be marked as running for the job to process it
            animator->setEnabled(true); // Has to be marked as enabled for the job to process it

            QTest::newRow("Missing clip and mapping")
                            << handler
                            << dirtyClipAnimators
                            << false;
        }

        {
            Handler *handler;
            AnimationClip *clip;
            ClipAnimator *animator;
            QVector<HClipAnimator> dirtyClipAnimators;
            handler = new Handler();
            clip = createAnimationClip(handler);

            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            animator->setClipId(clip->peerId());
            dirtyClipAnimators = (QVector<HClipAnimator>()
                                 << handler->clipAnimatorManager()->getOrAcquireHandle(animator->peerId()));

            animator->setRunning(true); // Has to be marked as running for the job to process it
            animator->setEnabled(true); // Has to be marked as enabled for the job to process it

            QTest::newRow("Missing mapper")
                            << handler
                            << dirtyClipAnimators
                            << false;
        }

        {
            Handler *handler;
            ClipAnimator *animator;
            QVector<HClipAnimator> dirtyClipAnimators;
            ChannelMapper *channelMapper;
            handler = new Handler();

            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            dirtyClipAnimators = (QVector<HClipAnimator>()
                                  << handler->clipAnimatorManager()->getOrAcquireHandle(animator->peerId()));

            channelMapper = createChannelMapper(handler);
            animator->setMapperId(channelMapper->peerId());
            animator->setRunning(true); // Has to be marked as running for the job to process it
            animator->setEnabled(false); // Has to be marked as enabled for the job to process it

            QTest::newRow("Missing clip")
                    << handler
                    << dirtyClipAnimators
                    << false;
        }

        {
            Handler *handler;
            AnimationClip *clip;
            ClipAnimator *animator;
            QVector<HClipAnimator> dirtyClipAnimators;
            ChannelMapper *channelMapper;
            handler = new Handler();

            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            dirtyClipAnimators = (QVector<HClipAnimator>()
                                 << handler->clipAnimatorManager()->getOrAcquireHandle(animator->peerId()));

            clip = createAnimationClip(handler);
            animator->setClipId(clip->peerId());

            channelMapper = createChannelMapper(handler);
            animator->setMapperId(channelMapper->peerId());
            animator->setRunning(true); // Has to be marked as running for the job to process it
            animator->setEnabled(false); // Has to be marked as enabled for the job to process it

            QTest::newRow("Aniamtor disabled")
                    << handler
                    << dirtyClipAnimators
                    << false;
        }

        {
            Handler *handler;
            AnimationClip *clip;
            ClipAnimator *animator;
            QVector<HClipAnimator> dirtyClipAnimators;
            ChannelMapper *channelMapper;
            handler = new Handler();

            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            dirtyClipAnimators = (QVector<HClipAnimator>()
                                 << handler->clipAnimatorManager()->getOrAcquireHandle(animator->peerId()));

            clip = createAnimationClip(handler);
            animator->setClipId(clip->peerId());

            channelMapper = createChannelMapper(handler);
            animator->setMapperId(channelMapper->peerId());
            animator->setRunning(false); // Has to be marked as running for the job to process it
            animator->setEnabled(true); // Has to be marked as enabled for the job to process it

            QTest::newRow("Animator not running")
                    << handler
                    << dirtyClipAnimators
                    << false;
        }

        {
            Handler *handler;
            AnimationClip *clip;
            ClipAnimator *animator;
            QVector<HClipAnimator> dirtyClipAnimators;
            ChannelMapper *channelMapper;
            handler = new Handler();

            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            dirtyClipAnimators = (QVector<HClipAnimator>()
                                 << handler->clipAnimatorManager()->getOrAcquireHandle(animator->peerId()));

            clip = createAnimationClip(handler);
            animator->setClipId(clip->peerId());

            channelMapper = createChannelMapper(handler);
            animator->setMapperId(channelMapper->peerId());
            animator->setRunning(true); // Has to be marked as running for the job to process it
            animator->setEnabled(true); // Has to be marked as enabled for the job to process it

            QTest::newRow("Animator running")
                    << handler
                    << dirtyClipAnimators
                    << true;
        }
    }

    void checkJob()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(QVector<HClipAnimator>, dirtyClipAnimators);
        QFETCH(bool, isRunning);
        FindRunningClipAnimatorsJob job;

        // WHEN
        job.setHandler(handler);
        job.setDirtyClipAnimators(dirtyClipAnimators);
        job.run();

        // THEN - check the resulting MappingData on the animator matches the expected results
        for (const auto &dirtyClipAnimatorHandle : dirtyClipAnimators) {

            auto runningClipHandle = std::find(std::begin(handler->runningClipAnimators()), std::end(handler->runningClipAnimators()), dirtyClipAnimatorHandle);
            QCOMPARE(runningClipHandle != std::end(handler->runningClipAnimators()), isRunning);
        }
    }
};

QTEST_APPLESS_MAIN(tst_FindRunningClipAnimatorsJob)

#include "tst_findrunningclipanimatorsjob.moc"
