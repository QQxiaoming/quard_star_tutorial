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

#include "qblitframebuffer.h"
#include "qblitframebuffer_p.h"

#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DRender/qframegraphnodecreatedchange.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

/*!
    \class Qt3DRender::QBlitFramebuffer
    \inmodule Qt3DRender
    \since 5.10
    \ingroup framegraph
    \brief FrameGraph node to transfer a rectangle of pixel values from one
    region of a render target to another.

    This node inserts a \c glBlitFrameBuffer or an equivalent into the command
    stream. This provides a more efficient method for copying rectangles
    between textures or surface backbuffers wrapped by QRenderTarget than
    drawing textured quads. It also supports scaling with the specified
    interpolation method.

    \note In practice the QBlitFramebuffer node will often be used in
    combination with QNoDraw since a blit should not involve issuing draw calls
    for any entities.
 */

/*!
    \qmltype BlitFramebuffer
    \inqmlmodule Qt3D.Render
    \instantiates Qt3DRender::QBlitFramebuffer
    \inherits FrameGraphNode
    \since 5.10
    \brief FrameGraph node to transfer a rectangle of pixel values from one
    region of a render target to another.

    This node inserts a \c glBlitFrameBuffer or an equivalent into the command
    stream. This provides a more efficient method for copying rectangles
    between textures or surface backbuffers wrapped by QRenderTarget than
    drawing textured quads. It also supports scaling with the specified
    interpolation method.

    \note In practice the BlitFramebuffer node will often be used in
    combination with NoDraw since a blit should not involve issuing draw calls
    for any entities.
*/

/*!
    \qmlproperty RenderTarget BlitFramebuffer::source

    Specifies the source render target. When not set, the source is assumed to
    be the default framebuffer (i.e. the backbuffer of the current surface), if
    there is one.

    \note the source and destination must not refer to the same render target.
 */

/*!
    \qmlproperty RenderTarget BlitFramebuffer::destination

    Specifies the destination render target. When not set, the destination is
    assumed to be the default framebuffer (i.e. the backbuffer of the current
    surface), if there is one.

    \note the source and destination must not refer to the same render target.
 */

/*!
    \qmlproperty Rect BlitFramebuffer::sourceRect

    Specifies the source rectangle. The coordinates are assumed to follow the
    normal Qt coordinate system, meaning Y runs from top to bottom.
 */

/*!
    \qmlproperty Rect BlitFramebuffer::destinationRect

    Specifies the destination rectangle. The coordinates are assumed to follow
    the normal Qt coordinate system, meaning Y runs from top to bottom.
 */

/*!
    \qmlproperty RenderTargetOutput.AttachmentPoint BlitFramebuffer::sourceAttachmentPoint

    Specifies the source attachment point. Defaults to
    RenderTargetOutput.AttachmentPoint.Color0.
 */

/*!
    \qmlproperty RenderTargetOutput.AttachmentPoint BlitFramebuffer::destinationAttachmentPoint

    Specifies the source attachment point. Defaults to
    RenderTargetOutput.AttachmentPoint.Color0.
 */

/*!
    \qmlproperty InterpolationMethod BlitFramebuffer::interpolationMethod

    Specifies the interpolation applied if the image is stretched. Defaults to Linear.
 */

QBlitFramebufferPrivate::QBlitFramebufferPrivate()
    : QFrameGraphNodePrivate()
    , m_source(nullptr)
    , m_destination(nullptr)
    , m_sourceRect(QRect())
    , m_destinationRect(QRect())
    , m_sourceAttachmentPoint(Qt3DRender::QRenderTargetOutput::AttachmentPoint::Color0)
    , m_destinationAttachmentPoint(Qt3DRender::QRenderTargetOutput::AttachmentPoint::Color0)
    , m_interpolationMethod(QBlitFramebuffer::Linear)
{
}

/*!
  Constructs a new QBlitFramebuffer with the given \a parent.
 */
