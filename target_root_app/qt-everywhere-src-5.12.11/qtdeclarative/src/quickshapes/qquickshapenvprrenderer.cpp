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

#include "qquickshapenvprrenderer_p.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <qmath.h>
#include <private/qpainterpath_p.h>
#include <private/qquickpath_p_p.h>

QT_BEGIN_NAMESPACE

void QQuickShapeNvprRenderer::beginSync(int totalCount)
{
    if (m_sp.count() != totalCount) {
        m_sp.resize(totalCount);
        m_accDirty |= DirtyList;
    }
}

void QQuickShapeNvprRenderer::setPath(int index, const QQuickPath *path)
{
    ShapePathGuiData &d(m_sp[index]);
    convertPath(path, &d);
    d.dirty |= DirtyPath;
    m_accDirty |= DirtyPath;
}

void QQuickShapeNvprRenderer::setStrokeColor(int index, const QColor &color)
{
    ShapePathGuiData &d(m_sp[index]);
    d.strokeColor = color;
    d.dirty |= DirtyStyle;
    m_accDirty |= DirtyStyle;
}

void QQuickShapeNvprRenderer::setStrokeWidth(int index, qreal w)
{
    ShapePathGuiData &d(m_sp[index]);
    d.strokeWidth = w;
    d.dirty |= DirtyStyle;
    m_accDirty |= DirtyStyle;
}

void QQuickShapeNvprRenderer::setFillColor(int index, const QColor &color)
{
    ShapePathGuiData &d(m_sp[index]);
    d.fillColor = color;
    d.dirty |= DirtyStyle;
    m_accDirty |= DirtyStyle;
}

void QQuickShapeNvprRenderer::setFillRule(int index, QQuickShapePath::FillRule fillRule)
{
    ShapePathGuiData &d(m_sp[index]);
    d.fillRule = fillRule;
    d.dirty |= DirtyFillRule;
    m_accDirty |= DirtyFillRule;
}

void QQuickShapeNvprRenderer::setJoinStyle(int index, QQuickShapePath::JoinStyle joinStyle, int miterLimit)
{
    ShapePathGuiData &d(m_sp[index]);
    d.joinStyle = joinStyle;
    d.miterLimit = miterLimit;
    d.dirty |= DirtyStyle;
    m_accDirty |= DirtyStyle;
}

void QQuickShapeNvprRenderer::setCapStyle(int index, QQuickShapePath::CapStyle capStyle)
{
    ShapePathGuiData &d(m_sp[index]);
    d.capStyle = capStyle;
    d.dirty |= DirtyStyle;
    m_accDirty |= DirtyStyle;
}

void QQuickShapeNvprRenderer::setStrokeStyle(int index, QQuickShapePath::StrokeStyle strokeStyle,
                                                qreal dashOffset, const QVector<qreal> &dashPattern)
{
    ShapePathGuiData &d(m_sp[index]);
    d.dashActive = strokeStyle == QQuickShapePath::DashLine;
    d.dashOffset = dashOffset;
    d.dashPattern = dashPattern;
    d.dirty |= DirtyDash;
    m_accDirty |= DirtyDash;
}

void QQuickShapeNvprRenderer::setFillGradient(int index, QQuickShapeGradient *gradient)
{
    ShapePathGuiData &d(m_sp[index]);
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
    d.dirty |= DirtyFillGradient;
    m_accDirty |= DirtyFillGradient;
}

void QQuickShapeNvprRenderer::endSync(bool)
{
}

void QQuickShapeNvprRenderer::setNode(QQuickShapeNvprRenderNode *node)
{
    if (m_node != node) {
        m_node = node;
        m_accDirty |= DirtyList;
    }
}

QDebug operator<<(QDebug debug, const QQuickShapeNvprRenderer::NvprPath &path)
{
    QDebugStateSaver saver(debug);
    debug.space().noquote();
    if (!path.str.isEmpty()) {
        debug << "Path with SVG string" << path.str;
        return debug;
    }
    debug << "Path with" << path.cmd.count() << "commands";
    int ci = 0;
    for (GLubyte cmd : path.cmd) {
        static struct { GLubyte cmd; const char *s; int coordCount; } nameTab[] = {
        { GL_MOVE_TO_NV, "moveTo", 2 },
        { GL_LINE_TO_NV, "lineTo", 2 },
        { GL_QUADRATIC_CURVE_TO_NV, "quadTo", 4 },
        { GL_CUBIC_CURVE_TO_NV, "cubicTo", 6 },
        { GL_LARGE_CW_ARC_TO_NV, "arcTo-large-CW", 5 },
        { GL_LARGE_CCW_ARC_TO_NV, "arcTo-large-CCW", 5 },
        { GL_SMALL_CW_ARC_TO_NV, "arcTo-small-CW", 5 },
        { GL_SMALL_CCW_ARC_TO_NV, "arcTo-small-CCW", 5 },
        { GL_CLOSE_PATH_NV, "closePath", 0 } };
        for (size_t i = 0; i < sizeof(nameTab) / sizeof(nameTab[0]); ++i) {
            if (nameTab[i].cmd == cmd) {
                QByteArray cs;
                for (int j = 0; j < nameTab[i].coordCount; ++j) {
                    cs.append(QByteArray::number(path.coord[ci++]));
                    cs.append(' ');
                }
                debug << "\n  " << nameTab[i].s << " " << cs;
                break;
            }
        }
    }
    return debug;
}

