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

#include <Qt3DRender/private/qblitframebuffer_p.h>
#include <Qt3DRender/private/blitframebuffer_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {

BlitFramebuffer::BlitFramebuffer()
    : FrameGraphNode(FrameGraphNode::BlitFramebuffer)
    , m_sourceRenderTargetId(Qt3DCore::QNodeId())
    , m_destinationRenderTargetId(Qt3DCore::QNodeId())
    , m_sourceRect(QRect())
    , m_destinationRect(QRect())
    , m_sourceAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color0)
    , m_destinationAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color0)
    , m_interpolationMethod(Qt3DRender::QBlitFramebuffer::Linear)
{
}

void BlitFramebuffer::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    if (e->type() == PropertyUpdated) {
        QPropertyUpdatedChangePtr propertyChange = qSharedPointerCast<QPropertyUpdatedChange>(e);
        if (propertyChange->propertyName() == QByteArrayLiteral("sourceRenderTarget")) {
            m_sourceRenderTargetId = propertyChange->value().value<QNodeId>();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("destinationRenderTarget")) {
            m_destinationRenderTargetId = propertyChange->value().value<QNodeId>();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("sourceRect")) {
            m_sourceRect = propertyChange->value().toRect();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("destinationRect")) {
            m_destinationRect = propertyChange->value().toRect();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("sourceAttachmentPoint")) {
            m_sourceAttachmentPoint = propertyChange->value().value<Qt3DRender::QRenderTargetOutput::AttachmentPoint>();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("destinationAttachmentPoint")) {
            m_destinationAttachmentPoint = propertyChange->value().value<Qt3DRender::QRenderTargetOutput::AttachmentPoint>();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("interpolationMethod")) {
            m_interpolationMethod = propertyChange->value().value<QBlitFramebuffer::InterpolationMethod>();
        }
        markDirty(AbstractRenderer::AllDirty);
    }
    FrameGraphNode::sceneChangeEvent(e);
}

void BlitFramebuffer::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change)
{
    FrameGraphNode::initializeFromPeer(change);
    const auto typedChange = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<QBlitFramebufferData> >(change);
    const auto &data = typedChange->data;
    m_sourceRect = data.m_sourceRect;
    m_destinationRect = data.m_destinationRect;
    m_sourceRenderTargetId = data.m_sourceRenderTargetId;
    m_destinationRenderTargetId = data.m_destinationRenderTargetId;
    m_sourceAttachmentPoint = data.m_sourceAttachmentPoint;
    m_destinationAttachmentPoint = data.m_destinationAttachmentPoint;
    m_interpolationMethod = data.m_interpolationMethod;
}

Qt3DRender::QRenderTargetOutput::AttachmentPoint BlitFramebuffer::destinationAttachmentPoint() const
{
    return m_destinationAttachmentPoint;
}

QBlitFramebuffer::InterpolationMethod BlitFramebuffer::interpolationMethod() const
{
    return m_interpolationMethod;
}

Qt3DRender::QRenderTargetOutput::AttachmentPoint BlitFramebuffer::sourceAttachmentPoint() const
{
    return m_sourceAttachmentPoint;
}

QRect BlitFramebuffer::destinationRect() const
{
    return m_destinationRect;
}

QRect BlitFramebuffer::sourceRect() const
{
    return m_sourceRect;
}

Qt3DCore::QNodeId BlitFramebuffer::destinationRenderTargetId() const
{
    return m_destinationRenderTargetId;
}

Qt3DCore::QNodeId BlitFramebuffer::sourceRenderTargetId() const
{
    return m_sourceRenderTargetId;
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
