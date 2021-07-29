/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
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

#include "qquickslider_p.h"
#include "qquickcontrol_p_p.h"
#include "qquickdeferredexecute_p_p.h"

#include <QtQuick/private/qquickwindow_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Slider
    \inherits Control
    \instantiates QQuickSlider
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-input
    \brief Used to select a value by sliding a handle along a track.

    \image qtquickcontrols2-slider.gif

    Slider is used to select a value by sliding a handle along a track.

    In the example below, custom \l from, \l value, and \l to values are set:

    \code
    Slider {
        from: 1
        value: 25
        to: 100
    }
    \endcode

    The \l position property is expressed as a fraction of the control's size,
    in the range \c {0.0 - 1.0}. The \l visualPosition property is
    the same, except that it is reversed in a
    \l {Right-to-left User Interfaces}{right-to-left} application. The
    visualPosition is useful for positioning the handle when styling Slider.
    In the example above, \l visualPosition will be \c 0.24 in a left-to-right
    application, and \c 0.76 in a right-to-left application.

    For a slider that allows the user to select a range by providing two
    handles, see \l RangeSlider.

    \sa {Customizing Slider}, {Input Controls}
*/

/*!
    \since QtQuick.Controls 2.2 (Qt 5.9)
    \qmlsignal QtQuick.Controls::Slider::moved()

    This signal is emitted when the slider has been interactively moved
    by the user by either touch, mouse, wheel, or keys.
*/

class QQuickSliderPrivate : public QQuickControlPrivate
{
    Q_DECLARE_PUBLIC(QQuickSlider)

public:
    qreal snapPosition(qreal position) const;
    qreal positionAt(const QPointF &point) const;
    void setPosition(qreal position);
    void updatePosition();

    void handlePress(const QPointF &point) override;
    void handleMove(const QPointF &point) override;
    void handleRelease(const QPointF &point) override;
    void handleUngrab() override;

    void cancelHandle();
    void executeHandle(bool complete = false);

    void itemImplicitWidthChanged(QQuickItem *item) override;
    void itemImplicitHeightChanged(QQuickItem *item) override;

    qreal from = 0;
    qreal to = 1;
    qreal value = 0;
    qreal position = 0;
    qreal stepSize = 0;
    qreal touchDragThreshold = -1; // in QQuickWindowPrivate::dragOverThreshold, '-1' implies using styleHints::startDragDistance()
    bool live = true;
    bool pressed = false;
    QPointF pressPoint;
    Qt::Orientation orientation = Qt::Horizontal;
    QQuickSlider::SnapMode snapMode = QQuickSlider::NoSnap;
    QQuickDeferredPointer<QQuickItem> handle;
};

qreal QQuickSliderPrivate::snapPosition(qreal position) const
{
    const qreal range = to - from;
    if (qFuzzyIsNull(range))
        return position;

    const qreal effectiveStep = stepSize / range;
    if (qFuzzyIsNull(effectiveStep))
        return position;

    return qRound(position / effectiveStep) * effectiveStep;
}

qreal QQuickSliderPrivate::positionAt(const QPointF &point) const
{
    Q_Q(const QQuickSlider);
    qreal pos = 0.0;
    if (orientation == Qt::Horizontal) {
        const qreal hw = handle ? handle->width() : 0;
        const qreal offset = hw / 2;
        const qreal extent = q->availableWidth() - hw;
        if (!qFuzzyIsNull(extent)) {
            if (q->isMirrored())
                pos = (q->width() - point.x() - q->rightPadding() - offset) / extent;
            else
                pos = (point.x() - q->leftPadding() - offset) / extent;
        }
    } else {
        const qreal hh = handle ? handle->height() : 0;
        const qreal offset = hh / 2;
        const qreal extent = q->availableHeight() - hh;
        if (!qFuzzyIsNull(extent))
            pos = (q->height() - point.y() - q->bottomPadding() - offset) / extent;
    }
    return qBound<qreal>(0.0, pos, 1.0);
}

