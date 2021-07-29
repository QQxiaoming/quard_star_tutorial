/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
    color: "beige"
    objectName: "beige root"

    function getTransformationDetails(item, pinchhandler) {
        return "scale:" + pinchhandler.scale.toFixed(2)
                + " rotation:" + pinchhandler.rotation.toFixed(2)
                + " translation:" + "(" + pinchhandler.translation.x.toFixed(2) + "," + pinchhandler.translation.y.toFixed(2) + ")"
    }

    Rectangle {
        id: container
        objectName: "container rect"
        width: 600
        height: 500
        color: pinch3.active ? "red" : "black"
        antialiasing: true
        Loader {
            source: "../touch/mpta-crosshairs.qml"
            anchors.fill: parent
            anchors.margins: 2
        }
        Item {
            anchors.fill: parent
            // In order for PinchHandler to get a chance to take a passive grab, it has to get the touchpoints first.
            // In order to get the touchpoints first, it has to be on top of the Z order: i.e. come last in paintOrderChildItems().
            // This is the opposite situation as with filtersChildMouseEvents: e.g. PinchArea would have wanted to be the parent,
            // if it even knew that trick (which it doesn't).
            PinchHandler {
                id: pinch3
                objectName: "3-finger pinch"
                target: container
                minimumPointCount: 3
                minimumScale: 0.1
                maximumScale: 10
                onActiveChanged: if (!active) fling.restart(centroid.velocity)
            }
            DragHandler {
                id: dragHandler
                objectName: "DragHandler"
                target: container
                acceptedModifiers: Qt.MetaModifier
                onActiveChanged: if (!active) fling.restart(point.velocity)
            }
        }
        MomentumAnimation { id: fling; target: container }
    }
    Text {
        anchors.bottom: parent.bottom
        text: pinch3.active ? getTransformationDetails(container, pinch3) :
            "Pinch with 3 fingers to scale, rotate and translate\nHold down Meta to drag with one finger or mouse"
    }
}
