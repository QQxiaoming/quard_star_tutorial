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
    id: root
    width: 640
    height: 480
    color: "black"

    Repeater {
        model: 2

        Image {
            id: ball
            objectName: "ball" + index
            source: "resources/redball.png"
            property real homeX: 200 + index * 200
            property real homeY: 200
            width: 80; height: 80; x: homeX; y: 200

            Text {
                anchors.centerIn: parent
                color: "white"
                text: momentum.velocity.x.toFixed(2) + "," + momentum.velocity.y.toFixed(2)
            }

            SequentialAnimation {
                id: anim

                function restart(vel) {
                    stop()
                    momentum.velocity = vel
                    start()
                }

                MomentumAnimation { id: momentum; target: ball }

                PauseAnimation { duration: 500 }

                ParallelAnimation {
                    id: ballReturn
                    NumberAnimation {
                        target: ball
                        property: "x"
                        to: homeX
                        duration: 1000
                        easing.period: 50
                        easing.type: Easing.OutElastic
                    }
                    NumberAnimation {
                        target: ball
                        property: "y"
                        to: homeY
                        duration: 1000
                        easing.type: Easing.OutElastic
                    }
                }
            }

            DragHandler {
                id: dragHandler
                objectName: "dragHandler" + index
                onActiveChanged: {
                    if (!active)
                        anim.restart(centroid.velocity)
                }
            }
            Rectangle {
                visible: dragHandler.active
                anchors.fill: parent
                anchors.margins: -5
                radius: width / 2
                opacity: 0.25
            }

            Rectangle {
                visible: width > 0
                width: dragHandler.centroid.velocity.length() * 100
                height: 2
                x: ball.width / 2
                y: ball.height / 2
                z: -1
                rotation: Math.atan2(dragHandler.centroid.velocity.y, dragHandler.centroid.velocity.x) * 180 / Math.PI
                transformOrigin: Item.BottomLeft
                antialiasing: true

                Image {
                    source: "resources/arrowhead.png"
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 16
                    height: 12
                    antialiasing: true
                }
            }
        }
    }
}
