/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qquickdraghandler_p.h"
#include <private/qquickwindow_p.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

static const qreal DragAngleToleranceDegrees = 10;

Q_LOGGING_CATEGORY(lcDragHandler, "qt.quick.handler.drag")

/*!
    \qmltype DragHandler
    \instantiates QQuickDragHandler
    \inherits MultiPointHandler
    \inqmlmodule QtQuick
    \ingroup qtquick-input-handlers
    \brief Handler for dragging.

    DragHandler is a handler that is used to interactively move an Item.
    Like other Input Handlers, by default it is fully functional, and
    manipulates its \l {PointerHandler::target} {target}.

    \snippet pointerHandlers/dragHandler.qml 0

    It has properties to restrict the range of dragging.

    If it is declared within one Item but is assigned a different
    \l {PointerHandler::target} {target}, then it handles events within the
    bounds of the \l {PointerHandler::parent} {parent} Item but
    manipulates the \c target Item instead:

    \snippet pointerHandlers/dragHandlerDifferentTarget.qml 0

    A third way to use it is to set \l {PointerHandler::target} {target} to
    \c null and react to property changes in some other way:

    \snippet pointerHandlers/dragHandlerNullTarget.qml 0

    If minimumPointCount and maximumPointCount are set to values larger than 1,
    the user will need to drag that many fingers in the same direction to start
    dragging. A multi-finger drag gesture can be detected independently of both
    a (default) single-finger DragHandler and a PinchHandler on the same Item,
    and thus can be used to adjust some other feature independently of the
    usual pinch behavior: for example adjust a tilt transformation, or adjust
    some other numeric value, if the \c target is set to null. But if the
    \c target is an Item, \c centroid is the point at which the drag begins and
    to which the \c target will be moved (subject to constraints).

    At this time, drag-and-drop is not yet supported.

    \sa Drag, MouseArea
*/

QQuickDragHandler::QQuickDragHandler(QQuickItem *parent)
    : QQuickMultiPointHandler(parent, 1, 1)
{
}

bool QQuickDragHandler::targetContainsCentroid()
{
    Q_ASSERT(parentItem() && target());
    return target()->contains(targetCentroidPosition());
}

QPointF QQuickDragHandler::targetCentroidPosition()
{
    QPointF pos = m_centroid.position();
    if (target() != parentItem())
        pos = parentItem()->mapToItem(target(), pos);
    return pos;
}

void QQuickDragHandler::onGrabChanged(QQuickPointerHandler *grabber, QQuickEventPoint::GrabTransition transition, QQuickEventPoint *point)
{
    QQuickMultiPointHandler::onGrabChanged(grabber, transition, point);
    if (grabber == this && transition == QQuickEventPoint::GrabExclusive && target()) {
        // In case the grab got handed over from another grabber, we might not get the Press.
        if (!m_pressedInsideTarget) {
            if (target() != parentItem())
                m_pressTargetPos = QPointF(target()->width(), target()->height()) / 2;
        } else if (m_pressTargetPos.isNull()) {
            m_pressTargetPos = targetCentroidPosition();
        }
    }
}

void QQuickDragHandler::onActiveChanged()
{
    QQuickMultiPointHandler::onActiveChanged();
    if (active()) {
        if (auto parent = parentItem()) {
            if (currentEvent()->asPointerTouchEvent())
                parent->setKeepTouchGrab(true);
            // tablet and mouse are treated the same by Item's legacy event handling, and
            // touch becomes synth-mouse for Flickable, so we need to prevent stealing
            // mouse grab too, whenever dragging occurs in an enabled direction
            parent->setKeepMouseGrab(true);
        }
    } else {
        m_pressTargetPos = QPointF();
        m_pressedInsideTarget = false;
        if (auto parent = parentItem()) {
            parent->setKeepTouchGrab(false);
            parent->setKeepMouseGrab(false);
        }
    }
}

