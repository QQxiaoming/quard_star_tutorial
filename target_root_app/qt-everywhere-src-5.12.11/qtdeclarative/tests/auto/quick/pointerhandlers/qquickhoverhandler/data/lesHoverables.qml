/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
    id: root
    width: 640
    height: 480
    color: "#444"

    Component {
        id: buttonsAndStuff
        Column {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8

            Rectangle {
                objectName: "buttonWithMA"
                width: parent.width
                height: 30
                color: buttonMA.pressed ? "lightsteelblue" : "#999"
                border.color: buttonMA.containsMouse ? "cyan" : "transparent"

                MouseArea {
                    id: buttonMA
                    objectName: "buttonMA"
                    hoverEnabled: true
                    anchors.fill: parent
                    onClicked: console.log("clicked MA")
                }

                Text {
                    anchors.centerIn: parent
                    text: "MouseArea"
                }
            }

            Rectangle {
                objectName: "buttonWithHH"
                width: parent.width
                height: 30
                color: flash ? "#999" : "white"
                border.color: buttonHH.hovered ? "cyan" : "transparent"
                property bool flash: true

                HoverHandler {
                    id: buttonHH
                    objectName: "buttonHH"
                    acceptedDevices: PointerDevice.AllDevices
                }

                TapHandler { }

                Text {
                    anchors.centerIn: parent
                    text: "HoverHandler"
                }
            }
        }
    }

    Rectangle {
        id: paddle
        objectName: "paddle"
        width: 100
        height: 100
        color: paddleHH.hovered ? "indianred" : "#888"
        x: (parent.width - width) / 2
        y: parent.height - 100
        radius: 10

        HoverHandler {
            id: paddleHH
            objectName: "paddleHH"
        }
    }

    Rectangle {
        objectName: "topSidebar"
        radius: 5
        antialiasing: true
        x: -radius
        y: -radius
        width: 120
        height: 200
        border.color: topSidebarHH.hovered ? "cyan" : "black"
        color: "#777"

        Rectangle {
            color: "cyan"
            width: 10
            height: width
            radius: width / 2
            visible: topSidebarHH.hovered
            x: topSidebarHH.point.position.x - width / 2
            y: topSidebarHH.point.position.y - height / 2
            z: 100
        }

        HoverHandler {
            id: topSidebarHH
            objectName: "topSidebarHH"
        }

        Loader {
            objectName: "topSidebarLoader"
            sourceComponent: buttonsAndStuff
            anchors.fill: parent
        }
    }

    Rectangle {
        objectName: "bottomSidebar"
        radius: 5
        antialiasing: true
        x: -radius
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -radius
        width: 120
        height: 200
        border.color: bottomSidebarMA.containsMouse ? "cyan" : "black"
        color: "#777"

        MouseArea {
            id: bottomSidebarMA
            objectName: "bottomSidebarMA"
            hoverEnabled: true
            anchors.fill: parent
        }

        Loader {
            objectName: "bottomSidebarLoader"
            sourceComponent: buttonsAndStuff
            anchors.fill: parent
        }
    }
}
