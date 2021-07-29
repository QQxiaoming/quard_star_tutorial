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

#include "qquickshapesoftwarerenderer_p.h"
#include <private/qquickpath_p_p.h>

QT_BEGIN_NAMESPACE

void QQuickShapeSoftwareRenderer::beginSync(int totalCount)
{
    if (m_sp.count() != totalCount) {
        m_sp.resize(totalCount);
        m_accDirty |= DirtyList;
    }
}

void QQuickShapeSoftwareRenderer::setPath(int index, const QQuickPath *path)
{
    ShapePathGuiData &d(m_sp[index]);
    d.path = path ? path->path() : QPainterPath();
    d.dirty |= DirtyPath;
    m_accDirty |= DirtyPath;
}

void QQuickShapeSoftwareRenderer::setStrokeColor(int index, const QColor &color)
{
    ShapePathGuiData &d(m_sp[index]);
    d.pen.setColor(color);
    d.dirty |= DirtyPen;
    m_accDirty |= DirtyPen;
}

void QQuickShapeSoftwareRenderer::setStrokeWidth(int index, qreal w)
{
    ShapePathGuiData &d(m_sp[index]);
    d.strokeWidth = w;
    if (w >= 0.0f)
        d.pen.setWidthF(w);
    d.dirty |= DirtyPen;
    m_accDirty |= DirtyPen;
}

void QQuickShapeSoftwareRenderer::setFillColor(int index, const QColor &color)
{
    ShapePathGuiData &d(m_sp[index]);
    d.fillColor = color;
    d.brush.setColor(color);
    d.dirty |= DirtyBrush;
    m_accDirty |= DirtyBrush;
}

void QQuickShapeSoftwareRenderer::setFillRule(int index, QQuickShapePath::FillRule fillRule)
{
    ShapePathGuiData &d(m_sp[index]);
    d.fillRule = Qt::FillRule(fillRule);
    d.dirty |= DirtyFillRule;
    m_accDirty |= DirtyFillRule;
}

void QQuickShapeSoftwareRenderer::setJoinStyle(int index, QQuickShapePath::JoinStyle joinStyle, int miterLimit)
{
    ShapePathGuiData &d(m_sp[index]);
    d.pen.setJoinStyle(Qt::PenJoinStyle(joinStyle));
    d.pen.setMiterLimit(miterLimit);
    d.dirty |= DirtyPen;
    m_accDirty |= DirtyPen;
}

void QQuickShapeSoftwareRenderer::setCapStyle(int index, QQuickShapePath::CapStyle capStyle)
{
    ShapePathGuiData &d(m_sp[index]);
    d.pen.setCapStyle(Qt::PenCapStyle(capStyle));
    d.dirty |= DirtyPen;
    m_accDirty |= DirtyPen;
}

void QQuickShapeSoftwareRenderer::setStrokeStyle(int index, QQuickShapePath::StrokeStyle strokeStyle,
                                                    qreal dashOffset, const QVector<qreal> &dashPattern)
{
    ShapePathGuiData &d(m_sp[index]);
    switch (strokeStyle) {
    case QQuickShapePath::SolidLine:
        d.pen.setStyle(Qt::SolidLine);
        break;
    case QQuickShapePath::DashLine:
        d.pen.setStyle(Qt::CustomDashLine);
        d.pen.setDashPattern(dashPattern);
        d.pen.setDashOffset(dashOffset);
        break;
    default:
        break;
    }
    d.dirty |= DirtyPen;
    m_accDirty |= DirtyPen;
}

static inline void setupPainterGradient(QGradient *painterGradient, const QQuickShapeGradient &g)
{
    painterGradient->setStops(g.gradientStops()); // sorted
    switch (g.spread()) {
    case QQuickShapeGradient::PadSpread:
        painterGradient->setSpread(QGradient::PadSpread);
        break;
    case QQuickShapeGradient::RepeatSpread:
        painterGradient->setSpread(QGradient::RepeatSpread);
        break;
    case QQuickShapeGradient::ReflectSpread:
        painterGradient->setSpread(QGradient::ReflectSpread);
        break;
    default:
        break;
    }
}

