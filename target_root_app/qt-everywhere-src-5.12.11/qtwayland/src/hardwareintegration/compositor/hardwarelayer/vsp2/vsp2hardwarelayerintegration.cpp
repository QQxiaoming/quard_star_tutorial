/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "vsp2hardwarelayerintegration.h"

extern "C" {
#define private priv
#include <wayland-kms.h>
#undef private
}

#include <private/qwaylandquickhardwarelayer_p.h>
#include <private/qwaylandquickitem_p.h>
#include <private/qwaylandview_p.h>
#include <QWaylandQuickOutput>
#include <QQuickWindow>

#include <QtPlatformHeaders/qeglfsfunctions.h>

QT_BEGIN_NAMESPACE

Vsp2Buffer::Vsp2Buffer(wl_kms_buffer *kmsBuffer)
    : dmabufFd(kmsBuffer->fd)
    , bytesPerLine(kmsBuffer->stride)
    , drmPixelFormat(kmsBuffer->format)
    , size(kmsBuffer->width, kmsBuffer->height)
{
}

Vsp2Layer::Vsp2Layer(QWaylandQuickHardwareLayer *hwLayer, Vsp2HardwareLayerIntegration *integration)
    : m_hwLayer(hwLayer)
{
    connect(hwLayer, &QWaylandQuickHardwareLayer::stackingLevelChanged, this, [integration](){
        integration->recreateVspLayers();
    });
    connect(hwLayer->waylandItem(), &QWaylandQuickItem::surfaceChanged, this, &Vsp2Layer::handleSurfaceChanged);
    connect(hwLayer->waylandItem(), &QQuickItem::opacityChanged, this, &Vsp2Layer::updateOpacity);
    connect(hwLayer->waylandItem()->window(), &QQuickWindow::afterSynchronizing, this, &Vsp2Layer::updatePosition);
    hwLayer->disableSceneGraphPainting();
    QWaylandViewPrivate::get(hwLayer->waylandItem()->view())->independentFrameCallback = true;
    handleSurfaceChanged();
}

void Vsp2Layer::enableVspLayer()
{
    auto *kmsBuffer = nextKmsBuffer();

    if (!kmsBuffer)
        return;

    m_buffer = Vsp2Buffer(kmsBuffer);
    updatePosition();
    auto *wlItem = m_hwLayer->waylandItem();
    m_screen = wlItem->window()->screen();
    m_layerIndex = QEglFSFunctions::vsp2AddLayer(m_screen, m_buffer.dmabufFd, m_buffer.size, m_position, m_buffer.drmPixelFormat, m_buffer.bytesPerLine);
    wlItem->surface()->frameStarted();
    updateOpacity();
}

void Vsp2Layer::disableVspLayer()
{
    QEglFSFunctions::vsp2RemoveLayer(m_screen, m_layerIndex);
    m_layerIndex = -1;
    m_screen = nullptr;
}

void Vsp2Layer::handleBufferCommitted()
{
    if (!isEnabled()) {
        enableVspLayer();
        return;
    }

    auto *kmsBuffer = nextKmsBuffer();

    Vsp2Buffer newBuffer(kmsBuffer);
    if (m_buffer.dmabufFd != -1) {
        bool formatChanged = false;
        formatChanged |= newBuffer.bytesPerLine != m_buffer.bytesPerLine;
        formatChanged |= newBuffer.size != m_buffer.size;
        formatChanged |= newBuffer.drmPixelFormat != m_buffer.drmPixelFormat;
        if (formatChanged) {
            qWarning() << "The VSP2 Wayland hardware layer integration doesn't support changing"
                       << "surface formats, this will most likely fail";
        }
    }

    m_buffer = newBuffer;
    auto *wlItem = m_hwLayer->waylandItem();
    m_screen = wlItem->window()->screen();
    QEglFSFunctions::vsp2SetLayerBuffer(m_screen, m_layerIndex, m_buffer.dmabufFd);
    wlItem->surface()->frameStarted();
}

void Vsp2Layer::handleSurfaceChanged()
{
    auto newSurface = m_hwLayer->waylandItem()->surface();

    if (Q_UNLIKELY(newSurface == m_surface))
        return;

    if (this->m_surface)
        disconnect(this->m_surface, &QWaylandSurface::redraw, this, &Vsp2Layer::handleBufferCommitted);
    if (newSurface)
        connect(newSurface, &QWaylandSurface::redraw, this, &Vsp2Layer::handleBufferCommitted, Qt::DirectConnection);

    this->m_surface = newSurface;
}