QBlitFramebuffer::QBlitFramebuffer(QNode *parent)
    : QFrameGraphNode(*new QBlitFramebufferPrivate, parent)
{

}

/*!
  \internal
 */
QBlitFramebuffer::QBlitFramebuffer(QBlitFramebufferPrivate &dd, QNode *parent)
    : QFrameGraphNode(dd, parent)
{
}

/*!
  Destructor.
 */
QBlitFramebuffer::~QBlitFramebuffer()
{

}

/*!
  \return the source render target.
 */
QRenderTarget *QBlitFramebuffer::source() const
{
    Q_D(const QBlitFramebuffer);
    return d->m_source;
}

/*!
  \return the destination render target.
 */
QRenderTarget *QBlitFramebuffer::destination() const
{
    Q_D(const QBlitFramebuffer);
    return d->m_destination;
}

/*!
  \return the source rectangle.
 */
QRectF QBlitFramebuffer::sourceRect() const
{
    Q_D(const QBlitFramebuffer);
    return d->m_sourceRect;
}

/*!
  \return the destination rectangle.
 */
QRectF QBlitFramebuffer::destinationRect() const
{
    Q_D(const QBlitFramebuffer);
    return d->m_destinationRect;
}

/*!
  \return the source attachment point.
 */
Qt3DRender::QRenderTargetOutput::AttachmentPoint QBlitFramebuffer::sourceAttachmentPoint() const
{
    Q_D(const QBlitFramebuffer);
    return d->m_sourceAttachmentPoint;
}

/*!
  \return the destination attachment point.
 */
QRenderTargetOutput::AttachmentPoint QBlitFramebuffer::destinationAttachmentPoint() const
{
    Q_D(const QBlitFramebuffer);
    return d->m_destinationAttachmentPoint;
}

/*!
  \return the interpolation method.
 */
QBlitFramebuffer::InterpolationMethod QBlitFramebuffer::interpolationMethod() const
{
    Q_D(const QBlitFramebuffer);
    return d->m_interpolationMethod;
}

/*!
    Sets the source render target. The default value is nullptr, in which
    case the source is assumed to be be the default framebuffer (i.e. the
    backbuffer of the current surface), if there is one.

    \note the source and destination must not refer to the same render target.

    \note As with other nodes, \a source gets automatically parented to the
    QBlitFramebuffer instance when no parent has been set. The lifetime is also
    tracked, meaning the source reverts to nullptr in case the currently set
    \a source is destroyed.
  */
void QBlitFramebuffer::setSource(QRenderTarget *source)
{
    Q_D(QBlitFramebuffer);
    if (d->m_source != source) {
        if (d->m_source) {
            // Remove bookkeeping connection
            d->unregisterDestructionHelper(d->m_source);
        }

        d->m_source = source;

        if (d->m_source) {
            // Ensures proper bookkeeping. Calls us back with nullptr in case the rt gets destroyed.
            d->registerDestructionHelper(d->m_source, &QBlitFramebuffer::setSource, d->m_source);

            if (!d->m_source->parent())
                d->m_source->setParent(this);
        }

        emit sourceChanged();
    }
}

/*!
    Sets the destination render target. The default value is nullptr, in which
    case the destination is assumed to be be the default framebuffer (i.e. the
    backbuffer of the current surface), if there is one.

    \note the source and destination must not refer to the same render target.

    \note As with other nodes, \a destination gets automatically parented to the
    QBlitFramebuffer instance when no parent has been set. The lifetime is also
    tracked, meaning the destination reverts to nullptr in case the currently set
    \a destination is destroyed.
  */