static inline void appendCoords(QVector<GLfloat> *v, QQuickCurve *c, QPointF *pos)
{
    QPointF p(c->hasRelativeX() ? pos->x() + c->relativeX() : c->x(),
              c->hasRelativeY() ? pos->y() + c->relativeY() : c->y());
    v->append(p.x());
    v->append(p.y());
    *pos = p;
}

static inline void appendControlCoords(QVector<GLfloat> *v, QQuickPathQuad *c, const QPointF &pos)
{
    QPointF p(c->hasRelativeControlX() ? pos.x() + c->relativeControlX() : c->controlX(),
              c->hasRelativeControlY() ? pos.y() + c->relativeControlY() : c->controlY());
    v->append(p.x());
    v->append(p.y());
}

static inline void appendControl1Coords(QVector<GLfloat> *v, QQuickPathCubic *c, const QPointF &pos)
{
    QPointF p(c->hasRelativeControl1X() ? pos.x() + c->relativeControl1X() : c->control1X(),
              c->hasRelativeControl1Y() ? pos.y() + c->relativeControl1Y() : c->control1Y());
    v->append(p.x());
    v->append(p.y());
}

static inline void appendControl2Coords(QVector<GLfloat> *v, QQuickPathCubic *c, const QPointF &pos)
{
    QPointF p(c->hasRelativeControl2X() ? pos.x() + c->relativeControl2X() : c->control2X(),
              c->hasRelativeControl2Y() ? pos.y() + c->relativeControl2Y() : c->control2Y());
    v->append(p.x());
    v->append(p.y());
}

void QQuickShapeNvprRenderer::convertPath(const QQuickPath *path, ShapePathGuiData *d)
{
    d->path = NvprPath();
    if (!path)
        return;

    const QList<QQuickPathElement *> &pp(QQuickPathPrivate::get(path)->_pathElements);
    if (pp.isEmpty())
        return;

    QPointF startPos(path->startX(), path->startY());
    QPointF pos(startPos);
    if (!qFuzzyIsNull(pos.x()) || !qFuzzyIsNull(pos.y())) {
        d->path.cmd.append(GL_MOVE_TO_NV);
        d->path.coord.append(pos.x());
        d->path.coord.append(pos.y());
    }

    for (QQuickPathElement *e : pp) {
        if (QQuickPathMove *o = qobject_cast<QQuickPathMove *>(e)) {
            d->path.cmd.append(GL_MOVE_TO_NV);
            appendCoords(&d->path.coord, o, &pos);
            startPos = pos;
        } else if (QQuickPathLine *o = qobject_cast<QQuickPathLine *>(e)) {
            d->path.cmd.append(GL_LINE_TO_NV);
            appendCoords(&d->path.coord, o, &pos);
        } else if (QQuickPathQuad *o = qobject_cast<QQuickPathQuad *>(e)) {
            d->path.cmd.append(GL_QUADRATIC_CURVE_TO_NV);
            appendControlCoords(&d->path.coord, o, pos);
            appendCoords(&d->path.coord, o, &pos);
        } else if (QQuickPathCubic *o = qobject_cast<QQuickPathCubic *>(e)) {
            d->path.cmd.append(GL_CUBIC_CURVE_TO_NV);
            appendControl1Coords(&d->path.coord, o, pos);
            appendControl2Coords(&d->path.coord, o, pos);
            appendCoords(&d->path.coord, o, &pos);
        } else if (QQuickPathArc *o = qobject_cast<QQuickPathArc *>(e)) {
            const bool sweepFlag = o->direction() == QQuickPathArc::Clockwise; // maps to CCW, not a typo
            GLenum cmd;
            if (o->useLargeArc())
                cmd = sweepFlag ? GL_LARGE_CCW_ARC_TO_NV : GL_LARGE_CW_ARC_TO_NV;
            else
                cmd = sweepFlag ? GL_SMALL_CCW_ARC_TO_NV : GL_SMALL_CW_ARC_TO_NV;
            d->path.cmd.append(cmd);
            d->path.coord.append(o->radiusX());
            d->path.coord.append(o->radiusY());
            d->path.coord.append(o->xAxisRotation());
            appendCoords(&d->path.coord, o, &pos);
        } else if (QQuickPathSvg *o = qobject_cast<QQuickPathSvg *>(e)) {
            // PathSvg cannot be combined with other elements. But take at
            // least startX and startY into account.
            if (d->path.str.isEmpty())
                d->path.str = QString(QStringLiteral("M %1 %2 ")).arg(pos.x()).arg(pos.y()).toUtf8();
            d->path.str.append(o->path().toUtf8());
        } else if (QQuickPathAngleArc *o = qobject_cast<QQuickPathAngleArc *>(e)) {
            QRectF rect(o->centerX() - o->radiusX(), o->centerY() - o->radiusY(), o->radiusX() * 2, o->radiusY() * 2);
            QPointF startPoint;
            QPointF endPoint;
            qt_find_ellipse_coords(rect, o->startAngle(), -o->sweepAngle(), &startPoint, &endPoint);

            // get to our starting position
            if (o->moveToStart())
                d->path.cmd.append(GL_MOVE_TO_NV);
            else
                d->path.cmd.append(GL_LINE_TO_NV); // ### should we check if startPoint == pos?
            d->path.coord.append(startPoint.x());
            d->path.coord.append(startPoint.y());

            const bool sweepFlag = o->sweepAngle() > 0; // maps to CCW, not a typo
            d->path.cmd.append(qAbs(o->sweepAngle()) > 180.0
                                 ? (sweepFlag ? GL_LARGE_CCW_ARC_TO_NV : GL_LARGE_CW_ARC_TO_NV)
                                 : (sweepFlag ? GL_SMALL_CCW_ARC_TO_NV : GL_SMALL_CW_ARC_TO_NV));
            d->path.coord.append(o->radiusX());
            d->path.coord.append(o->radiusY());
            d->path.coord.append(0); // xAxisRotation
            d->path.coord.append(endPoint.x());
            d->path.coord.append(endPoint.y());
            pos = endPoint;
        } else {
            qWarning() << "Shape/NVPR: unsupported Path element" << e;
        }
    }

    // For compatibility with QTriangulatingStroker. SVG and others would not
    // implicitly close the path when end_pos == start_pos (start_pos being the
    // last moveTo pos); that would still need an explicit 'z' or similar. We
    // don't have an explicit close command, so just fake a close when the
    // positions match.
    if (pos == startPos)
        d->path.cmd.append(GL_CLOSE_PATH_NV);
}

