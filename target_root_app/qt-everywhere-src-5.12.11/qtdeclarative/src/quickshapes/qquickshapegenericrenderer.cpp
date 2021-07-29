/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#include "qquickshapegenericrenderer_p.h"
#include <QtGui/private/qtriangulator_p.h>
#include <QtGui/private/qtriangulatingstroker_p.h>

#if QT_CONFIG(thread)
#include <QThreadPool>
#endif

#if QT_CONFIG(opengl)
#include <QSGVertexColorMaterial>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QtGui/private/qopenglextensions_p.h>
#endif

QT_BEGIN_NAMESPACE

static const qreal TRI_SCALE = 1;

struct ColoredVertex // must match QSGGeometry::ColoredPoint2D
{
    float x, y;
    QQuickShapeGenericRenderer::Color4ub color;
    void set(float nx, float ny, QQuickShapeGenericRenderer::Color4ub ncolor)
    {
        x = nx; y = ny; color = ncolor;
    }
};

static inline QQuickShapeGenericRenderer::Color4ub colorToColor4ub(const QColor &c)
{
    QQuickShapeGenericRenderer::Color4ub color = {
        uchar(qRound(c.redF() * c.alphaF() * 255)),
        uchar(qRound(c.greenF() * c.alphaF() * 255)),
        uchar(qRound(c.blueF() * c.alphaF() * 255)),
        uchar(qRound(c.alphaF() * 255))
    };
    return color;
}

QQuickShapeGenericStrokeFillNode::QQuickShapeGenericStrokeFillNode(QQuickWindow *window)
    : m_material(nullptr)
{
    setFlag(QSGNode::OwnsGeometry, true);
    setGeometry(new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0));
    activateMaterial(window, MatSolidColor);
#ifdef QSG_RUNTIME_DESCRIPTION
    qsgnode_set_description(this, QLatin1String("stroke-fill"));
#endif
}

void QQuickShapeGenericStrokeFillNode::activateMaterial(QQuickWindow *window, Material m)
{
    switch (m) {
    case MatSolidColor:
        // Use vertexcolor material. Items with different colors remain batchable
        // this way, at the expense of having to provide per-vertex color values.
        m_material.reset(QQuickShapeGenericMaterialFactory::createVertexColor(window));
        break;
    case MatLinearGradient:
        m_material.reset(QQuickShapeGenericMaterialFactory::createLinearGradient(window, this));
        break;
    case MatRadialGradient:
        m_material.reset(QQuickShapeGenericMaterialFactory::createRadialGradient(window, this));
        break;
    case MatConicalGradient:
        m_material.reset(QQuickShapeGenericMaterialFactory::createConicalGradient(window, this));
        break;
    default:
        qWarning("Unknown material %d", m);
        return;
    }

    if (material() != m_material.data())
        setMaterial(m_material.data());
}

static bool q_supportsElementIndexUint(QSGRendererInterface::GraphicsApi api)
{
    static bool elementIndexUint = true;
#if QT_CONFIG(opengl)
    if (api == QSGRendererInterface::OpenGL) {
        static bool elementIndexUintChecked = false;
        if (!elementIndexUintChecked) {
            elementIndexUintChecked = true;
            QOpenGLContext *context = QOpenGLContext::currentContext();
            QScopedPointer<QOpenGLContext> dummyContext;
            QScopedPointer<QOffscreenSurface> dummySurface;
            bool ok = true;
            if (!context) {
                dummyContext.reset(new QOpenGLContext);
                dummyContext->create();
                context = dummyContext.data();
                dummySurface.reset(new QOffscreenSurface);
                dummySurface->setFormat(context->format());
                dummySurface->create();
                ok = context->makeCurrent(dummySurface.data());
            }
            if (ok) {
                elementIndexUint = static_cast<QOpenGLExtensions *>(context->functions())->hasOpenGLExtension(
                            QOpenGLExtensions::ElementIndexUint);
            }
        }
    }
#else
    Q_UNUSED(api);
#endif
    return elementIndexUint;
}

QQuickShapeGenericRenderer::~QQuickShapeGenericRenderer()
{
    for (ShapePathData &d : m_sp) {
        if (d.pendingFill)
            d.pendingFill->orphaned = true;
        if (d.pendingStroke)
            d.pendingStroke->orphaned = true;
    }
}

// sync, and so triangulation too, happens on the gui thread
//    - except when async is set, in which case triangulation is moved to worker threads

void QQuickShapeGenericRenderer::beginSync(int totalCount)
{
    if (m_sp.count() != totalCount) {
        m_sp.resize(totalCount);
        m_accDirty |= DirtyList;
    }
    for (ShapePathData &d : m_sp)
        d.syncDirty = 0;
}

void QQuickShapeGenericRenderer::setPath(int index, const QQuickPath *path)
{
    ShapePathData &d(m_sp[index]);
    d.path = path ? path->path() : QPainterPath();
    d.syncDirty |= DirtyFillGeom | DirtyStrokeGeom;
}