void QQuickSliderPrivate::setPosition(qreal pos)
{
    Q_Q(QQuickSlider);
    pos = qBound<qreal>(0.0, pos, 1.0);
    if (qFuzzyCompare(position, pos))
        return;

    position = pos;
    emit q->positionChanged();
    emit q->visualPositionChanged();
}

void QQuickSliderPrivate::updatePosition()
{
    qreal pos = 0;
    if (!qFuzzyCompare(from, to))
        pos = (value - from) / (to - from);
    setPosition(pos);
}

void QQuickSliderPrivate::handlePress(const QPointF &point)
{
    Q_Q(QQuickSlider);
    QQuickControlPrivate::handlePress(point);
    pressPoint = point;
    q->setPressed(true);
}

void QQuickSliderPrivate::handleMove(const QPointF &point)
{
    Q_Q(QQuickSlider);
    QQuickControlPrivate::handleMove(point);
    const qreal oldPos = position;
    qreal pos = positionAt(point);
    if (snapMode == QQuickSlider::SnapAlways)
        pos = snapPosition(pos);
    if (live)
        q->setValue(q->valueAt(pos));
    else
        setPosition(pos);
    if (!qFuzzyCompare(pos, oldPos))
        emit q->moved();
}

void QQuickSliderPrivate::handleRelease(const QPointF &point)
{
    Q_Q(QQuickSlider);
    QQuickControlPrivate::handleRelease(point);
    pressPoint = QPointF();
    const qreal oldPos = position;
    qreal pos = positionAt(point);
    if (snapMode != QQuickSlider::NoSnap)
        pos = snapPosition(pos);
    qreal val = q->valueAt(pos);
    if (!qFuzzyCompare(val, value))
        q->setValue(val);
    else if (snapMode != QQuickSlider::NoSnap)
        setPosition(pos);
    if (!qFuzzyCompare(pos, oldPos))
        emit q->moved();
    q->setKeepMouseGrab(false);
    q->setKeepTouchGrab(false);
    q->setPressed(false);
}

void QQuickSliderPrivate::handleUngrab()
{
    Q_Q(QQuickSlider);
    QQuickControlPrivate::handleUngrab();
    pressPoint = QPointF();
    q->setPressed(false);
}

static inline QString handleName() { return QStringLiteral("handle"); }

void QQuickSliderPrivate::cancelHandle()
{
    Q_Q(QQuickSlider);
    quickCancelDeferred(q, handleName());
}

void QQuickSliderPrivate::executeHandle(bool complete)
{
    Q_Q(QQuickSlider);
    if (handle.wasExecuted())
        return;

    if (!handle || complete)
        quickBeginDeferred(q, handleName(), handle);
    if (complete)
        quickCompleteDeferred(q, handleName(), handle);
}

void QQuickSliderPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    Q_Q(QQuickSlider);
    QQuickControlPrivate::itemImplicitWidthChanged(item);
    if (item == handle)
        emit q->implicitHandleWidthChanged();
}

void QQuickSliderPrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    Q_Q(QQuickSlider);
    QQuickControlPrivate::itemImplicitHeightChanged(item);
    if (item == handle)
        emit q->implicitHandleHeightChanged();
}

QQuickSlider::QQuickSlider(QQuickItem *parent)
    : QQuickControl(*(new QQuickSliderPrivate), parent)
{
    setActiveFocusOnTab(true);
    setFocusPolicy(Qt::StrongFocus);
    setAcceptedMouseButtons(Qt::LeftButton);
#if QT_CONFIG(cursor)
    setCursor(Qt::ArrowCursor);
#endif
}

QQuickSlider::~QQuickSlider()
{
    Q_D(QQuickSlider);
    d->removeImplicitSizeListener(d->handle);
}

/*!
    \qmlproperty real QtQuick.Controls::Slider::from

    This property holds the starting value for the range. The default value is \c 0.0.

    \sa to, value
*/
qreal QQuickSlider::from() const
{
    Q_D(const QQuickSlider);
    return d->from;
}