static inline QVector4D qsg_premultiply(const QColor &c, float globalOpacity)
{
    const float o = c.alphaF() * globalOpacity;
    return QVector4D(c.redF() * o, c.greenF() * o, c.blueF() * o, o);
}

void QQuickShapeNvprRenderer::updateNode()
{
    // Called on the render thread with gui blocked -> update the node with its
    // own copy of all relevant data.

    if (!m_accDirty)
        return;

    const int count = m_sp.count();
    const bool listChanged = m_accDirty & DirtyList;
    if (listChanged)
        m_node->m_sp.resize(count);

    for (int i = 0; i < count; ++i) {
        ShapePathGuiData &src(m_sp[i]);
        QQuickShapeNvprRenderNode::ShapePathRenderData &dst(m_node->m_sp[i]);

        int dirty = src.dirty;
        src.dirty = 0;
        if (listChanged)
            dirty |= DirtyPath | DirtyStyle | DirtyFillRule | DirtyDash | DirtyFillGradient;

        // updateNode() can be called several times with different dirty
        // states before render() gets invoked. So accumulate.
        dst.dirty |= dirty;

        if (dirty & DirtyPath)
            dst.source = src.path;

        if (dirty & DirtyStyle) {
            dst.strokeWidth = src.strokeWidth;
            dst.strokeColor = qsg_premultiply(src.strokeColor, 1.0f);
            dst.fillColor = qsg_premultiply(src.fillColor, 1.0f);
            switch (src.joinStyle) {
            case QQuickShapePath::MiterJoin:
                dst.joinStyle = GL_MITER_TRUNCATE_NV;
                break;
            case QQuickShapePath::BevelJoin:
                dst.joinStyle = GL_BEVEL_NV;
                break;
            case QQuickShapePath::RoundJoin:
                dst.joinStyle = GL_ROUND_NV;
                break;
            default:
                Q_UNREACHABLE();
            }
            dst.miterLimit = src.miterLimit;
            switch (src.capStyle) {
            case QQuickShapePath::FlatCap:
                dst.capStyle = GL_FLAT;
                break;
            case QQuickShapePath::SquareCap:
                dst.capStyle = GL_SQUARE_NV;
                break;
            case QQuickShapePath::RoundCap:
                dst.capStyle = GL_ROUND_NV;
                break;
            default:
                Q_UNREACHABLE();
            }
        }

        if (dirty & DirtyFillRule) {
            switch (src.fillRule) {
            case QQuickShapePath::OddEvenFill:
                dst.fillRule = GL_INVERT;
                break;
            case QQuickShapePath::WindingFill:
                dst.fillRule = GL_COUNT_UP_NV;
                break;
            default:
                Q_UNREACHABLE();
            }
        }

        if (dirty & DirtyDash) {
            // Multiply by strokeWidth because the Shape API follows QPen
            // meaning the input dash pattern and dash offset here are in width units.
            dst.dashOffset = src.dashOffset * src.strokeWidth;
            if (src.dashActive) {
                if (src.dashPattern.isEmpty()) {
                    // default values for DashLine as defined in qpen.cpp
                    dst.dashPattern.resize(2);
                    dst.dashPattern[0]  = 4 * src.strokeWidth; // dash
                    dst.dashPattern[1]  = 2 * src.strokeWidth; // space
                } else {
                    dst.dashPattern.resize(src.dashPattern.count());
                    for (int i = 0; i < src.dashPattern.count(); ++i)
                        dst.dashPattern[i] = GLfloat(src.dashPattern[i]) * src.strokeWidth;

                    // QPen expects a dash pattern of even length and so should we
                    if (src.dashPattern.count() % 2 != 0) {
                        qWarning("QQuickShapeNvprRenderNode: dash pattern not of even length");
                        dst.dashPattern << src.strokeWidth;
                    }
                }
            } else {
                dst.dashPattern.clear();
            }
        }

        if (dirty & DirtyFillGradient) {
            dst.fillGradientActive = src.fillGradientActive;
            if (src.fillGradientActive)
                dst.fillGradient = src.fillGradient;
        }
    }

    m_node->markDirty(QSGNode::DirtyMaterial);
    m_accDirty = 0;
}

