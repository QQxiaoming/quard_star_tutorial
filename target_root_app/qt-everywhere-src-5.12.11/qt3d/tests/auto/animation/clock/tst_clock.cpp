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
#include <Qt3DAnimation/qclock.h>
#include <Qt3DAnimation/private/clock_p.h>
#include <Qt3DCore/private/qnode_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qbackendnode_p.h>
#include <qbackendnodetester.h>
#include <testpostmanarbiter.h>

class tst_Clock: public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

private Q_SLOTS:
    void checkPeerPropertyMirroring()
    {
        // GIVEN
        Qt3DAnimation::Animation::Clock backendClock;
        Qt3DAnimation::QClock clock;

        clock.setPlaybackRate(10.5);

        // WHEN
        simulateInitialization(&clock, &backendClock);

        // THEN
        QCOMPARE(backendClock.playbackRate(), clock.playbackRate());
    }

    void checkInitialAndCleanedUpState()
    {
        // GIVEN
        Qt3DAnimation::Animation::Clock backendClock;

        // THEN
        QCOMPARE(backendClock.playbackRate(), 1.0);

        // GIVEN
        Qt3DAnimation::QClock clock;
        clock.setPlaybackRate(10.5);

        // WHEN
        simulateInitialization(&clock, &backendClock);
        backendClock.cleanup();

        // THEN
        QCOMPARE(backendClock.playbackRate(), 1.0);
    }

    void checkSceneChangeEvents()
    {
        // GIVEN
        Qt3DAnimation::Animation::Clock backendClock;

        {
            // WHEN
            const bool newValue = false;
            const auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(Qt3DCore::QNodeId());
            change->setPropertyName("enabled");
            change->setValue(newValue);
            backendClock.sceneChangeEvent(change);

            // THEN
            QCOMPARE(backendClock.isEnabled(), newValue);
        }
        {
            // WHEN
            const double newPlaybackRateValue = 2.0;
            const auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(Qt3DCore::QNodeId());
            change->setPropertyName("playbackRate");
            change->setValue(newPlaybackRateValue);
            backendClock.sceneChangeEvent(change);

            // THEN
            QCOMPARE(backendClock.playbackRate(), newPlaybackRateValue);
        }
    }
};

QTEST_APPLESS_MAIN(tst_Clock)

#include "tst_clock.moc"
