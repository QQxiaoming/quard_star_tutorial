/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the manual tests of the Qt Toolkit.
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
import "qrc:/quick/shared/" as Examples

Item {
    width: 480
    height: 320

    Rectangle {
        id: rect
        anchors.fill: parent; anchors.margins: 40
        border.width: 3; border.color: "transparent"
        color: handler.pressed ? "lightsteelblue" : "darkgrey"

        TapHandler {
            id: handler
            acceptedButtons: (leftAllowedCB.checked ? Qt.LeftButton : Qt.NoButton) |
                             (middleAllowedCB.checked ? Qt.MiddleButton : Qt.NoButton) |
                             (rightAllowedCB.checked ? Qt.RightButton : Qt.NoButton)
            gesturePolicy: (policyDragThresholdCB.checked ? TapHandler.DragThreshold :
                            policyWithinBoundsCB.checked ? TapHandler.WithinBounds :
                            TapHandler.ReleaseWithinBounds)

            onCanceled: {
                console.log("canceled @ " + point.position)
                borderBlink.blinkColor = "red"
                borderBlink.start()
            }
            onTapped: { // 'eventPoint' is a signal parameter of type QQuickEventPoint*
                console.log("tapped button " + eventPoint.event.button + " @ " + eventPoint.scenePosition +
                            " on device '" + eventPoint.event.device.name + "' " + (tapCount > 1 ? (tapCount + " times") : "for the first time"))
                if (tapCount > 1) {
                    tapCountLabel.text = tapCount
                    flashAnimation.start()
                } else {
                    borderBlink.tapFeedback(eventPoint.event.button)
                }
            }
            onLongPressed: longPressFeedback.createObject(rect,
                {"x": point.position.x, "y": point.position.y,
                 "text": handler.timeHeld.toFixed(3) + " sec",
                 "color": buttonToBlinkColor(point.pressedButtons)})
        }

        Text {
            id: tapCountLabel
            anchors.centerIn: parent
            font.pixelSize: 72
            font.weight: Font.Black
            SequentialAnimation {
                id: flashAnimation
                PropertyAction { target: tapCountLabel; property: "visible"; value: true }
                PropertyAction { target: tapCountLabel; property: "opacity"; value: 1.0 }
                PropertyAction { target: tapCountLabel; property: "scale"; value: 1.0 }
                ParallelAnimation {
                    NumberAnimation {
                        target: tapCountLabel
                        property: "opacity"
                        to: 0
                        duration: 500
                    }
                    NumberAnimation {
                        target: tapCountLabel
                        property: "scale"
                        to: 1.5
                        duration: 500
                    }
                }
            }
        }

        Rectangle {
            id: expandingCircle
            radius: handler.timeHeld * 100
            visible: radius > 0 && handler.pressed
            border.width: 3
            border.color: buttonToBlinkColor(handler.point.pressedButtons)
            color: "transparent"
            width: radius * 2
            height: radius * 2
            x: handler.point.pressPosition.x - radius
            y: handler.point.pressPosition.y - radius
            opacity: 0.25
        }

        Component {
            id: longPressFeedback
            Text { }
        }

        SequentialAnimation {
            id: borderBlink
            property color blinkColor: "red"
            function tapFeedback(button) {
                blinkColor = buttonToBlinkColor(button);
                start();
            }
            loops: 3
            ScriptAction { script: rect.border.color = borderBlink.blinkColor }
            PauseAnimation { duration: 100 }
            ScriptAction { script: rect.border.color = "transparent" }
            PauseAnimation { duration: 100 }
        }
    }

    function buttonToBlinkColor(button) {
        switch (button) {
        case Qt.MiddleButton: return "orange";
        case Qt.RightButton:  return "magenta";
        default:              return "green";
        }
    }

    Row {
        spacing: 6
        Text { text: "accepted mouse clicks:"; anchors.verticalCenter: leftAllowedCB.verticalCenter }
        Examples.CheckBox {
            id: leftAllowedCB
            checked: true
            text: "left click"
        }
        Examples.CheckBox {
            id: middleAllowedCB
            text: "middle click"
        }
        Examples.CheckBox {
            id: rightAllowedCB
            text: "right click"
        }
        Text { text: "      gesture policy:"; anchors.verticalCenter: leftAllowedCB.verticalCenter }
        Examples.CheckBox {
            id: policyDragThresholdCB
            text: "drag threshold"
            onCheckedChanged: if (checked) {
                policyWithinBoundsCB.checked = false;
                policyReleaseWithinBoundsCB.checked = false;
            }
        }
        Examples.CheckBox {
            id: policyWithinBoundsCB
            text: "within bounds"
            onCheckedChanged: if (checked) {
                policyDragThresholdCB.checked = false;
                policyReleaseWithinBoundsCB.checked = false;
            }
        }
        Examples.CheckBox {
            id: policyReleaseWithinBoundsCB
            checked: true
            text: "release within bounds"
            onCheckedChanged: if (checked) {
                policyDragThresholdCB.checked = false;
                policyWithinBoundsCB.checked = false;
            }
        }
    }
}