bool QQuickShapeNvprRenderNode::nvprInited = false;
QQuickNvprFunctions QQuickShapeNvprRenderNode::nvpr;
QQuickNvprMaterialManager QQuickShapeNvprRenderNode::mtlmgr;

QQuickShapeNvprRenderNode::~QQuickShapeNvprRenderNode()
{
    releaseResources();
}

void QQuickShapeNvprRenderNode::releaseResources()
{
    for (ShapePathRenderData &d : m_sp) {
        if (d.path) {
            nvpr.deletePaths(d.path, 1);
            d.path = 0;
        }
        if (d.fallbackFbo) {
            delete d.fallbackFbo;
            d.fallbackFbo = nullptr;
        }
    }

    m_fallbackBlitter.destroy();
}

void QQuickNvprMaterialManager::create(QQuickNvprFunctions *nvpr)
{
    m_nvpr = nvpr;
}

void QQuickNvprMaterialManager::releaseResources()
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    for (MaterialDesc &mtl : m_materials) {
        if (mtl.ppl) {
            f->glDeleteProgramPipelines(1, &mtl.ppl);
            mtl = MaterialDesc();
        }
    }
}

QQuickNvprMaterialManager::MaterialDesc *QQuickNvprMaterialManager::activateMaterial(Material m)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    MaterialDesc &mtl(m_materials[m]);

    if (!mtl.ppl) {
        if (m == MatSolid) {
            static const char *fragSrc =
                    "#version 310 es\n"
                    "precision highp float;\n"
                    "out vec4 fragColor;\n"
                    "uniform vec4 color;\n"
                    "uniform float opacity;\n"
                    "void main() {\n"
                    "  fragColor = color * opacity;\n"
                    "}\n";
            if (!m_nvpr->createFragmentOnlyPipeline(fragSrc, &mtl.ppl, &mtl.prg)) {
                qWarning("NVPR: Failed to create shader pipeline for solid fill");
                return nullptr;
            }
            Q_ASSERT(mtl.ppl && mtl.prg);
            mtl.uniLoc[0] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "color");
            Q_ASSERT(mtl.uniLoc[0] >= 0);
            mtl.uniLoc[1] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "opacity");
            Q_ASSERT(mtl.uniLoc[1] >= 0);
        } else if (m == MatLinearGradient) {
            static const char *fragSrc =
                    "#version 310 es\n"
                    "precision highp float;\n"
                    "layout(location = 0) in vec2 uv;"
                    "uniform float opacity;\n"
                    "uniform sampler2D gradTab;\n"
                    "uniform vec2 gradStart;\n"
                    "uniform vec2 gradEnd;\n"
                    "out vec4 fragColor;\n"
                    "void main() {\n"
                    "  vec2 gradVec = gradEnd - gradStart;\n"
                    "  float gradTabIndex = dot(gradVec, uv - gradStart) / (gradVec.x * gradVec.x + gradVec.y * gradVec.y);\n"
                    "  fragColor = texture(gradTab, vec2(gradTabIndex, 0.5)) * opacity;\n"
                    "}\n";
            if (!m_nvpr->createFragmentOnlyPipeline(fragSrc, &mtl.ppl, &mtl.prg)) {
                qWarning("NVPR: Failed to create shader pipeline for linear gradient");
                return nullptr;
            }
            Q_ASSERT(mtl.ppl && mtl.prg);
            mtl.uniLoc[1] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "opacity");
            Q_ASSERT(mtl.uniLoc[1] >= 0);
            mtl.uniLoc[2] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "gradStart");
            Q_ASSERT(mtl.uniLoc[2] >= 0);
            mtl.uniLoc[3] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "gradEnd");
            Q_ASSERT(mtl.uniLoc[3] >= 0);
        } else if (m == MatRadialGradient) {
            static const char *fragSrc =
                    "#version 310 es\n"
                    "precision highp float;\n"
                    "uniform sampler2D gradTab;\n"
                    "uniform float opacity;\n"
                    "uniform vec2 focalToCenter;\n"
                    "uniform float centerRadius;\n"
                    "uniform float focalRadius;\n"
                    "uniform vec2 translationPoint;\n"
                    "layout(location = 0) in vec2 uv;\n"
                    "out vec4 fragColor;\n"
                    "void main() {\n"
                    "    vec2 coord = uv - translationPoint;\n"
                    "    float rd = centerRadius - focalRadius;\n"
                    "    float b = 2.0 * (rd * focalRadius + dot(coord, focalToCenter));\n"
                    "    float fmp2_m_radius2 = -focalToCenter.x * focalToCenter.x - focalToCenter.y * focalToCenter.y + rd * rd;\n"
                    "    float inverse_2_fmp2_m_radius2 = 1.0 / (2.0 * fmp2_m_radius2);\n"
                    "    float det = b * b - 4.0 * fmp2_m_radius2 * ((focalRadius * focalRadius) - dot(coord, coord));\n"
                    "    vec4 result = vec4(0.0);\n"
                    "    if (det >= 0.0) {\n"
                    "        float detSqrt = sqrt(det);\n"
                    "        float w = max((-b - detSqrt) * inverse_2_fmp2_m_radius2, (-b + detSqrt) * inverse_2_fmp2_m_radius2);\n"
                    "        if (focalRadius + w * (centerRadius - focalRadius) >= 0.0)\n"
                    "            result = texture(gradTab, vec2(w, 0.5)) * opacity;\n"
                    "    }\n"
                    "    fragColor = result;\n"
                    "}\n";
            if (!m_nvpr->createFragmentOnlyPipeline(fragSrc, &mtl.ppl, &mtl.prg)) {
                qWarning("NVPR: Failed to create shader pipeline for radial gradient");
                return nullptr;
            }
            Q_ASSERT(mtl.ppl && mtl.prg);
            mtl.uniLoc[1] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "opacity");
            Q_ASSERT(mtl.uniLoc[1] >= 0);
            mtl.uniLoc[2] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "focalToCenter");
            Q_ASSERT(mtl.uniLoc[2] >= 0);
            mtl.uniLoc[3] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "centerRadius");
            Q_ASSERT(mtl.uniLoc[3] >= 0);
            mtl.uniLoc[4] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "focalRadius");
            Q_ASSERT(mtl.uniLoc[4] >= 0);
            mtl.uniLoc[5] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "translationPoint");
            Q_ASSERT(mtl.uniLoc[5] >= 0);
        } else if (m == MatConicalGradient) {
            static const char *fragSrc =
                    "#version 310 es\n"
                    "precision highp float;\n"
                    "#define INVERSE_2PI 0.1591549430918953358\n"
                    "uniform sampler2D gradTab;\n"
                    "uniform float opacity;\n"
                    "uniform float angle;\n"
                    "uniform vec2 translationPoint;\n"
                    "layout(location = 0) in vec2 uv;\n"
                    "out vec4 fragColor;\n"
                    "void main() {\n"
                    "    vec2 coord = uv - translationPoint;\n"
                    "    float t;\n"
                    "    if (abs(coord.y) == abs(coord.x))\n"
                    "        t = (atan(-coord.y + 0.002, coord.x) + angle) * INVERSE_2PI;\n"
                    "    else\n"
                    "        t = (atan(-coord.y, coord.x) + angle) * INVERSE_2PI;\n"
                    "    fragColor = texture(gradTab, vec2(t - floor(t), 0.5)) * opacity;\n"
                    "}\n";
            if (!m_nvpr->createFragmentOnlyPipeline(fragSrc, &mtl.ppl, &mtl.prg)) {
                qWarning("NVPR: Failed to create shader pipeline for conical gradient");
                return nullptr;
            }
            Q_ASSERT(mtl.ppl && mtl.prg);
            mtl.uniLoc[1] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "opacity");
            Q_ASSERT(mtl.uniLoc[1] >= 0);
            mtl.uniLoc[2] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "angle");
            Q_ASSERT(mtl.uniLoc[2] >= 0);
            mtl.uniLoc[3] = f->glGetProgramResourceLocation(mtl.prg, GL_UNIFORM, "translationPoint");
            Q_ASSERT(mtl.uniLoc[3] >= 0);
        } else {
            Q_UNREACHABLE();
        }
    }

    f->glBindProgramPipeline(mtl.ppl);

    return &mtl;
}

