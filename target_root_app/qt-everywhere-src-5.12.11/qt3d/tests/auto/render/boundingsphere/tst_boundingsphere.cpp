/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
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

#include <QUrl>

#include <QtTest/QTest>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <Qt3DCore/private/qaspectjobmanager_p.h>
#include <QtQuick/qquickwindow.h>

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QPickTriangleEvent>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DRender/private/qrenderaspect_p.h>
#include <Qt3DRender/private/pickboundingvolumejob_p.h>
#include <Qt3DRender/private/updatemeshtrianglelistjob_p.h>
#include <Qt3DRender/private/updateworldboundingvolumejob_p.h>
#include <Qt3DRender/private/updateworldtransformjob_p.h>
#include <Qt3DRender/private/expandboundingvolumejob_p.h>
#include <Qt3DRender/private/calcboundingvolumejob_p.h>
#include <Qt3DRender/private/calcgeometrytrianglevolumes_p.h>
#include <Qt3DRender/private/loadbufferjob_p.h>
#include <Qt3DRender/private/updateentityhierarchyjob_p.h>
#include <Qt3DRender/private/buffermanager_p.h>
#include <Qt3DRender/private/geometryrenderermanager_p.h>
#include <Qt3DRender/private/sphere_p.h>

#include <Qt3DExtras/qspheremesh.h>
#include <Qt3DExtras/qcylindermesh.h>
#include <Qt3DExtras/qtorusmesh.h>
#include <Qt3DExtras/qcuboidmesh.h>
#include <Qt3DExtras/qplanemesh.h>

#include <qbackendnodetester.h>

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
    Qt3DRender::Render::AbstractRenderer *renderer() const { return d_func()->m_renderer; }

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

} // anonymous

class tst_BoundingSphere : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT
private:

