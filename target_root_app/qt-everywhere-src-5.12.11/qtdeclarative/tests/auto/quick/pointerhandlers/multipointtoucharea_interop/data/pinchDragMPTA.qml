/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.12

Rectangle {
    width: 1024; height: 600
    color: "beige"
    objectName: "beige root"

    Rectangle {
        id: container
        objectName: "container rect"
        width: 600
        height: 500
        color: "black"
        border.color: pinch3.active ? "red" : "black"
        border.width: 3
        antialiasing: true

        MultiPointTouchArea {
            id: mpta
            anchors.fill: parent
            //onGestureStarted: gesture.grab() // in case this is embedded in something that might steal
            touchPoints: [
                TouchPoint { property color color: "red" },
                TouchPoint { property color color: "orange" },
                TouchPoint { property color color: "lightsteelblue" },
                TouchPoint { property color color: "green" }
            ] }

        Repeater {
            model: 4

            Item {
                id: crosshairs
                property TouchPoint touchPoint
                x: touchPoint.x - width / 2
                y: touchPoint.y - height / 2
                width: 300; height: 300
                visible: touchPoint.pressed
                rotation: touchPoint.rotation

                Rectangle {
                    color: touchPoint.color
                    anchors.centerIn: parent
                    width: 2; height: parent.height
                    antialiasing: true
                }
                Rectangle {
                    color: touchPoint.color
                    anchors.centerIn: parent
                    width: parent.width; height: 2
                    antialiasing: true
                }
                Component.onCompleted: touchPoint = mpta.touchPoints[index]
            }
        }

        Item {
            objectName: "pinch and drag"
            anchors.fill: parent
            // In order for PinchHandler to get a chance to take a passive grab, it has to get the touchpoints first.
            // In order to get the touchpoints first, it has to be on top of the Z order: i.e. come last in paintOrderChildItems().
            // This is the opposite situation as with filtersChildMouseEvents: e.g. PinchArea would have wanted to be the parent,
            // if it even knew that trick (which it doesn't).
            DragHandler {
                id: dragHandler
                objectName: "DragHandler"
                target: container
                grabPermissions: PointerHandler.CanTakeOverFromItems
            }
            PinchHandler {
                id: pinch3
                objectName: "3-finger pinch"
                target: container
                minimumPointCount: 3
                minimumScale: 0.1
                maximumScale: 10
                grabPermissions: PointerHandler.CanTakeOverFromItems
            }
        }
    }
}