void Vsp2Layer::updatePosition()
{
    QWaylandQuickItem *wlItem = m_hwLayer->waylandItem();
    QRectF localGeometry(0, 0, wlItem->width(), wlItem->height());
    auto lastMatrix = QWaylandQuickItemPrivate::get(wlItem)->lastMatrix;
    auto globalGeometry = lastMatrix.mapRect(localGeometry);

    if (m_buffer.size != globalGeometry.size().toSize()) {
        qWarning() << "wl_buffer size != WaylandQuickItem size and scaling has not been"
                   << "implemented for the vsp2 hardware layer integration";
    }

    m_position = globalGeometry.topLeft().toPoint();
    if (isEnabled())
        QEglFSFunctions::vsp2SetLayerPosition(m_screen, m_layerIndex, m_position);
}

void Vsp2Layer::updateOpacity()
{
    if (isEnabled()) {
        qreal opacity = m_hwLayer->waylandItem()->opacity();
        QEglFSFunctions::vsp2SetLayerAlpha(m_screen, m_layerIndex, opacity);
    }
}

wl_kms_buffer *Vsp2Layer::nextKmsBuffer()
{
    Q_ASSERT(m_hwLayer && m_hwLayer->waylandItem());
    QWaylandQuickItem *wlItem = m_hwLayer->waylandItem();
    auto view = wlItem->view();
    Q_ASSERT(view);

    view->advance();
    auto wlBuffer = view->currentBuffer().wl_buffer();

    if (!wlBuffer)
        return nullptr;

    struct wl_kms_buffer *kmsBuffer = wayland_kms_buffer_get(wlBuffer);

    if (!kmsBuffer)
        qWarning() << "Failed to get wl_kms_buffer for wl_buffer:" << wl_resource_get_id(wlBuffer);

    return kmsBuffer;
}

void Vsp2HardwareLayerIntegration::enableVspLayers()
{
    for (auto &layer : qAsConst(m_layers)) {
        Q_ASSERT(!layer->isEnabled());
        layer->enableVspLayer();
    }
}

void Vsp2HardwareLayerIntegration::disableVspLayers()
{
    for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it) {
        if ((*it)->isEnabled())
            (*it)->disableVspLayer();
    }
}

void Vsp2HardwareLayerIntegration::sortLayersByDepth()
{
    std::sort(m_layers.begin(), m_layers.end(), [](auto &l1, auto &l2){
        return l1->hwLayer()->stackingLevel() < l2->hwLayer()->stackingLevel();
    });
}

void Vsp2HardwareLayerIntegration::recreateVspLayers() {
    disableVspLayers();
    sortLayersByDepth();
    enableVspLayers();
}

Vsp2HardwareLayerIntegration::Vsp2HardwareLayerIntegration()
{
    if (QGuiApplication::platformName() != "eglfs") {
        qWarning() << "Vsp2 layers are currently only supported on the eglfs platform plugin"
                   << "with the eglfs_kms_vsp2 device integration.\n"
                   << "You need to set QT_QPA_PLATFORM=eglfs and QT_QPA_EGLFS_INTEGRATION=eglfs_kms_vsp2";
    }
    static Vsp2HardwareLayerIntegration *s_instance = this;
    QEglFSFunctions::vsp2AddBlendListener(QGuiApplication::primaryScreen(), [](){
        s_instance->sendFrameCallbacks();
    });
}

void Vsp2HardwareLayerIntegration::add(QWaylandQuickHardwareLayer *hwLayer)
{
    disableVspLayers();
    m_layers.append(QSharedPointer<Vsp2Layer>(new Vsp2Layer(hwLayer, this)));
    sortLayersByDepth();
    enableVspLayers();
}

void Vsp2HardwareLayerIntegration::remove(QWaylandQuickHardwareLayer *hwLayer)
{
    disableVspLayers();
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if ((*it)->hwLayer() == hwLayer) {
            m_layers.erase(it);
            break;
        }
    }
    enableVspLayers();
}

void Vsp2HardwareLayerIntegration::sendFrameCallbacks()
{
    for (auto &layer : qAsConst(m_layers)) {
        if (auto *surface = layer->hwLayer()->waylandItem()->surface())
            surface->sendFrameCallbacks();
    }
}

QT_END_NAMESPACE
