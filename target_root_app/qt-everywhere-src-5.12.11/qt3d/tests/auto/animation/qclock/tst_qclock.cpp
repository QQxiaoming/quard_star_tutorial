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
#include <Qt3DAnimation/private/qclock_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qnodecreatedchange.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <QObject>
#include <QSignalSpy>
#include <testpostmanarbiter.h>

class tst_QClock : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qRegisterMetaType<Qt3DAnimation::QClock*>();
    }

    void checkDefaultConstruction()
    {
        // GIVEN
        Qt3DAnimation::QClock clock;

        // THEN
        QCOMPARE(clock.playbackRate(), 1.0);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Qt3DAnimation::QClock clock;

        // WHEN
        QSignalSpy spy(&clock, SIGNAL(playbackRateChanged(double)));
        const double newValue = 5.5;
        clock.setPlaybackRate(newValue);

        // THEN
        QVERIFY(spy.isValid());
        QCOMPARE(clock.playbackRate(), newValue);
        QCOMPARE(spy.count(), 1);

        // WHEN
        spy.clear();
        clock.setPlaybackRate(newValue);

        // THEN
        QCOMPARE(clock.playbackRate(), newValue);
        QCOMPARE(spy.count(), 0);
    }

    void checkCreationData()
    {
        // GIVEN
        Qt3DAnimation::QClock clock;
        clock.setPlaybackRate(10.f);

        // WHEN
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges;
        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&clock);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 1);

            const auto creationChangeData = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<Qt3DAnimation::QClockData>>(creationChanges.first());
            const Qt3DAnimation::QClockData data = creationChangeData->data;

            QCOMPARE(clock.id(), creationChangeData->subjectId());
            QCOMPARE(clock.isEnabled(), true);
            QCOMPARE(clock.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(clock.metaObject(), creationChangeData->metaObject());
            QCOMPARE(clock.playbackRate(), data.playbackRate);
        }

        // WHEN
        clock.setEnabled(false);
        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&clock);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 1);

            const auto creationChangeData = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<Qt3DAnimation::QClockData>>(creationChanges.first());

            QCOMPARE(clock.id(), creationChangeData->subjectId());
            QCOMPARE(clock.isEnabled(), false);
            QCOMPARE(clock.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(clock.metaObject(), creationChangeData->metaObject());
        }
    }

    void checkPropertyUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DAnimation::QClock clock;
        arbiter.setArbiterOnNode(&clock);

        {
            // WHEN
            clock.setPlaybackRate(10.5);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "playbackRate");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().value<double>(), clock.playbackRate());

            arbiter.events.clear();
        }

        {
            // WHEN
            clock.setPlaybackRate(10.5f);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }
};

QTEST_MAIN(tst_QClock)

#include "tst_qclock.moc"
