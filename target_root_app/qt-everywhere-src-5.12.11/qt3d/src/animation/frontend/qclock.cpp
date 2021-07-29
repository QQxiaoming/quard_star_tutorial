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

#include "qclock.h"
#include "qclock_p.h"
#include <Qt3DCore/qpropertyupdatedchange.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {

QClockPrivate::QClockPrivate()
    : Qt3DCore::QNodePrivate()
    , m_playbackRate(1.0f)
{
}

QClock::QClock(Qt3DCore::QNode* parent)
    : Qt3DCore::QNode(*new QClockPrivate, parent)
{
}

QClock::QClock(QClockPrivate &dd, Qt3DCore::QNode *parent)
    : Qt3DCore::QNode(dd, parent)
{
}

QClock::~QClock()
{
}
/*!
    \property Qt3DAnimation::QClock::playbackRate

    The playback speed of the animation.
*/

double QClock::playbackRate() const
{
    Q_D(const QClock);
    return d->m_playbackRate;
}

void QClock::setPlaybackRate(double playbackRate)
{
    Q_D(QClock);
    if (qFuzzyCompare(playbackRate, d->m_playbackRate))
        return;
    d->m_playbackRate = playbackRate;
    emit playbackRateChanged(playbackRate);
}

Qt3DCore::QNodeCreatedChangeBasePtr QClock::createNodeCreationChange() const
{
    auto creationChange = Qt3DCore::QNodeCreatedChangePtr<QClockData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QClock);
    data.playbackRate = d->m_playbackRate;
    return creationChange;
}

}

QT_END_NAMESPACE