void QQuickShapeGenericRenderer::setStrokeColor(int index, const QColor &color)
{
    ShapePathData &d(m_sp[index]);
    d.strokeColor = colorToColor4ub(color);
    d.syncDirty |= DirtyColor;
}

void QQuickShapeGenericRenderer::setStrokeWidth(int index, qreal w)
{
    ShapePathData &d(m_sp[index]);
    d.strokeWidth = w;
    if (w >= 0.0f)
        d.pen.setWidthF(w);
    d.syncDirty |= DirtyStrokeGeom;
}

void QQuickShapeGenericRenderer::setFillColor(int index, const QColor &color)
{
    ShapePathData &d(m_sp[index]);
    d.fillColor = colorToColor4ub(color);
    d.syncDirty |= DirtyColor;
}

void QQuickShapeGenericRenderer::setFillRule(int index, QQuickShapePath::FillRule fillRule)
{
    ShapePathData &d(m_sp[index]);
    d.fillRule = Qt::FillRule(fillRule);
    d.syncDirty |= DirtyFillGeom;
}

void QQuickShapeGenericRenderer::setJoinStyle(int index, QQuickShapePath::JoinStyle joinStyle, int miterLimit)
{
    ShapePathData &d(m_sp[index]);
    d.pen.setJoinStyle(Qt::PenJoinStyle(joinStyle));
    d.pen.setMiterLimit(miterLimit);
    d.syncDirty |= DirtyStrokeGeom;
}

void QQuickShapeGenericRenderer::setCapStyle(int index, QQuickShapePath::CapStyle capStyle)
{
    ShapePathData &d(m_sp[index]);
    d.pen.setCapStyle(Qt::PenCapStyle(capStyle));
    d.syncDirty |= DirtyStrokeGeom;
}

void QQuickShapeGenericRenderer::setStrokeStyle(int index, QQuickShapePath::StrokeStyle strokeStyle,
                                                   qreal dashOffset, const QVector<qreal> &dashPattern)
{
    ShapePathData &d(m_sp[index]);
    d.pen.setStyle(Qt::PenStyle(strokeStyle));
    if (strokeStyle == QQuickShapePath::DashLine) {
        d.pen.setDashPattern(dashPattern);
        d.pen.setDashOffset(dashOffset);
    }
    d.syncDirty |= DirtyStrokeGeom;
}

void QQuickShapeGenericRenderer::setFillGradient(int index, QQuickShapeGradient *gradient)
{
    ShapePathData &d(m_sp[index]);
    if (gradient) {
        d.fillGradient.stops = gradient->gradientStops(); // sorted
        d.fillGradient.spread = gradient->spread();
        if (QQuickShapeLinearGradient *g  = qobject_cast<QQuickShapeLinearGradient *>(gradient)) {
            d.fillGradientActive = LinearGradient;
            d.fillGradient.a = QPointF(g->x1(), g->y1());
            d.fillGradient.b = QPointF(g->x2(), g->y2());
        } else if (QQuickShapeRadialGradient *g = qobject_cast<QQuickShapeRadialGradient *>(gradient)) {
            d.fillGradientActive = RadialGradient;
            d.fillGradient.a = QPointF(g->centerX(), g->centerY());
            d.fillGradient.b = QPointF(g->focalX(), g->focalY());
            d.fillGradient.v0 = g->centerRadius();
            d.fillGradient.v1 = g->focalRadius();
        } else if (QQuickShapeConicalGradient *g = qobject_cast<QQuickShapeConicalGradient *>(gradient)) {
            d.fillGradientActive = ConicalGradient;
            d.fillGradient.a = QPointF(g->centerX(), g->centerY());
            d.fillGradient.v0 = g->angle();
        } else {
            Q_UNREACHABLE();
        }
    } else {
        d.fillGradientActive = NoGradient;
    }
    d.syncDirty |= DirtyFillGradient;
}

void QQuickShapeFillRunnable::run()
{
    QQuickShapeGenericRenderer::triangulateFill(path, fillColor, &fillVertices, &fillIndices, &indexType, supportsElementIndexUint);
    emit done(this);
}

void QQuickShapeStrokeRunnable::run()
{
    QQuickShapeGenericRenderer::triangulateStroke(path, pen, strokeColor, &strokeVertices, clipSize);
    emit done(this);
}

void QQuickShapeGenericRenderer::setAsyncCallback(void (*callback)(void *), void *data)
{
    m_asyncCallback = callback;
    m_asyncCallbackData = data;
}

#if QT_CONFIG(thread)
static QThreadPool *pathWorkThreadPool = nullptr;

static void deletePathWorkThreadPool()
{
    delete pathWorkThreadPool;
    pathWorkThreadPool = nullptr;
}
#endif

