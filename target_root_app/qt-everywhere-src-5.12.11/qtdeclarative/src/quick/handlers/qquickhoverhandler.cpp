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

#include "qquickhoverhandler_p.h"
#include <private/qquickpointerdevicehandler_p_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcHoverHandler, "qt.quick.handler.hover")

/*!
    \qmltype HoverHandler
    \instantiates QQuickHoverHandler
    \inherits SinglePointHandler
    \inqmlmodule QtQuick
    \ingroup qtquick-input-handlers
    \brief Handler for mouse and tablet hover.

    HoverHandler detects a hovering mouse or tablet stylus cursor.

    A binding to the \l hovered property is the easiest way to react when the
    cursor enters or leaves the \l {PointerHandler::parent}{parent} Item.
    The \l {SinglePointHandler::point}{point} property provides more detail,
    including the cursor position. The
    \l {PointerDeviceHandler::acceptedDevices}{acceptedDevices},
    \l {PointerDeviceHandler::acceptedPointerTypes}{acceptedPointerTypes},
    and \l {PointerDeviceHandler::acceptedModifiers}{acceptedModifiers}
    properties can be used to narrow the behavior to detect hovering of
    specific kinds of devices or while holding a modifier key.

    \sa MouseArea, PointHandler
*/

QQuickHoverHandler::QQuickHoverHandler(QQuickItem *parent)
    : QQuickSinglePointHandler(parent)
{
    // Tell QQuickPointerDeviceHandler::wantsPointerEvent() to ignore button state
    d_func()->acceptedButtons = Qt::NoButton;
    // Rule out the touchscreen for now (can be overridden in QML in case a hover-detecting touchscreen exists)
    setAcceptedDevices(static_cast<QQuickPointerDevice::DeviceType>(
        static_cast<int>(QQuickPointerDevice::AllDevices) ^ static_cast<int>(QQuickPointerDevice::TouchScreen)));
}

QQuickHoverHandler::~QQuickHoverHandler()
{
    if (auto parent = parentItem())
        QQuickItemPrivate::get(parent)->setHasHoverInChild(false);
}

void QQuickHoverHandler::componentComplete()
{
    parentItem()->setAcceptHoverEvents(true);
    QQuickItemPrivate::get(parentItem())->setHasHoverInChild(true);
}

bool QQuickHoverHandler::wantsPointerEvent(QQuickPointerEvent *event)
{
    QQuickEventPoint *point = event->point(0);
    if (QQuickPointerDeviceHandler::wantsPointerEvent(event) && wantsEventPoint(point) && parentContains(point)) {
        // assume this is a mouse event, so there's only one point
        setPointId(point->pointId());
        return true;
    }
    setHovered(false);
    return false;
}

void QQuickHoverHandler::handleEventPoint(QQuickEventPoint *point)
{
    setHovered(true);
    setPassiveGrab(point);
}

/*!
    \qmlproperty bool QtQuick::HoverHandler::hovered
    \readonly

    Holds true whenever any pointing device cursor (mouse or tablet) is within
    the bounds of the \c parent Item, extended by the
    \l {PointerHandler::margin}{margin}, if any.
*/
void QQuickHoverHandler::setHovered(bool hovered)
{
    if (m_hovered != hovered) {
        qCDebug(lcHoverHandler) << objectName() << "hovered" << m_hovered << "->" << hovered;
        m_hovered = hovered;
        emit hoveredChanged();
    }
}

QT_END_NAMESPACE