void QQuickSlider::setFrom(qreal from)
{
    Q_D(QQuickSlider);
    if (qFuzzyCompare(d->from, from))
        return;

    d->from = from;
    emit fromChanged();
    if (isComponentComplete()) {
        setValue(d->value);
        d->updatePosition();
    }
}

/*!
    \qmlproperty real QtQuick.Controls::Slider::to

    This property holds the end value for the range. The default value is \c 1.0.

    \sa from, value
*/
qreal QQuickSlider::to() const
{
    Q_D(const QQuickSlider);
    return d->to;
}

void QQuickSlider::setTo(qreal to)
{
    Q_D(QQuickSlider);
    if (qFuzzyCompare(d->to, to))
        return;

    d->to = to;
    emit toChanged();
    if (isComponentComplete()) {
        setValue(d->value);
        d->updatePosition();
    }
}

/*!
    \qmlproperty real QtQuick.Controls::Slider::value

    This property holds the value in the range \c from - \c to. The default value is \c 0.0.

    \sa position
*/
qreal QQuickSlider::value() const
{
    Q_D(const QQuickSlider);
    return d->value;
}

void QQuickSlider::setValue(qreal value)
{
    Q_D(QQuickSlider);
    if (isComponentComplete())
        value = d->from > d->to ? qBound(d->to, value, d->from) : qBound(d->from, value, d->to);

    if (qFuzzyCompare(d->value, value))
        return;

    d->value = value;
    d->updatePosition();
    emit valueChanged();
}

/*!
    \qmlproperty real QtQuick.Controls::Slider::position
    \readonly

    This property holds the logical position of the handle.

    The position is expressed as a fraction of the control's size, in the range
    \c {0.0 - 1.0}. For visualizing a slider, the right-to-left aware
    \l visualPosition should be used instead.

    \sa value, visualPosition, valueAt()
*/
qreal QQuickSlider::position() const
{
    Q_D(const QQuickSlider);
    return d->position;
}

/*!
    \qmlproperty real QtQuick.Controls::Slider::visualPosition
    \readonly

    This property holds the visual position of the handle.

    The position is expressed as a fraction of the control's size, in the range
    \c {0.0 - 1.0}. When the control is \l {Control::mirrored}{mirrored}, the
    value is equal to \c {1.0 - position}. This makes the value suitable for
    visualizing the slider, taking right-to-left support into account.

    \sa position
*/
qreal QQuickSlider::visualPosition() const
{
    Q_D(const QQuickSlider);
    if (d->orientation == Qt::Vertical || isMirrored())
        return 1.0 - d->position;
    return d->position;
}

/*!
    \qmlproperty real QtQuick.Controls::Slider::stepSize

    This property holds the step size. The default value is \c 0.0.

    \sa snapMode, increase(), decrease()
*/
qreal QQuickSlider::stepSize() const
{
    Q_D(const QQuickSlider);
    return d->stepSize;
}

void QQuickSlider::setStepSize(qreal step)
{
    Q_D(QQuickSlider);
    if (qFuzzyCompare(d->stepSize, step))
        return;

    d->stepSize = step;
    emit stepSizeChanged();
}

/*!
    \qmlproperty enumeration QtQuick.Controls::Slider::snapMode

    This property holds the snap mode.

    The snap mode determines how the slider handle behaves with
    regards to the \l stepSize.

    Possible values:
    \value Slider.NoSnap The slider does not snap (default).
    \value Slider.SnapAlways The slider snaps while the handle is dragged.
    \value Slider.SnapOnRelease The slider does not snap while being dragged, but only after the handle is released.

    In the following table, the various modes are illustrated with animations.
    The movement of the mouse cursor and the \l stepSize (\c 0.2) are identical
    in each animation.

    \table
    \header
        \row \li \b Value \li \b Example
        \row \li \c Slider.NoSnap \li \image qtquickcontrols2-slider-nosnap.gif
        \row \li \c Slider.SnapAlways \li \image qtquickcontrols2-slider-snapalways.gif
        \row \li \c Slider.SnapOnRelease \li \image qtquickcontrols2-slider-snaponrelease.gif
    \endtable

    \sa stepSize
*/
QQuickSlider::SnapMode QQuickSlider::snapMode() const
{
    Q_D(const QQuickSlider);
    return d->snapMode;
}