void QQuickShapeGenericRenderer::endSync(bool async)
{
#if !QT_CONFIG(thread)
    // Force synchronous mode for the no-thread configuration due
    // to lack of QThreadPool.
    async = false;
#endif

    bool didKickOffAsync = false;

    for (int i = 0; i < m_sp.count(); ++i) {
        ShapePathData &d(m_sp[i]);
        if (!d.syncDirty)
            continue;

        m_accDirty |= d.syncDirty;

        // Use a shadow dirty flag in order to avoid losing state in case there are
        // multiple syncs with different dirty flags before we get to updateNode()
        // on the render thread (with the gui thread blocked). For our purposes
        // here syncDirty is still required since geometry regeneration must only
        // happen when there was an actual change in this particular sync round.
        d.effectiveDirty |= d.syncDirty;

        if (d.path.isEmpty()) {
            d.fillVertices.clear();
            d.fillIndices.clear();
            d.strokeVertices.clear();
            continue;
        }

#if QT_CONFIG(thread)
        if (async && !pathWorkThreadPool) {
            qAddPostRoutine(deletePathWorkThreadPool);
            pathWorkThreadPool = new QThreadPool;
            const int idealCount = QThread::idealThreadCount();
            pathWorkThreadPool->setMaxThreadCount(idealCount > 0 ? idealCount * 2 : 4);
        }
#endif
        if ((d.syncDirty & DirtyFillGeom) && d.fillColor.a) {
            d.path.setFillRule(d.fillRule);
            if (m_api == QSGRendererInterface::Unknown)
                m_api = m_item->window()->rendererInterface()->graphicsApi();
            if (async) {
                QQuickShapeFillRunnable *r = new QQuickShapeFillRunnable;
                r->setAutoDelete(false);
                if (d.pendingFill)
                    d.pendingFill->orphaned = true;
                d.pendingFill = r;
                r->path = d.path;
                r->fillColor = d.fillColor;
                r->supportsElementIndexUint = q_supportsElementIndexUint(m_api);
                // Unlikely in practice but in theory m_sp could be
                // resized. Therefore, capture 'i' instead of 'd'.
                QObject::connect(r, &QQuickShapeFillRunnable::done, qApp, [this, i](QQuickShapeFillRunnable *r) {
                    // Bail out when orphaned (meaning either another run was
                    // started after this one, or the renderer got destroyed).
                    if (!r->orphaned && i < m_sp.count()) {
                        ShapePathData &d(m_sp[i]);
                        d.fillVertices = r->fillVertices;
                        d.fillIndices = r->fillIndices;
                        d.indexType = r->indexType;
                        d.pendingFill = nullptr;
                        d.effectiveDirty |= DirtyFillGeom;
                        maybeUpdateAsyncItem();
                    }
                    r->deleteLater();
                });
                didKickOffAsync = true;
#if QT_CONFIG(thread)
                pathWorkThreadPool->start(r);
#endif
            } else {
                triangulateFill(d.path, d.fillColor, &d.fillVertices, &d.fillIndices, &d.indexType, q_supportsElementIndexUint(m_api));
            }
        }

        if ((d.syncDirty & DirtyStrokeGeom) && d.strokeWidth >= 0.0f && d.strokeColor.a) {
            if (async) {
                QQuickShapeStrokeRunnable *r = new QQuickShapeStrokeRunnable;
                r->setAutoDelete(false);
                if (d.pendingStroke)
                    d.pendingStroke->orphaned = true;
                d.pendingStroke = r;
                r->path = d.path;
                r->pen = d.pen;
                r->strokeColor = d.strokeColor;
                r->clipSize = QSize(m_item->width(), m_item->height());
                QObject::connect(r, &QQuickShapeStrokeRunnable::done, qApp, [this, i](QQuickShapeStrokeRunnable *r) {
                    if (!r->orphaned && i < m_sp.count()) {
                        ShapePathData &d(m_sp[i]);
                        d.strokeVertices = r->strokeVertices;
                        d.pendingStroke = nullptr;
                        d.effectiveDirty |= DirtyStrokeGeom;
                        maybeUpdateAsyncItem();
                    }
                    r->deleteLater();
                });
                didKickOffAsync = true;
#if QT_CONFIG(thread)
                pathWorkThreadPool->start(r);
#endif
            } else {
                triangulateStroke(d.path, d.pen, d.strokeColor, &d.strokeVertices,
                                  QSize(m_item->width(), m_item->height()));
            }
        }
    }

    if (!didKickOffAsync && async && m_asyncCallback)
        m_asyncCallback(m_asyncCallbackData);
}

void QQuickShapeGenericRenderer::maybeUpdateAsyncItem()
{
    for (const ShapePathData &d : qAsConst(m_sp)) {
        if (d.pendingFill || d.pendingStroke)
            return;
    }
    m_accDirty |= DirtyFillGeom | DirtyStrokeGeom;
    m_item->update();
    if (m_asyncCallback)
        m_asyncCallback(m_asyncCallbackData);
}