void QQuickShapeNvprRenderNode::updatePath(ShapePathRenderData *d)
{
    if (d->dirty & QQuickShapeNvprRenderer::DirtyPath) {
        if (!d->path) {
            d->path = nvpr.genPaths(1);
            Q_ASSERT(d->path != 0);
        }
        if (d->source.str.isEmpty()) {
            nvpr.pathCommands(d->path, d->source.cmd.count(), d->source.cmd.constData(),
                              d->source.coord.count(), GL_FLOAT, d->source.coord.constData());
        } else {
            nvpr.pathString(d->path, GL_PATH_FORMAT_SVG_NV, d->source.str.count(), d->source.str.constData());
        }
    }

    if (d->dirty & QQuickShapeNvprRenderer::DirtyStyle) {
        nvpr.pathParameterf(d->path, GL_PATH_STROKE_WIDTH_NV, d->strokeWidth);
        nvpr.pathParameteri(d->path, GL_PATH_JOIN_STYLE_NV, d->joinStyle);
        nvpr.pathParameteri(d->path, GL_PATH_MITER_LIMIT_NV, d->miterLimit);
        nvpr.pathParameteri(d->path, GL_PATH_END_CAPS_NV, d->capStyle);
        nvpr.pathParameteri(d->path, GL_PATH_DASH_CAPS_NV, d->capStyle);
    }

    if (d->dirty & QQuickShapeNvprRenderer::DirtyDash) {
        nvpr.pathParameterf(d->path, GL_PATH_DASH_OFFSET_NV, d->dashOffset);
        // count == 0 -> no dash
        nvpr.pathDashArray(d->path, d->dashPattern.count(), d->dashPattern.constData());
    }

    if (d->dirty)
        d->fallbackValid = false;
}

