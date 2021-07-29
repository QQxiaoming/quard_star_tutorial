/****************************************************************************
**
** Copyright (C) 2016 Paul Lemire
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
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/filterproximitydistancejob_p.h>
#include <Qt3DRender/private/updatetreeenabledjob_p.h>
#include <Qt3DRender/qproximityfilter.h>

#include "testaspect.h"

namespace {

Qt3DCore::QEntity *buildEntityAtDistance(float distance, Qt3DCore::QEntity *parent)
{
    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity(parent);
    Qt3DCore::QTransform *transform = new Qt3DCore::QTransform(parent);
    const QVector3D t = QVector3D(1.0f, 0.0f, 0.0f) * distance;

    transform->setTranslation(t);
    entity->addComponent(transform);

    return entity;
}

} // anonymous

class tst_ProximityFiltering : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void checkInitialState()
    {
        // GIVEN
        Qt3DRender::Render::FilterProximityDistanceJob filterJob;

        // THEN
        QCOMPARE(filterJob.hasProximityFilter(), false);
        QCOMPARE(filterJob.filteredEntities().size(), 0);
        QCOMPARE(filterJob.proximityFilterIds().size(), 0);
        QVERIFY(filterJob.manager() == nullptr);
    }

    void filterEntities_data()
    {
        QTest::addColumn<Qt3DCore::QEntity *>("entitySubtree");
        QTest::addColumn<Qt3DCore::QNodeIdVector>("proximityFilterIds");
        QTest::addColumn<Qt3DCore::QNodeIdVector>("expectedSelectedEntities");

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            QTest::newRow("ShouldSelectAll") << rootEntity
                                             << Qt3DCore::QNodeIdVector()
                                             << (Qt3DCore::QNodeIdVector()
                                                 << rootEntity->id()
                                                 << childEntity1->id()
                                                 << childEntity2->id()
                                                 << childEntity3->id()
                                                 );
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);
            Q_UNUSED(childEntity1);
            Q_UNUSED(childEntity2);
            Q_UNUSED(childEntity3);

            Qt3DRender::QProximityFilter *proximityFilter = new Qt3DRender::QProximityFilter(rootEntity);

            QTest::newRow("ShouldSelectNone") << rootEntity
                                              << (Qt3DCore::QNodeIdVector() << proximityFilter->id())
                                              << (Qt3DCore::QNodeIdVector());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *targetEntity = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity1 = buildEntityAtDistance(50.0f, rootEntity);
            Qt3DCore::QEntity *childEntity2 = buildEntityAtDistance(25.0f, rootEntity);
            Qt3DCore::QEntity *childEntity3 = buildEntityAtDistance(75.0f, rootEntity);
            Qt3DRender::QProximityFilter *proximityFilter = new Qt3DRender::QProximityFilter(rootEntity);
            proximityFilter->setDistanceThreshold(30.0f);

            // Note: rootEntity BoundingSphere will be centered in vec3(75.0f / 2.0, 0.0f 0.0f);

            // Note: we cannot set rootEntity here as that would mean
            // that the parent of the root would then be the proximity filter
            // (since rootEntity has no parent) but this isn't valid in the way
            // we have build the test
            // Also rootEntity is centered based on the size of the child it contains
            proximityFilter->setEntity(targetEntity);

            Q_UNUSED(childEntity1);
            Q_UNUSED(childEntity3);

            QTest::newRow("ShouldSelectChild2") << rootEntity
                                                << (Qt3DCore::QNodeIdVector() << proximityFilter->id())
                                                << (Qt3DCore::QNodeIdVector()
                                                    << targetEntity->id()
                                                    << childEntity2->id()
                                                    );
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *targetEntity = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity1 = buildEntityAtDistance(50.0f, rootEntity);
            Qt3DCore::QEntity *childEntity2 = buildEntityAtDistance(25.0f, rootEntity);
            Qt3DCore::QEntity *childEntity3 = buildEntityAtDistance(49.9f, rootEntity);
            Qt3DRender::QProximityFilter *proximityFilter = new Qt3DRender::QProximityFilter(rootEntity);
            proximityFilter->setDistanceThreshold(50.0f);

            // Note: rootEntity BoundingSphere will be centered in vec3(50.0f / 2.0, 0.0f 0.0f);

            // Note: we cannot set rootEntity here as that would mean
            // that the parent of the root would then be the proximity filter
            // (since rootEntity has no parent) but this isn't valid in the way
            // we have build the test
            // Also rootEntity is centered based on the size of the child it contains
            proximityFilter->setEntity(targetEntity);

            QTest::newRow("ShouldSelectRootChild123") << rootEntity
                                                      << (Qt3DCore::QNodeIdVector() << proximityFilter->id())
                                                      << (Qt3DCore::QNodeIdVector()
                                                          << rootEntity->id()
                                                          << targetEntity->id()
                                                          << childEntity1->id()
                                                          << childEntity2->id()
                                                          << childEntity3->id()
                                                          );
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *targetEntity = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity1 = buildEntityAtDistance(51.0f, rootEntity);
            Qt3DCore::QEntity *childEntity2 = buildEntityAtDistance(75.0f, rootEntity);
            Qt3DCore::QEntity *childEntity3 = buildEntityAtDistance(883.0f, rootEntity);
            Qt3DRender::QProximityFilter *proximityFilter = new Qt3DRender::QProximityFilter(rootEntity);
            proximityFilter->setDistanceThreshold(50.0f);

            // Note: rootEntity BoundingSphere will be centered in vec3(883.0f / 2.0, 0.0f 0.0f);

            // Note: we cannot set rootEntity here as that would mean
            // that the parent of the root would then be the proximity filter
            // (since rootEntity has no parent) but this isn't valid in the way
            // we have build the test
            // Also rootEntity is centered based on the size of the child it contains
            proximityFilter->setEntity(targetEntity);

            Q_UNUSED(childEntity1);
            Q_UNUSED(childEntity2);
            Q_UNUSED(childEntity3);

            QTest::newRow("ShouldSelectNoneButTarget") << rootEntity
                                                       << (Qt3DCore::QNodeIdVector() << proximityFilter->id())
                                                       << (Qt3DCore::QNodeIdVector() << targetEntity->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *targetEntity = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity1 = buildEntityAtDistance(50.0f, rootEntity);
            Qt3DCore::QEntity *childEntity2 = buildEntityAtDistance(150.0f, rootEntity);
            Qt3DCore::QEntity *childEntity3 = buildEntityAtDistance(25.0f, rootEntity);

            Qt3DRender::QProximityFilter *proximityFilter = new Qt3DRender::QProximityFilter(rootEntity);
            proximityFilter->setDistanceThreshold(50.0f);

            Qt3DRender::QProximityFilter *proximityFilter2 = new Qt3DRender::QProximityFilter(rootEntity);
            proximityFilter2->setDistanceThreshold(30.0f);

            // Note: rootEntity BoundingSphere will be centered in vec3(150.0f / 2.0, 0.0f 0.0f);

            // Note: we cannot set rootEntity here as that would mean
            // that the parent of the root would then be the proximity filter
            // (since rootEntity has no parent) but this isn't valid in the way
            // we have build the test
            // Also rootEntity is centered based on the size of the child it contains
            proximityFilter->setEntity(targetEntity);
            proximityFilter2->setEntity(targetEntity);

            Q_UNUSED(childEntity2);

            QTest::newRow("Nested-Step1") << rootEntity
                                          << (Qt3DCore::QNodeIdVector() << proximityFilter->id())
                                          << (Qt3DCore::QNodeIdVector()
                                              << targetEntity->id()
                                              << childEntity1->id()
                                              << childEntity3->id()
                                              );
            QTest::newRow("Nested-Step2") << rootEntity
                                          << (Qt3DCore::QNodeIdVector() << proximityFilter->id() << proximityFilter2->id())
                                          << (Qt3DCore::QNodeIdVector()
                                              << targetEntity->id()
                                              << childEntity3->id()
                                              );
        }
    }

    void filterEntities()
    {
        QFETCH(Qt3DCore::QEntity *, entitySubtree);
        QFETCH(Qt3DCore::QNodeIdVector, proximityFilterIds);
        QFETCH(Qt3DCore::QNodeIdVector, expectedSelectedEntities);

        // GIVEN
        QScopedPointer<Qt3DRender::TestAspect> aspect(new Qt3DRender::TestAspect(entitySubtree));

        // WHEN
        Qt3DRender::Render::Entity *backendRoot = aspect->nodeManagers()->renderNodesManager()->getOrCreateResource(entitySubtree->id());

        Qt3DRender::Render::UpdateEntityHierarchyJob updateEntitiesJob;
        updateEntitiesJob.setManager(aspect->nodeManagers());
        updateEntitiesJob.run();

        Qt3DRender::Render::UpdateTreeEnabledJob updateTreeEnabledJob;
        updateTreeEnabledJob.setRoot(backendRoot);
        updateTreeEnabledJob.run();

        Qt3DRender::Render::UpdateWorldTransformJob updateWorldTransform;
        updateWorldTransform.setRoot(backendRoot);
        updateWorldTransform.run();

        Qt3DRender::Render::CalculateBoundingVolumeJob calcBVolume;
        calcBVolume.setManagers(aspect->nodeManagers());
        calcBVolume.setRoot(backendRoot);
        calcBVolume.run();

        Qt3DRender::Render::UpdateWorldBoundingVolumeJob updateWorldBVolume;
        updateWorldBVolume.setManager(aspect->nodeManagers()->renderNodesManager());
        updateWorldBVolume.run();

        Qt3DRender::Render::ExpandBoundingVolumeJob expandBVolume;
        expandBVolume.setRoot(backendRoot);
        expandBVolume.run();

        // WHEN
        Qt3DRender::Render::FilterProximityDistanceJob filterJob;
        filterJob.setProximityFilterIds(proximityFilterIds);
        filterJob.setManager(aspect->nodeManagers());
        filterJob.run();

        // THEN
        const QVector<Qt3DRender::Render::Entity *> filterEntities = filterJob.filteredEntities();
        QCOMPARE(filterEntities.size(), expectedSelectedEntities.size());

        for (auto i = 0, m = expectedSelectedEntities.size(); i < m; ++i)
            QCOMPARE(filterEntities.at(i)->peerId(), expectedSelectedEntities.at(i));
    }
};

QTEST_MAIN(tst_ProximityFiltering)

#include "tst_proximityfiltering.moc"
