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

#include "qwaylandquickhardwarelayer_p.h"

#include <QtWaylandCompositor/private/qwlhardwarelayerintegration_p.h>
#include <QtWaylandCompositor/private/qwlhardwarelayerintegrationfactory_p.h>

#include <QtCore/private/qobject_p.h>
#include <QMatrix4x4>

QT_BEGIN_NAMESPACE

class QWaylandQuickHardwareLayerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWaylandQuickHardwareLayer)
public:
    QtWayland::HardwareLayerIntegration *layerIntegration();
    QWaylandQuickItem *m_waylandItem = nullptr;
    int m_stackingLevel = 0;
    QMatrix4x4 m_matrixFromRenderThread;
    static QtWayland::HardwareLayerIntegration *s_hardwareLayerIntegration;
};

QtWayland::HardwareLayerIntegration *QWaylandQuickHardwareLayerPrivate::s_hardwareLayerIntegration = nullptr;

QtWayland::HardwareLayerIntegration *QWaylandQuickHardwareLayerPrivate::layerIntegration()
{
    if (!s_hardwareLayerIntegration) {
        QStringList keys = QtWayland::HardwareLayerIntegrationFactory::keys();

        QString environmentKey = QString::fromLocal8Bit(qgetenv("QT_WAYLAND_HARDWARE_LAYER_INTEGRATION").constData());
        if (!environmentKey.isEmpty()) {
            if (keys.contains(environmentKey)) {
                s_hardwareLayerIntegration = QtWayland::HardwareLayerIntegrationFactory::create(environmentKey, QStringList());
            } else {
                qWarning() << "Unknown hardware layer integration:" << environmentKey
                           << "Valid layer integrations are" << keys;
            }
        } else if (!keys.isEmpty()) {
            s_hardwareLayerIntegration = QtWayland::HardwareLayerIntegrationFactory::create(keys.first(), QStringList());
        } else {
            qWarning() << "No wayland hardware layer integrations found";
        }
    }

    return s_hardwareLayerIntegration;
}

/*!
 * \qmltype WaylandHardwareLayer
 * \inqmlmodule QtWayland.Compositor
 * \preliminary
 * \brief Makes a parent WaylandQuickItem use hardware layers for rendering.
 *
 * This item needs to be a descendant of a WaylandQuickItem or a derivative,
 * (i.e. ShellSurfaceItem or similar)
 *
 * The Surface of the parent WaylandQuickItem will be drawn in a hardware specific way instead
 * of the regular way using the QtQuick scene graph. On some platforms, the WaylandQuickItem's
 * current buffer and the scene graph can be blended in a separate step. This makes it possible for
 * clients to update continuously without triggering a full redraw of the compositor scene graph for
 * each frame.
 *
 * The preferred hardware layer integration may be overridden by setting the
 * QT_WAYLAND_HARDWARE_LAYER_INTEGRATION environment variable.
 */

QWaylandQuickHardwareLayer::QWaylandQuickHardwareLayer(QObject *parent)
    : QObject(*new QWaylandQuickHardwareLayerPrivate(), parent)
{
}

QWaylandQuickHardwareLayer::~QWaylandQuickHardwareLayer()
{
    Q_D(QWaylandQuickHardwareLayer);
    if (d->layerIntegration())
        d->layerIntegration()->remove(this);
}

/*!
 * \qmlproperty int QtWaylandCompositor::WaylandHardwareLayer::stackingLevel
 *
 * This property holds the stacking level of this hardware layer relative to other hardware layers,
 * and can be used to sort hardware layers. I.e. a layer with a higher level is rendered on top of
 * one with a lower level.
 *
 * Layers with level 0 will be drawn in an implementation defined order on top of the compositor
 * scene graph.
 *
 * Layers with a level below 0 are drawn beneath the compositor scene graph, if supported by the
 * hardware layer integration.
 */
int QWaylandQuickHardwareLayer::stackingLevel() const
{
    Q_D(const QWaylandQuickHardwareLayer);
    return d->m_stackingLevel;
}

void QWaylandQuickHardwareLayer::setStackingLevel(int level)
{
    Q_D(QWaylandQuickHardwareLayer);
    if (level == d->m_stackingLevel)
        return;

    d->m_stackingLevel = level;
    emit stackingLevelChanged();
}

QWaylandQuickItem *QWaylandQuickHardwareLayer::waylandItem() const
{
    Q_D(const QWaylandQuickHardwareLayer);
    return d->m_waylandItem;
}

void QWaylandQuickHardwareLayer::classBegin()
{
    Q_D(QWaylandQuickHardwareLayer);
    for (QObject *p = parent(); p != nullptr; p = p->parent()) {
        if (auto *waylandItem = qobject_cast<QWaylandQuickItem *>(p)) {
            d->m_waylandItem = waylandItem;
            break;
        }
    }
}

void QWaylandQuickHardwareLayer::componentComplete()
{
    Q_D(QWaylandQuickHardwareLayer);
    Q_ASSERT(d->m_waylandItem);
    if (auto integration = d->layerIntegration())
        integration->add(this);
    else
        qWarning() << "No hardware layer integration. WaylandHarwareLayer has no effect.";
}

void QWaylandQuickHardwareLayer::disableSceneGraphPainting()
{
    waylandItem()->setPaintEnabled(false);
}

QT_END_NAMESPACE
