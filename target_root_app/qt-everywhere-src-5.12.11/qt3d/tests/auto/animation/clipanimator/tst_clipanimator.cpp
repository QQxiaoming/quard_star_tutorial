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
#include <Qt3DAnimation/private/clipanimator_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DAnimation/private/handle_types_p.h>
#include <Qt3DAnimation/qanimationcliploader.h>
#include <Qt3DAnimation/qclipanimator.h>
#include <Qt3DAnimation/qclock.h>
#include <Qt3DAnimation/qchannelmapper.h>
#include <Qt3DCore/private/qnode_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qbackendnode_p.h>
#include <qbackendnodetester.h>
#include <testpostmanarbiter.h>

class tst_ClipAnimator: public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

private Q_SLOTS:
    void checkPeerPropertyMirroring()
    {
        // GIVEN
        Qt3DAnimation::Animation::Handler handler;
        Qt3DAnimation::Animation::ClipAnimator backendAnimator;
        backendAnimator.setHandler(&handler);
        Qt3DAnimation::QClipAnimator animator;
        auto clip = new Qt3DAnimation::QAnimationClipLoader();
        auto clock = new Qt3DAnimation::QClock();

        animator.setClip(clip);
        animator.setClock(clock);
        animator.setLoopCount(10);
        animator.setNormalizedTime(0.5f);

        // WHEN
        simulateInitialization(&animator, &backendAnimator);

        // THEN
        QCOMPARE(backendAnimator.peerId(), animator.id());
        QCOMPARE(backendAnimator.isEnabled(), animator.isEnabled());
        QCOMPARE(backendAnimator.clipId(), clip->id());
        QCOMPARE(backendAnimator.clockId(), clock->id());
        QCOMPARE(backendAnimator.isRunning(), animator.isRunning());
        QCOMPARE(backendAnimator.loops(), animator.loopCount());
        QCOMPARE(backendAnimator.normalizedLocalTime(), animator.normalizedTime());

        QCOMPARE(handler.m_dirtyClipAnimators.size(), 1);
        QCOMPARE(handler.m_dirtyClipAnimatorMaps.size(), 1);
    }

    void checkInitialAndCleanedUpState()
    {
        // GIVEN
        Qt3DAnimation::Animation::Handler handler;
        Qt3DAnimation::Animation::ClipAnimator backendAnimator;
        backendAnimator.setHandler(&handler);

        // THEN
        QVERIFY(backendAnimator.peerId().isNull());
        QCOMPARE(backendAnimator.isEnabled(), false);
        QCOMPARE(backendAnimator.clipId(), Qt3DCore::QNodeId());
        QCOMPARE(backendAnimator.clockId(), Qt3DCore::QNodeId());
        QCOMPARE(backendAnimator.isRunning(), false);
        QCOMPARE(backendAnimator.loops(), 1);
        QCOMPARE(backendAnimator.normalizedLocalTime(), -1.0);

        // GIVEN
        Qt3DAnimation::QClipAnimator animator;
        auto clip = new Qt3DAnimation::QAnimationClipLoader();
        auto clock = new Qt3DAnimation::QClock();
        animator.setClip(clip);
        animator.setClock(clock);
        animator.setRunning(true);
        animator.setLoopCount(25);
        animator.setNormalizedTime(1.0f);

        // WHEN
        simulateInitialization(&animator, &backendAnimator);
        backendAnimator.setClipId(Qt3DCore::QNodeId::createId());
        backendAnimator.setClockId(Qt3DCore::QNodeId::createId());
        backendAnimator.cleanup();

        // THEN
        QCOMPARE(backendAnimator.clipId(), Qt3DCore::QNodeId());
        QCOMPARE(backendAnimator.clockId(), Qt3DCore::QNodeId());
        QCOMPARE(backendAnimator.isEnabled(), false);
        QCOMPARE(backendAnimator.isRunning(), false);
        QCOMPARE(backendAnimator.loops(), 1);
        QCOMPARE(backendAnimator.normalizedLocalTime(), -1.0f);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Qt3DAnimation::Animation::Handler handler;
        Qt3DAnimation::Animation::ClipAnimator backendAnimator;
        backendAnimator.setHandler(&handler);
        Qt3DCore::QPropertyUpdatedChangePtr updateChange;

        handler.clipAnimatorManager()->getOrAcquireHandle(backendAnimator.peerId());

        // WHEN
        updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
        updateChange->setPropertyName("enabled");
        updateChange->setValue(true);
        backendAnimator.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendAnimator.isEnabled(), true);

        // WHEN
        auto newClip = new Qt3DAnimation::QAnimationClipLoader();
        updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
        updateChange->setPropertyName("clip");
        updateChange->setValue(QVariant::fromValue(newClip->id()));
        backendAnimator.sceneChangeEvent(updateChange);
        // THEN
        QCOMPARE(backendAnimator.clipId(), newClip->id());
        QCOMPARE(handler.m_dirtyClipAnimators.size(), 1);
        QCOMPARE(handler.m_dirtyClipAnimatorMaps.size(), 1);

        handler.m_dirtyClipAnimators.clear();
        handler.m_dirtyClipAnimatorMaps.clear();

        // WHEN
        auto clock = new Qt3DAnimation::QClock();
        updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
        updateChange->setPropertyName("clock");
        updateChange->setValue(QVariant::fromValue(clock->id()));
        backendAnimator.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendAnimator.clockId(), clock->id());
        QCOMPARE(handler.m_dirtyClipAnimators.size(), 1);
        QCOMPARE(handler.m_dirtyClipAnimatorMaps.size(), 0);

        handler.m_dirtyClipAnimators.clear();

        // WHEN
        auto mapper = new Qt3DAnimation::QChannelMapper();
        updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
        updateChange->setPropertyName("channelMapper");
        updateChange->setValue(QVariant::fromValue(mapper->id()));
        backendAnimator.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendAnimator.mapperId(), mapper->id());
        QCOMPARE(handler.m_dirtyClipAnimators.size(), 0);
        QCOMPARE(handler.m_dirtyClipAnimatorMaps.size(), 1);

        handler.m_dirtyClipAnimatorMaps.clear();

        // WHEN
        updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
        updateChange->setPropertyName("running");
        updateChange->setValue(true);
        backendAnimator.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendAnimator.isRunning(), true);
        QCOMPARE(handler.m_dirtyClipAnimators.size(), 1);
        QCOMPARE(handler.m_dirtyClipAnimatorMaps.size(), 1);

        handler.m_dirtyClipAnimators.clear();
        handler.m_dirtyClipAnimatorMaps.clear();

        // WHEN
        updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
        updateChange->setPropertyName("loops");
        updateChange->setValue(64);
        backendAnimator.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendAnimator.loops(), 64);
        QCOMPARE(handler.m_dirtyClipAnimators.size(), 1);
        QCOMPARE(handler.m_dirtyClipAnimatorMaps.size(), 0);

        handler.m_dirtyClipAnimators.clear();

        // WHEN
        updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
        updateChange->setPropertyName("normalizedTime");
        updateChange->setValue(0.5f);
        backendAnimator.sceneChangeEvent(updateChange);

        // THEN
        QVERIFY(qFuzzyCompare(backendAnimator.normalizedLocalTime(), 0.5f));
        QCOMPARE(handler.m_dirtyClipAnimators.size(), 1);
        QCOMPARE(handler.m_dirtyClipAnimatorMaps.size(), 0);

        handler.m_dirtyClipAnimators.clear();
    }
};

QTEST_APPLESS_MAIN(tst_ClipAnimator)

#include "tst_clipanimator.moc"
