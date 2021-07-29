/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
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

#include "qmultisampleantialiasing.h"
#include "qrenderstate_p.h"
#include <private/qnode_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

/*!
    \class Qt3DRender::QMultiSampleAntiAliasing
    \brief Enable multisample antialiasing.
    \since 5.7
    \ingroup renderstates
    \inmodule Qt3DRender

    A Qt3DRender::QMultiSampleAntiAliasing class enables multisample antialiasing.

    It can be added to a QRenderPass by calling QRenderPass::addRenderState():

    \code
    QRenderPass *renderPass = new QRenderPass();

    QMultiSampleAntiAliasing *msaa = new QMultiSampleAntiAliasing();
    renderPass->addRenderState(msaa);
    \endcode

    Or a QRenderStateSet by calling QRenderStateSet::addRenderState():

    \code
    QRenderStateSet *renderStateSet = new QRenderStateSet();

    QMultiSampleAntiAliasing *msaa = new QMultiSampleAntiAliasing();
    renderStateSet->addRenderState(msaa);
    \endcode

    For multisampling to take effect, the render target must have been allocated
    with multisampling enabled:

    \code
    QTexture2DMultisample *colorTex = new QTexture2DMultisample;
    colorTex->setFormat(QAbstractTexture::RGBA8_UNorm);
    colorTex->setWidth(1024);
    colorTex->setHeight(1024);

    QRenderTargetOutput *color = new QRenderTargetOutput;
    color->setAttachmentPoint(QRenderTargetOutput::Color0);
    color->setTexture(colorTex);

    QTexture2DMultisample *depthStencilTex = new QTexture2DMultisample;
    depthStencilTex->setFormat(QAbstractTexture::RGBA8_UNorm);
    depthStencilTex->setWidth(1024);
    depthStencilTex->setHeight(1024);

    QRenderTargetOutput *depthStencil = new QRenderTargetOutput;
    depthStencil->setAttachmentPoint(QRenderTargetOutput::DepthStencil);
    depthStencil->setTexture(depthStencilTex);

    Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
    renderTarget->addOutput(color);
    renderTarget->addOutput(depthStencil);
    \endcode

    \include code/src_render_renderstates_qmultisampleantialiasing.qdocinc

    \note When using OpenGL as the graphics API, glEnable(GL_MULTISAMPLE) will be called if
    QMultiSampleAntiAliasing has been added to the render states.
 */

/*!
    \qmltype MultiSampleAntiAliasing
    \brief Enable multisample antialiasing.
    \since 5.7
    \ingroup renderstates
    \inqmlmodule Qt3D.Render
    \inherits RenderState
    \instantiates Qt3DRender::QMultiSampleAntiAliasing

    A MultiSampleAntiAliasing type enables multisample antialiasing.

    It can be added to a RenderPass:

    \qml
    RenderPass {
        shaderProgram: ShaderProgram {
            // ...
        }
        renderStates: [
            MultiSampleAntiAliasing {}
        ]
    }
    \endqml

    Or a RenderStateSet:

    \qml
    RenderStateSet {
        renderStates: [
            MultiSampleAntiAliasing {}
        ]
    }
    \endqml

    For multisampling to take effect, the render target must have been allocated
    with multisampling enabled:

    \qml
    RenderTarget {
        attachments: [
            RenderTargetOutput {
                attachmentPoint: RenderTargetOutput.Color0
                texture: Texture2DMultisample {
                    width: 1024
                    height: 1024
                    format: Texture.RGBA8_UNorm
                }
            },
            RenderTargetOutput {
                attachmentPoint: RenderTargetOutput.DepthStencil
                texture: Texture2DMultisample{
                    width: 1024
                    height: 1024
                    format: Texture.D24S8
                }
            }
        ]
    }
    \endqml

    Further, the shader code must use multisampling sampler types and texelFetch() instead
    of texture().

    \include code/src_render_renderstates_qmultisampleantialiasing.qdocinc

    \note When using OpenGL as the graphics API, glEnable(GL_MULTISAMPLE) will be called if
    MultiSampleAntiAliasing has been added to the render states.
 */

class QMultiSampleAntiAliasingPrivate : public QRenderStatePrivate
{
public:
    QMultiSampleAntiAliasingPrivate()
        : QRenderStatePrivate(Render::MSAAEnabledStateMask)
    {
    }

    Q_DECLARE_PUBLIC(QMultiSampleAntiAliasing)
};

/*!
    The constructor creates a new QMultiSampleAntiAliasing::QMultiSampleAntiAliasing
    instance with the specified \a parent.
 */
QMultiSampleAntiAliasing::QMultiSampleAntiAliasing(QNode *parent)
    : QRenderState(*new QMultiSampleAntiAliasingPrivate, parent)
{
}

/*! \internal */
QMultiSampleAntiAliasing::~QMultiSampleAntiAliasing()
{
}

} // namespace Qt3DRender

QT_END_NAMESPACE

