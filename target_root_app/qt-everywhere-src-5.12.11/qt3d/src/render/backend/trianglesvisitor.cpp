/****************************************************************************
**
** Copyright (C) 2015 Paul Lemire paul.lemire350@gmail.com
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

#include "trianglesvisitor_p.h"
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
#include <Qt3DRender/private/visitorutils_p.h>
#include <Qt3DRender/private/bufferutils_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

namespace Render {

namespace {

bool isTriangleBased(Qt3DRender::QGeometryRenderer::PrimitiveType type) Q_DECL_NOTHROW
{
    switch (type) {
    case Qt3DRender::QGeometryRenderer::Triangles:
    case Qt3DRender::QGeometryRenderer::TriangleStrip:
    case Qt3DRender::QGeometryRenderer::TriangleFan:
    case Qt3DRender::QGeometryRenderer::TrianglesAdjacency:
    case Qt3DRender::QGeometryRenderer::TriangleStripAdjacency:
        return true;
    default:
        return false;
    }
}

// TO DO: Add methods for triangle strip adjacency
// What about primitive restart ?

// indices, vertices are already offset
template<typename index, typename vertex>
void traverseTrianglesIndexed(index *indices,
                              vertex *vertices,
                              const BufferInfo &indexInfo,
                              const BufferInfo &vertexInfo,
                              TrianglesVisitor* visitor)
{
    uint i = 0;
    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);
    const uint verticesStride = vertexInfo.byteStride ? vertexInfo.byteStride / sizeof(vertex) : maxVerticesDataSize;

    uint ndx[3];
    Vector3D abc[3];
    while (i < indexInfo.count) {
        for (uint u = 0; u < 3; ++u) {
            ndx[u] = indices[i + u];
            uint idx = ndx[u] * verticesStride;
            for (uint j = 0; j < maxVerticesDataSize; ++j) {
                abc[u][j] = vertices[idx + j];
            }
        }
        visitor->visit(ndx[2], abc[2], ndx[1], abc[1], ndx[0], abc[0]);
        i += 3;
    }
}

// vertices are already offset
template<typename vertex>
void traverseTriangles(vertex *vertices,
                       const BufferInfo &vertexInfo,
                       TrianglesVisitor* visitor)
{
    uint i = 0;

    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);
    const uint verticesStride = vertexInfo.byteStride ? vertexInfo.byteStride / sizeof(vertex) : maxVerticesDataSize;

    uint ndx[3];
    Vector3D abc[3];
    while (i < vertexInfo.count) {
        for (uint u = 0; u < 3; ++u) {
            ndx[u] = (i + u);
            uint idx = ndx[u] * verticesStride;
            for (uint j = 0; j < maxVerticesDataSize; ++j) {
                abc[u][j] = vertices[idx + j];
            }
        }
        visitor->visit(ndx[2], abc[2], ndx[1], abc[1], ndx[0], abc[0]);
        i += 3;
    }
}

static inline bool checkDegenerate(const uint ndx[3], const uint idx, const uint u)
{
    for (uint j = 0; j < u; ++j) {
        if (idx == ndx[j])
            return true;
    }
    return false;
}

// indices, vertices are already offset
template<typename index, typename vertex>
void traverseTriangleStripIndexed(index *indices,
                                  vertex *vertices,
                                  const BufferInfo &indexInfo,
                                  const BufferInfo &vertexInfo,
                                  TrianglesVisitor* visitor)
{
    uint i = 0;
    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);
    const uint verticesStride = vertexInfo.byteStride ? vertexInfo.byteStride / sizeof(vertex) : maxVerticesDataSize;

    uint ndx[3];
    Vector3D abc[3];
    while (i < indexInfo.count - 2) {
        if (indexInfo.restartEnabled && indexInfo.restartIndexValue == static_cast<int>(indices[i + 2])) {
            i += 3;
            continue;
        }
        bool degenerate = false;
        for (uint u = 0; u < 3; ++u) {
            ndx[u] = indices[i + u];
            if (checkDegenerate(ndx, ndx[u], u)) {
                degenerate = true;
                break;
            }
            uint idx = ndx[u] * verticesStride;
            for (uint j = 0; j < maxVerticesDataSize; ++j)
                abc[u][j] = vertices[idx + j];
        }
        if (!degenerate)
            visitor->visit(ndx[2], abc[2], ndx[1], abc[1], ndx[0], abc[0]);
        ++i;
    }
}

// vertices are already offset
template<typename vertex>
void traverseTriangleStrip(vertex *vertices,
                           const BufferInfo &vertexInfo,
                           TrianglesVisitor* visitor)
{
    uint i = 0;

    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);
    const uint verticesStride = vertexInfo.byteStride ? vertexInfo.byteStride / sizeof(vertex) : maxVerticesDataSize;

    uint ndx[3];
    Vector3D abc[3];
    while (i < vertexInfo.count - 2) {
        for (uint u = 0; u < 3; ++u) {
            ndx[u] = (i + u);
            uint idx = ndx[u] * verticesStride;
            for (uint j = 0; j < maxVerticesDataSize; ++j) {
                abc[u][j] = vertices[idx + j];
            }
        }
        visitor->visit(ndx[2], abc[2], ndx[1], abc[1], ndx[0], abc[0]);
        ++i;
    }
}

// indices, vertices are already offset
template<typename index, typename vertex>
void traverseTriangleFanIndexed(index *indices,
                                vertex *vertices,
                                const BufferInfo &indexInfo,
                                const BufferInfo &vertexInfo,
                                TrianglesVisitor* visitor)
{
    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);
    const uint verticesStride = vertexInfo.byteStride ? vertexInfo.byteStride / sizeof(vertex) : maxVerticesDataSize;

    uint ndx[3];
    Vector3D abc[3];

    for (uint j = 0; j < maxVerticesDataSize; ++j) {
        abc[0][j] = vertices[static_cast<int>(indices[0]) * verticesStride + j];
    }
    ndx[0] = indices[0];
    uint i = 1;
    while (i < indexInfo.count - 1) {
        if (indexInfo.restartEnabled && indexInfo.restartIndexValue == static_cast<int>(indices[i + 1])) {
            ndx[0] = indices[i + 2];
            i += 3;
            continue;
        }
        for (uint u = 0; u < 2; ++u) {
            ndx[u + 1] = indices[i + u];
            uint idx = ndx[u + 1] * verticesStride;
            for (uint j = 0; j < maxVerticesDataSize; ++j) {
                abc[u + 1][j] = vertices[idx + j];
            }
        }
        visitor->visit(ndx[2], abc[2], ndx[1], abc[1], ndx[0], abc[0]);
        ++i;
    }
}

// vertices are already offset
template<typename vertex>
void traverseTriangleFan(vertex *vertices,
                         const BufferInfo &vertexInfo,
                         TrianglesVisitor* visitor)
{
    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);
    const uint verticesStride = vertexInfo.byteStride ? vertexInfo.byteStride / sizeof(vertex) : maxVerticesDataSize;

    uint ndx[3];
    Vector3D abc[3];

    for (uint j = 0; j < maxVerticesDataSize; ++j) {
        abc[0][j] = vertices[j];
    }
    ndx[0] = 0;

    uint i = 1;
    while (i < vertexInfo.count - 1) {
        for (uint u = 0; u < 2; ++u) {
            ndx[u + 1] = (i + u);
            uint idx = ndx[u + 1] * verticesStride;
            for (uint j = 0; j < maxVerticesDataSize; ++j) {
                abc[u + 1][j] = vertices[idx + j];
            }
        }
        visitor->visit(ndx[2], abc[2], ndx[1], abc[1], ndx[0], abc[0]);
        i += 1;
    }
}

// indices, vertices are already offset
template<typename index, typename vertex>
void traverseTriangleAdjacencyIndexed(index *indices,
                                      vertex *vertices,
                                      const BufferInfo &indexInfo,
                                      const BufferInfo &vertexInfo,
                                      TrianglesVisitor* visitor)
{
    uint i = 0;
    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);
    const uint verticesStride = vertexInfo.byteStride ? vertexInfo.byteStride / sizeof(vertex) : maxVerticesDataSize;

    uint ndx[3];
    Vector3D abc[3];
    while (i < indexInfo.count) {
        for (uint u = 0; u < 6; u += 2) {
            ndx[u / 2] = indices[i + u];
            uint idx = ndx[u / 2] * verticesStride;
            for (uint j = 0; j < maxVerticesDataSize; ++j) {
                abc[u / 2][j] = vertices[idx + j];
            }
        }
        visitor->visit(ndx[2], abc[2], ndx[1], abc[1], ndx[0], abc[0]);
        i += 6;
    }
}

// vertices are already offset
template<typename Vertex>
void traverseTriangleAdjacency(Vertex *vertices,
                               const BufferInfo &vertexInfo,
                               TrianglesVisitor* visitor)
{
    uint i = 0;

    const uint maxVerticesDataSize = qMin(vertexInfo.dataSize, 3U);
    const uint verticesStride = vertexInfo.byteStride ? vertexInfo.byteStride / sizeof(Vertex) : maxVerticesDataSize;

    uint ndx[3];
    Vector3D abc[3];
    while (i < vertexInfo.count) {
        for (uint u = 0; u < 6; u += 2) {
            ndx[u / 2] = (i + u);
            uint idx = ndx[u / 2] * verticesStride;
            for (uint j = 0; j < maxVerticesDataSize; ++j) {
                abc[u / 2][j] = vertices[idx + j];
            }
        }
        visitor->visit(ndx[2], abc[2], ndx[1], abc[1], ndx[0], abc[0]);
        i += 6;
    }
}

template<typename Coordinate>
Vector4D readCoordinate(const BufferInfo &info, Coordinate *coordinates, uint index)
{
    const uint stride = info.byteStride ? info.byteStride / sizeof(Coordinate) : info.dataSize;
    Vector4D ret(0, 0, 0, 1.0f);
    coordinates += stride * index;
    for (uint e = 0; e < info.dataSize; ++e)
        ret[e] = coordinates[e];
    return ret;
}


template <QAttribute::VertexBaseType> struct EnumToType;
template <> struct EnumToType<QAttribute::Byte> { typedef const char type; };
template <> struct EnumToType<QAttribute::UnsignedByte> { typedef const uchar type; };
template <> struct EnumToType<QAttribute::Short> { typedef const short type; };
template <> struct EnumToType<QAttribute::UnsignedShort> { typedef const ushort type; };
template <> struct EnumToType<QAttribute::Int> { typedef const int type; };
template <> struct EnumToType<QAttribute::UnsignedInt> { typedef const uint type; };
template <> struct EnumToType<QAttribute::Float> { typedef const float type; };
template <> struct EnumToType<QAttribute::Double> { typedef const double type; };

template<QAttribute::VertexBaseType v>
typename EnumToType<v>::type *castToType(const QByteArray &u, uint byteOffset)
{
    return reinterpret_cast< typename EnumToType<v>::type *>(u.constData() + byteOffset);
}

Vector4D readBuffer(const BufferInfo &info, uint index)
{
    switch (info.type) {
    case QAttribute::Byte:
        return readCoordinate(info, BufferTypeInfo::castToType<QAttribute::Byte>(info.data, info.byteOffset), index);
    case QAttribute::UnsignedByte:
        return readCoordinate(info, BufferTypeInfo::castToType<QAttribute::UnsignedByte>(info.data, info.byteOffset), index);
    case QAttribute::Short:
        return readCoordinate(info, BufferTypeInfo::castToType<QAttribute::Short>(info.data, info.byteOffset), index);
    case QAttribute::UnsignedShort:
        return readCoordinate(info, BufferTypeInfo::castToType<QAttribute::UnsignedShort>(info.data, info.byteOffset), index);
    case QAttribute::Int:
        return readCoordinate(info, BufferTypeInfo::castToType<QAttribute::Int>(info.data, info.byteOffset), index);
    case QAttribute::UnsignedInt:
        return readCoordinate(info, BufferTypeInfo::castToType<QAttribute::UnsignedInt>(info.data, info.byteOffset), index);
    case QAttribute::Float:
        return readCoordinate(info, BufferTypeInfo::castToType<QAttribute::Float>(info.data, info.byteOffset), index);
    case QAttribute::Double:
        return readCoordinate(info, BufferTypeInfo::castToType<QAttribute::Double>(info.data, info.byteOffset), index);
    default:
        break;
    }
    return Vector4D();
}

template<typename Index, typename Visitor>
struct IndexedVertexExecutor
{
    template<typename Vertex>
    void operator ()(const BufferInfo &vertexInfo, Vertex * vertices)
    {
        switch (m_primitiveType) {
        case Qt3DRender::QGeometryRenderer::Triangles:
            traverseTrianglesIndexed(m_indices, vertices, m_indexBufferInfo, vertexInfo, m_visitor);
            return;
        case Qt3DRender::QGeometryRenderer::TriangleStrip:
            traverseTriangleStripIndexed(m_indices, vertices, m_indexBufferInfo, vertexInfo, m_visitor);
            return;
        case Qt3DRender::QGeometryRenderer::TriangleFan:
            traverseTriangleFanIndexed(m_indices, vertices, m_indexBufferInfo, vertexInfo, m_visitor);
            return;
        case Qt3DRender::QGeometryRenderer::TrianglesAdjacency:
            traverseTriangleAdjacencyIndexed(m_indices, vertices, m_indexBufferInfo, vertexInfo, m_visitor);
            return;
        case Qt3DRender::QGeometryRenderer::TriangleStripAdjacency: // fall through
        default:
            return;
        }
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
        case Qt3DRender::QGeometryRenderer::Triangles:
            traverseTriangles(vertices, vertexInfo, m_visitor);
            return;
        case Qt3DRender::QGeometryRenderer::TriangleStrip:
            traverseTriangleStrip(vertices, vertexInfo, m_visitor);
            return;
        case Qt3DRender::QGeometryRenderer::TriangleFan:
            traverseTriangleFan(vertices, vertexInfo, m_visitor);
            return;
        case Qt3DRender::QGeometryRenderer::TrianglesAdjacency:
            traverseTriangleAdjacency(vertices, vertexInfo, m_visitor);
            return;
        case Qt3DRender::QGeometryRenderer::TriangleStripAdjacency:     // fall through
        default:
            return;
        }
    }

    Qt3DRender::QGeometryRenderer::PrimitiveType m_primitiveType;
    Visitor* m_visitor;
};

} // anonymous


TrianglesVisitor::~TrianglesVisitor()
{

}

void TrianglesVisitor::apply(const Qt3DCore::QEntity *entity)
{
    GeometryRenderer *renderer = m_manager->geometryRendererManager()->lookupResource(entity->id());
    apply(renderer, entity->id());
}

void TrianglesVisitor::apply(const GeometryRenderer *renderer, const Qt3DCore::QNodeId id)
{
    m_nodeId = id;
    if (renderer && renderer->instanceCount() == 1 && isTriangleBased(renderer->primitiveType())) {
        Visitor::visitPrimitives<VertexExecutor<TrianglesVisitor>,
                IndexExecutor<TrianglesVisitor>, TrianglesVisitor>(m_manager, renderer, this);
    }
}

bool CoordinateReader::setGeometry(const GeometryRenderer *renderer, const QString &attributeName)
{
    if (renderer == nullptr || renderer->instanceCount() != 1
            || !isTriangleBased(renderer->primitiveType())) {
        return false;
    }

    Geometry *geom = m_manager->lookupResource<Geometry, GeometryManager>(renderer->geometryId());

    if (!geom)
        return false;

    Attribute *attribute = nullptr;

    const auto attrIds = geom->attributes();
    for (const Qt3DCore::QNodeId attrId : attrIds) {
        attribute = m_manager->lookupResource<Attribute, AttributeManager>(attrId);
        if (attribute){
            if (attribute->name() == attributeName
                    || (attributeName == QStringLiteral("default")
                        && attribute->name() == QAttribute::defaultTextureCoordinateAttributeName())) {
                break;
            }
        }
        attribute = nullptr;
    }

    if (!attribute)
        return false;

    m_attribute = attribute;
    m_buffer = m_manager->lookupResource<Buffer, BufferManager>(attribute->bufferId());

    m_bufferInfo.data = m_buffer->data();
    m_bufferInfo.type = m_attribute->vertexBaseType();
    m_bufferInfo.byteOffset = m_attribute->byteOffset();
    m_bufferInfo.byteStride = m_attribute->byteStride();
    m_bufferInfo.dataSize = m_attribute->vertexSize();
    m_bufferInfo.count = m_attribute->count();
    return true;
}

Vector4D CoordinateReader::getCoordinate(uint vertexIndex)
{
    return readBuffer(m_bufferInfo, vertexIndex);
}

} // namespace Render

} // namespace Qt3DRender

QT_END_NAMESPACE
