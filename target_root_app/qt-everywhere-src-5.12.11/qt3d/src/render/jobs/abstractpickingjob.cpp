/****************************************************************************
**
** Copyright (C) 2018 Klaralvdalens Datakonsult AB (KDAB).
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

#include "abstractpickingjob_p.h"
#include <Qt3DRender/qgeometryrenderer.h>
#include <Qt3DRender/private/renderer_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/objectpicker_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/geometryrenderer_p.h>
#include <Qt3DRender/private/rendersettings_p.h>
#include <Qt3DRender/private/trianglesvisitor_p.h>
#include <Qt3DRender/private/job_common_p.h>
#include <QtGui/qoffscreensurface.h>
#include <QtGui/qwindow.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

namespace Render {

AbstractPickingJob::AbstractPickingJob()
    : m_manager(nullptr)
    , m_node(nullptr)
    , m_frameGraphRoot(nullptr)
    , m_renderSettings(nullptr)
{
}

void AbstractPickingJob::setRoot(Entity *root)
{
    m_node = root;
}

void AbstractPickingJob::setFrameGraphRoot(FrameGraphNode *frameGraphRoot)
{
    m_frameGraphRoot = frameGraphRoot;
}

void AbstractPickingJob::setRenderSettings(RenderSettings *settings)
{
    m_renderSettings = settings;
}

void AbstractPickingJob::setManagers(NodeManagers *manager)
{
    m_manager = manager;
}

void AbstractPickingJob::run()
{
    Q_ASSERT(m_frameGraphRoot && m_renderSettings && m_node && m_manager);
    runHelper();
}

RayCasting::QRay3D AbstractPickingJob::intersectionRay(const QPoint &pos, const Matrix4x4 &viewMatrix,
                                                       const Matrix4x4 &projectionMatrix, const QRect &viewport)
{
    Vector3D nearPos = Vector3D(pos.x(), pos.y(), 0.0f);
    nearPos = nearPos.unproject(viewMatrix, projectionMatrix, viewport);
    Vector3D farPos = Vector3D(pos.x(), pos.y(), 1.0f);
    farPos = farPos.unproject(viewMatrix, projectionMatrix, viewport);

    return RayCasting::QRay3D(nearPos,
                              (farPos - nearPos).normalized(),
                              (farPos - nearPos).length());
}

QRect AbstractPickingJob::windowViewport(const QSize &area, const QRectF &relativeViewport) const
{
    if (area.isValid()) {
        const int areaWidth = area.width();
        const int areaHeight = area.height();
        return QRect(relativeViewport.x() * areaWidth,
                     (1.0 - relativeViewport.y() - relativeViewport.height()) * areaHeight,
                     relativeViewport.width() * areaWidth,
                     relativeViewport.height() * areaHeight);
    }
    return relativeViewport.toRect();
}

RayCasting::QRay3D AbstractPickingJob::rayForViewportAndCamera(const PickingUtils::ViewportCameraAreaDetails &vca,
                                                               QObject *eventSource,
                                                               const QPoint &pos) const
{
    static RayCasting::QRay3D invalidRay({}, {}, 0.f);

    if (!vca.area.isValid())
        return invalidRay;

    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    Render::CameraLens::viewMatrixForCamera(m_manager->renderNodesManager(),
                                            vca.cameraId,
                                            viewMatrix,
                                            projectionMatrix);
    // Returns viewport rect in GL coordinates (y inverted)
    const QRect viewport = windowViewport(vca.area, vca.viewport);
    // In GL the y is inverted compared to Qt
    const QPoint glCorrectPos = QPoint(pos.x(), vca.area.height() - pos.y());

    if (!viewport.contains(glCorrectPos))
        return invalidRay;
    if (vca.surface) {
        QSurface *surface = nullptr;
        if (eventSource) {
            QWindow *window = qobject_cast<QWindow *>(eventSource);
            if (window) {
                surface = static_cast<QSurface *>(window);
            } else {
                QOffscreenSurface *offscreen = qobject_cast<QOffscreenSurface *>(eventSource);
                if (offscreen)
                    surface = static_cast<QSurface *>(offscreen);
            }
        }
        if (surface && vca.surface != surface)
            return invalidRay;
    }

    const auto ray = intersectionRay(glCorrectPos, viewMatrix, projectionMatrix, viewport);
    return ray;
}

} // namespace Render

} // namespace Qt3DRender

QT_END_NAMESPACE