// the stroke/fill triangulation functions may be invoked either on the gui
// thread or some worker thread and must thus be self-contained.
void QQuickShapeGenericRenderer::triangulateFill(const QPainterPath &path,
                                                    const Color4ub &fillColor,
                                                    VertexContainerType *fillVertices,
                                                    IndexContainerType *fillIndices,
                                                    QSGGeometry::Type *indexType,
                                                    bool supportsElementIndexUint)
{
    const QVectorPath &vp = qtVectorPathForPath(path);

    QTriangleSet ts = qTriangulate(vp, QTransform::fromScale(TRI_SCALE, TRI_SCALE), 1, supportsElementIndexUint);
    const int vertexCount = ts.vertices.count() / 2; // just a qreal vector with x,y hence the / 2
    fillVertices->resize(vertexCount);
    ColoredVertex *vdst = reinterpret_cast<ColoredVertex *>(fillVertices->data());
    const qreal *vsrc = ts.vertices.constData();
    for (int i = 0; i < vertexCount; ++i)
        vdst[i].set(vsrc[i * 2] / TRI_SCALE, vsrc[i * 2 + 1] / TRI_SCALE, fillColor);

    size_t indexByteSize;
    if (ts.indices.type() == QVertexIndexVector::UnsignedShort) {
        *indexType = QSGGeometry::UnsignedShortType;
        // fillIndices is still QVector<quint32>. Just resize to N/2 and pack
        // the N quint16s into it.
        fillIndices->resize(ts.indices.size() / 2);
        indexByteSize = ts.indices.size() * sizeof(quint16);
    } else {
        *indexType = QSGGeometry::UnsignedIntType;
        fillIndices->resize(ts.indices.size());
        indexByteSize = ts.indices.size() * sizeof(quint32);
    }
    memcpy(fillIndices->data(), ts.indices.data(), indexByteSize);
}

void QQuickShapeGenericRenderer::triangulateStroke(const QPainterPath &path,
                                                      const QPen &pen,
                                                      const Color4ub &strokeColor,
                                                      VertexContainerType *strokeVertices,
                                                      const QSize &clipSize)
{
    const QVectorPath &vp = qtVectorPathForPath(path);
    const QRectF clip(QPointF(0, 0), clipSize);
    const qreal inverseScale = 1.0 / TRI_SCALE;

    QTriangulatingStroker stroker;
    stroker.setInvScale(inverseScale);

    if (pen.style() == Qt::SolidLine) {
        stroker.process(vp, pen, clip, nullptr);
    } else {
        QDashedStrokeProcessor dashStroker;
        dashStroker.setInvScale(inverseScale);
        dashStroker.process(vp, pen, clip, nullptr);
        QVectorPath dashStroke(dashStroker.points(), dashStroker.elementCount(),
                               dashStroker.elementTypes(), 0);
        stroker.process(dashStroke, pen, clip, nullptr);
    }

    if (!stroker.vertexCount()) {
        strokeVertices->clear();
        return;
    }

    const int vertexCount = stroker.vertexCount() / 2; // just a float vector with x,y hence the / 2
    strokeVertices->resize(vertexCount);
    ColoredVertex *vdst = reinterpret_cast<ColoredVertex *>(strokeVertices->data());
    const float *vsrc = stroker.vertices();
    for (int i = 0; i < vertexCount; ++i)
        vdst[i].set(vsrc[i * 2], vsrc[i * 2 + 1], strokeColor);
}

void QQuickShapeGenericRenderer::setRootNode(QQuickShapeGenericNode *node)
{
    if (m_rootNode != node) {
        m_rootNode = node;
        m_accDirty |= DirtyList;
    }
}

