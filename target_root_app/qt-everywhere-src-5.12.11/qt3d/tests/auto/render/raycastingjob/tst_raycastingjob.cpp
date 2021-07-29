/****************************************************************************
**
** Copyright (C) 2018 Klaralvdalens Datakonsult AB (KDAB).
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

#include "qmlscenereader.h"
#include "testpostmanarbiter.h"

#include <QtTest/QTest>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <Qt3DCore/private/qaspectjobmanager_p.h>
#include <QtQuick/qquickwindow.h>

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRayCaster>
#include <Qt3DRender/QScreenRayCaster>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DRender/private/qrenderaspect_p.h>
#include <Qt3DRender/private/raycastingjob_p.h>
#include <Qt3DRender/private/pickboundingvolumeutils_p.h>
#include <Qt3DRender/private/updatemeshtrianglelistjob_p.h>
#include <Qt3DRender/private/updateworldboundingvolumejob_p.h>
#include <Qt3DRender/private/updateworldtransformjob_p.h>
#include <Qt3DRender/private/expandboundingvolumejob_p.h>
#include <Qt3DRender/private/calcboundingvolumejob_p.h>
#include <Qt3DRender/private/updateentityhierarchyjob_p.h>
#include <Qt3DRender/private/calcgeometrytrianglevolumes_p.h>
#include <Qt3DRender/private/loadbufferjob_p.h>
#include <Qt3DRender/private/buffermanager_p.h>
#include <Qt3DRender/private/geometryrenderermanager_p.h>

#include <private/qpickevent_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

class TestAspect : public Qt3DRender::QRenderAspect
{
public:
    TestAspect(Qt3DCore::QNode *root)
        : Qt3DRender::QRenderAspect(Qt3DRender::QRenderAspect::Synchronous)
        , m_sceneRoot(nullptr)
    {
        QRenderAspect::onRegistered();

        const Qt3DCore::QNodeCreatedChangeGenerator generator(root);
        const QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges = generator.creationChanges();

        d_func()->setRootAndCreateNodes(qobject_cast<Qt3DCore::QEntity *>(root), creationChanges);

        Render::Entity *rootEntity = nodeManagers()->lookupResource<Render::Entity, Render::EntityManager>(rootEntityId());
        Q_ASSERT(rootEntity);
        m_sceneRoot = rootEntity;
    }

    ~TestAspect()
    {
        QRenderAspect::onUnregistered();
    }

    void onRegistered() { QRenderAspect::onRegistered(); }
    void onUnregistered() { QRenderAspect::onUnregistered(); }

    Qt3DRender::Render::NodeManagers *nodeManagers() const { return d_func()->m_renderer->nodeManagers(); }
    Qt3DRender::Render::FrameGraphNode *frameGraphRoot() const { return d_func()->m_renderer->frameGraphRoot(); }
    Qt3DRender::Render::RenderSettings *renderSettings() const { return d_func()->m_renderer->settings(); }
    Qt3DRender::Render::Entity *sceneRoot() const { return m_sceneRoot; }

private:
    Render::Entity *m_sceneRoot;
};

} // namespace Qt3DRender

QT_END_NAMESPACE

namespace {

void runRequiredJobs(Qt3DRender::TestAspect *test)
{
    Qt3DRender::Render::UpdateEntityHierarchyJob updateEntitiesJob;
    updateEntitiesJob.setManager(test->nodeManagers());
    updateEntitiesJob.run();

    Qt3DRender::Render::UpdateWorldTransformJob updateWorldTransform;
    updateWorldTransform.setRoot(test->sceneRoot());
    updateWorldTransform.run();

    // For each buffer
    QVector<Qt3DRender::Render::HBuffer> bufferHandles = test->nodeManagers()->bufferManager()->activeHandles();
    for (auto bufferHandle : bufferHandles) {
        Qt3DRender::Render::LoadBufferJob loadBuffer(bufferHandle);
        loadBuffer.setNodeManager(test->nodeManagers());
        loadBuffer.run();
    }

    Qt3DRender::Render::CalculateBoundingVolumeJob calcBVolume;
    calcBVolume.setManagers(test->nodeManagers());
    calcBVolume.setRoot(test->sceneRoot());
    calcBVolume.run();

    Qt3DRender::Render::UpdateWorldBoundingVolumeJob updateWorldBVolume;
    updateWorldBVolume.setManager(test->nodeManagers()->renderNodesManager());
    updateWorldBVolume.run();

    Qt3DRender::Render::ExpandBoundingVolumeJob expandBVolume;
    expandBVolume.setRoot(test->sceneRoot());
    expandBVolume.run();

    Qt3DRender::Render::UpdateMeshTriangleListJob updateTriangleList;
    updateTriangleList.setManagers(test->nodeManagers());
    updateTriangleList.run();

    // For each geometry id
    QVector<Qt3DRender::Render::HGeometryRenderer> geometryRenderHandles = test->nodeManagers()->geometryRendererManager()->activeHandles();
    for (auto geometryRenderHandle : geometryRenderHandles) {
        Qt3DCore::QNodeId geometryRendererId = test->nodeManagers()->geometryRendererManager()->data(geometryRenderHandle)->peerId();
        Qt3DRender::Render::CalcGeometryTriangleVolumes calcGeometryTriangles(geometryRendererId, test->nodeManagers());
        calcGeometryTriangles.run();
    }
}

void initializeJob(Qt3DRender::Render::RayCastingJob *job, Qt3DRender::TestAspect *test)
{
    job->setFrameGraphRoot(test->frameGraphRoot());
    job->setRoot(test->sceneRoot());
    job->setManagers(test->nodeManagers());
    job->setRenderSettings(test->renderSettings());
}

} // anonymous

class tst_RayCastingJob : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void worldSpaceRayCaster_data()
    {
        QTest::addColumn<QUrl>("source");
        QTest::addColumn<QVector3D>("rayOrigin");
        QTest::addColumn<QVector3D>("rayDirection");
        QTest::addColumn<float>("rayLength");
        QTest::addColumn<int>("numIntersections");

        QTest::newRow("left entity") << QUrl("qrc:/testscene_worldraycasting.qml") << QVector3D(-5, 0, 4) << QVector3D(0, 0, -1) << 20.f << 1;
        QTest::newRow("no entity") << QUrl("qrc:/testscene_worldraycasting.qml") << QVector3D(0, 0, 4) << QVector3D(0, 0, -1) << 20.f  << 0;
        QTest::newRow("both entities") << QUrl("qrc:/testscene_worldraycasting.qml") << QVector3D(-8, 0, 0) << QVector3D(1, 0, 0) << 20.f  << 2;
        QTest::newRow("infinite ray") << QUrl("qrc:/testscene_worldraycasting.qml") << QVector3D(-5, 0, 4) << QVector3D(0, 0, -1) << -1.f << 1;
        QTest::newRow("short ray") << QUrl("qrc:/testscene_worldraycasting.qml") << QVector3D(-5, 0, 4) << QVector3D(0, 0, -1) << 2.f << 0;
        QTest::newRow("discard filter - right entity") << QUrl("qrc:/testscene_worldraycastinglayer.qml") << QVector3D(5, 0, 4) << QVector3D(0, 0, -1) << 20.f  << 0;
        QTest::newRow("discard filter - both entities") << QUrl("qrc:/testscene_worldraycastinglayer.qml") << QVector3D(-8, 0, 0) << QVector3D(1, 0, 0) << 20.f  << 1;
        QTest::newRow("multi layer - left entity") << QUrl("qrc:/testscene_worldraycastingalllayers.qml") << QVector3D(-5, 0, 4) << QVector3D(0, 0, -1) << 20.f  << 0;
        QTest::newRow("multi layer - right entity") << QUrl("qrc:/testscene_worldraycastingalllayers.qml") << QVector3D(5, 0, 4) << QVector3D(0, 0, -1) << 20.f  << 1;
        QTest::newRow("parent layer") << QUrl("qrc:/testscene_worldraycastingparentlayer.qml") << QVector3D(-8, 0, 0) << QVector3D(1, 0, 0) << 20.f  << 1;
    }

    void worldSpaceRayCaster()
    {
        QFETCH(QUrl, source);
        QFETCH(QVector3D, rayOrigin);
        QFETCH(QVector3D, rayDirection);
        QFETCH(float, rayLength);
        QFETCH(int, numIntersections);

        // GIVEN
        QmlSceneReader sceneReader(source);
        QScopedPointer<Qt3DCore::QEntity> root(qobject_cast<Qt3DCore::QEntity *>(sceneReader.root()));
        QVERIFY(root);

        Qt3DCore::QComponentVector rootComponents = root->components();
        Qt3DRender::QRayCaster *rayCaster = nullptr;
        for (Qt3DCore::QComponent *c: qAsConst(rootComponents)) {
            rayCaster = qobject_cast<Qt3DRender::QRayCaster *>(c);
            if (rayCaster)
                break;
        }
        QVERIFY(rayCaster);

        rayCaster->trigger(rayOrigin, rayDirection, rayLength);

        QScopedPointer<Qt3DRender::TestAspect> test(new Qt3DRender::TestAspect(root.data()));
        TestArbiter arbiter;

        // Runs Required jobs
        runRequiredJobs(test.data());

        Qt3DRender::Render::RayCaster *backendRayCaster = test->nodeManagers()->rayCasterManager()->lookupResource(rayCaster->id());
        QVERIFY(backendRayCaster);
        Qt3DCore::QBackendNodePrivate::get(backendRayCaster)->setArbiter(&arbiter);

        // WHEN
        Qt3DRender::Render::RayCastingJob rayCastingJob;
        initializeJob(&rayCastingJob, test.data());

        bool earlyReturn = !rayCastingJob.runHelper();

        // THEN
        QVERIFY(!earlyReturn);
        QVERIFY(!backendRayCaster->isEnabled());
        QCOMPARE(arbiter.events.count(), 2); // hits & disable
        Qt3DCore::QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "hits");
        Qt3DRender::QRayCaster::Hits hits = change->value().value<Qt3DRender::QRayCaster::Hits>();
        QCOMPARE(hits.size(), numIntersections);

        if (numIntersections)
            QVERIFY(hits.first().entityId());
    }

    void screenSpaceRayCaster_data()
    {
        QTest::addColumn<QUrl>("source");
        QTest::addColumn<QPoint>("rayPosition");
        QTest::addColumn<int>("numIntersections");

        QTest::newRow("left entity") << QUrl("qrc:/testscene_screenraycasting.qml") << QPoint(200, 280) << 1;
        QTest::newRow("no entity") << QUrl("qrc:/testscene_screenraycasting.qml") << QPoint(300, 300) << 0;
    }

    void screenSpaceRayCaster()
    {
        QFETCH(QUrl, source);
        QFETCH(QPoint, rayPosition);
        QFETCH(int, numIntersections);

        // GIVEN
        QmlSceneReader sceneReader(source);
        QScopedPointer<Qt3DCore::QEntity> root(qobject_cast<Qt3DCore::QEntity *>(sceneReader.root()));
        QVERIFY(root);

        Qt3DCore::QComponentVector rootComponents = root->components();
        Qt3DRender::QScreenRayCaster *rayCaster = nullptr;
        for (Qt3DCore::QComponent *c: qAsConst(rootComponents)) {
            rayCaster = qobject_cast<Qt3DRender::QScreenRayCaster *>(c);
            if (rayCaster)
                break;
        }
        QVERIFY(rayCaster);

        rayCaster->trigger(rayPosition);

        QScopedPointer<Qt3DRender::TestAspect> test(new Qt3DRender::TestAspect(root.data()));
        TestArbiter arbiter;

        // Runs Required jobs
        runRequiredJobs(test.data());

        Qt3DRender::Render::RayCaster *backendRayCaster = test->nodeManagers()->rayCasterManager()->lookupResource(rayCaster->id());
        QVERIFY(backendRayCaster);
        Qt3DCore::QBackendNodePrivate::get(backendRayCaster)->setArbiter(&arbiter);

        // WHEN
        Qt3DRender::Render::RayCastingJob rayCastingJob;
        initializeJob(&rayCastingJob, test.data());

        bool earlyReturn = !rayCastingJob.runHelper();

        // THEN
        QVERIFY(!earlyReturn);
        QVERIFY(!backendRayCaster->isEnabled());
        QCOMPARE(arbiter.events.count(), 2); // hits & disable
        Qt3DCore::QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "hits");
        Qt3DRender::QScreenRayCaster::Hits hits = change->value().value<Qt3DRender::QScreenRayCaster::Hits>();
        QCOMPARE(hits.size(), numIntersections);

        if (numIntersections)
            QVERIFY(hits.first().entityId());
    }

};

QTEST_MAIN(tst_RayCastingJob)

#include "tst_raycastingjob.moc"
