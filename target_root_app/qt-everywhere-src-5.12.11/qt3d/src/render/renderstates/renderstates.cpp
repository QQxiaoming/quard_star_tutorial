/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Copyright (C) 2016 The Qt Company Ltd and/or its subsidiary(-ies).
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

#include "renderstates_p.h"

#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DRender/qrenderstate.h>
#include <Qt3DRender/qcullface.h>
#include <Qt3DRender/qpointsize.h>

#include <Qt3DRender/private/graphicscontext_p.h>
#include <Qt3DRender/private/qstenciloperation_p.h>
#include <Qt3DRender/private/qstenciltest_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
namespace Render {

void RenderStateImpl::updateProperty(const char *name, const QVariant &value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

void BlendEquationArguments::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("sourceRgb")) std::get<0>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("destinationRgb")) std::get<1>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("sourceAlpha")) std::get<2>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("destinationAlpha")) std::get<3>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("enabled")) std::get<4>(m_values) = value.toBool();
    else if (name == QByteArrayLiteral("bufferIndex")) std::get<5>(m_values) = value.toInt();
}

void BlendEquation::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("blendFunction")) std::get<0>(m_values) = value.toInt();
}

void AlphaFunc::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("alphaFunction"))
        std::get<0>(m_values) = value.toInt();
    if (name == QByteArrayLiteral("referenceValue"))
        std::get<1>(m_values) = value.toFloat();
}

void MSAAEnabled::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("enabled"))
        std::get<0>(m_values) = value.toBool();
}

void DepthTest::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("depthFunction")) std::get<0>(m_values) = value.toInt();
}

void CullFace::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("mode")) std::get<0>(m_values) = value.toInt();
}

void FrontFace::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("direction")) std::get<0>(m_values) = value.toInt();
}

void NoDepthMask::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("mask")) std::get<0>(m_values) = value.toBool();
}

void ScissorTest::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("left")) std::get<0>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("bottom")) std::get<1>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("width")) std::get<2>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("height")) std::get<3>(m_values) = value.toInt();
}

void StencilTest::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("arguments")) {
        const QStencilTestData data = value.value<QStencilTestData>();
        std::get<0>(m_values) = data.front.stencilFunction;
        std::get<1>(m_values) = data.front.referenceValue;
        std::get<2>(m_values) = data.front.comparisonMask;
        std::get<3>(m_values) = data.back.stencilFunction;
        std::get<4>(m_values) = data.back.referenceValue;
        std::get<5>(m_values) = data.back.comparisonMask;
    }
}

void PointSize::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("sizeMode")) std::get<0>(m_values) = (value.toInt() == QPointSize::Programmable);
    else if (name == QByteArrayLiteral("value")) std::get<1>(m_values) = value.toFloat();
}

void PolygonOffset::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("scaleFactor")) std::get<0>(m_values) = value.toFloat();
    else if (name == QByteArrayLiteral("depthSteps")) std::get<1>(m_values) = value.toFloat();
}

void ColorMask::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("redMasked")) std::get<0>(m_values) = value.toBool();
    else if (name == QByteArrayLiteral("greenMasked")) std::get<1>(m_values) = value.toBool();
    else if (name == QByteArrayLiteral("blueMasked")) std::get<2>(m_values) = value.toBool();
    else if (name == QByteArrayLiteral("alphaMasked")) std::get<3>(m_values) = value.toBool();
}

void ClipPlane::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("planeIndex")) std::get<0>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("normal")) std::get<1>(m_values) = value.value<QVector3D>();
    else if (name == QByteArrayLiteral("distance")) std::get<2>(m_values) = value.toFloat();
}

void StencilOp::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("arguments")) {
        const QStencilOperationData data = value.value<QStencilOperationData>();
        std::get<0>(m_values) = data.front.stencilTestFailureOperation;
        std::get<1>(m_values) = data.front.depthTestFailureOperation;
        std::get<2>(m_values) = data.front.allTestsPassOperation;
        std::get<3>(m_values) = data.back.stencilTestFailureOperation;
        std::get<4>(m_values) = data.back.depthTestFailureOperation;
        std::get<5>(m_values) = data.back.allTestsPassOperation;
    }
}

void StencilMask::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("frontOutputMask")) std::get<0>(m_values) = value.toInt();
    else if (name == QByteArrayLiteral("backOutputMask")) std::get<1>(m_values) = value.toInt();
}

void LineWidth::updateProperty(const char *name, const QVariant &value)
{
    if (name == QByteArrayLiteral("value"))
        std::get<0>(m_values) = value.toFloat();
    else if (name == QByteArrayLiteral("smooth"))
        std::get<1>(m_values) = value.toBool();
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
