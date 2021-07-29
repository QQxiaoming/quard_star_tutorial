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

#include "qquickpinchhandler_p.h"
#include <QtQml/qqmlinfo.h>
#include <QtQuick/qquickwindow.h>
#include <private/qsgadaptationlayer_p.h>
#include <private/qquickitem_p.h>
#include <private/qguiapplication_p.h>
#include <private/qquickwindow_p.h>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <qpa/qplatformnativeinterface.h>
#include <math.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcPinchHandler, "qt.quick.handler.pinch")

/*!
    \qmltype PinchHandler
    \instantiates QQuickPinchHandler
    \inherits MultiPointHandler
    \inqmlmodule QtQuick
    \ingroup qtquick-input-handlers
    \brief Handler for pinch gestures.

    PinchHandler is a handler that interprets a multi-finger gesture to
    interactively rotate, zoom, and drag an Item. Like other Input Handlers,
    by default it is fully functional, and manipulates its \l target,
    which is the Item within which it is declared.

    \snippet pointerHandlers/pinchHandler.qml 0

    It has properties to restrict the range of dragging, rotation, and zoom.

    If it is declared within one Item but is assigned a different \l target, it
    handles events within the bounds of the outer Item but manipulates the
    \c target Item instead:

    \snippet pointerHandlers/pinchHandlerDifferentTarget.qml 0

    A third way to use it is to set \l target to \c null and react to property
    changes in some other way:

    \snippet pointerHandlers/pinchHandlerNullTarget.qml 0

    \image touchpoints-pinchhandler.png

    \sa PinchArea
*/

QQuickPinchHandler::QQuickPinchHandler(QQuickItem *parent)
    : QQuickMultiPointHandler(parent, 2)
{
}

/*!
    \qmlproperty real QtQuick::PinchHandler::minimumScale

    The minimum acceptable \l {Item::scale}{scale} to be applied
    to the \l target.
*/
void QQuickPinchHandler::setMinimumScale(qreal minimumScale)
{
    if (qFuzzyCompare(m_minimumScale, minimumScale))
        return;

    m_minimumScale = minimumScale;
    emit minimumScaleChanged();
}

/*!
    \qmlproperty real QtQuick::PinchHandler::maximumScale

    The maximum acceptable \l {Item::scale}{scale} to be applied
    to the \l target.
*/
void QQuickPinchHandler::setMaximumScale(qreal maximumScale)
{
    if (qFuzzyCompare(m_maximumScale, maximumScale))
        return;

    m_maximumScale = maximumScale;
    emit maximumScaleChanged();
}

/*!
    \qmlproperty real QtQuick::PinchHandler::minimumRotation

    The minimum acceptable \l {Item::rotation}{rotation} to be applied
    to the \l target.
*/
void QQuickPinchHandler::setMinimumRotation(qreal minimumRotation)
{
    if (qFuzzyCompare(m_minimumRotation, minimumRotation))
        return;

    m_minimumRotation = minimumRotation;
    emit minimumRotationChanged();
}

/*!
    \qmlproperty real QtQuick::PinchHandler::maximumRotation

    The maximum acceptable \l {Item::rotation}{rotation} to be applied
    to the \l target.
*/
void QQuickPinchHandler::setMaximumRotation(qreal maximumRotation)
{
    if (qFuzzyCompare(m_maximumRotation, maximumRotation))
        return;

    m_maximumRotation = maximumRotation;
    emit maximumRotationChanged();
}

#if QT_DEPRECATED_SINCE(5, 12)
void QQuickPinchHandler::warnAboutMinMaxDeprecated() const
{
    qmlWarning(this) << "min and max constraints are now part of the xAxis and yAxis properties";
}

void QQuickPinchHandler::setMinimumX(qreal minX)
{
    warnAboutMinMaxDeprecated();
    if (qFuzzyCompare(m_minimumX, minX))
        return;
    m_minimumX = minX;
    emit minimumXChanged();
}

