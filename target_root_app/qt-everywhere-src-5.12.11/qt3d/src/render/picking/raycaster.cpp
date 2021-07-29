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

#include "raycaster_p.h"
#include "qpickevent.h"
#include <Qt3DRender/private/abstractrenderer_p.h>
#include <Qt3DRender/qabstractraycaster.h>
#include <Qt3DRender/private/qabstractraycaster_p.h>
#include <Qt3DRender/private/raycastingjob_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {

namespace Render {

RayCaster::RayCaster()
    : BackendNode(QBackendNode::ReadWrite)
{
}

RayCaster::~RayCaster()
{
    notifyJob();
}

QAbstractRayCasterPrivate::RayCasterType RayCaster::type() const
{
    return m_type;
}

QAbstractRayCaster::RunMode RayCaster::runMode() const
{
    return m_runMode;
}

QVector3D RayCaster::origin() const
{
    return m_origin;
}

QVector3D RayCaster::direction() const
{
    return m_direction;
}

float RayCaster::length() const
{
    return m_length;
}

QPoint RayCaster::position() const
{
    return m_position;
}

Qt3DCore::QNodeIdVector RayCaster::layerIds() const
{
    return m_layerIds;
}

QAbstractRayCaster::FilterMode RayCaster::filterMode() const
{
    return m_filterMode;
}

void RayCaster::cleanup()
{
    BackendNode::setEnabled(false);
    m_type = QAbstractRayCasterPrivate::WorldSpaceRayCaster;
    m_runMode = QAbstractRayCaster::SingleShot;
    m_direction = QVector3D(0.f, 0.f, 1.f);
    m_origin = {};
    m_length = 0.f;
    m_position = {};
    m_filterMode = QAbstractRayCaster::AcceptAllMatchingLayers;
    m_layerIds.clear();
    notifyJob();
}

void RayCaster::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change)
{
    const auto typedChange = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<QAbstractRayCasterData>>(change);
    const auto &data = typedChange->data;
    m_type = data.casterType;
    m_runMode = data.runMode;
    m_origin = data.origin;
    m_direction = data.direction;
    m_length = data.length;
    m_position = data.position;
    m_filterMode = data.filterMode;
    m_layerIds = data.layerIds;
}

void RayCaster::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    switch (e->type()) {
    case PropertyValueAdded: {
        const auto change = qSharedPointerCast<QPropertyNodeAddedChange>(e);
        if (change->propertyName() == QByteArrayLiteral("layer")) {
            m_layerIds.append(change->addedNodeId());
            markDirty(AbstractRenderer::LayersDirty);
            notifyJob();
        }
        break;
    }

    case PropertyValueRemoved: {
        const auto change = qSharedPointerCast<QPropertyNodeRemovedChange>(e);
        if (change->propertyName() == QByteArrayLiteral("layer")) {
            m_layerIds.removeOne(change->removedNodeId());
            markDirty(AbstractRenderer::LayersDirty);
            notifyJob();
        }
        break;
    }

    case PropertyUpdated: {
        const Qt3DCore::QPropertyUpdatedChangePtr propertyChange = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(e);

        if (propertyChange->propertyName() == QByteArrayLiteral("origin")) {
            m_origin = propertyChange->value().value<QVector3D>();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("direction")) {
            m_direction = propertyChange->value().value<QVector3D>();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("length")) {
            m_length = propertyChange->value().toFloat();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("position")) {
            m_position = propertyChange->value().toPoint();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("runMode")) {
            m_runMode = propertyChange->value().value<QAbstractRayCaster::RunMode>();
        } else if (propertyChange->propertyName() == QByteArrayLiteral("filterMode")) {
            m_filterMode = propertyChange->value().value<QAbstractRayCaster::FilterMode>();
        }

        notifyJob();
        markDirty(AbstractRenderer::AllDirty);
        break;
    }

    default:
        break;
    }

    BackendNode::sceneChangeEvent(e);
}

void RayCaster::dispatchHits(const QAbstractRayCaster::Hits &hits)
{
    auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
    e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
    e->setPropertyName("hits");
    e->setValue(QVariant::fromValue(hits));
    notifyObservers(e);

    if (m_runMode == QAbstractRayCaster::SingleShot) {
        setEnabled(false);
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
        e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
        e->setPropertyName("enabled");
        e->setValue(false);
        notifyObservers(e);
    }
}

void RayCaster::notifyJob()
{
    if (m_renderer && m_renderer->rayCastingJob())
        qSharedPointerCast<RayCastingJob>(m_renderer->rayCastingJob())->markCastersDirty();
}

} // Render

} // Qt3DRender

QT_END_NAMESPACE
