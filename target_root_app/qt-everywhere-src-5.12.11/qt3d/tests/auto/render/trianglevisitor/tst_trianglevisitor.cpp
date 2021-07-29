/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
#include <private/trianglesvisitor_p.h>
#include <private/nodemanagers_p.h>
#include <private/managers_p.h>
#include <private/geometryrenderer_p.h>
#include <private/geometryrenderermanager_p.h>
#include <private/buffermanager_p.h>
#include "testrenderer.h"

using namespace Qt3DRender::Render;

class TestVisitor : public TrianglesVisitor
{
public:
    TestVisitor(NodeManagers *manager)
        : TrianglesVisitor(manager)
    {

    }

    virtual void visit(uint andx, const Vector3D &a, uint bndx, const Vector3D &b, uint cndx, const Vector3D &c)
    {
        m_triangles.push_back(TestTriangle(andx, a, bndx, b, cndx, c));
    }

    NodeManagers *nodeManagers() const
    {
        return m_manager;
    }

    Qt3DCore::QNodeId nodeId() const
    {
        return m_nodeId;
    }

    uint triangleCount() const
    {
        return m_triangles.size();
    }

    bool verifyTriangle(uint triangle, uint andx, uint bndx, uint cndx, Vector3D a, Vector3D b, Vector3D c) const
    {
        if (triangle >= uint(m_triangles.size()))
            return false;
        if (andx != m_triangles[triangle].abcndx[0]
             || bndx != m_triangles[triangle].abcndx[1]
             || cndx != m_triangles[triangle].abcndx[2])
            return false;

        if (!qFuzzyCompare(a, m_triangles[triangle].abc[0])
             || !qFuzzyCompare(b, m_triangles[triangle].abc[1])
             || !qFuzzyCompare(c, m_triangles[triangle].abc[2]))
            return false;

        return true;
    }
private:
    struct TestTriangle
    {
        uint abcndx[3];
        Vector3D abc[3];
        TestTriangle()
        {
            abcndx[0] = abcndx[1] = abcndx[2] = uint(-1);
        }

        TestTriangle(uint andx, const Vector3D &a, uint bndx, const Vector3D &b, uint cndx, const Vector3D &c)
        {
            abcndx[0] = andx;
            abcndx[1] = bndx;
            abcndx[2] = cndx;
            abc[0] = a;
            abc[1] = b;
            abc[2] = c;
        }
    };
    QVector<TestTriangle> m_triangles;
};