// on the render thread with gui blocked
void QQuickShapeGenericRenderer::updateNode()
{
    if (!m_rootNode || !m_accDirty)
        return;

//                     [   m_rootNode   ]
//                     /       /        /
// #0          [  fill  ] [  stroke  ] [   next   ]
//                                    /     /      |
// #1                       [  fill  ] [  stroke  ] [   next   ]
//                                                 /      /     |
// #2                                     [  fill  ] [ stroke ] [  next  ]
//                                                                 ...
// ...

    QQuickShapeGenericNode **nodePtr = &m_rootNode;
    QQuickShapeGenericNode *prevNode = nullptr;

    for (ShapePathData &d : m_sp) {
        if (!*nodePtr) {
            Q_ASSERT(prevNode);
            *nodePtr = new QQuickShapeGenericNode;
            prevNode->m_next = *nodePtr;
            prevNode->appendChildNode(*nodePtr);
        }

        QQuickShapeGenericNode *node = *nodePtr;

        if (m_accDirty & DirtyList)
            d.effectiveDirty |= DirtyFillGeom | DirtyStrokeGeom | DirtyColor | DirtyFillGradient;

        if (!d.effectiveDirty) {
            prevNode = node;
            nodePtr = &node->m_next;
            continue;
        }

        if (d.fillColor.a == 0) {
            delete node->m_fillNode;
            node->m_fillNode = nullptr;
        } else if (!node->m_fillNode) {
            node->m_fillNode = new QQuickShapeGenericStrokeFillNode(m_item->window());
            if (node->m_strokeNode)
                node->removeChildNode(node->m_strokeNode);
            node->appendChildNode(node->m_fillNode);
            if (node->m_strokeNode)
                node->appendChildNode(node->m_strokeNode);
            d.effectiveDirty |= DirtyFillGeom;
        }

        if (d.strokeWidth < 0.0f || d.strokeColor.a == 0) {
            delete node->m_strokeNode;
            node->m_strokeNode = nullptr;
        } else if (!node->m_strokeNode) {
            node->m_strokeNode = new QQuickShapeGenericStrokeFillNode(m_item->window());
            node->appendChildNode(node->m_strokeNode);
            d.effectiveDirty |= DirtyStrokeGeom;
        }

        updateFillNode(&d, node);
        updateStrokeNode(&d, node);

        d.effectiveDirty = 0;

        prevNode = node;
        nodePtr = &node->m_next;
    }

    if (*nodePtr && prevNode) {
        prevNode->removeChildNode(*nodePtr);
        delete *nodePtr;
        *nodePtr = nullptr;
    }

    m_accDirty = 0;
}

void QQuickShapeGenericRenderer::updateShadowDataInNode(ShapePathData *d, QQuickShapeGenericStrokeFillNode *n)
{
    if (d->fillGradientActive) {
        if (d->effectiveDirty & DirtyFillGradient)
            n->m_fillGradient = d->fillGradient;
    }
}

void QQuickShapeGenericRenderer::updateFillNode(ShapePathData *d, QQuickShapeGenericNode *node)
{
    if (!node->m_fillNode)
        return;
    if (!(d->effectiveDirty & (DirtyFillGeom | DirtyColor | DirtyFillGradient)))
        return;

    // Make a copy of the data that will be accessed by the material on
    // the render thread. This must be done even when we bail out below.
    QQuickShapeGenericStrokeFillNode *n = node->m_fillNode;
    updateShadowDataInNode(d, n);

    QSGGeometry *g = n->geometry();
    if (d->fillVertices.isEmpty()) {
        if (g->vertexCount() || g->indexCount()) {
            g->allocate(0, 0);
            n->markDirty(QSGNode::DirtyGeometry);
        }
        return;
    }

    if (d->fillGradientActive) {
        QQuickShapeGenericStrokeFillNode::Material gradMat;
        switch (d->fillGradientActive) {
        case LinearGradient:
            gradMat = QQuickShapeGenericStrokeFillNode::MatLinearGradient;
            break;
        case RadialGradient:
            gradMat = QQuickShapeGenericStrokeFillNode::MatRadialGradient;
            break;
        case ConicalGradient:
            gradMat = QQuickShapeGenericStrokeFillNode::MatConicalGradient;
            break;
        default:
            Q_UNREACHABLE();
            return;
        }
        n->activateMaterial(m_item->window(), gradMat);
        if (d->effectiveDirty & DirtyFillGradient) {
            // Gradients are implemented via a texture-based material.
            n->markDirty(QSGNode::DirtyMaterial);
            // stop here if only the gradient changed; no need to touch the geometry
            if (!(d->effectiveDirty & DirtyFillGeom))
                return;
        }
    } else {
        n->activateMaterial(m_item->window(), QQuickShapeGenericStrokeFillNode::MatSolidColor);
        // fast path for updating only color values when no change in vertex positions
        if ((d->effectiveDirty & DirtyColor) && !(d->effectiveDirty & DirtyFillGeom)) {
            ColoredVertex *vdst = reinterpret_cast<ColoredVertex *>(g->vertexData());
            for (int i = 0; i < g->vertexCount(); ++i)
                vdst[i].set(vdst[i].x, vdst[i].y, d->fillColor);
            n->markDirty(QSGNode::DirtyGeometry);
            return;
        }
    }

    const int indexCount = d->indexType == QSGGeometry::UnsignedShortType
            ? d->fillIndices.count() * 2 : d->fillIndices.count();
    if (g->indexType() != d->indexType) {
        g = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(),
                            d->fillVertices.count(), indexCount, d->indexType);
        n->setGeometry(g);
    } else {
        g->allocate(d->fillVertices.count(), indexCount);
    }
    g->setDrawingMode(QSGGeometry::DrawTriangles);
    memcpy(g->vertexData(), d->fillVertices.constData(), g->vertexCount() * g->sizeOfVertex());
    memcpy(g->indexData(), d->fillIndices.constData(), g->indexCount() * g->sizeOfIndex());

    n->markDirty(QSGNode::DirtyGeometry);
}