void QBlitFramebuffer::setDestination(QRenderTarget *destination)
{
    Q_D(QBlitFramebuffer);
    if (d->m_destination != destination) {
        if (d->m_destination) {
            // Remove bookkeeping connection
            d->unregisterDestructionHelper(d->m_destination);
        }

        d->m_destination = destination;

        if (d->m_destination) {
            // Ensures proper bookkeeping. Calls us back with nullptr in case the rt gets destroyed.
            d->registerDestructionHelper(d->m_destination, &QBlitFramebuffer::setDestination, d->m_destination);

            if (!d->m_destination->parent())
                d->m_destination->setParent(this);
        }

        emit destinationChanged();
    }
}

/*!
    Sets the source rectangle to \a inputRect. The coordinates are assumed to
    follow the normal Qt coordinate system, meaning Y runs from top to bottom.
 */
void QBlitFramebuffer::setSourceRect(const QRectF &inputRect)
{
    Q_D(QBlitFramebuffer);
    if (d->m_sourceRect != inputRect) {
        d->m_sourceRect = inputRect.toRect();
        emit sourceRectChanged();
    }
}

/*!
    Sets the destination rectangle to \a outputRect. The coordinates are assumed
    to follow the normal Qt coordinate system, meaning Y runs from top to
    bottom.
 */
void QBlitFramebuffer::setDestinationRect(const QRectF &outputRect)
{
    Q_D(QBlitFramebuffer);
    if (d->m_destinationRect != outputRect) {
        d->m_destinationRect = outputRect.toRect();
        emit destinationRectChanged();
    }
}

/*!
    Sets the \a sourceAttachmentPoint. Defaults to
    Qt3DRender::QRenderTargetOutput::AttachmentPoint::Color0.
 */
void QBlitFramebuffer::setSourceAttachmentPoint(Qt3DRender::QRenderTargetOutput::AttachmentPoint sourceAttachmentPoint)
{
    Q_D(QBlitFramebuffer);
    if (d->m_sourceAttachmentPoint != sourceAttachmentPoint) {
        d->m_sourceAttachmentPoint = sourceAttachmentPoint;
        emit sourceAttachmentPointChanged();
    }
}

/*!
    Sets the \a destinationAttachmentPoint. Defaults to
    Qt3DRender::QRenderTargetOutput::AttachmentPoint::Color0.
 */
void QBlitFramebuffer::setDestinationAttachmentPoint(QRenderTargetOutput::AttachmentPoint destinationAttachmentPoint)
{
    Q_D(QBlitFramebuffer);
    if (d->m_destinationAttachmentPoint != destinationAttachmentPoint) {
        d->m_destinationAttachmentPoint = destinationAttachmentPoint;
        emit destinationAttachmentPointChanged();
    }
}

/*!
    Sets the \a interpolationMethod that is applied if the image is stretched.
    Defaults to Linear.
  */
void QBlitFramebuffer::setInterpolationMethod(QBlitFramebuffer::InterpolationMethod interpolationMethod)
{
    Q_D(QBlitFramebuffer);
    if (d->m_interpolationMethod != interpolationMethod) {
        d->m_interpolationMethod = interpolationMethod;
        emit interpolationMethodChanged();
    }
}

/*!
    \internal
 */
Qt3DCore::QNodeCreatedChangeBasePtr QBlitFramebuffer::createNodeCreationChange() const
{
    auto creationChange = QFrameGraphNodeCreatedChangePtr<QBlitFramebufferData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QBlitFramebuffer);
    data.m_sourceRect = d->m_sourceRect;
    data.m_destinationRect = d->m_destinationRect;
    data.m_sourceRenderTargetId = Qt3DCore::qIdForNode(d->m_source);
    data.m_destinationRenderTargetId = Qt3DCore::qIdForNode(d->m_destination);
    data.m_sourceAttachmentPoint = d->m_sourceAttachmentPoint;
    data.m_destinationAttachmentPoint = d->m_destinationAttachmentPoint;
    data.m_interpolationMethod = d->m_interpolationMethod;

    return creationChange;
}

} // namespace Qt3DRender

QT_END_NAMESPACE
