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
#include <qbackendnodetester.h>
#include <Qt3DRender/qgeometryrenderer.h>
#include <Qt3DRender/qbuffer.h>
#include <private/segmentsvisitor_p.h>
#include <private/nodemanagers_p.h>
#include <private/managers_p.h>
#include <private/geometryrenderer_p.h>
#include <private/geometryrenderermanager_p.h>
#include <private/buffermanager_p.h>
#include "testrenderer.h"

using namespace Qt3DRender::Render;

class TestVisitor : public SegmentsVisitor
{
public:
    TestVisitor(NodeManagers *manager)
        : SegmentsVisitor(manager)
    {

    }

    virtual void visit(uint andx, const Vector3D &a, uint bndx, const Vector3D &b)
    {
        m_segments.push_back(TestSegment(andx, a, bndx, b));
    }

    NodeManagers *nodeManagers() const
    {
        return m_manager;
    }

    Qt3DCore::QNodeId nodeId() const
    {
        return m_nodeId;
    }

    uint segmentCount() const
    {
        return m_segments.size();
    }

    bool verifySegment(uint segment, uint andx, uint bndx, Vector3D a, Vector3D b) const
    {
        if (segment >= uint(m_segments.size()))
            return false;
        if (andx != m_segments[segment].abcndx[0]
             || bndx != m_segments[segment].abcndx[1])
            return false;

        if (!qFuzzyCompare(a, m_segments[segment].abc[0])
             || !qFuzzyCompare(b, m_segments[segment].abc[1]))
            return false;

        return true;
    }
private:
    struct TestSegment
    {
        uint abcndx[2];
        Vector3D abc[2];
        TestSegment()
        {
            abcndx[0] = abcndx[1] = uint(-1);
        }

        TestSegment(uint andx, const Vector3D &a, uint bndx, const Vector3D &b)
        {
            abcndx[0] = andx;
            abcndx[1] = bndx;
            abc[0] = a;
            abc[1] = b;
        }
    };
    QVector<TestSegment> m_segments;
};

class tst_SegmentVisitor : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