void QQuickShapeSoftwareRenderer::setFillGradient(int index, QQuickShapeGradient *gradient)
{
    ShapePathGuiData &d(m_sp[index]);
    if (QQuickShapeLinearGradient *g = qobject_cast<QQuickShapeLinearGradient *>(gradient)) {
        QLinearGradient painterGradient(g->x1(), g->y1(), g->x2(), g->y2());
        setupPainterGradient(&painterGradient, *g);
        d.brush = QBrush(painterGradient);
    } else if (QQuickShapeRadialGradient *g = qobject_cast<QQuickShapeRadialGradient *>(gradient)) {
        QRadialGradient painterGradient(g->centerX(), g->centerY(), g->centerRadius(),
                                        g->focalX(), g->focalY(), g->focalRadius());
        setupPainterGradient(&painterGradient, *g);
        d.brush = QBrush(painterGradient);
    } else if (QQuickShapeConicalGradient *g = qobject_cast<QQuickShapeConicalGradient *>(gradient)) {
        QConicalGradient painterGradient(g->centerX(), g->centerY(), g->angle());
        setupPainterGradient(&painterGradient, *g);
        d.brush = QBrush(painterGradient);
    } else {
        d.brush = QBrush(d.fillColor);
    }
    d.dirty |= DirtyBrush;
    m_accDirty |= DirtyBrush;
}

void QQuickShapeSoftwareRenderer::endSync(bool)
{
}

void QQuickShapeSoftwareRenderer::setNode(QQuickShapeSoftwareRenderNode *node)
{
    if (m_node != node) {
        m_node = node;
        m_accDirty |= DirtyList;
    }
}

void QQuickShapeSoftwareRenderer::updateNode()
{
    if (!m_accDirty)
        return;

    const int count = m_sp.count();
    const bool listChanged = m_accDirty & DirtyList;
    if (listChanged)
        m_node->m_sp.resize(count);

    m_node->m_boundingRect = QRectF();

    for (int i = 0; i < count; ++i) {
        ShapePathGuiData &src(m_sp[i]);
        QQuickShapeSoftwareRenderNode::ShapePathRenderData &dst(m_node->m_sp[i]);

        if (listChanged || (src.dirty & DirtyPath)) {
            dst.path = src.path;
            dst.path.setFillRule(src.fillRule);
        }

        if (listChanged || (src.dirty & DirtyFillRule))
            dst.path.setFillRule(src.fillRule);

        if (listChanged || (src.dirty & DirtyPen)) {
            dst.pen = src.pen;
            dst.strokeWidth = src.strokeWidth;
        }

        if (listChanged || (src.dirty & DirtyBrush))
            dst.brush = src.brush;

        src.dirty = 0;

        QRectF br = dst.path.boundingRect();
        const float sw = qMax(1.0f, dst.strokeWidth);
        br.adjust(-sw, -sw, sw, sw);
        m_node->m_boundingRect |= br;
    }

    m_node->markDirty(QSGNode::DirtyMaterial);
    m_accDirty = 0;
}

QQuickShapeSoftwareRenderNode::QQuickShapeSoftwareRenderNode(QQuickShape *item)
    : m_item(item)
{
}

QQuickShapeSoftwareRenderNode::~QQuickShapeSoftwareRenderNode()
{
    releaseResources();
}

void QQuickShapeSoftwareRenderNode::releaseResources()
{
}

void QQuickShapeSoftwareRenderNode::render(const RenderState *state)
{
    if (m_sp.isEmpty())
        return;

    QSGRendererInterface *rif = m_item->window()->rendererInterface();
    QPainter *p = static_cast<QPainter *>(rif->getResource(m_item->window(), QSGRendererInterface::PainterResource));
    Q_ASSERT(p);

    const QRegion *clipRegion = state->clipRegion();
    if (clipRegion && !clipRegion->isEmpty())
        p->setClipRegion(*clipRegion, Qt::ReplaceClip); // must be done before setTransform

    p->setTransform(matrix()->toTransform());
    p->setOpacity(inheritedOpacity());

    for (const ShapePathRenderData &d : qAsConst(m_sp)) {
        p->setPen(d.strokeWidth >= 0.0f && d.pen.color() != Qt::transparent ? d.pen : Qt::NoPen);
        p->setBrush(d.brush.color() != Qt::transparent ? d.brush : Qt::NoBrush);
        p->drawPath(d.path);
    }
}

QSGRenderNode::StateFlags QQuickShapeSoftwareRenderNode::changedStates() const
{
    return nullptr;
}

QSGRenderNode::RenderingFlags QQuickShapeSoftwareRenderNode::flags() const
{
    return BoundedRectRendering; // avoid fullscreen updates by saying we won't draw outside rect()
}

QRectF QQuickShapeSoftwareRenderNode::rect() const
{
    return m_boundingRect;
}

QT_END_NAMESPACE