void QQuickSlider::setSnapMode(SnapMode mode)
{
    Q_D(QQuickSlider);
    if (d->snapMode == mode)
        return;

    d->snapMode = mode;
    emit snapModeChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::Slider::pressed

    This property holds whether the slider is pressed by either touch, mouse,
    or keys.
*/
bool QQuickSlider::isPressed() const
{
    Q_D(const QQuickSlider);
    return d->pressed;
}

void QQuickSlider::setPressed(bool pressed)
{
    Q_D(QQuickSlider);
    if (d->pressed == pressed)
        return;

    d->pressed = pressed;
    setAccessibleProperty("pressed", pressed);
    emit pressedChanged();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty bool QtQuick.Controls::Slider::horizontal
    \readonly

    This property holds whether the slider is horizontal.

    \sa orientation
*/
bool QQuickSlider::isHorizontal() const
{
    Q_D(const QQuickSlider);
    return d->orientation == Qt::Horizontal;
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty bool QtQuick.Controls::Slider::vertical
    \readonly

    This property holds whether the slider is vertical.

    \sa orientation
*/
bool QQuickSlider::isVertical() const
{
    Q_D(const QQuickSlider);
    return d->orientation == Qt::Vertical;
}

/*!
    \qmlproperty enumeration QtQuick.Controls::Slider::orientation

    This property holds the orientation.

    Possible values:
    \value Qt.Horizontal Horizontal (default)
    \value Qt.Vertical Vertical

    \sa horizontal, vertical
*/
Qt::Orientation QQuickSlider::orientation() const
{
    Q_D(const QQuickSlider);
    return d->orientation;
}

void QQuickSlider::setOrientation(Qt::Orientation orientation)
{
    Q_D(QQuickSlider);
    if (d->orientation == orientation)
        return;

    d->orientation = orientation;
    emit orientationChanged();
}

/*!
    \qmlproperty Item QtQuick.Controls::Slider::handle

    This property holds the handle item.

    \sa {Customizing Slider}
*/
QQuickItem *QQuickSlider::handle() const
{
    QQuickSliderPrivate *d = const_cast<QQuickSliderPrivate *>(d_func());
    if (!d->handle)
        d->executeHandle();
    return d->handle;
}

void QQuickSlider::setHandle(QQuickItem *handle)
{
    Q_D(QQuickSlider);
    if (d->handle == handle)
        return;

    if (!d->handle.isExecuting())
        d->cancelHandle();

    const qreal oldImplicitHandleWidth = implicitHandleWidth();
    const qreal oldImplicitHandleHeight = implicitHandleHeight();

    d->removeImplicitSizeListener(d->handle);
    delete d->handle;
    d->handle = handle;

    if (handle) {
        if (!handle->parentItem())
            handle->setParentItem(this);
        d->addImplicitSizeListener(handle);
    }

    if (!qFuzzyCompare(oldImplicitHandleWidth, implicitHandleWidth()))
        emit implicitHandleWidthChanged();
    if (!qFuzzyCompare(oldImplicitHandleHeight, implicitHandleHeight()))
        emit implicitHandleHeightChanged();
    if (!d->handle.isExecuting())
        emit handleChanged();
}

/*!
    \since QtQuick.Controls 2.1 (Qt 5.8)
    \qmlmethod real QtQuick.Controls::Slider::valueAt(real position)

    Returns the value for the given \a position.

    \sa value, position
*/
qreal QQuickSlider::valueAt(qreal position) const
{
    Q_D(const QQuickSlider);
    const qreal value = (d->to - d->from) * position;
    if (qFuzzyIsNull(d->stepSize))
        return d->from + value;
    return d->from + qRound(value / d->stepSize) * d->stepSize;
}

/*!
    \since QtQuick.Controls 2.2 (Qt 5.9)
    \qmlproperty bool QtQuick.Controls::Slider::live

    This property holds whether the slider provides live updates for the \l value
    property while the handle is dragged.

    The default value is \c true.

    \sa value, valueAt()
*/
bool QQuickSlider::live() const
{
    Q_D(const QQuickSlider);
    return d->live;
}

void QQuickSlider::setLive(bool live)
{
    Q_D(QQuickSlider);
    if (d->live == live)
        return;

    d->live = live;
    emit liveChanged();
}

/*!
    \qmlmethod void QtQuick.Controls::Slider::increase()

    Increases the value by \l stepSize or \c 0.1 if stepSize is not defined.

    \sa stepSize
*/
void QQuickSlider::increase()
{
    Q_D(QQuickSlider);
    qreal step = qFuzzyIsNull(d->stepSize) ? 0.1 : d->stepSize;
    setValue(d->value + step);
}

/*!
    \qmlmethod void QtQuick.Controls::Slider::decrease()

    Decreases the value by \l stepSize or \c 0.1 if stepSize is not defined.

    \sa stepSize
*/
void QQuickSlider::decrease()
{
    Q_D(QQuickSlider);
    qreal step = qFuzzyIsNull(d->stepSize) ? 0.1 : d->stepSize;
    setValue(d->value - step);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty qreal QtQuick.Controls::Slider::touchDragThreshold

    This property holds the threshold (in logical pixels) at which a touch drag event will be initiated.
    The mouse drag threshold won't be affected.
    The default value is \c Qt.styleHints.startDragDistance.

    \sa QStyleHints
*/
qreal QQuickSlider::touchDragThreshold() const
{
    Q_D(const QQuickSlider);
    return d->touchDragThreshold;
}

void QQuickSlider::setTouchDragThreshold(qreal touchDragThreshold)
{
    Q_D(QQuickSlider);
    if (d->touchDragThreshold == touchDragThreshold)
        return;

    d->touchDragThreshold = touchDragThreshold;
    emit touchDragThresholdChanged();
}

void QQuickSlider::resetTouchDragThreshold()
{
    setTouchDragThreshold(-1);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Slider::implicitHandleWidth
    \readonly

    This property holds the implicit handle width.

    The value is equal to \c {handle ? handle.implicitWidth : 0}.

    This is typically used, together with \l {Control::}{implicitContentWidth} and
    \l {Control::}{implicitBackgroundWidth}, to calculate the \l {Item::}{implicitWidth}.

    \sa implicitHandleHeight
*/
qreal QQuickSlider::implicitHandleWidth() const
{
    Q_D(const QQuickSlider);
    if (!d->handle)
        return 0;
    return d->handle->implicitWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Slider::implicitHandleHeight
    \readonly

    This property holds the implicit handle height.

    The value is equal to \c {handle ? handle.implicitHeight : 0}.

    This is typically used, together with \l {Control::}{implicitContentHeight} and
    \l {Control::}{implicitBackgroundHeight}, to calculate the \l {Item::}{implicitHeight}.

    \sa implicitHandleWidth
*/
qreal QQuickSlider::implicitHandleHeight() const
{
    Q_D(const QQuickSlider);
    if (!d->handle)
        return 0;
    return d->handle->implicitHeight();
}

void QQuickSlider::keyPressEvent(QKeyEvent *event)
{
    Q_D(QQuickSlider);
    QQuickControl::keyPressEvent(event);

    const qreal oldValue = d->value;
    if (d->orientation == Qt::Horizontal) {
        if (event->key() == Qt::Key_Left) {
            setPressed(true);
            if (isMirrored())
                increase();
            else
                decrease();
            event->accept();
        } else if (event->key() == Qt::Key_Right) {
            setPressed(true);
            if (isMirrored())
                decrease();
            else
                increase();
            event->accept();
        }
    } else {
        if (event->key() == Qt::Key_Up) {
            setPressed(true);
            increase();
            event->accept();
        } else if (event->key() == Qt::Key_Down) {
            setPressed(true);
            decrease();
            event->accept();
        }
    }
    if (!qFuzzyCompare(d->value, oldValue))
        emit moved();
}

void QQuickSlider::keyReleaseEvent(QKeyEvent *event)
{
    QQuickControl::keyReleaseEvent(event);
    setPressed(false);
}

void QQuickSlider::mousePressEvent(QMouseEvent *event)
{
    Q_D(QQuickSlider);
    QQuickControl::mousePressEvent(event);
    d->handleMove(event->localPos());
    setKeepMouseGrab(true);
}

#if QT_CONFIG(quicktemplates2_multitouch)
void QQuickSlider::touchEvent(QTouchEvent *event)
{
    Q_D(QQuickSlider);
    switch (event->type()) {
    case QEvent::TouchUpdate:
        for (const QTouchEvent::TouchPoint &point : event->touchPoints()) {
            if (!d->acceptTouch(point))
                continue;

            switch (point.state()) {
            case Qt::TouchPointPressed:
                d->handlePress(point.pos());
                break;
            case Qt::TouchPointMoved:
                if (!keepTouchGrab()) {
                    if (d->orientation == Qt::Horizontal)
                        setKeepTouchGrab(QQuickWindowPrivate::dragOverThreshold(point.pos().x() - d->pressPoint.x(), Qt::XAxis, &point, qRound(d->touchDragThreshold)));
                    else
                        setKeepTouchGrab(QQuickWindowPrivate::dragOverThreshold(point.pos().y() - d->pressPoint.y(), Qt::YAxis, &point, qRound(d->touchDragThreshold)));
                }
                if (keepTouchGrab())
                    d->handleMove(point.pos());
                break;
            case Qt::TouchPointReleased:
                d->handleRelease(point.pos());
                break;
            default:
                break;
            }
        }
        break;

    default:
        QQuickControl::touchEvent(event);
        break;
    }
}
#endif

#if QT_CONFIG(wheelevent)
void QQuickSlider::wheelEvent(QWheelEvent *event)
{
    Q_D(QQuickSlider);
    QQuickControl::wheelEvent(event);
    if (d->wheelEnabled) {
        const qreal oldValue = d->value;
        const QPointF angle = event->angleDelta();
        const qreal delta = (qFuzzyIsNull(angle.y()) ? angle.x() : (event->inverted() ? -angle.y() : angle.y())) / QWheelEvent::DefaultDeltasPerStep;
        const qreal step = qFuzzyIsNull(d->stepSize) ? 0.1 : d->stepSize;
        setValue(oldValue + step * delta);
        const bool wasMoved = !qFuzzyCompare(d->value, oldValue);
        if (wasMoved)
            emit moved();
    }
}
#endif

void QQuickSlider::mirrorChange()
{
    QQuickControl::mirrorChange();
    emit visualPositionChanged();
}

void QQuickSlider::componentComplete()
{
    Q_D(QQuickSlider);
    d->executeHandle(true);
    QQuickControl::componentComplete();
    setValue(d->value);
    d->updatePosition();
}

#if QT_CONFIG(accessibility)
void QQuickSlider::accessibilityActiveChanged(bool active)
{
    QQuickControl::accessibilityActiveChanged(active);

    Q_D(QQuickSlider);
    if (active)
        setAccessibleProperty("pressed", d->pressed);
}

QAccessible::Role QQuickSlider::accessibleRole() const
{
    return QAccessible::Slider;
}
#endif

QT_END_NAMESPACE
