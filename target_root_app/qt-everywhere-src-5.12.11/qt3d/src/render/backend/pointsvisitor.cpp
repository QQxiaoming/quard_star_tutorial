/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "pointsvisitor_p.h"
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qgeometryrenderer.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/buffermanager_p.h>
#include <Qt3DRender/private/geometryrenderer_p.h>
#include <Qt3DRender/private/geometryrenderermanager_p.h>
#include <Qt3DRender/private/geometry_p.h>
#include <Qt3DRender/private/attribute_p.h>
#include <Qt3DRender/private/buffer_p.h>
#include <Qt3DRender/private/trianglesvisitor_p.h>
#include <Qt3DRender/private/visitorutils_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

namespace Render {

namespace {

// indices, vertices are already offset
template<typename Index, typename Vertex>
void traverseCoordinatesIndexed(Index *indices,
                                Vertex *vertices,
                                const BufferInfo &indexInfo,
                                const BufferInfo &vertexInfo,
                                PointsVisitor *visitor)
{
    uint i = 0;
    const uint verticesStride = vertexInfo.byteStride / sizeof(Vertex);
    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);

    uint ndx;
    Vector3D abc;
    while (i < indexInfo.count) {
        ndx = indices[i];
        const uint idx = ndx * verticesStride;
        for (uint j = 0; j < maxVerticesDataSize; ++j) {
            abc[j] = vertices[idx + j];
        }
        visitor->visit(ndx, abc);
        ++i;
    }
}

// vertices are already offset
template<typename Vertex>
void traverseCoordinates(Vertex *vertices,
                         const BufferInfo &vertexInfo,
                         PointsVisitor *visitor)
{
    const uint verticesStride = vertexInfo.byteStride / sizeof(Vertex);
    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);

    uint ndx = 0;
    Vector3D abc;
    while (ndx < vertexInfo.count) {
        const uint idx = ndx * verticesStride;
        for (uint j = 0; j < maxVerticesDataSize; ++j)
            abc[j] = vertices[idx + j];
        visitor->visit(ndx, abc);
        ++ndx;
    }
}

template<typename Index, typename Visitor>
struct IndexedVertexExecutor
{
    template<typename Vertex>
    void operator ()(const BufferInfo &vertexInfo, Vertex * vertices)
    {
        traverseCoordinatesIndexed(m_indices, vertices, m_indexBufferInfo, vertexInfo, m_visitor);
    }

    BufferInfo m_indexBufferInfo;
    Index *m_indices;
    Qt3DRender::QGeometryRenderer::PrimitiveType m_primitiveType;
    Visitor* m_visitor;
};

template<typename Visitor>
struct IndexExecutor
{
    template<typename Index>
    void operator ()( const BufferInfo &indexInfo, Index *indices)
    {
        IndexedVertexExecutor<Index, Visitor> exec;
        exec.m_primitiveType = m_primitiveType;
        exec.m_indices = indices;
        exec.m_indexBufferInfo = indexInfo;
        exec.m_visitor = m_visitor;
        Qt3DRender::Render::Visitor::processBuffer(m_vertexBufferInfo, exec);
    }

    BufferInfo m_vertexBufferInfo;
    Qt3DRender::QGeometryRenderer::PrimitiveType m_primitiveType;
    Visitor* m_visitor;
};

template<typename Visitor>
struct VertexExecutor
{
    template<typename Vertex>
    void operator ()(const BufferInfo &vertexInfo, Vertex *vertices)
    {
        switch (m_primitiveType) {
        case Qt3DRender::QGeometryRenderer::Points:
            traverseCoordinates(vertices, vertexInfo, m_visitor);
            return;
        default:
            Q_UNREACHABLE();
            return;
        }
    }

    Qt3DRender::QGeometryRenderer::PrimitiveType m_primitiveType;
    Visitor* m_visitor;
};

} // anonymous


PointsVisitor::~PointsVisitor()
{

}

void PointsVisitor::apply(const Qt3DCore::QEntity *entity)
{
    GeometryRenderer *renderer = m_manager->geometryRendererManager()->lookupResource(entity->id());
    apply(renderer, entity->id());
}

void PointsVisitor::apply(const GeometryRenderer *renderer, const Qt3DCore::QNodeId id)
{
    m_nodeId = id;
    if (renderer && renderer->instanceCount() == 1) {
        Visitor::visitPrimitives<VertexExecutor<PointsVisitor>,
                IndexExecutor<PointsVisitor>, PointsVisitor>(m_manager, renderer, this);
    }
}

} // namespace Render

} // namespace Qt3DRender

QT_END_NAMESPACE