void QQuickShapeGenericRenderer::updateStrokeNode(ShapePathData *d, QQuickShapeGenericNode *node)
{
    if (!node->m_strokeNode)
        return;
    if (!(d->effectiveDirty & (DirtyStrokeGeom | DirtyColor)))
        return;

    QQuickShapeGenericStrokeFillNode *n = node->m_strokeNode;
    QSGGeometry *g = n->geometry();
    if (d->strokeVertices.isEmpty()) {
        if (g->vertexCount() || g->indexCount()) {
            g->allocate(0, 0);
            n->markDirty(QSGNode::DirtyGeometry);
        }
        return;
    }

    n->markDirty(QSGNode::DirtyGeometry);

    // Async loading runs update once, bails out above, then updates again once
    // ready. Set the material dirty then. This is in-line with fill where the
    // first activateMaterial() achieves the same.
    if (!g->vertexCount())
        n->markDirty(QSGNode::DirtyMaterial);

    if ((d->effectiveDirty & DirtyColor) && !(d->effectiveDirty & DirtyStrokeGeom)) {
        ColoredVertex *vdst = reinterpret_cast<ColoredVertex *>(g->vertexData());
        for (int i = 0; i < g->vertexCount(); ++i)
            vdst[i].set(vdst[i].x, vdst[i].y, d->strokeColor);
        return;
    }

    g->allocate(d->strokeVertices.count(), 0);
    g->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    memcpy(g->vertexData(), d->strokeVertices.constData(), g->vertexCount() * g->sizeOfVertex());
}

QSGMaterial *QQuickShapeGenericMaterialFactory::createVertexColor(QQuickWindow *window)
{
    QSGRendererInterface::GraphicsApi api = window->rendererInterface()->graphicsApi();

#if QT_CONFIG(opengl)
    if (api == QSGRendererInterface::OpenGL)
        return new QSGVertexColorMaterial;
#endif

    qWarning("Vertex-color material: Unsupported graphics API %d", api);
    return nullptr;
}

QSGMaterial *QQuickShapeGenericMaterialFactory::createLinearGradient(QQuickWindow *window,
                                                                     QQuickShapeGenericStrokeFillNode *node)
{
    QSGRendererInterface::GraphicsApi api = window->rendererInterface()->graphicsApi();

#if QT_CONFIG(opengl)
    if (api == QSGRendererInterface::OpenGL)
        return new QQuickShapeLinearGradientMaterial(node);
#else
        Q_UNUSED(node);
#endif

    qWarning("Linear gradient material: Unsupported graphics API %d", api);
    return nullptr;
}

QSGMaterial *QQuickShapeGenericMaterialFactory::createRadialGradient(QQuickWindow *window,
                                                                     QQuickShapeGenericStrokeFillNode *node)
{
    QSGRendererInterface::GraphicsApi api = window->rendererInterface()->graphicsApi();

#if QT_CONFIG(opengl)
    if (api == QSGRendererInterface::OpenGL)
        return new QQuickShapeRadialGradientMaterial(node);
#else
        Q_UNUSED(node);
#endif

    qWarning("Radial gradient material: Unsupported graphics API %d", api);
    return nullptr;
}

QSGMaterial *QQuickShapeGenericMaterialFactory::createConicalGradient(QQuickWindow *window,
                                                                      QQuickShapeGenericStrokeFillNode *node)
{
    QSGRendererInterface::GraphicsApi api = window->rendererInterface()->graphicsApi();

#if QT_CONFIG(opengl)
    if (api == QSGRendererInterface::OpenGL)
        return new QQuickShapeConicalGradientMaterial(node);
#else
        Q_UNUSED(node);
#endif

    qWarning("Conical gradient material: Unsupported graphics API %d", api);
    return nullptr;
}

#if QT_CONFIG(opengl)

QSGMaterialType QQuickShapeLinearGradientShader::type;

QQuickShapeLinearGradientShader::QQuickShapeLinearGradientShader()
{
    setShaderSourceFile(QOpenGLShader::Vertex,
                        QStringLiteral(":/qt-project.org/shapes/shaders/lineargradient.vert"));
    setShaderSourceFile(QOpenGLShader::Fragment,
                        QStringLiteral(":/qt-project.org/shapes/shaders/lineargradient.frag"));
}

void QQuickShapeLinearGradientShader::initialize()
{
    m_opacityLoc = program()->uniformLocation("opacity");
    m_matrixLoc = program()->uniformLocation("matrix");
    m_gradStartLoc = program()->uniformLocation("gradStart");
    m_gradEndLoc = program()->uniformLocation("gradEnd");
}

