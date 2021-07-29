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

#include "qabstractclipanimator.h"
#include "qabstractclipanimator_p.h"
#include <Qt3DAnimation/qchannelmapper.h>
#include <Qt3DAnimation/qclock.h>
#include <Qt3DAnimation/private/qanimationcallbacktrigger_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {

QAbstractClipAnimatorPrivate::QAbstractClipAnimatorPrivate()
    : Qt3DCore::QComponentPrivate()
    , m_mapper(nullptr)
    , m_clock(nullptr)
    , m_running(false)
    , m_loops(1)
    , m_normalizedTime(0.0f)
{
}

bool QAbstractClipAnimatorPrivate::canPlay() const
{
    return true;
}

/*!
    \qmltype AbstractClipAnimator
    \instantiates Qt3DAnimation::QAbstractClipAnimator
    \inqmlmodule Qt3D.Animation
    \since 5.9

    \brief AbstractClipAnimator is the base class for types providing animation playback
    capabilities.

    Subclasses of AbstractClipAnimator can be aggregated by an Entity to
    provide animation capabilities. The animator components provide an
    interface for controlling the animation (e.g. start, stop). Each animator
    type requires some form of animation data such as an AbstractAnimationClip
    as well as a ChannelMapper which describes how the channels in the
    animation clip should be mapped onto the properties of the objects you wish
    to animate.

    The following subclasses are available:

    \list
    \li Qt3D.Animation.ClipAnimator
    \li Qt3D.Animation.BlendedClipAnimator
    \endlist
*/

/*!
    \class Qt3DAnimation::QAbstractClipAnimator
    \inherits Qt3DCore::QComponent

    \inmodule Qt3DAnimation
    \since 5.9

    \brief QAbstractClipAnimator is the base class for types providing animation playback
    capabilities.

    Subclasses of QAbstractClipAnimator can be aggregated by a QEntity to
    provide animation capabilities. The animator components provide an
    interface for controlling the animation (e.g. start, stop). Each animator
    type requires some form of animation data such as a QAbstractAnimationClip
    as well as a QChannelMapper which describes how the channels in the
    animation clip should be mapped onto the properties of the objects you wish
    to animate.

    The following subclasses are available:

    \list
    \li Qt3DAnimation::QClipAnimator
    \li Qt3DAnimation::QBlendedClipAnimator
    \endlist
*/

QAbstractClipAnimator::QAbstractClipAnimator(Qt3DCore::QNode *parent)
    : Qt3DCore::QComponent(*new QAbstractClipAnimatorPrivate, parent)
{
}

QAbstractClipAnimator::QAbstractClipAnimator(QAbstractClipAnimatorPrivate &dd, Qt3DCore::QNode *parent)
    : Qt3DCore::QComponent(dd, parent)
{
}

/*! \internal */
void QAbstractClipAnimator::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change)
{
    if (change->type() == Qt3DCore::CallbackTriggered) {
        QAnimationCallbackTriggerPtr callbackTrigger = qSharedPointerCast<Qt3DAnimation::QAnimationCallbackTrigger>(change);
        if (callbackTrigger->callback())
            callbackTrigger->callback()->valueChanged(callbackTrigger->value());
    } else {
        QComponent::sceneChangeEvent(change);
    }
}

QAbstractClipAnimator::~QAbstractClipAnimator()
{
}

/*!
    \property Qt3DAnimation::QAbstractClipAnimator::running

    This property holds a boolean indicating whether the animation is currently running.
*/

/*!
    Returns a boolean indicating whether the animation is currently running.
*/
bool QAbstractClipAnimator::isRunning() const
{
    Q_D(const QAbstractClipAnimator);
    return d->m_running;
}

/*!
    \property Qt3DAnimation::QAbstractClipAnimator::channelMapper

    This property holds the ChannelMapper that controls how the channels in
    the animation clip map onto the properties of the target objects.
*/

QChannelMapper *QAbstractClipAnimator::channelMapper() const
{
    Q_D(const QAbstractClipAnimator);
    return d->m_mapper;
}

