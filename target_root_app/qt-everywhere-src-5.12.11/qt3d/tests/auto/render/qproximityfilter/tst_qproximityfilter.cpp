/****************************************************************************
**
** Copyright (C) 2017 Paul Lemire <paul.lemire350@gmail.com>
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
#include <Qt3DRender/qproximityfilter.h>
#include <Qt3DRender/private/qproximityfilter_p.h>
#include <QObject>
#include <QSignalSpy>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <Qt3DCore/qnodecreatedchange.h>
#include <Qt3DCore/qentity.h>
#include "testpostmanarbiter.h"

class tst_QProximityFilter : public QObject
{
    Q_OBJECT

public:
    tst_QProximityFilter() : QObject()
    {
        qRegisterMetaType<Qt3DCore::QEntity*>();
    }

private Q_SLOTS:

    void checkDefaultConstruction()
    {
        // GIVEN
        Qt3DRender::QProximityFilter proximityFilter;

        // THEN
        QVERIFY(proximityFilter.entity() == nullptr);
        QCOMPARE(proximityFilter.distanceThreshold(), 0.0f);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Qt3DRender::QProximityFilter proximityFilter;

        {
            // WHEN
            Qt3DCore::QEntity entity;

            QSignalSpy spy(&proximityFilter, SIGNAL(entityChanged(Qt3DCore::QEntity *)));
            Qt3DCore::QEntity *newValue = &entity;
            proximityFilter.setEntity(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(proximityFilter.entity(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            proximityFilter.setEntity(newValue);

            // THEN
            QCOMPARE(proximityFilter.entity(), newValue);
            QCOMPARE(spy.count(), 0);
        }
        {
            // WHEN
            QSignalSpy spy(&proximityFilter, SIGNAL(distanceThresholdChanged(float)));
            const float newValue = 883.0f;
            proximityFilter.setDistanceThreshold(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(proximityFilter.distanceThreshold(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            proximityFilter.setDistanceThreshold(newValue);

            // THEN
            QCOMPARE(proximityFilter.distanceThreshold(), newValue);
            QCOMPARE(spy.count(), 0);
        }
    }

    void checkCreationData()
    {
        // GIVEN
        Qt3DRender::QProximityFilter proximityFilter;
        Qt3DCore::QEntity entity;

        proximityFilter.setEntity(&entity);
        proximityFilter.setDistanceThreshold(1584.0f);

        // WHEN
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges;

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&proximityFilter);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 2); // Entity creation change is the second change

            const auto creationChangeData = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<Qt3DRender::QProximityFilterData>>(creationChanges.first());
            const Qt3DRender::QProximityFilterData cloneData = creationChangeData->data;

            QCOMPARE(proximityFilter.entity()->id(), cloneData.entityId);
            QCOMPARE(proximityFilter.distanceThreshold(), cloneData.distanceThreshold);
            QCOMPARE(proximityFilter.id(), creationChangeData->subjectId());
            QCOMPARE(proximityFilter.isEnabled(), true);
            QCOMPARE(proximityFilter.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(proximityFilter.metaObject(), creationChangeData->metaObject());
        }

        // WHEN
        proximityFilter.setEnabled(false);

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&proximityFilter);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 2); // Entity creation change is the second change

            const auto creationChangeData = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<Qt3DRender::QProximityFilterData>>(creationChanges.first());
            const Qt3DRender::QProximityFilterData cloneData = creationChangeData->data;

            QCOMPARE(proximityFilter.entity()->id(), cloneData.entityId);
            QCOMPARE(proximityFilter.distanceThreshold(), cloneData.distanceThreshold);
            QCOMPARE(proximityFilter.id(), creationChangeData->subjectId());
            QCOMPARE(proximityFilter.isEnabled(), false);
            QCOMPARE(proximityFilter.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(proximityFilter.metaObject(), creationChangeData->metaObject());
        }
    }

    void checkEntityUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QProximityFilter proximityFilter;
        arbiter.setArbiterOnNode(&proximityFilter);
        Qt3DCore::QEntity entity;

        {
            // WHEN
            proximityFilter.setEntity(&entity);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "entity");
            QCOMPARE(change->value().value<Qt3DCore::QNodeId>(), proximityFilter.entity()->id());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            proximityFilter.setEntity(&entity);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

    }

    void checkDistanceThresholdUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QProximityFilter proximityFilter;
        arbiter.setArbiterOnNode(&proximityFilter);

        {
            // WHEN
            proximityFilter.setDistanceThreshold(454.0f);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "distanceThreshold");
            QCOMPARE(change->value().value<float>(), proximityFilter.distanceThreshold());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            proximityFilter.setDistanceThreshold(454.0f);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

    }

    void checkEntityBookkeeping()
    {
        // GIVEN
        QScopedPointer<Qt3DRender::QProximityFilter> proximityFilter(new Qt3DRender::QProximityFilter);
        {
            // WHEN
            Qt3DCore::QEntity entity;
            proximityFilter->setEntity(&entity);

            // THEN
            QCOMPARE(entity.parent(), proximityFilter.data());
            QCOMPARE(proximityFilter->entity(), &entity);
        }

        // THEN (Should not crash and parameter be unset)
        QVERIFY(proximityFilter->entity() == nullptr);

        {
            // WHEN
            Qt3DRender::QProximityFilter someOtherProximityFilter;
            QScopedPointer<Qt3DCore::QEntity> entity(new Qt3DCore::QEntity(&someOtherProximityFilter));
            proximityFilter->setEntity(entity.data());

            // THEN
            QCOMPARE(entity->parent(), &someOtherProximityFilter);
            QCOMPARE(proximityFilter->entity(), entity.data());

            // WHEN
            proximityFilter.reset();
            entity.reset();

            // THEN Should not crash when the camera is destroyed (tests for failed removal of destruction helper)
        }
    }
};

QTEST_MAIN(tst_QProximityFilter)

#include "tst_qproximityfilter.moc"