void QQuickPinchHandler::setMaximumX(qreal maxX)
{
    warnAboutMinMaxDeprecated();
    if (qFuzzyCompare(m_maximumX, maxX))
        return;
    m_maximumX = maxX;
    emit maximumXChanged();
}

void QQuickPinchHandler::setMinimumY(qreal minY)
{
    warnAboutMinMaxDeprecated();
    if (qFuzzyCompare(m_minimumY, minY))
        return;
    m_minimumY = minY;
    emit minimumYChanged();
}

void QQuickPinchHandler::setMaximumY(qreal maxY)
{
    warnAboutMinMaxDeprecated();
    if (qFuzzyCompare(m_maximumY, maxY))
        return;
    m_maximumY = maxY;
    emit maximumYChanged();
}
#endif

bool QQuickPinchHandler::wantsPointerEvent(QQuickPointerEvent *event)
{
    if (!QQuickMultiPointHandler::wantsPointerEvent(event))
        return false;

#if QT_CONFIG(gestures)
    if (const auto gesture = event->asPointerNativeGestureEvent()) {
        if (minimumPointCount() == 2) {
            switch (gesture->type()) {
            case Qt::BeginNativeGesture:
            case Qt::EndNativeGesture:
            case Qt::ZoomNativeGesture:
            case Qt::RotateNativeGesture:
                return parentContains(event->point(0));
            default:
                return false;
            }
        } else {
            return false;
        }
    }
#endif

    return true;
}

/*!
    \qmlpropertygroup QtQuick::PinchHandler::xAxis
    \qmlproperty real QtQuick::PinchHandler::xAxis.minimum
    \qmlproperty real QtQuick::PinchHandler::xAxis.maximum
    \qmlproperty bool QtQuick::PinchHandler::xAxis.enabled

    \c xAxis controls the constraints for horizontal translation of the \l target item.

    \c minimum is the minimum acceptable x coordinate of the translation.
    \c maximum is the maximum acceptable x coordinate of the translation.
    If \c enabled is true, horizontal dragging is allowed.
 */

/*!
    \qmlpropertygroup QtQuick::PinchHandler::yAxis
    \qmlproperty real QtQuick::PinchHandler::yAxis.minimum
    \qmlproperty real QtQuick::PinchHandler::yAxis.maximum
    \qmlproperty bool QtQuick::PinchHandler::yAxis.enabled

    \c yAxis controls the constraints for vertical translation of the \l target item.

    \c minimum is the minimum acceptable y coordinate of the translation.
    \c maximum is the maximum acceptable y coordinate of the translation.
    If \c enabled is true, vertical dragging is allowed.
 */

/*!
    \qmlproperty int QtQuick::PinchHandler::minimumTouchPoints

    The pinch begins when this number of fingers are pressed.
    Until then, PinchHandler tracks the positions of any pressed fingers,
    but if it's an insufficient number, it does not scale or rotate
    its \l target, and the \l active property will remain false.
*/

/*!
    \qmlproperty bool QtQuick::PinchHandler::active

    This property is true when all the constraints (epecially \l minimumTouchPoints)
    are satisfied and the \l target, if any, is being manipulated.
*/

void QQuickPinchHandler::onActiveChanged()
{
    QQuickMultiPointHandler::onActiveChanged();
    if (active()) {
        m_startMatrix = QMatrix4x4();
        m_startAngles = angles(m_centroid.sceneGrabPosition());
        m_startDistance = averageTouchPointDistance(m_centroid.sceneGrabPosition());
        m_activeRotation = 0;
        m_activeTranslation = QVector2D();
        if (const QQuickItem *t = target()) {
            m_startScale = t->scale(); // TODO incompatible with independent x/y scaling
            m_startRotation = t->rotation();
            QVector3D xformOrigin(t->transformOriginPoint());
            m_startMatrix.translate(float(t->x()), float(t->y()));
            m_startMatrix.translate(xformOrigin);
            m_startMatrix.scale(float(m_startScale));
            m_startMatrix.rotate(float(m_startRotation), 0, 0, -1);
            m_startMatrix.translate(-xformOrigin);
        } else {
            m_startScale = m_accumulatedScale;
            m_startRotation = 0;
        }
        qCDebug(lcPinchHandler) << "activated with starting scale" << m_startScale << "rotation" << m_startRotation;
    } else {
        qCDebug(lcPinchHandler) << "deactivated with scale" << m_activeScale << "rotation" << m_activeRotation;
    }
}

