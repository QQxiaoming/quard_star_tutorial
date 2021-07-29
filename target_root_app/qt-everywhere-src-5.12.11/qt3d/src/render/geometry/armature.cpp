/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
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

#include "armature_p.h"

#include <Qt3DCore/qpropertyupdatedchange.h>

#include <Qt3DCore/private/qarmature_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {
namespace Render {

Armature::Armature()
    : BackendNode(Qt3DCore::QBackendNode::ReadOnly)
{
}

void Armature::cleanup()
{
    m_skeletonId = Qt3DCore::QNodeId();
    setEnabled(false);
}

void Armature::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change)
{
    const auto typedChange = qSharedPointerCast<QNodeCreatedChange<QArmatureData>>(change);
    m_skeletonId = typedChange->data.skeletonId;
}

void Armature::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    switch (e->type()) {
    case Qt3DCore::PropertyUpdated: {
        const auto change = qSharedPointerCast<QPropertyUpdatedChange>(e);
        if (change->propertyName() == QByteArrayLiteral("skeleton"))
            m_skeletonId = change->value().value<QNodeId>();
        break;
    }

    default:
        break;
    }
    QBackendNode::sceneChangeEvent(e);
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
