/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.9
import QtQuick.Shapes 1.0

Rectangle {
    id: root
    width: 1024
    height: 768

    property color col: "lightsteelblue"
    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.tint(root.col, "#20FFFFFF") }
        GradientStop { position: 0.1; color: Qt.tint(root.col, "#20AAAAAA") }
        GradientStop { position: 0.9; color: Qt.tint(root.col, "#20666666") }
        GradientStop { position: 1.0; color: Qt.tint(root.col, "#20000000") }
    }

    ListModel {
        id: pathGalleryModel
        ListElement {
            name: "Stroke and fill"
            shapeUrl: "tapableTriangle.qml"
        }
        ListElement {
            name: "Stroke or fill only"
            shapeUrl: "item2.qml"
        }
        ListElement {
            name: "Dash pattern"
            shapeUrl: "item4.qml"
        }
        ListElement {
            name: "Linear gradient"
            shapeUrl: "item5.qml"
        }
        ListElement {
            name: "Radial gradient"
            shapeUrl: "item3.qml"
        }
        ListElement {
            name: "Fill rules"
            shapeUrl: "item6.qml"
        }
        ListElement {
            name: "Join styles"
            shapeUrl: "item7.qml"
        }
        ListElement {
            name: "Cap styles"
            shapeUrl: "item8.qml"
        }
        ListElement {
            name: "Quadratic curve"
            shapeUrl: "item9.qml"
        }
        ListElement {
            name: "Cubic curve"
            shapeUrl: "item10.qml"
        }
        ListElement {
            name: "Elliptical arc"
            shapeUrl: "item11.qml"
        }
        ListElement {
            name: "Gradient spread modes"
            shapeUrl: "item12.qml"
        }
        ListElement {
            name: "Arc direction"
            shapeUrl: "item13.qml"
        }
        ListElement {
            name: "Large/small arc"
            shapeUrl: "item14.qml"
        }
        ListElement {
            name: "Arc rotation"
            shapeUrl: "item15.qml"
        }
        ListElement {
            name: "Tiger"
            shapeUrl: "item17.qml"
        }
    }

    property int gridSpacing: 10

    Component {
        id: pathGalleryDelegate
        Rectangle {
            border.color: "purple"
            width: grid.cellWidth - root.gridSpacing
            height: grid.cellHeight - root.gridSpacing
            Column {
                anchors.fill: parent
                anchors.margins: 4
                Item {
                    width: parent.width
                    height: parent.height - delegText.height
                    Loader {
                        source: Qt.resolvedUrl(shapeUrl)
                        anchors.fill: parent
                    }
                }
                Text {
                    id: delegText
                    text: model.name
                    font.pointSize: 16
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 10
        color: "lightBlue"
        clip: true

        GridView {
            id: grid
            anchors.fill: parent
            anchors.margins: root.gridSpacing
            cellWidth: 300
            cellHeight: 300
            delegate: pathGalleryDelegate
            model: pathGalleryModel
        }
    }

    Text {
        anchors.right: parent.right
        Shape { id: dummyShape; ShapePath { } } // used only to get the renderer type
        color: "darkBlue"
        font.pointSize: 12
        property variant rendererStrings: [ "Unknown", "Generic (QtGui triangulator)", "GL_NV_path_rendering", "Software (QPainter)" ]
        text: "Active Shape backend: " + rendererStrings[dummyShape.rendererType]
        SequentialAnimation on opacity {
            NumberAnimation { from: 1; to: 0; duration: 5000 }
            PauseAnimation { duration: 5000 }
            NumberAnimation { from: 0; to: 1; duration: 1000 }
            PauseAnimation { duration: 5000 }
            loops: Animation.Infinite
        }
    }
}
