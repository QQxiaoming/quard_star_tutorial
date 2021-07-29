/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#include "qquickmultipointhandler_p.h"
#include <private/qquickitem_p.h>
#include <QLineF>
#include <QMouseEvent>
#include <QDebug>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MultiPointHandler
    \since 5.10
    \preliminary
    \instantiates QQuickMultiPointHandler
    \inherits PointerDeviceHandler
    \inqmlmodule QtQuick
    \brief Abstract handler for multi-point Pointer Events.

    An intermediate class (not registered as a QML type)
    for any type of handler which requires and acts upon a specific number
    of multiple touchpoints.
*/
QQuickMultiPointHandler::QQuickMultiPointHandler(QQuickItem *parent, int minimumPointCount, int maximumPointCount)
    : QQuickPointerDeviceHandler(parent)
    , m_minimumPointCount(minimumPointCount)
    , m_maximumPointCount(maximumPointCount)
{
}

bool QQuickMultiPointHandler::wantsPointerEvent(QQuickPointerEvent *event)
{
    if (!QQuickPointerDeviceHandler::wantsPointerEvent(event))
        return false;

    if (event->asPointerScrollEvent())
        return false;

    bool ret = false;
#if QT_CONFIG(gestures)
    if (event->asPointerNativeGestureEvent() && event->point(0)->state() != QQuickEventPoint::Released)
        ret = true;
#endif

    // If points were pressed or released within parentItem, reset stored state
    // and check eligible points again. This class of handlers is intended to
    // handle a specific number of points, so a differing number of points will
    // usually result in different behavior. But otherwise if the currentPoints
    // are all still there in the event, we're good to go (do not reset
    // m_currentPoints, because we don't want to lose the pressPosition, and do
    // not want to reshuffle the order either).
    const QVector<QQuickEventPoint *> candidatePoints = eligiblePoints(event);
    if (candidatePoints.count() != m_currentPoints.count()) {
        m_currentPoints.clear();
        if (active()) {
            setActive(false);
            m_centroid.reset();
            emit centroidChanged();
        }
    } else if (hasCurrentPoints(event)) {
        return true;
    }

    ret = ret || (candidatePoints.size() >= minimumPointCount() && candidatePoints.size() <= maximumPointCount());
    if (ret) {
        const int c = candidatePoints.count();
        m_currentPoints.resize(c);
        for (int i = 0; i < c; ++i) {
            m_currentPoints[i].reset(candidatePoints[i]);
            m_currentPoints[i].localize(parentItem());
        }
    } else {
        m_currentPoints.clear();
    }
    return ret;
}

void QQuickMultiPointHandler::handlePointerEventImpl(QQuickPointerEvent *event)
{
    QQuickPointerHandler::handlePointerEventImpl(event);
    // event's points can be reordered since the previous event, which is why m_currentPoints
    // is _not_ a shallow copy of the QQuickPointerTouchEvent::m_touchPoints vector.
    // So we have to update our m_currentPoints instances based on the given event.
    for (QQuickHandlerPoint &p : m_currentPoints) {
        const QQuickEventPoint *ep = event->pointById(p.id());
        if (ep)
            p.reset(ep);
    }
    QPointF sceneGrabPos = m_centroid.sceneGrabPosition();
    m_centroid.reset(m_currentPoints);
    m_centroid.m_sceneGrabPosition = sceneGrabPos; // preserve as it was
    emit centroidChanged();
}

void QQuickMultiPointHandler::onActiveChanged()
{
    if (active()) {
        m_centroid.m_sceneGrabPosition = m_centroid.m_scenePosition;
    } else {
        // Don't call m_centroid.reset() here, because in a QML onActiveChanged
        // callback, we'd like to see what the position _was_, what the velocity _was_, etc.
        // (having them undefined is not useful)
        // But pressedButtons and pressedModifiers are meant to be more real-time than those
        // (which seems a bit inconsistent, from one side).
        m_centroid.m_pressedButtons = Qt::NoButton;
        m_centroid.m_pressedModifiers = Qt::NoModifier;
    }
}

void QQuickMultiPointHandler::onGrabChanged(QQuickPointerHandler *, QQuickEventPoint::GrabTransition transition, QQuickEventPoint *)
{
    // If another handler or item takes over this set of points, assume it has
    // decided that it's the better fit for them. Don't immediately re-grab
    // at the next opportunity. This should help to avoid grab cycles
    // (e.g. between DragHandler and PinchHandler).
    if (transition == QQuickEventPoint::UngrabExclusive || transition == QQuickEventPoint::CancelGrabExclusive)
        m_currentPoints.clear();
}

QVector<QQuickEventPoint *> QQuickMultiPointHandler::eligiblePoints(QQuickPointerEvent *event)
{
    QVector<QQuickEventPoint *> ret;
    int c = event->pointCount();
    // If one or more points are newly pressed or released, all non-released points are candidates for this handler.
    // In other cases however, check whether it would be OK to steal the grab if the handler chooses to do that.
    bool stealingAllowed = event->isPressEvent() || event->isReleaseEvent();
    for (int i = 0; i < c; ++i) {
        QQuickEventPoint *p = event->point(i);
        if (QQuickPointerMouseEvent *me = event->asPointerMouseEvent()) {
            if (me->buttons() == Qt::NoButton)
                continue;
        }
        if (!stealingAllowed) {
            QObject *exclusiveGrabber = p->exclusiveGrabber();
            if (exclusiveGrabber && exclusiveGrabber != this && !canGrab(p))
                continue;
        }
        if (p->state() != QQuickEventPoint::Released && wantsEventPoint(p))
            ret << p;
    }
    return ret;
}

