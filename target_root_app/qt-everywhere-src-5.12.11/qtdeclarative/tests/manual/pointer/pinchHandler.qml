/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
import "content"

Rectangle {
    width: 1024; height: 600
    color: "#eee"

    function getTransformationDetails(item, pinchhandler) {
        return "\n\npinch.scale:" + pinchhandler.scale.toFixed(2)
        + "\npinch.rotation:" + pinchhandler.rotation.toFixed(2)
        + "\npinch.translation:" + "(" + pinchhandler.translation.x.toFixed(2) + "," + pinchhandler.translation.y.toFixed(2) + ")"
        + "\nrect.scale: " + item.scale.toFixed(2)
        + "\nrect.rotation: " + item.rotation.toFixed(2)
        + "\nrect.position: " + "(" + item.x.toFixed(2) + "," + item.y.toFixed(2) + ")"
    }

    function activePincher() {
        if (grandparentPinch.active)
            return grandparentPinch
        else if (parentPinch.active)
            return parentPinch
        else if (pinch2.active)
            return pinch2
        return pinch3       // always return a pinch handler, even when its inactive. The indicator will be invisble anyway.
    }

    Rectangle {
        width: parent.width - 100; height: parent.height - 100; x: 50; y: 50
        color: "beige"
        border.width: grandparentPinch.active ? 2 : 0
        border.color: border.width > 0 ? "red" : "transparent"
        antialiasing: true

        PinchHandler {
            id: grandparentPinch
            objectName: "grandparent pinch"
            minimumScale: 0.5
            maximumScale: 3
            minimumPointCount: 3
        }

        Text {
            text: "Pinch with 3 fingers to scale, rotate and translate"
                  + getTransformationDetails(parent, grandparentPinch)
        }

        Rectangle {
            width: parent.width - 100; height: parent.height - 100; x: 50; y: 50
            color: "#ffe0e0e0"
            antialiasing: true

            PinchHandler {
                id: parentPinch
                objectName: "parent pinch"
                minimumScale: 0.5
                maximumScale: 3
            }

            Text {
                text: "Pinch with 2 fingers to scale, rotate and translate"
                      + getTransformationDetails(parent, parentPinch)
            }

            Rectangle {
                id: rect2
                width: 400
                height: 300
                color: "lightsteelblue"
                antialiasing: true
                x: 100
                y: 200
                rotation: 30
                transformOrigin: Item.TopRight
                border.width: (lsbDragHandler.active || pinch2.active) ? 2 : 0
                border.color: border.width > 0 ? "red" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "Pinch with 2 fingers to scale, rotate and translate\nDrag with 1 finger"
                          + getTransformationDetails(rect2, pinch2) + "\nz " + rect2.z
                }
                DragHandler {
                    id: lsbDragHandler
                    objectName: "lightsteelblue drag"
                }
                PinchHandler {
                    id: pinch2
                    objectName: "lightsteelblue pinch"
                    minimumRotation: -45
                    maximumRotation: 45
                    minimumScale: 0.5
                    maximumScale: 3
                    minimumX: 0
                    maximumX: 600
                    // acceptedModifiers: Qt.ControlModifier
                }
                TapHandler { gesturePolicy: TapHandler.DragThreshold; onTapped: rect2.z = rect3.z + 1 }
            }

            Rectangle {
                id: rect3
                x: 500
                width: 400
                height: 300
                color: "wheat"
                antialiasing: true
                border.width: (wheatDragHandler.active || pinch3.active) ? 2 : 0
                border.color: border.width > 0 ? "red" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "Pinch with 3 fingers to scale, rotate and translate\nDrag with 1 finger"
                          + getTransformationDetails(rect3, pinch3) + "\nz " + rect3.z
                }
                DragHandler {
                    id: wheatDragHandler
                    objectName: "wheat drag"
                }
                PinchHandler {
                    id: pinch3
                    objectName: "wheat 3-finger pinch"
                    minimumPointCount: 3
                    minimumScale: 0.1
                    maximumScale: 10
                    onActiveChanged: {
                        if (!active)
                            anim.restart(centroid.velocity)
                    }
                }
                TapHandler { gesturePolicy: TapHandler.DragThreshold; onTapped: rect3.z = rect2.z + 1 }
                MomentumAnimation { id: anim; target: rect3 }
            }
        }
    }
    Rectangle {
        id: centroidIndicator
        property QtObject pincher: activePincher()
        x: pincher.centroid.scenePosition.x - radius
        y: pincher.centroid.scenePosition.y - radius
        z: 1
        visible: pincher.active
        radius: width / 2
        width: 10
        height: width
        color: "red"
    }
}