void QQuickDragHandler::handlePointerEventImpl(QQuickPointerEvent *event)
{
    QQuickMultiPointHandler::handlePointerEventImpl(event);
    event->setAccepted(true);

    if (active()) {
        // Calculate drag delta, taking into account the axis enabled constraint
        // i.e. if xAxis is not enabled, then ignore the horizontal component of the actual movement
        QVector2D accumulatedDragDelta = QVector2D(m_centroid.scenePosition() - m_centroid.scenePressPosition());
        if (!m_xAxis.enabled())
            accumulatedDragDelta.setX(0);
        if (!m_yAxis.enabled())
            accumulatedDragDelta.setY(0);
        setTranslation(accumulatedDragDelta);
    } else {
        // Check that all points have been dragged past the drag threshold,
        // to the extent that the constraints allow,
        // and in approximately the same direction
        qreal minAngle =  361;
        qreal maxAngle = -361;
        bool allOverThreshold = !event->isReleaseEvent();
        QVector <QQuickEventPoint *> chosenPoints;

        if (event->isPressEvent())
            m_pressedInsideTarget = target() && m_currentPoints.count() > 0;

        for (const QQuickHandlerPoint &p : m_currentPoints) {
            if (!allOverThreshold)
                break;
            QQuickEventPoint *point = event->pointById(p.id());
            chosenPoints << point;
            setPassiveGrab(point);
            // Calculate drag delta, taking into account the axis enabled constraint
            // i.e. if xAxis is not enabled, then ignore the horizontal component of the actual movement
            QVector2D accumulatedDragDelta = QVector2D(point->scenePosition() - point->scenePressPosition());
            if (!m_xAxis.enabled()) {
                // If horizontal dragging is disallowed, but the user is dragging
                // mostly horizontally, then don't activate.
                if (qAbs(accumulatedDragDelta.x()) > qAbs(accumulatedDragDelta.y()))
                    accumulatedDragDelta.setY(0);
                accumulatedDragDelta.setX(0);
            }
            if (!m_yAxis.enabled()) {
                // If vertical dragging is disallowed, but the user is dragging
                // mostly vertically, then don't activate.
                if (qAbs(accumulatedDragDelta.y()) > qAbs(accumulatedDragDelta.x()))
                    accumulatedDragDelta.setX(0);
                accumulatedDragDelta.setY(0);
            }
            qreal angle = std::atan2(accumulatedDragDelta.y(), accumulatedDragDelta.x()) * 180 / M_PI;
            bool overThreshold = QQuickWindowPrivate::dragOverThreshold(accumulatedDragDelta);
            qCDebug(lcDragHandler) << "movement" << accumulatedDragDelta << "angle" << angle << "of point" << point
                                   << "pressed @" << point->scenePressPosition() << "over threshold?" << overThreshold;
            minAngle = qMin(angle, minAngle);
            maxAngle = qMax(angle, maxAngle);
            if (allOverThreshold && !overThreshold)
                allOverThreshold = false;

            if (event->isPressEvent()) {
                // m_pressedInsideTarget should stay true iff ALL points in which DragHandler is interested
                // have been pressed inside the target() Item.  (E.g. in a Slider the parent might be the
                // whole control while the target is just the knob.)
                if (target()) {
                    const QPointF localPressPos = target()->mapFromScene(point->scenePressPosition());
                    m_pressedInsideTarget &= target()->contains(localPressPos);
                    m_pressTargetPos = targetCentroidPosition();
                }
                // QQuickWindowPrivate::deliverToPassiveGrabbers() skips subsequent delivery if the event is filtered.
                // (That affects behavior for mouse but not for touch, because Flickable only handles mouse.)
                // So we have to compensate by accepting the event here to avoid any parent Flickable from
                // getting the event via direct delivery and grabbing too soon.
                point->setAccepted(event->asPointerMouseEvent()); // stop propagation iff it's a mouse event
            }
        }
        if (allOverThreshold) {
            qreal angleDiff = maxAngle - minAngle;
            if (angleDiff > 180)
                angleDiff = 360 - angleDiff;
            qCDebug(lcDragHandler) << "angle min" << minAngle << "max" << maxAngle << "range" << angleDiff;
            if (angleDiff < DragAngleToleranceDegrees && grabPoints(chosenPoints))
                setActive(true);
        }
    }
    if (active() && target() && target()->parentItem()) {
        const QPointF newTargetTopLeft = targetCentroidPosition() - m_pressTargetPos;
        const QPointF xformOrigin = target()->transformOriginPoint();
        const QPointF targetXformOrigin = newTargetTopLeft + xformOrigin;
        QPointF pos = target()->parentItem()->mapFromItem(target(), targetXformOrigin);
        pos -= xformOrigin;
        QPointF targetItemPos = target()->position();
        if (!m_xAxis.enabled())
            pos.setX(targetItemPos.x());
        if (!m_yAxis.enabled())
            pos.setY(targetItemPos.y());
        enforceAxisConstraints(&pos);
        moveTarget(pos);
    }
}

void QQuickDragHandler::enforceConstraints()
{
    if (!target() || !target()->parentItem())
        return;
    QPointF pos = target()->position();
    QPointF copy(pos);
    enforceAxisConstraints(&pos);
    if (pos != copy)
        target()->setPosition(pos);
}

void QQuickDragHandler::enforceAxisConstraints(QPointF *localPos)
{
    if (m_xAxis.enabled())
        localPos->setX(qBound(m_xAxis.minimum(), localPos->x(), m_xAxis.maximum()));
    if (m_yAxis.enabled())
        localPos->setY(qBound(m_yAxis.minimum(), localPos->y(), m_yAxis.maximum()));
}

void QQuickDragHandler::setTranslation(const QVector2D &trans)
{
    if (trans == m_translation) // fuzzy compare?
        return;
    m_translation = trans;
    emit translationChanged();
}

/*!
    \qmlpropertygroup QtQuick::DragHandler::xAxis
    \qmlproperty real QtQuick::DragHandler::xAxis.minimum
    \qmlproperty real QtQuick::DragHandler::xAxis.maximum
    \qmlproperty bool QtQuick::DragHandler::xAxis.enabled

    \c xAxis controls the constraints for horizontal dragging.

    \c minimum is the minimum acceptable value of \l {Item::x}{x} to be
    applied to the \l {PointerHandler::target} {target}.
    \c maximum is the maximum acceptable value of \l {Item::x}{x} to be
    applied to the \l {PointerHandler::target} {target}.
    If \c enabled is true, horizontal dragging is allowed.
 */

/*!
    \qmlpropertygroup QtQuick::DragHandler::yAxis
    \qmlproperty real QtQuick::DragHandler::yAxis.minimum
    \qmlproperty real QtQuick::DragHandler::yAxis.maximum
    \qmlproperty bool QtQuick::DragHandler::yAxis.enabled

    \c yAxis controls the constraints for vertical dragging.

    \c minimum is the minimum acceptable value of \l {Item::y}{y} to be
    applied to the \l {PointerHandler::target} {target}.
    \c maximum is the maximum acceptable value of \l {Item::y}{y} to be
    applied to the \l {PointerHandler::target} {target}.
    If \c enabled is true, vertical dragging is allowed.
 */

/*!
    \readonly
    \qmlproperty QVector2D QtQuick::DragHandler::translation

    The translation since the gesture began.
*/

QT_END_NAMESPACE