private Q_SLOTS:

    void checkInitialize()
    {
        // WHEN
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        TestVisitor visitor(nodeManagers.data());

        // THEN
        QCOMPARE(visitor.nodeManagers(), nodeManagers.data());
    }

    void checkApplyEntity()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        QScopedPointer<Qt3DCore::QEntity> entity(new Qt3DCore::QEntity());
        TestVisitor visitor(nodeManagers.data());

        // WHEN
        visitor.apply(entity.data());

        // THEN
        QCOMPARE(visitor.nodeId(), entity->id());
        QCOMPARE(visitor.segmentCount(), uint(0));
    }

    void checkApplyGeometryRenderer()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        QScopedPointer<GeometryRenderer> geometryRenderer(new GeometryRenderer());
        TestVisitor visitor(nodeManagers.data());

        // WHEN
        visitor.apply(geometryRenderer.data(), Qt3DCore::QNodeId());

        // THEN
        // tadaa, nothing should happen
    }

    void testVisitSegments()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 2 * 3);
        float *dataPtr = reinterpret_cast<float *>(data.data());
        dataPtr[0] = 0;
        dataPtr[1] = 0;
        dataPtr[2] = 1.0f;
        dataPtr[3] = 1.0f;
        dataPtr[4] = 0;
        dataPtr[5] = 0;

        dataPtr[6] = 0;
        dataPtr[7] = 1.0f;
        dataPtr[8] = 0;
        dataPtr[9] = 0;
        dataPtr[10] = 0;
        dataPtr[11] = 1.0f;

        dataPtr[12] = 1.0f;
        dataPtr[13] = 0;
        dataPtr[14] = 0;
        dataPtr[15] = 0;
        dataPtr[16] = 1.0f;
        dataPtr[17] = 0;

        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(6);
        positionAttribute->setByteStride(3*4);
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
        geometry->addAttribute(positionAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);

        Attribute *backendAttribute = nodeManagers->attributeManager()->getOrCreateResource(positionAttribute->id());
        backendAttribute->setRenderer(&renderer);
        simulateInitialization(positionAttribute.data(), backendAttribute);

        Geometry *backendGeometry = nodeManagers->geometryManager()->getOrCreateResource(geometry->id());
        backendGeometry->setRenderer(&renderer);
        simulateInitialization(geometry, backendGeometry);

        GeometryRenderer *backendRenderer = nodeManagers->geometryRendererManager()->getOrCreateResource(geometryRenderer->id());
        backendRenderer->setRenderer(&renderer);
        backendRenderer->setManager(nodeManagers->geometryRendererManager());
        simulateInitialization(geometryRenderer.data(), backendRenderer);

        // WHEN
        visitor.apply(backendRenderer, Qt3DCore::QNodeId());

        // THEN
        QCOMPARE(visitor.segmentCount(), uint(3));
        QVERIFY(visitor.verifySegment(0, 0,1, Vector3D(0,0,1), Vector3D(1,0,0)));
        QVERIFY(visitor.verifySegment(1, 2,3, Vector3D(0,1,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifySegment(2, 4,5, Vector3D(1,0,0), Vector3D(0,1,0)));
    }

    void testVisitSegmentsIndexed()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QAttribute> indexAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        QScopedPointer<Qt3DRender::QBuffer> indexDataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 4);
        float *dataPtr = reinterpret_cast<float *>(data.data());
        dataPtr[0] = 0;
        dataPtr[1] = 0;
        dataPtr[2] = 0.0f;
        dataPtr[3] = 1.0f;
        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.0;
        dataPtr[7] = 1.0f;
        dataPtr[8] = 0;
        dataPtr[9] = 0;
        dataPtr[10] = 1.0f;
        dataPtr[11] = 0;
        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 2 * 5);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 1;
        iDataPtr[3] = 2;
        iDataPtr[4] = 2;
        iDataPtr[5] = 3;
        iDataPtr[6] = 0;
        iDataPtr[7] = 2;
        iDataPtr[8] = 1;
        iDataPtr[9] = 3;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(4);
        positionAttribute->setByteStride(3*sizeof(float));
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);

        indexAttribute->setBuffer(indexDataBuffer.data());
        indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
        indexAttribute->setCount(2*5);
        indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);

        geometry->addAttribute(positionAttribute.data());
        geometry->addAttribute(indexAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);

        Attribute *backendAttribute = nodeManagers->attributeManager()->getOrCreateResource(positionAttribute->id());
        backendAttribute->setRenderer(&renderer);
        simulateInitialization(positionAttribute.data(), backendAttribute);

        Attribute *backendIndexAttribute = nodeManagers->attributeManager()->getOrCreateResource(indexAttribute->id());
        backendIndexAttribute->setRenderer(&renderer);
        simulateInitialization(indexAttribute.data(), backendIndexAttribute);

        Geometry *backendGeometry = nodeManagers->geometryManager()->getOrCreateResource(geometry->id());
        backendGeometry->setRenderer(&renderer);
        simulateInitialization(geometry, backendGeometry);

        GeometryRenderer *backendRenderer = nodeManagers->geometryRendererManager()->getOrCreateResource(geometryRenderer->id());
        backendRenderer->setRenderer(&renderer);
        backendRenderer->setManager(nodeManagers->geometryRendererManager());
        simulateInitialization(geometryRenderer.data(), backendRenderer);

        // WHEN
        visitor.apply(backendRenderer, Qt3DCore::QNodeId());

        // THEN
        QCOMPARE(visitor.segmentCount(), uint(5));
        QVERIFY(visitor.verifySegment(0, 0,1, Vector3D(0,0,0), Vector3D(1,0,0)));
        QVERIFY(visitor.verifySegment(1, 1,2, Vector3D(1,0,0), Vector3D(1,1,0)));
        QVERIFY(visitor.verifySegment(2, 2,3, Vector3D(1,1,0), Vector3D(0,1,0)));
        QVERIFY(visitor.verifySegment(3, 0,2, Vector3D(0,0,0), Vector3D(1,1,0)));
        QVERIFY(visitor.verifySegment(4, 1,3, Vector3D(1,0,0), Vector3D(0,1,0)));
    }

    void testVisitLineStrip()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 4);
        float *dataPtr = reinterpret_cast<float *>(data.data());
        dataPtr[0] = 0;
        dataPtr[1] = 0;
        dataPtr[2] = 0.0f;
        dataPtr[3] = 1.0f;
        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.0f;
        dataPtr[7] = 1.0f;
        dataPtr[8] = 0;
        dataPtr[9] = 0;
        dataPtr[10] = 1.0f;
        dataPtr[11] = 0;
        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(4);
        positionAttribute->setByteStride(3*4);
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
        geometry->addAttribute(positionAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::LineStrip);

        Attribute *backendAttribute = nodeManagers->attributeManager()->getOrCreateResource(positionAttribute->id());
        backendAttribute->setRenderer(&renderer);
        simulateInitialization(positionAttribute.data(), backendAttribute);

        Geometry *backendGeometry = nodeManagers->geometryManager()->getOrCreateResource(geometry->id());
        backendGeometry->setRenderer(&renderer);
        simulateInitialization(geometry, backendGeometry);

        GeometryRenderer *backendRenderer = nodeManagers->geometryRendererManager()->getOrCreateResource(geometryRenderer->id());
        backendRenderer->setRenderer(&renderer);
        backendRenderer->setManager(nodeManagers->geometryRendererManager());
        simulateInitialization(geometryRenderer.data(), backendRenderer);

        // WHEN
        visitor.apply(backendRenderer, Qt3DCore::QNodeId());

        // THEN
        QCOMPARE(visitor.segmentCount(), uint(3));
        QVERIFY(visitor.verifySegment(0, 0,1, Vector3D(0,0,0), Vector3D(1,0,0)));
        QVERIFY(visitor.verifySegment(1, 1,2, Vector3D(1,0,0), Vector3D(1,1,0)));
        QVERIFY(visitor.verifySegment(2, 2,3, Vector3D(1,1,0), Vector3D(0,1,0)));
    }

    void testVisitListStripIndexed()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QAttribute> indexAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        QScopedPointer<Qt3DRender::QBuffer> indexDataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 4);
        float *dataPtr = reinterpret_cast<float *>(data.data());
        dataPtr[0] = 0;
        dataPtr[1] = 0;
        dataPtr[2] = 0.0f;
        dataPtr[3] = 1.0f;
        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.0;
        dataPtr[7] = 1.0f;
        dataPtr[8] = 0;
        dataPtr[9] = 0;
        dataPtr[10] = 1.0f;
        dataPtr[11] = 0;
        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 7);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 2;
        iDataPtr[3] = 3;
        iDataPtr[4] = static_cast<uint>(-1);
        iDataPtr[5] = 0;
        iDataPtr[6] = 1;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(4);
        positionAttribute->setByteStride(3*sizeof(float));
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);

        indexAttribute->setBuffer(indexDataBuffer.data());
        indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
        indexAttribute->setCount(7);
        indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);

        geometry->addAttribute(positionAttribute.data());
        geometry->addAttribute(indexAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::LineStrip);
        geometryRenderer->setPrimitiveRestartEnabled(true);
        geometryRenderer->setRestartIndexValue(-1);

        Attribute *backendAttribute = nodeManagers->attributeManager()->getOrCreateResource(positionAttribute->id());
        backendAttribute->setRenderer(&renderer);
        simulateInitialization(positionAttribute.data(), backendAttribute);

        Attribute *backendIndexAttribute = nodeManagers->attributeManager()->getOrCreateResource(indexAttribute->id());
        backendIndexAttribute->setRenderer(&renderer);
        simulateInitialization(indexAttribute.data(), backendIndexAttribute);

        Geometry *backendGeometry = nodeManagers->geometryManager()->getOrCreateResource(geometry->id());
        backendGeometry->setRenderer(&renderer);
        simulateInitialization(geometry, backendGeometry);

        GeometryRenderer *backendRenderer = nodeManagers->geometryRendererManager()->getOrCreateResource(geometryRenderer->id());
        backendRenderer->setRenderer(&renderer);
        backendRenderer->setManager(nodeManagers->geometryRendererManager());
        simulateInitialization(geometryRenderer.data(), backendRenderer);

        // WHEN
        visitor.apply(backendRenderer, Qt3DCore::QNodeId());

        // THEN
        QCOMPARE(visitor.segmentCount(), uint(4));
        QVERIFY(visitor.verifySegment(0, 0,1, Vector3D(0,0,0), Vector3D(1,0,0)));
        QVERIFY(visitor.verifySegment(1, 1,2, Vector3D(1,0,0), Vector3D(1,1,0)));
        QVERIFY(visitor.verifySegment(2, 2,3, Vector3D(1,1,0), Vector3D(0,1,0)));
        QVERIFY(visitor.verifySegment(3, 0,1, Vector3D(0,0,0), Vector3D(1,0,0)));
    }

    void testVisitLineLoop()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 4);
        float *dataPtr = reinterpret_cast<float *>(data.data());
        dataPtr[0] = 0;
        dataPtr[1] = 0;
        dataPtr[2] = 0.0f;
        dataPtr[3] = 1.0f;
        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.0f;
        dataPtr[7] = 1.0f;
        dataPtr[8] = 0;
        dataPtr[9] = 0;
        dataPtr[10] = 1.0f;
        dataPtr[11] = 0;
        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(4);
        positionAttribute->setByteStride(3*4);
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
        geometry->addAttribute(positionAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::LineLoop);

        Attribute *backendAttribute = nodeManagers->attributeManager()->getOrCreateResource(positionAttribute->id());
        backendAttribute->setRenderer(&renderer);
        simulateInitialization(positionAttribute.data(), backendAttribute);

        Geometry *backendGeometry = nodeManagers->geometryManager()->getOrCreateResource(geometry->id());
        backendGeometry->setRenderer(&renderer);
        simulateInitialization(geometry, backendGeometry);

        GeometryRenderer *backendRenderer = nodeManagers->geometryRendererManager()->getOrCreateResource(geometryRenderer->id());
        backendRenderer->setRenderer(&renderer);
        backendRenderer->setManager(nodeManagers->geometryRendererManager());
        simulateInitialization(geometryRenderer.data(), backendRenderer);

        // WHEN
        visitor.apply(backendRenderer, Qt3DCore::QNodeId());

        // THEN
        QCOMPARE(visitor.segmentCount(), uint(4));
        QVERIFY(visitor.verifySegment(0, 0,1, Vector3D(0,0,0), Vector3D(1,0,0)));
        QVERIFY(visitor.verifySegment(1, 1,2, Vector3D(1,0,0), Vector3D(1,1,0)));
        QVERIFY(visitor.verifySegment(2, 2,3, Vector3D(1,1,0), Vector3D(0,1,0)));
        QVERIFY(visitor.verifySegment(3, 3,0, Vector3D(0,1,0), Vector3D(0,0,0)));
    }

    void testVisitLineLoopIndexed()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QAttribute> indexAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        QScopedPointer<Qt3DRender::QBuffer> indexDataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 4);
        float *dataPtr = reinterpret_cast<float *>(data.data());
        dataPtr[0] = 0;
        dataPtr[1] = 0;
        dataPtr[2] = 0.0f;
        dataPtr[3] = 1.0f;
        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.0;
        dataPtr[7] = 1.0f;
        dataPtr[8] = 0;
        dataPtr[9] = 0;
        dataPtr[10] = 1.0f;
        dataPtr[11] = 0;
        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 8);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 2;
        iDataPtr[3] = 3;
        iDataPtr[4] = static_cast<uint>(-1);
        iDataPtr[5] = 0;
        iDataPtr[6] = 1;
        iDataPtr[7] = 2;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(4);
        positionAttribute->setByteStride(3*sizeof(float));
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);

        indexAttribute->setBuffer(indexDataBuffer.data());
        indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
        indexAttribute->setCount(8);
        indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);

        geometry->addAttribute(positionAttribute.data());
        geometry->addAttribute(indexAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::LineLoop);
        geometryRenderer->setPrimitiveRestartEnabled(true);
        geometryRenderer->setRestartIndexValue(-1);

        Attribute *backendAttribute = nodeManagers->attributeManager()->getOrCreateResource(positionAttribute->id());
        backendAttribute->setRenderer(&renderer);
        simulateInitialization(positionAttribute.data(), backendAttribute);

        Attribute *backendIndexAttribute = nodeManagers->attributeManager()->getOrCreateResource(indexAttribute->id());
        backendIndexAttribute->setRenderer(&renderer);
        simulateInitialization(indexAttribute.data(), backendIndexAttribute);

        Geometry *backendGeometry = nodeManagers->geometryManager()->getOrCreateResource(geometry->id());
        backendGeometry->setRenderer(&renderer);
        simulateInitialization(geometry, backendGeometry);

        GeometryRenderer *backendRenderer = nodeManagers->geometryRendererManager()->getOrCreateResource(geometryRenderer->id());
        backendRenderer->setRenderer(&renderer);
        backendRenderer->setManager(nodeManagers->geometryRendererManager());
        simulateInitialization(geometryRenderer.data(), backendRenderer);

        // WHEN
        visitor.apply(backendRenderer, Qt3DCore::QNodeId());

        // THEN
        QCOMPARE(visitor.segmentCount(), uint(7));
        QVERIFY(visitor.verifySegment(0, 0,1, Vector3D(0,0,0), Vector3D(1,0,0)));
        QVERIFY(visitor.verifySegment(1, 1,2, Vector3D(1,0,0), Vector3D(1,1,0)));
        QVERIFY(visitor.verifySegment(2, 2,3, Vector3D(1,1,0), Vector3D(0,1,0)));
        QVERIFY(visitor.verifySegment(3, 3,0, Vector3D(0,1,0), Vector3D(0,0,0)));
        QVERIFY(visitor.verifySegment(4, 0,1, Vector3D(0,0,0), Vector3D(1,0,0)));
        QVERIFY(visitor.verifySegment(5, 1,2, Vector3D(1,0,0), Vector3D(1,1,0)));
        QVERIFY(visitor.verifySegment(6, 2,0, Vector3D(1,1,0), Vector3D(0,0,0)));
    }

    void testVisitLineAdjacency()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 2 * 2);
        float *dataPtr = reinterpret_cast<float *>(data.data());
        dataPtr[0] = 0;
        dataPtr[1] = 0;
        dataPtr[2] = 1.0f;
        dataPtr[3] = 1.0f;
        dataPtr[4] = 0;
        dataPtr[5] = 0;

        dataPtr[6] = 0;
        dataPtr[7] = 1.0f;
        dataPtr[8] = 0;
        dataPtr[9] = 1.0f;
        dataPtr[10] = 0;
        dataPtr[12] = 0;
        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(4);
        positionAttribute->setByteStride(3*4);
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
        geometry->addAttribute(positionAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::LinesAdjacency);

        Attribute *backendAttribute = nodeManagers->attributeManager()->getOrCreateResource(positionAttribute->id());
        backendAttribute->setRenderer(&renderer);
        simulateInitialization(positionAttribute.data(), backendAttribute);

        Geometry *backendGeometry = nodeManagers->geometryManager()->getOrCreateResource(geometry->id());
        backendGeometry->setRenderer(&renderer);
        simulateInitialization(geometry, backendGeometry);

        GeometryRenderer *backendRenderer = nodeManagers->geometryRendererManager()->getOrCreateResource(geometryRenderer->id());
        backendRenderer->setRenderer(&renderer);
        backendRenderer->setManager(nodeManagers->geometryRendererManager());
        simulateInitialization(geometryRenderer.data(), backendRenderer);

        // WHEN
        visitor.apply(backendRenderer, Qt3DCore::QNodeId());

        // THEN
        QCOMPARE(visitor.segmentCount(), uint(1));
        QVERIFY(visitor.verifySegment(0, 1, 2, Vector3D(1,0,0), Vector3D(0,1,0)));
    }

    void testVisitLinesAdjacencyIndexed()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QAttribute> indexAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        QScopedPointer<Qt3DRender::QBuffer> indexDataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 2 * 2);
        float *dataPtr = reinterpret_cast<float *>(data.data());
        dataPtr[0] = 0;
        dataPtr[1] = 0;
        dataPtr[2] = 1.0f;
        dataPtr[3] = 1.0f;
        dataPtr[4] = 0;
        dataPtr[5] = 0;

        dataPtr[6] = 0;
        dataPtr[7] = 1.0f;
        dataPtr[8] = 0;
        dataPtr[9] = 1.0f;
        dataPtr[10] = 0;
        dataPtr[12] = 0;
        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 4);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 2;
        iDataPtr[3] = 3;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(4);
        positionAttribute->setByteStride(3*sizeof(float));
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);

        indexAttribute->setBuffer(indexDataBuffer.data());
        indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
        indexAttribute->setCount(4);
        indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);

        geometry->addAttribute(positionAttribute.data());
        geometry->addAttribute(indexAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::LinesAdjacency);

        Attribute *backendAttribute = nodeManagers->attributeManager()->getOrCreateResource(positionAttribute->id());
        backendAttribute->setRenderer(&renderer);
        simulateInitialization(positionAttribute.data(), backendAttribute);

        Attribute *backendIndexAttribute = nodeManagers->attributeManager()->getOrCreateResource(indexAttribute->id());
        backendIndexAttribute->setRenderer(&renderer);
        simulateInitialization(indexAttribute.data(), backendIndexAttribute);

        Geometry *backendGeometry = nodeManagers->geometryManager()->getOrCreateResource(geometry->id());
        backendGeometry->setRenderer(&renderer);
        simulateInitialization(geometry, backendGeometry);

        GeometryRenderer *backendRenderer = nodeManagers->geometryRendererManager()->getOrCreateResource(geometryRenderer->id());
        backendRenderer->setRenderer(&renderer);
        backendRenderer->setManager(nodeManagers->geometryRendererManager());
        simulateInitialization(geometryRenderer.data(), backendRenderer);

        // WHEN
        visitor.apply(backendRenderer, Qt3DCore::QNodeId());

        // THEN
        QCOMPARE(visitor.segmentCount(), uint(1));
        QVERIFY(visitor.verifySegment(0, 1, 2, Vector3D(1,0,0), Vector3D(0,1,0)));
    }
};

QTEST_MAIN(tst_SegmentVisitor)

#include "tst_segmentvisitor.moc"
