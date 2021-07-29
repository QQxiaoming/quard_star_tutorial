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

    Row {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 40

        Column {
            spacing: 40

            Text {
                text: "Original"
            }

            // A simple Shape without anything special.
            Rectangle {
                color: "lightGray"
                width: 400
                height: 200

                Shape {
                    x: 30
                    y: 20
                    width: 50
                    height: 50
                    scale: 2

                    ShapePath {
                        strokeColor: "green"
                        NumberAnimation on strokeWidth { from: 1; to: 20; duration: 5000 }
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap

                        startX: 40; startY: 30
                        PathQuad { x: 50; y: 80; controlX: 0; controlY: 80 }
                        PathLine { x: 150; y: 80 }
                        PathQuad { x: 160; y: 30; controlX: 200; controlY: 80 }
                    }
                }
            }

            Text {
                text: "Supersampling (2x)"
            }

            // Now let's use 2x supersampling via layers. This way the entire subtree
            // is rendered into an FBO twice the size and then drawn with linear
            // filtering. This allows having some level of AA even when there is no
            // support for multisample framebuffers.
            Rectangle {
                id: supersampledItem
                color: "lightGray"
                width: 400
                height: 200

                layer.enabled: true
                layer.smooth: true
                layer.textureSize: Qt.size(supersampledItem.width * 2, supersampledItem.height * 2)

                Shape {
                    x: 30
                    y: 20
                    width: 50
                    height: 50
                    scale: 2

                    ShapePath {
                        strokeColor: "green"
                        NumberAnimation on strokeWidth { from: 1; to: 20; duration: 5000 }
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap

                        startX: 40; startY: 30
                        PathQuad { x: 50; y: 80; controlX: 0; controlY: 80 }
                        PathLine { x: 150; y: 80 }
                        PathQuad { x: 160; y: 30; controlX: 200; controlY: 80 }
                    }
                }
            }
        }

        Column {
            spacing: 40

            Text {
                text: "Multisampling (4x)"
            }

            // Now let's use 4x MSAA, again via layers. This needs support for
            // multisample renderbuffers and framebuffer blits.
            Rectangle {
                color: "lightGray"
                width: 400
                height: 200

                layer.enabled: true
                layer.smooth: true
                layer.samples: 4

                Shape {
                    x: 30
                    y: 20
                    width: 50
                    height: 50
                    scale: 2

                    ShapePath {
                        strokeColor: "green"
                        NumberAnimation on strokeWidth { from: 1; to: 20; duration: 5000 }
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap

                        startX: 40; startY: 30
                        PathQuad { x: 50; y: 80; controlX: 0; controlY: 80 }
                        PathLine { x: 150; y: 80 }
                        PathQuad { x: 160; y: 30; controlX: 200; controlY: 80 }
                    }
                }
            }
        }
    }
}
