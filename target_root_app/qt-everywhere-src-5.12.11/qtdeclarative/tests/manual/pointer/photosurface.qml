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
import QtQuick.Dialogs 1.0
import Qt.labs.folderlistmodel 1.0
import "content"

Rectangle {
    id: root
    visible: true
    width: 1024; height: 600
    color: "black"
    property int highestZ: 0
    property real defaultSize: 200
    property real surfaceViewportRatio: 1.5

    FileDialog {
        id: fileDialog
        title: "Choose a folder with some images"
        selectFolder: true
        onAccepted: folderModel.folder = fileUrl + "/"
    }
    Shortcut {
        id: openShortcut
        sequence: StandardKey.Open
        onActivated: fileDialog.open()
    }

    FakeFlickable {
        id: flick
        anchors.fill: parent
        contentWidth: width * 2
        contentHeight: height * 2
        Repeater {
            model: FolderListModel {
                id: folderModel
                folder: "resources/"
                objectName: "folderModel"
                showDirs: false
                nameFilters: ["*.png", "*.jpg", "*.gif"]
            }
            Rectangle {
                id: photoFrame
                objectName: "frame-" + fileName
                width: image.width * (1 + 0.10 * image.height / image.width)
                height: image.height * 1.10
                scale: defaultSize / Math.max(image.sourceSize.width, image.sourceSize.height)
                Behavior on scale { NumberAnimation { duration: 200 } }
                Behavior on x { NumberAnimation { duration: 200 } }
                Behavior on y { NumberAnimation { duration: 200 } }
                border.color: pinchHandler.active || dragHandler.active ? "red" : "black"
                border.width: 2
                smooth: true
                antialiasing: true
                Component.onCompleted: {
                    x = Math.random() * root.width - width / 2
                    y = Math.random() * root.height - height / 2
                    rotation = Math.random() * 13 - 6
                }
                Image {
                    id: image
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                    source: folderModel.folder + fileName
                    antialiasing: true
                }

                MomentumAnimation { id: anim; target: photoFrame }

                DragHandler {
                    id: dragHandler
                    onActiveChanged: {
                        if (!active)
                            anim.restart(point.velocity)
                    }
                }

                PinchHandler {
                    id: pinchHandler
                    minimumRotation: -360
                    maximumRotation: 360
                    minimumScale: 0.1
                    maximumScale: 10
                    property real zRestore: 0
                }
            }
        }
    }

    Rectangle {
        id: verticalScrollDecorator
        anchors.right: parent.right
        anchors.margins: 2
        color: "cyan"
        border.color: "black"
        border.width: 1
        width: 5
        radius: 2
        antialiasing: true
        height: flick.height * (flick.height / flick.contentHeight) - (width - anchors.margins) * 2
        y:  -flick.contentY * (flick.height / flick.contentHeight)
        NumberAnimation on opacity { id: vfade; to: 0; duration: 500 }
        onYChanged: { opacity = 1.0; fadeTimer.restart() }
    }

    Rectangle {
        id: horizontalScrollDecorator
        anchors.bottom: parent.bottom
        anchors.margins: 2
        color: "cyan"
        border.color: "black"
        border.width: 1
        height: 5
        radius: 2
        antialiasing: true
        width: flick.width * (flick.width / flick.contentWidth) - (height - anchors.margins) * 2
        x:  -flick.contentX * (flick.width / flick.contentWidth)
        NumberAnimation on opacity { id: hfade; to: 0; duration: 500 }
        onXChanged: { opacity = 1.0; fadeTimer.restart() }
    }

    Timer { id: fadeTimer; interval: 1000; onTriggered: { hfade.start(); vfade.start() } }

    Text {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        color: "darkgrey"
        wrapMode: Text.WordWrap
        font.pointSize: 8
        text: "Press " + openShortcut.nativeText + " to choose a different image folder\n" +
              "On a touchscreen: use two fingers to zoom and rotate, one finger to drag\n" +
              "With a mouse: drag normally"
    }
}