void QQuickShapeNvprRenderNode::renderStroke(ShapePathRenderData *d, int strokeStencilValue, int writeMask)
{
    QQuickNvprMaterialManager::MaterialDesc *mtl = mtlmgr.activateMaterial(QQuickNvprMaterialManager::MatSolid);
    f->glProgramUniform4f(mtl->prg, mtl->uniLoc[0],
            d->strokeColor.x(), d->strokeColor.y(), d->strokeColor.z(), d->strokeColor.w());
    f->glProgramUniform1f(mtl->prg, mtl->uniLoc[1], inheritedOpacity());

    nvpr.stencilThenCoverStrokePath(d->path, strokeStencilValue, writeMask, GL_CONVEX_HULL_NV);
}

void QQuickShapeNvprRenderNode::renderFill(ShapePathRenderData *d)
{
    QQuickNvprMaterialManager::MaterialDesc *mtl = nullptr;
    if (d->fillGradientActive) {
        QQuickShapeGradient::SpreadMode spread = d->fillGradient.spread;
        if (d->fillGradientActive == QQuickAbstractPathRenderer::LinearGradient) {
            mtl = mtlmgr.activateMaterial(QQuickNvprMaterialManager::MatLinearGradient);
            // uv = vec2(coeff[0] * x + coeff[1] * y + coeff[2], coeff[3] * x + coeff[4] * y + coeff[5])
            // where x and y are in path coordinate space, which is just what
            // we need since the gradient's start and stop are in that space too.
            GLfloat coeff[6] = { 1, 0, 0,
                                 0, 1, 0 };
            nvpr.programPathFragmentInputGen(mtl->prg, 0, GL_OBJECT_LINEAR_NV, 2, coeff);
            f->glProgramUniform2f(mtl->prg, mtl->uniLoc[2], d->fillGradient.a.x(), d->fillGradient.a.y());
            f->glProgramUniform2f(mtl->prg, mtl->uniLoc[3], d->fillGradient.b.x(), d->fillGradient.b.y());
        } else if (d->fillGradientActive == QQuickAbstractPathRenderer::RadialGradient) {
            mtl = mtlmgr.activateMaterial(QQuickNvprMaterialManager::MatRadialGradient);
            // simply drive uv (location 0) with x and y, just like for the linear gradient
            GLfloat coeff[6] = { 1, 0, 0,
                                 0, 1, 0 };
            nvpr.programPathFragmentInputGen(mtl->prg, 0, GL_OBJECT_LINEAR_NV, 2, coeff);

            const QPointF centerPoint = d->fillGradient.a;
            const QPointF focalPoint = d->fillGradient.b;
            const QPointF focalToCenter = centerPoint - focalPoint;
            const GLfloat centerRadius = d->fillGradient.v0;
            const GLfloat focalRadius = d->fillGradient.v1;

            f->glProgramUniform2f(mtl->prg, mtl->uniLoc[2], focalToCenter.x(), focalToCenter.y());
            f->glProgramUniform1f(mtl->prg, mtl->uniLoc[3], centerRadius);
            f->glProgramUniform1f(mtl->prg, mtl->uniLoc[4], focalRadius);
            f->glProgramUniform2f(mtl->prg, mtl->uniLoc[5], focalPoint.x(), focalPoint.y());
        } else if (d->fillGradientActive == QQuickAbstractPathRenderer::ConicalGradient) {
            mtl = mtlmgr.activateMaterial(QQuickNvprMaterialManager::MatConicalGradient);
            // same old
            GLfloat coeff[6] = { 1, 0, 0,
                                 0, 1, 0 };
            nvpr.programPathFragmentInputGen(mtl->prg, 0, GL_OBJECT_LINEAR_NV, 2, coeff);

            const QPointF centerPoint = d->fillGradient.a;
            const GLfloat angle = -qDegreesToRadians(d->fillGradient.v0);

            f->glProgramUniform1f(mtl->prg, mtl->uniLoc[2], angle);
            f->glProgramUniform2f(mtl->prg, mtl->uniLoc[3], centerPoint.x(), centerPoint.y());

            spread = QQuickShapeGradient::RepeatSpread;
        } else {
            Q_UNREACHABLE();
        }
        const QQuickShapeGradientCache::Key cacheKey(d->fillGradient.stops, spread);
        QSGTexture *tx = QQuickShapeGradientCache::currentCache()->get(cacheKey);
        tx->bind();
    } else {
        mtl = mtlmgr.activateMaterial(QQuickNvprMaterialManager::MatSolid);
        f->glProgramUniform4f(mtl->prg, mtl->uniLoc[0],
                d->fillColor.x(), d->fillColor.y(), d->fillColor.z(), d->fillColor.w());
    }
    f->glProgramUniform1f(mtl->prg, mtl->uniLoc[1], inheritedOpacity());

    const int writeMask = 0xFF;
    nvpr.stencilThenCoverFillPath(d->path, d->fillRule, writeMask, GL_BOUNDING_BOX_NV);
}