/*!
    \qmlproperty int Qt3DAnimation::AbstractClipAnimator::loops

    This property holds the number of times the animation should play.

    By default, loops is 1: the animation will play through once and then stop.

    If set to QAbstractClipAnimator::Infinite, the animation will continuously
    repeat until it is explicitly stopped.
*/
/*!
    \enum QAbstractClipAnimator::Loops

    Holds the number of times the animation should play.

    \value Infinite
         This will repeat the loop continuously until it is explicitly
         stopped.

*/
/*!
    \property Qt3DAnimation::QAbstractClipAnimator::loops

    Holds the number of times the animation should play.

    The value is 1 by default: the animation will be played once and then stop.

    If set to QAbstractClipAnimator::Infinite, the animation will continuously
    repeat until it is explicitly stopped.
*/

/*!
    Returns the number of times the animation should play.

    The value is 1 by default: the animation will play through once and then stop.

    If set to QAbstractClipAnimator::Infinite, the animation will continuously
    repeat until it is explicitly stopped.
*/
int QAbstractClipAnimator::loopCount() const
{
    Q_D(const QAbstractClipAnimator);
    return d->m_loops;
}
/*!
    \property Qt3DAnimation::QAbstractClipAnimator::clock

    The clock controls the speed with which an animation is played.
*/
QClock *QAbstractClipAnimator::clock() const
{
    Q_D(const QAbstractClipAnimator);
    return d->m_clock;
}

/*!
    \property Qt3DAnimation::QAbstractClipAnimator::normalizedTime

    This property holds the clips normalized time.
*/
float QAbstractClipAnimator::normalizedTime() const
{
    Q_D(const QAbstractClipAnimator);
    return d->m_normalizedTime;
}

void QAbstractClipAnimator::setRunning(bool running)
{
    Q_D(QAbstractClipAnimator);
    if (d->m_running == running)
        return;

    if (running && !d->canPlay())
        return;

    d->m_running = running;
    emit runningChanged(running);
}

void QAbstractClipAnimator::setChannelMapper(QChannelMapper *mapping)
{
    Q_D(QAbstractClipAnimator);
    if (d->m_mapper == mapping)
        return;

    if (d->m_mapper)
        d->unregisterDestructionHelper(d->m_mapper);

    if (mapping && !mapping->parent())
        mapping->setParent(this);
    d->m_mapper = mapping;

    // Ensures proper bookkeeping
    if (d->m_mapper)
        d->registerDestructionHelper(d->m_mapper, &QAbstractClipAnimator::setChannelMapper, d->m_mapper);
    emit channelMapperChanged(mapping);
}

void QAbstractClipAnimator::setLoopCount(int loops)
{
    Q_D(QAbstractClipAnimator);
    if (d->m_loops == loops)
        return;

    d->m_loops = loops;
    emit loopCountChanged(loops);
}

void QAbstractClipAnimator::setClock(QClock *clock)
{
    Q_D(QAbstractClipAnimator);
    if (d->m_clock == clock)
        return;

    if (d->m_clock)
        d->unregisterDestructionHelper(d->m_clock);

    if (clock && !clock->parent())
        clock->setParent(this);
    d->m_clock = clock;

    if (d->m_clock)
        d->registerDestructionHelper(d->m_clock, &QAbstractClipAnimator::setClock, d->m_clock);
    emit clockChanged(clock);
}

void QAbstractClipAnimator::setNormalizedTime(float timeFraction)
{
    Q_D(QAbstractClipAnimator);
    const bool validTime = !(timeFraction < 0.0f) && !(timeFraction > 1.0f);
    if (!validTime) {
        qWarning("Time value %f is not valid, needs to be in the range 0.0 to 1.0", timeFraction);
        return;
    }

    if (qFuzzyCompare(d->m_normalizedTime, timeFraction))
        return;

    d->m_normalizedTime = timeFraction;
    emit normalizedTimeChanged(timeFraction);
}

/*!
    Starts the animation.
*/
void QAbstractClipAnimator::start()
{
    setRunning(true);
}

/*!
    Stops the animation.
*/
void QAbstractClipAnimator::stop()
{
    setRunning(false);
}

} // namespace Qt3DAnimation

QT_END_NAMESPACE