void QQuickPinchHandler::handlePointerEventImpl(QQuickPointerEvent *event)
{
    if (Q_UNLIKELY(lcPinchHandler().isDebugEnabled())) {
        for (const QQuickHandlerPoint &p : m_currentPoints)
            qCDebug(lcPinchHandler) << hex << p.id() << p.sceneGrabPosition() << "->" << p.scenePosition();
    }
    QQuickMultiPointHandler::handlePointerEventImpl(event);

    qreal dist = 0;
#if QT_CONFIG(gestures)
    if (const auto gesture = event->asPointerNativeGestureEvent()) {
        m_centroid.reset(event->point(0));
        switch (gesture->type()) {
        case Qt::EndNativeGesture:
            m_activeScale = 1;
            m_activeRotation = 0;
            m_activeTranslation = QVector2D();
            m_centroid.reset();
            setActive(false);
            emit updated();
            return;
        case Qt::ZoomNativeGesture:
            m_activeScale *= 1 + gesture->value();
            break;
        case Qt::RotateNativeGesture:
            m_activeRotation += gesture->value();
            break;
        default:
            // Nothing of interest (which is unexpected, because wantsPointerEvent() should have returned false)
            return;
        }
        if (!active()) {
            setActive(true);
            // Native gestures for 2-finger pinch do not allow dragging, so
            // the centroid won't move during the gesture, and translation stays at zero
            m_activeTranslation = QVector2D();
        }
    } else
#endif // QT_CONFIG(gestures)
    {
        const bool containsReleasedPoints = event->isReleaseEvent();
        QVector<QQuickEventPoint *> chosenPoints;
        for (const QQuickHandlerPoint &p : m_currentPoints) {
            QQuickEventPoint *ep = event->pointById(p.id());
            chosenPoints << ep;
        }
        if (!active()) {
            // Verify that at least one of the points has moved beyond threshold needed to activate the handler
            int numberOfPointsDraggedOverThreshold = 0;
            QVector2D accumulatedDrag;
            const QVector2D currentCentroid(m_centroid.scenePosition());
            const QVector2D pressCentroid(m_centroid.scenePressPosition());

            QStyleHints *styleHints = QGuiApplication::styleHints();
            const int dragThreshold = styleHints->startDragDistance();
            const int dragThresholdSquared = dragThreshold * dragThreshold;

            double accumulatedCentroidDistance = 0;     // Used to detect scale
            if (event->isPressEvent())
                m_accumulatedStartCentroidDistance = 0;   // Used to detect scale

            float accumulatedMovementMagnitude = 0;

            for (QQuickEventPoint *point : qAsConst(chosenPoints)) {
                if (!containsReleasedPoints) {
                    accumulatedDrag += QVector2D(point->scenePressPosition() - point->scenePosition());
                    /*
                       In order to detect a drag, we want to check if all points have moved more or
                       less in the same direction.

                       We then take each point, and convert the point to a local coordinate system where
                       the centroid is the origin. This is done both for the press positions and the
                       current positions. We will then have two positions:

                       - pressCentroidRelativePosition
                           is the start point relative to the press centroid
                       - currentCentroidRelativePosition
                           is the current point relative to the current centroid

                       If those two points are far enough apart, it might not be considered as a drag
                       anymore. (Note that the threshold will matched to the average of the relative
                       movement of all the points). Therefore, a big relative movement will make a big
                       contribution to the average relative movement.

                       The algorithm then can be described as:
                         For each point:
                          - Calculate vector pressCentroidRelativePosition (from the press centroid to the press position)
                          - Calculate vector currentCentroidRelativePosition (from the current centroid to the current position)
                          - Calculate the relative movement vector:

                             centroidRelativeMovement = currentCentroidRelativePosition - pressCentroidRelativePosition

                           and measure its magnitude. Add the magnitude to the accumulatedMovementMagnitude.

                         Finally, if the accumulatedMovementMagnitude is below some threshold, it means
                         that the points were stationary or they were moved in parallel (e.g. the hand
                         was moved, but the relative position between each finger remained very much
                         the same). This is then used to rule out if there is a rotation or scale.
                    */
                    QVector2D pressCentroidRelativePosition = QVector2D(point->scenePosition()) - currentCentroid;
                    QVector2D currentCentroidRelativePosition = QVector2D(point->scenePressPosition()) - pressCentroid;
                    QVector2D centroidRelativeMovement = currentCentroidRelativePosition - pressCentroidRelativePosition;
                    accumulatedMovementMagnitude += centroidRelativeMovement.length();

                    accumulatedCentroidDistance += qreal(pressCentroidRelativePosition.length());
                    if (event->isPressEvent())
                        m_accumulatedStartCentroidDistance += qreal((QVector2D(point->scenePressPosition()) - pressCentroid).length());
                } else {
                    setPassiveGrab(point);
                }
                if (point->state() == QQuickEventPoint::Pressed) {
                    point->setAccepted(false); // don't stop propagation
                    setPassiveGrab(point);
                }
                if (QQuickWindowPrivate::dragOverThreshold(point))
                    ++numberOfPointsDraggedOverThreshold;
            }

            const bool requiredNumberOfPointsDraggedOverThreshold = numberOfPointsDraggedOverThreshold >= minimumPointCount() && numberOfPointsDraggedOverThreshold <= maximumPointCount();
            accumulatedMovementMagnitude /= m_currentPoints.count();

            QVector2D avgDrag = accumulatedDrag / m_currentPoints.count();
            if (!xAxis()->enabled())
                avgDrag.setX(0);
            if (!yAxis()->enabled())
                avgDrag.setY(0);

            const qreal centroidMovementDelta = qreal((currentCentroid - pressCentroid).length());

            qreal distanceToCentroidDelta = qAbs(accumulatedCentroidDistance - m_accumulatedStartCentroidDistance); // Used to detect scale
            if (numberOfPointsDraggedOverThreshold >= 1) {
                if (requiredNumberOfPointsDraggedOverThreshold && avgDrag.lengthSquared() >= dragThresholdSquared && accumulatedMovementMagnitude < dragThreshold) {
                    // Drag
                    if (grabPoints(chosenPoints))
                        setActive(true);
                } else if (distanceToCentroidDelta > dragThreshold) {    // all points should in accumulation have been moved beyond threshold (?)
                    // Scale
                    if (grabPoints(chosenPoints))
                        setActive(true);
                } else if (distanceToCentroidDelta < dragThreshold && (centroidMovementDelta < dragThreshold)) {
                    // Rotate
                    // Since it wasn't a scale and if we exceeded the dragthreshold, and the
                    // centroid didn't moved much, the points must have been moved around the centroid.
                    if (grabPoints(chosenPoints))
                        setActive(true);
                }
            }
            if (!active())
                return;
        }

        // avoid mapping the minima and maxima, as they might have unmappable values
        // such as -inf/+inf. Because of this we perform the bounding to min/max in local coords.
        // 1. scale
        dist = averageTouchPointDistance(m_centroid.scenePosition());
        m_activeScale = dist / m_startDistance;
        m_activeScale = qBound(m_minimumScale/m_startScale, m_activeScale, m_maximumScale/m_startScale);

        // 2. rotate
        QVector<PointData> newAngles = angles(m_centroid.scenePosition());
        const qreal angleDelta = averageAngleDelta(m_startAngles, newAngles);
        m_activeRotation += angleDelta;
        m_startAngles = std::move(newAngles);

        if (!containsReleasedPoints)
            acceptPoints(chosenPoints);
    }

    const qreal totalRotation = m_startRotation + m_activeRotation;
    const qreal rotation = qBound(m_minimumRotation, totalRotation, m_maximumRotation);
    m_activeRotation += (rotation - totalRotation);   //adjust for the potential bounding above
    m_accumulatedScale = m_startScale * m_activeScale;

    if (target() && target()->parentItem()) {
        const QPointF centroidParentPos = target()->parentItem()->mapFromScene(m_centroid.scenePosition());
        // 3. Drag/translate
        const QPointF centroidStartParentPos = target()->parentItem()->mapFromScene(m_centroid.sceneGrabPosition());
        m_activeTranslation = QVector2D(centroidParentPos - centroidStartParentPos);
        // apply rotation + scaling around the centroid - then apply translation.
        QMatrix4x4 mat;

        const QVector3D centroidParentVector(centroidParentPos);
        mat.translate(centroidParentVector);
        mat.rotate(float(m_activeRotation), 0, 0, 1);
        mat.scale(float(m_activeScale));
        mat.translate(-centroidParentVector);
        mat.translate(QVector3D(m_activeTranslation));

        mat = mat * m_startMatrix;

        QPointF xformOriginPoint = target()->transformOriginPoint();
        QPointF pos = mat * xformOriginPoint;
        pos -= xformOriginPoint;

        if (xAxis()->enabled())
            pos.setX(qBound(xAxis()->minimum(), pos.x(), xAxis()->maximum()));
        else
            pos.rx() -= qreal(m_activeTranslation.x());
        if (yAxis()->enabled())
            pos.setY(qBound(yAxis()->minimum(), pos.y(), yAxis()->maximum()));
        else
            pos.ry() -= qreal(m_activeTranslation.y());

        target()->setPosition(pos);
        target()->setRotation(rotation);
        target()->setScale(m_accumulatedScale);
    } else {
        m_activeTranslation = QVector2D(m_centroid.scenePosition() - m_centroid.scenePressPosition());
    }

    qCDebug(lcPinchHandler) << "centroid" << m_centroid.scenePressPosition() << "->"  << m_centroid.scenePosition()
                            << ", distance" << m_startDistance << "->" << dist
                            << ", startScale" << m_startScale << "->" << m_accumulatedScale
                            << ", activeRotation" << m_activeRotation
                            << ", rotation" << rotation
                            << " from " << event->device()->type();

    emit updated();
}