void QQuickShapeLinearGradientShader::updateState(const RenderState &state, QSGMaterial *mat, QSGMaterial *)
{
    QQuickShapeLinearGradientMaterial *m = static_cast<QQuickShapeLinearGradientMaterial *>(mat);

    if (state.isOpacityDirty())
        program()->setUniformValue(m_opacityLoc, state.opacity());

    if (state.isMatrixDirty())
        program()->setUniformValue(m_matrixLoc, state.combinedMatrix());

    QQuickShapeGenericStrokeFillNode *node = m->node();
    program()->setUniformValue(m_gradStartLoc, QVector2D(node->m_fillGradient.a));
    program()->setUniformValue(m_gradEndLoc, QVector2D(node->m_fillGradient.b));

    const QQuickShapeGradientCache::Key cacheKey(node->m_fillGradient.stops, node->m_fillGradient.spread);
    QSGTexture *tx = QQuickShapeGradientCache::currentCache()->get(cacheKey);
    tx->bind();
}

char const *const *QQuickShapeLinearGradientShader::attributeNames() const
{
    static const char *const attr[] = { "vertexCoord", "vertexColor", nullptr };
    return attr;
}

int QQuickShapeLinearGradientMaterial::compare(const QSGMaterial *other) const
{
    Q_ASSERT(other && type() == other->type());
    const QQuickShapeLinearGradientMaterial *m = static_cast<const QQuickShapeLinearGradientMaterial *>(other);

    QQuickShapeGenericStrokeFillNode *a = node();
    QQuickShapeGenericStrokeFillNode *b = m->node();
    Q_ASSERT(a && b);
    if (a == b)
        return 0;

    const QQuickAbstractPathRenderer::GradientDesc *ga = &a->m_fillGradient;
    const QQuickAbstractPathRenderer::GradientDesc *gb = &b->m_fillGradient;

    if (int d = ga->spread - gb->spread)
        return d;

    if (int d = ga->a.x() - gb->a.x())
        return d;
    if (int d = ga->a.y() - gb->a.y())
        return d;
    if (int d = ga->b.x() - gb->b.x())
        return d;
    if (int d = ga->b.y() - gb->b.y())
        return d;

    if (int d = ga->stops.count() - gb->stops.count())
        return d;

    for (int i = 0; i < ga->stops.count(); ++i) {
        if (int d = ga->stops[i].first - gb->stops[i].first)
            return d;
        if (int d = ga->stops[i].second.rgba() - gb->stops[i].second.rgba())
            return d;
    }

    return 0;
}

QSGMaterialType QQuickShapeRadialGradientShader::type;

QQuickShapeRadialGradientShader::QQuickShapeRadialGradientShader()
{
    setShaderSourceFile(QOpenGLShader::Vertex,
                        QStringLiteral(":/qt-project.org/shapes/shaders/radialgradient.vert"));
    setShaderSourceFile(QOpenGLShader::Fragment,
                        QStringLiteral(":/qt-project.org/shapes/shaders/radialgradient.frag"));
}

void QQuickShapeRadialGradientShader::initialize()
{
    QOpenGLShaderProgram *prog = program();
    m_opacityLoc = prog->uniformLocation("opacity");
    m_matrixLoc = prog->uniformLocation("matrix");
    m_translationPointLoc = prog->uniformLocation("translationPoint");
    m_focalToCenterLoc = prog->uniformLocation("focalToCenter");
    m_centerRadiusLoc = prog->uniformLocation("centerRadius");
    m_focalRadiusLoc = prog->uniformLocation("focalRadius");
}

void QQuickShapeRadialGradientShader::updateState(const RenderState &state, QSGMaterial *mat, QSGMaterial *)
{
    QQuickShapeRadialGradientMaterial *m = static_cast<QQuickShapeRadialGradientMaterial *>(mat);

    if (state.isOpacityDirty())
        program()->setUniformValue(m_opacityLoc, state.opacity());

    if (state.isMatrixDirty())
        program()->setUniformValue(m_matrixLoc, state.combinedMatrix());

    QQuickShapeGenericStrokeFillNode *node = m->node();

    const QPointF centerPoint = node->m_fillGradient.a;
    const QPointF focalPoint = node->m_fillGradient.b;
    const QPointF focalToCenter = centerPoint - focalPoint;
    const GLfloat centerRadius = node->m_fillGradient.v0;
    const GLfloat focalRadius = node->m_fillGradient.v1;

    program()->setUniformValue(m_translationPointLoc, focalPoint);
    program()->setUniformValue(m_centerRadiusLoc, centerRadius);
    program()->setUniformValue(m_focalRadiusLoc, focalRadius);
    program()->setUniformValue(m_focalToCenterLoc, focalToCenter);

    const QQuickShapeGradientCache::Key cacheKey(node->m_fillGradient.stops, node->m_fillGradient.spread);
    QSGTexture *tx = QQuickShapeGradientCache::currentCache()->get(cacheKey);
    tx->bind();
}

char const *const *QQuickShapeRadialGradientShader::attributeNames() const
{
    static const char *const attr[] = { "vertexCoord", "vertexColor", nullptr };
    return attr;
}