void QQuickShapeNvprRenderNode::renderOffscreenFill(ShapePathRenderData *d)
{
    if (d->fallbackValid && d->fallbackFbo)
        return;

    GLfloat bb[4];
    nvpr.getPathParameterfv(d->path, GL_PATH_STROKE_BOUNDING_BOX_NV, bb);
    QSize sz = QSizeF(bb[2] - bb[0] + 1, bb[3] - bb[1] + 1).toSize();
    d->fallbackSize = QSize(qMax(32, sz.width()), qMax(32, sz.height()));
    d->fallbackTopLeft = QPointF(bb[0], bb[1]);

    if (d->fallbackFbo && d->fallbackFbo->size() != d->fallbackSize) {
        delete d->fallbackFbo;
        d->fallbackFbo = nullptr;
    }
    if (!d->fallbackFbo)
        d->fallbackFbo = new QOpenGLFramebufferObject(d->fallbackSize, QOpenGLFramebufferObject::CombinedDepthStencil);
    if (!d->fallbackFbo->bind())
        return;

    GLint prevViewport[4];
    f->glGetIntegerv(GL_VIEWPORT, prevViewport);

    f->glViewport(0, 0, d->fallbackSize.width(), d->fallbackSize.height());
    f->glDisable(GL_DEPTH_TEST);
    f->glClearColor(0, 0, 0, 0);
    f->glClearStencil(0);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    f->glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    f->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    QMatrix4x4 mv;
    mv.translate(-d->fallbackTopLeft.x(), -d->fallbackTopLeft.y());
    nvpr.matrixLoadf(GL_PATH_MODELVIEW_NV, mv.constData());
    QMatrix4x4 proj;
    proj.ortho(0, d->fallbackSize.width(), d->fallbackSize.height(), 0, 1, -1);
    nvpr.matrixLoadf(GL_PATH_PROJECTION_NV, proj.constData());

    renderFill(d);

    d->fallbackFbo->release();
    f->glEnable(GL_DEPTH_TEST);
    f->glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

    d->fallbackValid = true;
}

void QQuickShapeNvprRenderNode::setupStencilForCover(bool stencilClip, int sv)
{
    if (!stencilClip) {
        // Assume stencil buffer is cleared to 0 for each frame.
        // Within the frame dppass=GL_ZERO for glStencilOp ensures stencil is reset and so no need to clear.
        f->glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
        f->glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
    } else {
        f->glStencilFunc(GL_LESS, sv, 0xFF); // pass if (sv & 0xFF) < (stencil_value & 0xFF)
        f->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // dppass: replace with the original value (clip's stencil ref value)
    }
}

void QQuickShapeNvprRenderNode::render(const RenderState *state)
{
    f = QOpenGLContext::currentContext()->extraFunctions();

    if (!nvprInited) {
        if (!nvpr.create()) {
            qWarning("NVPR init failed");
            return;
        }
        mtlmgr.create(&nvpr);
        nvprInited = true;
    }

    f->glUseProgram(0);
    f->glStencilMask(~0);
    f->glEnable(GL_STENCIL_TEST);

    const bool stencilClip = state->stencilEnabled();
    // when true, the stencil buffer already has a clip path with a ref value of sv
    const int sv = state->stencilValue();
    const bool hasScissor = state->scissorEnabled();

    if (hasScissor) {
        // scissor rect is already set, just enable scissoring
        f->glEnable(GL_SCISSOR_TEST);
    }

    // Depth test against the opaque batches rendered before.
    f->glEnable(GL_DEPTH_TEST);
    f->glDepthFunc(GL_LESS);
    nvpr.pathCoverDepthFunc(GL_LESS);
    nvpr.pathStencilDepthOffset(-0.05f, -1);

    bool reloadMatrices = true;

    for (ShapePathRenderData &d : m_sp) {
        updatePath(&d);

        const bool hasFill = d.hasFill();
        const bool hasStroke = d.hasStroke();

        if (hasFill && stencilClip) {
            // Fall back to a texture when complex clipping is in use and we have
            // to fill. Reconciling glStencilFillPath's and the scenegraph's clip
            // stencil semantics has not succeeded so far...
            if (hasScissor)
                f->glDisable(GL_SCISSOR_TEST);
            renderOffscreenFill(&d);
            reloadMatrices = true;
            if (hasScissor)
                f->glEnable(GL_SCISSOR_TEST);
        }

        if (reloadMatrices) {
            reloadMatrices = false;
            nvpr.matrixLoadf(GL_PATH_MODELVIEW_NV, matrix()->constData());
            nvpr.matrixLoadf(GL_PATH_PROJECTION_NV, state->projectionMatrix()->constData());
        }

        // Fill!
        if (hasFill) {
            if (!stencilClip) {
                setupStencilForCover(false, 0);
                renderFill(&d);
            } else {
                if (!m_fallbackBlitter.isCreated())
                    m_fallbackBlitter.create();
                f->glStencilFunc(GL_EQUAL, sv, 0xFF);
                f->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                QMatrix4x4 mv = *matrix();
                mv.translate(d.fallbackTopLeft.x(), d.fallbackTopLeft.y());
                m_fallbackBlitter.texturedQuad(d.fallbackFbo->texture(), d.fallbackFbo->size(),
                                               *state->projectionMatrix(), mv,
                                               inheritedOpacity());
            }
        }

        // Stroke!
        if (hasStroke) {
            const int strokeStencilValue = 0x80;
            const int writeMask = 0x80;

            setupStencilForCover(stencilClip, sv);
            if (stencilClip) {
                // for the stencil step (eff. read mask == 0xFF & ~writeMask)
                nvpr.pathStencilFunc(GL_EQUAL, sv, 0xFF);
                // With stencilCLip == true the read mask for the stencil test before the stencil step is 0x7F.
                // This assumes the clip stencil value is <= 127.
                if (sv >= strokeStencilValue)
                    qWarning("Shape/NVPR: stencil clip ref value %d too large; expect rendering errors", sv);
            }

            renderStroke(&d, strokeStencilValue, writeMask);
        }

        if (stencilClip)
            nvpr.pathStencilFunc(GL_ALWAYS, 0, ~0);

        d.dirty = 0;
    }

    f->glBindProgramPipeline(0);
}