private Q_SLOTS:
    void checkExtraGeometries_data()
    {
        QTest::addColumn<QString>("qmlFile");
        QTest::addColumn<QVector3D>("sphereCenter");
        QTest::addColumn<float>("sphereRadius");
        QTest::newRow("SphereMesh") << "qrc:/sphere.qml" << QVector3D(0.f, 0.f, 0.f) << 1.f;
        QTest::newRow("CubeMesh") << "qrc:/cube.qml" << QVector3D(0.0928356f, -0.212021f, -0.0467958f) << 1.07583f; // weird!
    }

    void checkExtraGeometries()
    {
        // GIVEN
        QFETCH(QString, qmlFile);
        QFETCH(QVector3D, sphereCenter);
        QFETCH(float, sphereRadius);

        QUrl qmlUrl(qmlFile);
        QmlSceneReader sceneReader(qmlUrl);
        QScopedPointer<Qt3DCore::QNode> root(qobject_cast<Qt3DCore::QNode *>(sceneReader.root()));
        QVERIFY(root);

        QScopedPointer<Qt3DRender::TestAspect> test(new Qt3DRender::TestAspect(root.data()));

        // Runs Required jobs
        runRequiredJobs(test.data());

        // THEN
        QVERIFY(test->sceneRoot()->worldBoundingVolumeWithChildren());
        const auto boundingSphere = test->sceneRoot()->worldBoundingVolumeWithChildren();
        qDebug() << qmlFile << boundingSphere->radius() << boundingSphere->center();
        QCOMPARE(boundingSphere->radius(), sphereRadius);
        QVERIFY(qAbs(boundingSphere->center().x() - sphereCenter.x()) < 0.000001f);     // qFuzzyCompare hates 0s
        QVERIFY(qAbs(boundingSphere->center().y() - sphereCenter.y()) < 0.000001f);
        QVERIFY(qAbs(boundingSphere->center().z() - sphereCenter.z()) < 0.000001f);
    }

    void checkCustomGeometry_data()
    {
        QTest::addColumn<int>("drawVertexCount");
        QTest::addColumn<int>("indexByteOffset");
        QTest::addColumn<QVector3D>("expectedCenter");
        QTest::addColumn<float>("expectedRadius");
        QTest::addColumn<bool>("withPrimitiveRestart");
        QTest::newRow("all") << 0 << 0 << QVector3D(-0.488892f, 0.0192147f, -75.4804f) << 25.5442f << false;
        QTest::newRow("first only") << 3 << 0 << QVector3D(0, 1, -100) << 1.0f << false;
        QTest::newRow("second only") << 3 << int(3 * sizeof(ushort)) << QVector3D(0, -1, -50) << 1.0f << false;
        QTest::newRow("all with primitive restart") << 0 << 0 << QVector3D(-0.488892f, 0.0192147f, -75.4804f) << 25.5442f << true;
        QTest::newRow("first only with primitive restart") << 4 << 0 << QVector3D(0, 1, -100) << 1.0f << true;
        QTest::newRow("second only with primitive restart") << 4 << int(3 * sizeof(ushort)) << QVector3D(0, -1, -50) << 1.0f << true;
    }

    void checkCustomGeometry()
    {
        QFETCH(int, drawVertexCount);
        QFETCH(int, indexByteOffset);
        QFETCH(QVector3D, expectedCenter);
        QFETCH(float, expectedRadius);
        QFETCH(bool, withPrimitiveRestart);

        // two triangles with different Z, and an index buffer
        QByteArray vdata;
        vdata.resize(6 * 3 * sizeof(float));
        float *vp = reinterpret_cast<float *>(vdata.data());
        *vp++ = -1.0f;
        *vp++ = 1.0f;
        *vp++ = -100.0f;
        *vp++ = 0.0f;
        *vp++ = 0.0f;
        *vp++ = -100.0f;
        *vp++ = 1.0f;
        *vp++ = 1.0f;
        *vp++ = -100.0f;

        *vp++ = -1.0f;
        *vp++ = -1.0f;
        *vp++ = -50.0f;
        *vp++ = 0.0f;
        *vp++ = 0.0f;
        *vp++ = -50.0f;
        *vp++ = 1.0f;
        *vp++ = -1.0f;
        *vp++ = -50.0f;

        QByteArray idata;
        const int indexCount = withPrimitiveRestart ? 7 : 6;
        idata.resize(indexCount * sizeof(ushort));
        ushort *ip = reinterpret_cast<ushort *>(idata.data());
        *ip++ = 0;
        *ip++ = 1;
        *ip++ = 2;
        if (withPrimitiveRestart)
            *ip++ = 65535;
        *ip++ = 3;
        *ip++ = 4;
        *ip++ = 5;

        QScopedPointer<Qt3DCore::QEntity> entity(new Qt3DCore::QEntity);
        QScopedPointer<Qt3DRender::TestAspect> test(new Qt3DRender::TestAspect(entity.data()));
        Qt3DRender::QBuffer *vbuffer = new Qt3DRender::QBuffer;
        Qt3DRender::QBuffer *ibuffer = new Qt3DRender::QBuffer;

        vbuffer->setData(vdata);
        Qt3DRender::Render::Buffer *vbufferBackend = test->nodeManagers()->bufferManager()->getOrCreateResource(vbuffer->id());
        vbufferBackend->setRenderer(test->renderer());
        vbufferBackend->setManager(test->nodeManagers()->bufferManager());
        simulateInitialization(vbuffer, vbufferBackend);

        ibuffer->setData(idata);
        Qt3DRender::Render::Buffer *ibufferBackend = test->nodeManagers()->bufferManager()->getOrCreateResource(ibuffer->id());
        ibufferBackend->setRenderer(test->renderer());
        ibufferBackend->setManager(test->nodeManagers()->bufferManager());
        simulateInitialization(ibuffer, ibufferBackend);

        Qt3DRender::QGeometry *g = new Qt3DRender::QGeometry;
        for (int i = 0; i < 2; ++i)
            g->addAttribute(new Qt3DRender::QAttribute);

        const QVector<Qt3DRender::QAttribute *> attrs = g->attributes();
        Qt3DRender::QAttribute *attr = attrs[0];
        attr->setBuffer(vbuffer);
        attr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        attr->setVertexBaseType(Qt3DRender::QAttribute::Float);
        attr->setVertexSize(3);
        attr->setCount(6);
        attr->setByteOffset(0);
        attr->setByteStride(3 * sizeof(float));

        attr = attrs[1];
        attr->setBuffer(ibuffer);
        attr->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
        attr->setVertexBaseType(Qt3DRender::QAttribute::UnsignedShort);
        attr->setVertexSize(1);
        attr->setCount(indexCount);
        attr->setByteOffset(indexByteOffset);

        Qt3DRender::QGeometryRenderer *gr = new Qt3DRender::QGeometryRenderer;
        gr->setVertexCount(drawVertexCount); // when 0, indexAttribute->count() is used instead
        gr->setPrimitiveRestartEnabled(withPrimitiveRestart);
        if (withPrimitiveRestart)
            gr->setRestartIndexValue(65535);
        gr->setGeometry(g);
        entity->addComponent(gr);

        Qt3DRender::Render::Attribute *attr0Backend = test->nodeManagers()->attributeManager()->getOrCreateResource(attrs[0]->id());
        attr0Backend->setRenderer(test->renderer());
        simulateInitialization(attrs[0], attr0Backend);
        Qt3DRender::Render::Attribute *attr1Backend = test->nodeManagers()->attributeManager()->getOrCreateResource(attrs[1]->id());
        attr1Backend->setRenderer(test->renderer());
        simulateInitialization(attrs[1], attr1Backend);

        Qt3DRender::Render::Geometry *gBackend = test->nodeManagers()->geometryManager()->getOrCreateResource(g->id());
        gBackend->setRenderer(test->renderer());
        simulateInitialization(g, gBackend);

        Qt3DRender::Render::GeometryRenderer *grBackend = test->nodeManagers()->geometryRendererManager()->getOrCreateResource(gr->id());
        grBackend->setRenderer(test->renderer());
        grBackend->setManager(test->nodeManagers()->geometryRendererManager());
        simulateInitialization(gr, grBackend);

        Qt3DRender::Render::Entity *entityBackend = test->nodeManagers()->renderNodesManager()->getOrCreateResource(entity->id());
        entityBackend->setRenderer(test->renderer());
        simulateInitialization(entity.data(), entityBackend);

        Qt3DRender::Render::CalculateBoundingVolumeJob calcBVolume;
        calcBVolume.setManagers(test->nodeManagers());
        calcBVolume.setRoot(test->sceneRoot());
        calcBVolume.run();

        Vector3D center = entityBackend->localBoundingVolume()->center();
        float radius = entityBackend->localBoundingVolume()->radius();
        qDebug() << radius << center;

        // truncate and compare integers only
        QVERIFY(int(radius) == int(expectedRadius));
        QVERIFY(int(center.x()) == int(expectedCenter.x()));
        QVERIFY(int(center.y()) == int(expectedCenter.y()));
        QVERIFY(int(center.z()) == int(expectedCenter.z()));
    }

    void checkCustomPackedGeometry()
    {
        int drawVertexCount = 6;
        QVector3D expectedCenter(-0.488892f, 0.0192147f, -75.4804f);
        float expectedRadius = 25.5442f;

        // two triangles with different Z
        QByteArray vdata;
        vdata.resize(6 * 3 * sizeof(float));
        float *vp = reinterpret_cast<float *>(vdata.data());
        *vp++ = -1.0f;
        *vp++ = 1.0f;
        *vp++ = -100.0f;
        *vp++ = 0.0f;
        *vp++ = 0.0f;
        *vp++ = -100.0f;
        *vp++ = 1.0f;
        *vp++ = 1.0f;
        *vp++ = -100.0f;

        *vp++ = -1.0f;
        *vp++ = -1.0f;
        *vp++ = -50.0f;
        *vp++ = 0.0f;
        *vp++ = 0.0f;
        *vp++ = -50.0f;
        *vp++ = 1.0f;
        *vp++ = -1.0f;
        *vp++ = -50.0f;

        QScopedPointer<Qt3DCore::QEntity> entity(new Qt3DCore::QEntity);
        QScopedPointer<Qt3DRender::TestAspect> test(new Qt3DRender::TestAspect(entity.data()));
        Qt3DRender::QBuffer *vbuffer = new Qt3DRender::QBuffer;

        vbuffer->setData(vdata);
        Qt3DRender::Render::Buffer *vbufferBackend = test->nodeManagers()->bufferManager()->getOrCreateResource(vbuffer->id());
        vbufferBackend->setRenderer(test->renderer());
        vbufferBackend->setManager(test->nodeManagers()->bufferManager());
        simulateInitialization(vbuffer, vbufferBackend);

        Qt3DRender::QGeometry *g = new Qt3DRender::QGeometry;
        g->addAttribute(new Qt3DRender::QAttribute);

        const QVector<Qt3DRender::QAttribute *> attrs = g->attributes();
        Qt3DRender::QAttribute *attr = attrs[0];
        attr->setBuffer(vbuffer);
        attr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        attr->setVertexBaseType(Qt3DRender::QAttribute::Float);
        attr->setVertexSize(3);
        attr->setCount(6);
        attr->setByteOffset(0);
        attr->setByteStride(0);

        Qt3DRender::QGeometryRenderer *gr = new Qt3DRender::QGeometryRenderer;
        gr->setVertexCount(drawVertexCount);
        gr->setGeometry(g);
        entity->addComponent(gr);

        Qt3DRender::Render::Attribute *attr0Backend = test->nodeManagers()->attributeManager()->getOrCreateResource(attrs[0]->id());
        attr0Backend->setRenderer(test->renderer());
        simulateInitialization(attrs[0], attr0Backend);

        Qt3DRender::Render::Geometry *gBackend = test->nodeManagers()->geometryManager()->getOrCreateResource(g->id());
        gBackend->setRenderer(test->renderer());
        simulateInitialization(g, gBackend);

        Qt3DRender::Render::GeometryRenderer *grBackend = test->nodeManagers()->geometryRendererManager()->getOrCreateResource(gr->id());
        grBackend->setRenderer(test->renderer());
        grBackend->setManager(test->nodeManagers()->geometryRendererManager());
        simulateInitialization(gr, grBackend);

        Qt3DRender::Render::Entity *entityBackend = test->nodeManagers()->renderNodesManager()->getOrCreateResource(entity->id());
        entityBackend->setRenderer(test->renderer());
        simulateInitialization(entity.data(), entityBackend);

        Qt3DRender::Render::CalculateBoundingVolumeJob calcBVolume;
        calcBVolume.setManagers(test->nodeManagers());
        calcBVolume.setRoot(test->sceneRoot());
        calcBVolume.run();

        Vector3D center = entityBackend->localBoundingVolume()->center();
        float radius = entityBackend->localBoundingVolume()->radius();
        qDebug() << radius << center;

        // truncate and compare integers only
        QVERIFY(int(radius) == int(expectedRadius));
        QVERIFY(int(center.x()) == int(expectedCenter.x()));
        QVERIFY(int(center.y()) == int(expectedCenter.y()));
        QVERIFY(int(center.z()) == int(expectedCenter.z()));
    }
};

QTEST_MAIN(tst_BoundingSphere)

#include "tst_boundingsphere.moc"
