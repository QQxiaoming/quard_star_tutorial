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
#include <Qt3DAnimation/qskeletonmapping.h>
#include <Qt3DAnimation/private/qskeletonmapping_p.h>
#include <Qt3DCore/qskeleton.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qnodecreatedchange.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <QObject>
#include <QSignalSpy>
#include <testpostmanarbiter.h>

class tst_QSkeletonMapping : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qRegisterMetaType<Qt3DCore::QAbstractSkeleton*>();
    }

    void checkDefaultConstruction()
    {
        // GIVEN
        Qt3DAnimation::QSkeletonMapping mapping;

        // THEN
        QCOMPARE(mapping.skeleton(), static_cast<Qt3DCore::QAbstractSkeleton *>(nullptr));
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Qt3DAnimation::QSkeletonMapping mapping;

        {
            // WHEN
            QSignalSpy spy(&mapping, SIGNAL(skeletonChanged(Qt3DCore::QAbstractSkeleton*)));
            auto newValue = new Qt3DCore::QSkeleton();
            mapping.setSkeleton(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(mapping.skeleton(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            mapping.setSkeleton(newValue);

            // THEN
            QCOMPARE(mapping.skeleton(), newValue);
            QCOMPARE(spy.count(), 0);
        }
    }

    void checkCreationData()
    {
        // GIVEN
        Qt3DAnimation::QSkeletonMapping mapping;
        auto target = new Qt3DCore::QSkeleton;

        mapping.setSkeleton(target);

        // WHEN
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges;

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&mapping);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 2); // 1 for mapping, 1 for skeleton

            const auto creationChangeData = qSharedPointerCast<Qt3DAnimation::QChannelMappingCreatedChange<Qt3DAnimation::QSkeletonMappingData>>(creationChanges.first());
            const Qt3DAnimation::QSkeletonMappingData data = creationChangeData->data;

            QCOMPARE(mapping.id(), creationChangeData->subjectId());
            QCOMPARE(mapping.isEnabled(), true);
            QCOMPARE(mapping.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(mapping.metaObject(), creationChangeData->metaObject());
            QCOMPARE(creationChangeData->type(), Qt3DAnimation::QChannelMappingCreatedChangeBase::SkeletonMapping);
            QCOMPARE(mapping.skeleton()->id(), data.skeletonId);
        }

        // WHEN
        mapping.setEnabled(false);

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&mapping);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 2); // 1 for mapping, 1 for skeleton

            const auto creationChangeData = qSharedPointerCast<Qt3DAnimation::QChannelMappingCreatedChange<Qt3DAnimation::QSkeletonMappingData>>(creationChanges.first());
            const Qt3DAnimation::QSkeletonMappingData data = creationChangeData->data;

            QCOMPARE(mapping.id(), creationChangeData->subjectId());
            QCOMPARE(mapping.isEnabled(), false);
            QCOMPARE(mapping.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(mapping.metaObject(), creationChangeData->metaObject());
            QCOMPARE(creationChangeData->type(), Qt3DAnimation::QChannelMappingCreatedChangeBase::SkeletonMapping);
            QCOMPARE(mapping.skeleton()->id(), data.skeletonId);
        }
    }

    void checkPropertyUpdateChanges()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DAnimation::QSkeletonMapping mapping;
        arbiter.setArbiterOnNode(&mapping);

        {
            // WHEN
            auto target = new Qt3DCore::QSkeleton();
            mapping.setSkeleton(target);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "skeleton");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().value<Qt3DCore::QNodeId>(), mapping.skeleton()->id());

            arbiter.events.clear();

            // WHEN
            mapping.setSkeleton(target);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }
};

QTEST_MAIN(tst_QSkeletonMapping)

#include "tst_qskeletonmapping.moc"