class tst_TriangleVisitor : public Qt3DCore::QBackendNodeTester
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
        QVERIFY(visitor.triangleCount() == 0);
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

    void testVisitTriangles()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 3 * 2);
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
        positionAttribute->setByteStride(0);
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
        geometry->addAttribute(positionAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

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
        QVERIFY(visitor.triangleCount() == 2);
        QVERIFY(visitor.verifyTriangle(0, 2,1,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 5,4,3, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
    }

    void testVisitTrianglesIndexed()
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
        data.resize(sizeof(float) * 3 * 3 * 2);
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

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 3 * 5);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 2;
        iDataPtr[3] = 3;
        iDataPtr[4] = 4;
        iDataPtr[5] = 5;
        iDataPtr[6] = 5;
        iDataPtr[7] = 1;
        iDataPtr[8] = 0;
        iDataPtr[9] = 4;
        iDataPtr[10] = 3;
        iDataPtr[11] = 2;
        iDataPtr[12] = 0;
        iDataPtr[13] = 2;
        iDataPtr[14] = 4;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(6);
        positionAttribute->setByteStride(3*sizeof(float));
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);

        indexAttribute->setBuffer(indexDataBuffer.data());
        indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
        indexAttribute->setCount(3*5);
        indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);

        geometry->addAttribute(positionAttribute.data());
        geometry->addAttribute(indexAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

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
        QVERIFY(visitor.triangleCount() == 5);
        QVERIFY(visitor.verifyTriangle(0, 2,1,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 5,4,3, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(2, 0,1,5, Vector3D(0,0,1), Vector3D(1,0,0), Vector3D(0,1,0)));
        QVERIFY(visitor.verifyTriangle(3, 2,3,4, Vector3D(0,1,0), Vector3D(0,0,1), Vector3D(1,0,0)));
        QVERIFY(visitor.verifyTriangle(4, 4,2,0, Vector3D(1,0,0), Vector3D(0,1,0), Vector3D(0,0,1)));
    }

    void testVisitTriangleStrip()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 3 * 2);
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
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::TriangleStrip);

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
        QVERIFY(visitor.triangleCount() == 4);
        QVERIFY(visitor.verifyTriangle(0, 2,1,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 3,2,1, Vector3D(0,0,1), Vector3D(0,1,0), Vector3D(1,0,0)));
        QVERIFY(visitor.verifyTriangle(2, 4,3,2, Vector3D(1,0,0), Vector3D(0,0,1), Vector3D(0,1,0)));
        QVERIFY(visitor.verifyTriangle(3, 5,4,3, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
    }

    void testVisitTriangleStripIndexed()
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
        data.resize(sizeof(float) * 3 * 3 * 2);
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

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 4 * 4);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 2;
        iDataPtr[3] = 3;
        iDataPtr[4] = 4;
        iDataPtr[5] = 5;
        iDataPtr[6] = 5;
        iDataPtr[7] = 1;
        iDataPtr[8] = 0;
        iDataPtr[9] = 4;
        iDataPtr[10] = 3;
        iDataPtr[11] = 2;
        iDataPtr[12] = static_cast<uint>(-1);
        iDataPtr[13] = 0;
        iDataPtr[14] = 1;
        iDataPtr[15] = 2;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(6);
        positionAttribute->setByteStride(3*sizeof(float));
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);

        indexAttribute->setBuffer(indexDataBuffer.data());
        indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
        indexAttribute->setCount(4*4);
        indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);

        geometry->addAttribute(positionAttribute.data());
        geometry->addAttribute(indexAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::TriangleStrip);
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
        QCOMPARE(visitor.triangleCount(), 9U);
        QVERIFY(visitor.verifyTriangle(0, 2,1,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 3,2,1, Vector3D(0,0,1), Vector3D(0,1,0), Vector3D(1,0,0)));
        QVERIFY(visitor.verifyTriangle(2, 4,3,2, Vector3D(1,0,0), Vector3D(0,0,1), Vector3D(0,1,0)));
        QVERIFY(visitor.verifyTriangle(3, 5,4,3, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(4, 0,1,5, Vector3D(0,0,1), Vector3D(1,0,0), Vector3D(0,1,0)));
        QVERIFY(visitor.verifyTriangle(5, 4,0,1, Vector3D(1,0,0), Vector3D(0,0,1), Vector3D(1,0,0)));
        QVERIFY(visitor.verifyTriangle(6, 3,4,0, Vector3D(0,0,1), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(7, 2,3,4, Vector3D(0,1,0), Vector3D(0,0,1), Vector3D(1,0,0)));
        QVERIFY(visitor.verifyTriangle(8, 2,1,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
    }

    void testVisitTriangleFan()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 3 * 2);
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
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::TriangleFan);

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
        QVERIFY(visitor.triangleCount() == 4);
        QVERIFY(visitor.verifyTriangle(0, 2,1,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 3,2,0, Vector3D(0,0,1), Vector3D(0,1,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(2, 4,3,0, Vector3D(1,0,0), Vector3D(0,0,1), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(3, 5,4,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
    }

    void testVisitTriangleFanIndexed()
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
        data.resize(sizeof(float) * 3 * 3 * 2);
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

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 10);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 2;
        iDataPtr[3] = 3;
        iDataPtr[4] = 4;
        iDataPtr[5] = 5;
        iDataPtr[6] = static_cast<uint>(-1);
        iDataPtr[7] = 0;
        iDataPtr[8] = 1;
        iDataPtr[9] = 2;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(6);
        positionAttribute->setByteStride(3*sizeof(float));
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);

        indexAttribute->setBuffer(indexDataBuffer.data());
        indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
        indexAttribute->setCount(10);
        indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);

        geometry->addAttribute(positionAttribute.data());
        geometry->addAttribute(indexAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::TriangleFan);
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
        QCOMPARE(visitor.triangleCount(), 5U);
        QVERIFY(visitor.verifyTriangle(0, 2,1,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 3,2,0, Vector3D(0,0,1), Vector3D(0,1,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(2, 4,3,0, Vector3D(1,0,0), Vector3D(0,0,1), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(3, 5,4,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(4, 2,1,0, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,0,1)));
    }

    void testVisitTrianglesAdjacency()
    {
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 3 * 2);
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
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::TrianglesAdjacency);

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
        QVERIFY(visitor.triangleCount() == 1);
        QVERIFY(visitor.verifyTriangle(0, 4,2,0, Vector3D(1,0,0), Vector3D(0,1,0), Vector3D(0,0,1)));
    }

    void testVisitTrianglesAdjacencyIndexed()
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
        data.resize(sizeof(float) * 3 * 3 * 2);
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

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 3 * 4);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 2;
        iDataPtr[3] = 3;
        iDataPtr[4] = 4;
        iDataPtr[5] = 5;

        iDataPtr[6] = 5;
        iDataPtr[7] = 1;
        iDataPtr[8] = 0;
        iDataPtr[9] = 4;
        iDataPtr[10] = 3;
        iDataPtr[11] = 2;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(6);
        positionAttribute->setByteStride(3*sizeof(float));
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);

        indexAttribute->setBuffer(indexDataBuffer.data());
        indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
        indexAttribute->setCount(3*4);
        indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);

        geometry->addAttribute(positionAttribute.data());
        geometry->addAttribute(indexAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::TrianglesAdjacency);

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
        QVERIFY(visitor.triangleCount() == 2);
        QVERIFY(visitor.verifyTriangle(0, 4,2,0, Vector3D(1,0,0), Vector3D(0,1,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 3,0,5, Vector3D(0,0,1), Vector3D(0,0,1), Vector3D(0,1,0)));
    }

    void testVisitTriangleStripAdjacency()
    {
        QSKIP("TriangleStripAdjacency not implemented");
        QScopedPointer<NodeManagers> nodeManagers(new NodeManagers());
        Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry();
        QScopedPointer<Qt3DRender::QGeometryRenderer> geometryRenderer(new Qt3DRender::QGeometryRenderer());
        QScopedPointer<Qt3DRender::QAttribute> positionAttribute(new Qt3DRender::QAttribute());
        QScopedPointer<Qt3DRender::QBuffer> dataBuffer(new Qt3DRender::QBuffer());
        TestVisitor visitor(nodeManagers.data());
        TestRenderer renderer;

        QByteArray data;
        data.resize(sizeof(float) * 3 * 8);
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

        dataPtr[18] = 1.0f;
        dataPtr[19] = 1.0f;
        dataPtr[20] = 1.0f;

        dataPtr[21] = 0;
        dataPtr[22] = 0;
        dataPtr[22] = 0;
        dataBuffer->setData(data);
        Buffer *backendBuffer = nodeManagers->bufferManager()->getOrCreateResource(dataBuffer->id());
        backendBuffer->setRenderer(&renderer);
        backendBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(dataBuffer.data(), backendBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(8);
        positionAttribute->setByteStride(3*4);
        positionAttribute->setByteOffset(0);
        positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
        geometry->addAttribute(positionAttribute.data());

        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::TriangleStripAdjacency);

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
        QVERIFY(visitor.triangleCount() == 2);
        QVERIFY(visitor.verifyTriangle(0, 4,2,0, Vector3D(1,0,0), Vector3D(0,1,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 6,4,2, Vector3D(1,1,1), Vector3D(1,0,0), Vector3D(0,1,0)));
    }

    void testVisitTriangleStripAdjacencyIndexed()
    {
        QSKIP("TriangleStripAdjacency not implemented");
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
        data.resize(sizeof(float) * 3 * 3 * 2);
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

        QByteArray indexData;
        indexData.resize(sizeof(uint) * 8);
        uint *iDataPtr = reinterpret_cast<uint *>(indexData.data());
        iDataPtr[0] = 0;
        iDataPtr[1] = 1;
        iDataPtr[2] = 2;
        iDataPtr[3] = 3;
        iDataPtr[4] = 4;
        iDataPtr[5] = 5;
        iDataPtr[6] = 5;
        iDataPtr[7] = 1;
        indexDataBuffer->setData(indexData);

        Buffer *backendIndexBuffer = nodeManagers->bufferManager()->getOrCreateResource(indexDataBuffer->id());
        backendIndexBuffer->setRenderer(&renderer);
        backendIndexBuffer->setManager(nodeManagers->bufferManager());
        simulateInitialization(indexDataBuffer.data(), backendIndexBuffer);

        positionAttribute->setBuffer(dataBuffer.data());
        positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setCount(6);
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
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::TriangleStrip);

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
        QVERIFY(visitor.triangleCount() == 2);
        QVERIFY(visitor.verifyTriangle(0, 4,2,0, Vector3D(1,0,0), Vector3D(0,1,0), Vector3D(0,0,1)));
        QVERIFY(visitor.verifyTriangle(1, 5,4,2, Vector3D(0,1,0), Vector3D(1,0,0), Vector3D(0,1,0)));
    }
};

QTEST_MAIN(tst_TriangleVisitor)

#include "tst_trianglevisitor.moc"
