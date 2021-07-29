/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderstatenode_p.h"
#include <Qt3DRender/qrenderstate.h>
#include <Qt3DRender/private/qrenderstatecreatedchange_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>

#include <Qt3DRender/qalphacoverage.h>
#include <Qt3DRender/qalphatest.h>
#include <Qt3DRender/private/qalphatest_p.h>
#include <Qt3DRender/qblendequation.h>
#include <Qt3DRender/private/qblendequation_p.h>
#include <Qt3DRender/qblendequationarguments.h>
#include <Qt3DRender/private/qblendequationarguments_p.h>
#include <Qt3DRender/qcolormask.h>
#include <Qt3DRender/private/qcolormask_p.h>
#include <Qt3DRender/qcullface.h>
#include <Qt3DRender/private/qcullface_p.h>
#include <Qt3DRender/qnodepthmask.h>
#include <Qt3DRender/qdepthtest.h>
#include <Qt3DRender/private/qdepthtest_p.h>
#include <Qt3DRender/qdithering.h>
#include <Qt3DRender/qfrontface.h>
#include <Qt3DRender/private/qfrontface_p.h>
#include <Qt3DRender/qpointsize.h>
#include <Qt3DRender/private/qpointsize_p.h>
#include <Qt3DRender/qpolygonoffset.h>
#include <Qt3DRender/private/qpolygonoffset_p.h>
#include <Qt3DRender/qscissortest.h>
#include <Qt3DRender/private/qscissortest_p.h>
#include <Qt3DRender/qstenciltest.h>
#include <Qt3DRender/private/qstenciltest_p.h>
#include <Qt3DRender/qstenciltestarguments.h>
#include <Qt3DRender/private/qstenciltestarguments_p.h>
#include <Qt3DRender/qclipplane.h>
#include <Qt3DRender/private/qclipplane_p.h>
#include <Qt3DRender/qmultisampleantialiasing.h>
#include <Qt3DRender/qseamlesscubemap.h>
#include <Qt3DRender/qstenciloperation.h>
#include <Qt3DRender/private/qstenciloperation_p.h>
#include <Qt3DRender/qstenciloperationarguments.h>
#include <Qt3DRender/private/qstenciloperationarguments_p.h>
#include <Qt3DRender/qstencilmask.h>
#include <Qt3DRender/private/qstencilmask_p.h>
#include <Qt3DRender/qlinewidth.h>
#include <Qt3DRender/private/qlinewidth_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
namespace Render {

namespace {

StateVariant createStateImplementation(const Qt3DRender::QRenderStateCreatedChangeBasePtr renderStateChange)
{
    switch (renderStateChange->renderStateType()) {

    case AlphaCoverageStateMask: {
        return StateVariant::createState<AlphaCoverage>();
    }

    case AlphaTestMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QAlphaTestData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<AlphaFunc>(data.alphaFunction, data.referenceValue);
    }

    case BlendStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QBlendEquationData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<BlendEquation>(data.blendFunction);
    }

    case BlendEquationArgumentsMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QBlendEquationArgumentsData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<BlendEquationArguments>(
                    data.sourceRgb, data.destinationRgb,
                    data.sourceAlpha, data.destinationAlpha,
                    renderStateChange->isNodeEnabled(),
                    data.bufferIndex);
    }

    case MSAAEnabledStateMask: {
        return StateVariant::createState<MSAAEnabled>(renderStateChange->isNodeEnabled());
    }

    case CullFaceStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QCullFaceData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<CullFace>(data.mode);
    }

    case DepthWriteStateMask: {
        return StateVariant::createState<NoDepthMask>(false);
    }

    case DepthTestStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QDepthTestData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<DepthTest>(data.depthFunction);
    }

    case FrontFaceStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QFrontFaceData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<FrontFace>(data.direction);
    }

    case ScissorStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QScissorTestData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<ScissorTest>(data.left, data.bottom,
                                                      data.width, data.height);
    }

    case StencilTestStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QStencilTestData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<StencilTest>(data.front.stencilFunction,
                                                      data.front.referenceValue,
                                                      data.front.comparisonMask,
                                                      data.back.stencilFunction,
                                                      data.back.referenceValue,
                                                      data.back.comparisonMask);
    }

    case PointSizeMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QPointSizeData>>(renderStateChange);
        const auto &data = typedChange->data;
        const bool isProgrammable = (data.sizeMode == QPointSize::Programmable);
        return StateVariant::createState<PointSize>(isProgrammable, data.value);
    }

    case PolygonOffsetStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QPolygonOffsetData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<PolygonOffset>(data.scaleFactor, data.depthSteps);
    }

    case ColorStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QColorMaskData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<ColorMask>(data.redMasked, data.greenMasked,
                                                    data.blueMasked, data.alphaMasked);
    }

    case ClipPlaneMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QClipPlaneData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<ClipPlane>(data.planeIndex,
                                                    data.normal,
                                                    data.distance);
    }

    case SeamlessCubemapMask: {
        return StateVariant::createState<SeamlessCubemap>();
    }

    case StencilOpMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QStencilOperationData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<StencilOp>(data.front.stencilTestFailureOperation,
                                                    data.front.depthTestFailureOperation,
                                                    data.front.allTestsPassOperation,
                                                    data.back.stencilTestFailureOperation,
                                                    data.back.depthTestFailureOperation,
                                                    data.back.allTestsPassOperation);
    }

    case StencilWriteStateMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QStencilMaskData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<StencilMask>(data.frontOutputMask,
                                                      data.backOutputMask);
    }

    case DitheringStateMask: {
        return StateVariant::createState<Dithering>();
    }

    case LineWidthMask: {
        const auto typedChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChange<QLineWidthData>>(renderStateChange);
        const auto &data = typedChange->data;
        return StateVariant::createState<LineWidth>(data.value, data.smooth);
    }

    default:
        Q_UNREACHABLE();
        return StateVariant();
    }
}

} // anonymous

RenderStateNode::RenderStateNode()
    : BackendNode()
{
}

RenderStateNode::~RenderStateNode()
{
    cleanup();
}

void RenderStateNode::cleanup()
{
}

void RenderStateNode::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &chang3)
{
    cleanup();
    const auto renderStateChange = qSharedPointerCast<Qt3DRender::QRenderStateCreatedChangeBase>(chang3);
    m_impl = createStateImplementation(renderStateChange);
}

void RenderStateNode::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    if (e->type() == Qt3DCore::PropertyUpdated) {
        Qt3DCore::QPropertyUpdatedChangePtr propertyChange = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(e);
        m_impl.state()->updateProperty(propertyChange->propertyName(), propertyChange->value());
        markDirty(AbstractRenderer::AllDirty);
    }
    BackendNode::sceneChangeEvent(e);
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