/*!
     \qmlproperty int MultiPointHandler::minimumPointCount

     The minimum number of touchpoints required to activate this handler.

     If a smaller number of touchpoints are in contact with the
     \l {PointerHandler::parent}{parent}, they will be ignored.

     Any ignored points are eligible to activate other Input Handlers that
     have different constraints, on the same Item or on other Items.

     The default value is 2.
*/
void QQuickMultiPointHandler::setMinimumPointCount(int c)
{
    if (m_minimumPointCount == c)
        return;

    m_minimumPointCount = c;
    emit minimumPointCountChanged();
    if (m_maximumPointCount < 0)
        emit maximumPointCountChanged();
}

/*!
     \qmlproperty int MultiPointHandler::maximumPointCount

     The maximum number of touchpoints this handler can utilize.

     If a larger number of touchpoints are in contact with the
     \l {PointerHandler::parent}{parent}, the required number of points will be
     chosen in the order that they are pressed, and the remaining points will
     be ignored.

     Any ignored points are eligible to activate other Input Handlers that
     have different constraints, on the same Item or on other Items.

     The default value is the same as \l minimumPointCount.
*/
void QQuickMultiPointHandler::setMaximumPointCount(int maximumPointCount)
{
    if (m_maximumPointCount == maximumPointCount)
        return;

    m_maximumPointCount = maximumPointCount;
    emit maximumPointCountChanged();
}

bool QQuickMultiPointHandler::hasCurrentPoints(QQuickPointerEvent *event)
{
    if (event->pointCount() < m_currentPoints.size() || m_currentPoints.size() == 0)
        return false;
    // TODO optimize: either ensure the points are sorted,
    // or use std::equal with a predicate
    for (const QQuickHandlerPoint &p : qAsConst(m_currentPoints)) {
        const QQuickEventPoint *ep = event->pointById(p.id());
        if (!ep)
            return false;
        if (ep->state() == QQuickEventPoint::Released)
            return false;
    }
    return true;
}

qreal QQuickMultiPointHandler::averageTouchPointDistance(const QPointF &ref)
{
    qreal ret = 0;
    if (Q_UNLIKELY(m_currentPoints.size() == 0))
        return ret;
    for (const QQuickHandlerPoint &p : m_currentPoints)
        ret += QVector2D(p.scenePosition() - ref).length();
    return ret / m_currentPoints.size();
}

qreal QQuickMultiPointHandler::averageStartingDistance(const QPointF &ref)
{
    // TODO cache it in setActive()?
    qreal ret = 0;
    if (Q_UNLIKELY(m_currentPoints.size() == 0))
        return ret;
    for (const QQuickHandlerPoint &p : m_currentPoints)
        ret += QVector2D(p.sceneGrabPosition() - ref).length();
    return ret / m_currentPoints.size();
}

QVector<QQuickMultiPointHandler::PointData> QQuickMultiPointHandler::angles(const QPointF &ref) const
{
    QVector<PointData> angles;
    angles.reserve(m_currentPoints.count());
    for (const QQuickHandlerPoint &p : m_currentPoints) {
        qreal angle = QLineF(ref, p.scenePosition()).angle();
        angles.append(PointData(p.id(), -angle));     // convert to clockwise, to be consistent with QQuickItem::rotation
    }
    return angles;
}

qreal QQuickMultiPointHandler::averageAngleDelta(const QVector<PointData> &old, const QVector<PointData> &newAngles)
{
    qreal avgAngleDelta = 0;
    int numSamples = 0;

    auto oldBegin = old.constBegin();

    for (PointData newData : newAngles) {
        quint64 id = newData.id;
        auto it = std::find_if(oldBegin, old.constEnd(), [id] (PointData pd) { return pd.id == id; });
        qreal angleD = 0;
        if (it != old.constEnd()) {
            PointData oldData = *it;
            // We might rotate from 359 degrees to 1 degree. However, this
            // should be interpreted as a rotation of +2 degrees instead of
            // -358 degrees. Therefore, we call remainder() to translate the angle
            // to be in the range [-180, 180] (-350 to +10 etc)
            angleD = remainder(newData.angle - oldData.angle, qreal(360));
            // optimization: narrow down the O(n^2) search to optimally O(n)
            // if both vectors have the same points and they are in the same order
            if (it == oldBegin)
                ++oldBegin;
            numSamples++;
        }
        avgAngleDelta += angleD;
    }
    if (numSamples > 1)
        avgAngleDelta /= numSamples;

    return avgAngleDelta;
}

void QQuickMultiPointHandler::acceptPoints(const QVector<QQuickEventPoint *> &points)
{
    for (QQuickEventPoint* point : points)
        point->setAccepted();
}

bool QQuickMultiPointHandler::grabPoints(QVector<QQuickEventPoint *> points)
{
    if (points.isEmpty())
        return false;
    bool allowed = true;
    for (QQuickEventPoint* point : points) {
        if (point->exclusiveGrabber() != this && !canGrab(point)) {
            allowed = false;
            break;
        }
    }
    if (allowed) {
        for (QQuickEventPoint* point : points)
            setExclusiveGrab(point);
    }
    return allowed;
}

void QQuickMultiPointHandler::moveTarget(QPointF pos)
{
    target()->setPosition(pos);
    m_centroid.m_position = target()->mapFromScene(m_centroid.m_scenePosition);
}

/*!
    \readonly
    \qmlproperty QtQuick::HandlerPoint QtQuick::MultiPointHandler::centroid

    A point exactly in the middle of the currently-pressed touch points.
    If only one point is pressed, it's the same as that point.
    A handler that has a \l target will normally transform it relative to this point.
*/

QT_END_NAMESPACE