QSGRenderNode::StateFlags QQuickShapeNvprRenderNode::changedStates() const
{
    return BlendState | StencilState | DepthState | ScissorState;
}

QSGRenderNode::RenderingFlags QQuickShapeNvprRenderNode::flags() const
{
    return DepthAwareRendering; // avoid hitting the less optimal no-opaque-batch path in the renderer
}

bool QQuickShapeNvprRenderNode::isSupported()
{
    static const bool nvprDisabled = qEnvironmentVariableIntValue("QT_NO_NVPR") != 0;
    return !nvprDisabled && QQuickNvprFunctions::isSupported();
}

bool QQuickNvprBlitter::create()
{
    if (isCreated())
        destroy();

    m_program = new QOpenGLShaderProgram;
    if (QOpenGLContext::currentContext()->format().profile() == QSurfaceFormat::CoreProfile) {
        m_program->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/qt-project.org/shapes/shaders/blit_core.vert"));
        m_program->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/qt-project.org/shapes/shaders/blit_core.frag"));
    } else {
        m_program->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/qt-project.org/shapes/shaders/blit.vert"));
        m_program->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/qt-project.org/shapes/shaders/blit.frag"));
    }
    m_program->bindAttributeLocation("qt_Vertex", 0);
    m_program->bindAttributeLocation("qt_MultiTexCoord0", 1);
    if (!m_program->link())
        return false;

    m_matrixLoc = m_program->uniformLocation("qt_Matrix");
    m_opacityLoc = m_program->uniformLocation("qt_Opacity");

    m_buffer = new QOpenGLBuffer;
    if (!m_buffer->create())
        return false;
    m_buffer->bind();
    m_buffer->allocate(4 * sizeof(GLfloat) * 6);
    m_buffer->release();

    return true;
}

void QQuickNvprBlitter::destroy()
{
    if (m_program) {
        delete m_program;
        m_program = nullptr;
    }
    if (m_buffer) {
        delete m_buffer;
        m_buffer = nullptr;
    }
}

void QQuickNvprBlitter::texturedQuad(GLuint textureId, const QSize &size,
                                     const QMatrix4x4 &proj, const QMatrix4x4 &modelview,
                                     float opacity)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    m_program->bind();

    QMatrix4x4 m = proj * modelview;
    m_program->setUniformValue(m_matrixLoc, m);
    m_program->setUniformValue(m_opacityLoc, opacity);

    m_buffer->bind();

    if (size != m_prevSize) {
        m_prevSize = size;

        QPointF p0(size.width() - 1, size.height() - 1);
        QPointF p1(0, 0);
        QPointF p2(0, size.height() - 1);
        QPointF p3(size.width() - 1, 0);

        GLfloat vertices[6 * 4] = {
            GLfloat(p0.x()), GLfloat(p0.y()), 1, 0,
            GLfloat(p1.x()), GLfloat(p1.y()), 0, 1,
            GLfloat(p2.x()), GLfloat(p2.y()), 0, 0,

            GLfloat(p0.x()), GLfloat(p0.y()), 1, 0,
            GLfloat(p3.x()), GLfloat(p3.y()), 1, 1,
            GLfloat(p1.x()), GLfloat(p1.y()), 0, 1,
        };

        m_buffer->write(0, vertices, sizeof(vertices));
    }

    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (const void *) (2 * sizeof(GLfloat)));

    f->glBindTexture(GL_TEXTURE_2D, textureId);

    f->glDrawArrays(GL_TRIANGLES, 0, 6);

    f->glBindTexture(GL_TEXTURE_2D, 0);
    m_buffer->release();
    m_program->release();
}

QT_END_NAMESPACE