/*!
    \readonly
    \qmlproperty QtQuick::HandlerPoint QtQuick::PinchHandler::centroid

    A point exactly in the middle of the currently-pressed touch points.
    The \l target will be rotated around this point.
*/

/*!
    \readonly
    \qmlproperty real QtQuick::PinchHandler::scale

    The scale factor that will automatically be set on the \l target if it is not null.
    Otherwise, bindings can be used to do arbitrary things with this value.
    While the pinch gesture is being performed, it is continuously multiplied by
    \l activeScale; after the gesture ends, it stays the same; and when the next
    pinch gesture begins, it begins to be multiplied by activeScale again.
*/

/*!
    \readonly
    \qmlproperty real QtQuick::PinchHandler::activeScale

    The scale factor while the pinch gesture is being performed.
    It is 1.0 when the gesture begins, increases as the touchpoints are spread
    apart, and decreases as the touchpoints are brought together.
    If \l target is not null, its \l {Item::scale}{scale} will be automatically
    multiplied by this value.
    Otherwise, bindings can be used to do arbitrary things with this value.
*/

/*!
    \readonly
    \qmlproperty real QtQuick::PinchHandler::rotation

    The rotation of the pinch gesture in degrees, with positive values clockwise.
    It is 0 when the gesture begins. If \l target is not null, this will be
    automatically applied to its \l {Item::rotation}{rotation}. Otherwise,
    bindings can be used to do arbitrary things with this value.
*/

/*!
    \readonly
    \qmlproperty QVector2D QtQuick::PinchHandler::translation

    The translation of the gesture \l centroid. It is \c (0, 0) when the
    gesture begins.
*/

QT_END_NAMESPACE
