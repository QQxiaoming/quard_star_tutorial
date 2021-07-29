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

#include "clock_p.h"
#include <Qt3DAnimation/qclock.h>
#include <Qt3DAnimation/private/qclock_p.h>
#include <Qt3DAnimation/private/animationutils_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qpropertyupdatedchangebase_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

Clock::Clock()
    : BackendNode(ReadOnly)
    , m_playbackRate(1.f)
{
}

void Clock::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change)
{
    const auto typedChange = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<QClockData>>(change);
    const auto &data = typedChange->data;
    m_playbackRate = data.playbackRate;
}

void Clock::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    switch (e->type()) {
    case Qt3DCore::PropertyUpdated: {
        const auto change = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(e);
        if (change->propertyName() == QByteArrayLiteral("playbackRate")) {
            m_playbackRate = change.data()->value().toDouble();
        }
        break;
    }

    default:
        break;
    }
    QBackendNode::sceneChangeEvent(e);
}

void Clock::cleanup()
{
    m_playbackRate = 1.f;
}

} // namespace Animation
} // namespace Qt3DAnimation

QT_END_NAMESPACE