int QQuickShapeRadialGradientMaterial::compare(const QSGMaterial *other) const
{
    Q_ASSERT(other && type() == other->type());
    const QQuickShapeRadialGradientMaterial *m = static_cast<const QQuickShapeRadialGradientMaterial *>(other);

    QQuickShapeGenericStrokeFillNode *a = node();
    QQuickShapeGenericStrokeFillNode *b = m->node();
    Q_ASSERT(a && b);
    if (a == b)
        return 0;

    const QQuickAbstractPathRenderer::GradientDesc *ga = &a->m_fillGradient;
    const QQuickAbstractPathRenderer::GradientDesc *gb = &b->m_fillGradient;

    if (int d = ga->spread - gb->spread)
        return d;

    if (int d = ga->a.x() - gb->a.x())
        return d;
    if (int d = ga->a.y() - gb->a.y())
        return d;
    if (int d = ga->b.x() - gb->b.x())
        return d;
    if (int d = ga->b.y() - gb->b.y())
        return d;

    if (int d = ga->v0 - gb->v0)
        return d;
    if (int d = ga->v1 - gb->v1)
        return d;

    if (int d = ga->stops.count() - gb->stops.count())
        return d;

    for (int i = 0; i < ga->stops.count(); ++i) {
        if (int d = ga->stops[i].first - gb->stops[i].first)
            return d;
        if (int d = ga->stops[i].second.rgba() - gb->stops[i].second.rgba())
            return d;
    }

    return 0;
}

QSGMaterialType QQuickShapeConicalGradientShader::type;

QQuickShapeConicalGradientShader::QQuickShapeConicalGradientShader()
{
    setShaderSourceFile(QOpenGLShader::Vertex,
                        QStringLiteral(":/qt-project.org/shapes/shaders/conicalgradient.vert"));
    setShaderSourceFile(QOpenGLShader::Fragment,
                        QStringLiteral(":/qt-project.org/shapes/shaders/conicalgradient.frag"));
}

void QQuickShapeConicalGradientShader::initialize()
{
    QOpenGLShaderProgram *prog = program();
    m_opacityLoc = prog->uniformLocation("opacity");
    m_matrixLoc = prog->uniformLocation("matrix");
    m_angleLoc = prog->uniformLocation("angle");
    m_translationPointLoc = prog->uniformLocation("translationPoint");
}

void QQuickShapeConicalGradientShader::updateState(const RenderState &state, QSGMaterial *mat, QSGMaterial *)
{
    QQuickShapeConicalGradientMaterial *m = static_cast<QQuickShapeConicalGradientMaterial *>(mat);

    if (state.isOpacityDirty())
        program()->setUniformValue(m_opacityLoc, state.opacity());

    if (state.isMatrixDirty())
        program()->setUniformValue(m_matrixLoc, state.combinedMatrix());

    QQuickShapeGenericStrokeFillNode *node = m->node();

    const QPointF centerPoint = node->m_fillGradient.a;
    const GLfloat angle = -qDegreesToRadians(node->m_fillGradient.v0);

    program()->setUniformValue(m_angleLoc, angle);
    program()->setUniformValue(m_translationPointLoc, centerPoint);

    const QQuickShapeGradientCache::Key cacheKey(node->m_fillGradient.stops, QQuickShapeGradient::RepeatSpread);
    QSGTexture *tx = QQuickShapeGradientCache::currentCache()->get(cacheKey);
    tx->bind();
}

char const *const *QQuickShapeConicalGradientShader::attributeNames() const
{
    static const char *const attr[] = { "vertexCoord", "vertexColor", nullptr };
    return attr;
}

int QQuickShapeConicalGradientMaterial::compare(const QSGMaterial *other) const
{
    Q_ASSERT(other && type() == other->type());
    const QQuickShapeConicalGradientMaterial *m = static_cast<const QQuickShapeConicalGradientMaterial *>(other);

    QQuickShapeGenericStrokeFillNode *a = node();
    QQuickShapeGenericStrokeFillNode *b = m->node();
    Q_ASSERT(a && b);
    if (a == b)
        return 0;

    const QQuickAbstractPathRenderer::GradientDesc *ga = &a->m_fillGradient;
    const QQuickAbstractPathRenderer::GradientDesc *gb = &b->m_fillGradient;

    if (int d = ga->a.x() - gb->a.x())
        return d;
    if (int d = ga->a.y() - gb->a.y())
        return d;

    if (int d = ga->v0 - gb->v0)
        return d;

    if (int d = ga->stops.count() - gb->stops.count())
        return d;

    for (int i = 0; i < ga->stops.count(); ++i) {
        if (int d = ga->stops[i].first - gb->stops[i].first)
            return d;
        if (int d = ga->stops[i].second.rgba() - gb->stops[i].second.rgba())
            return d;
    }

    return 0;
}

#endif // QT_CONFIG(opengl)

QT_END_NAMESPACE
