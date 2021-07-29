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
    color: "lightGray"

    Item {
        width: 200
        height: 200
        anchors.centerIn: parent

        Shape {
            id: shape
            anchors.fill: parent

            ShapePath {
                strokeWidth: 4
                strokeColor: "black"
                fillGradient: ConicalGradient {
                    id: conGrad
                    centerX: 100; centerY: 75
                    NumberAnimation on angle { from: 0; to: 360; duration: 10000; loops: Animation.Infinite }
                    GradientStop { position: 0; color: "#00000000" }
                    GradientStop { position: 0.10; color: "#ffe0cc73" }
                    GradientStop { position: 0.17; color: "#ffc6a006" }
                    GradientStop { position: 0.46; color: "#ff600659" }
                    GradientStop { position: 0.72; color: "#ff0680ac" }
                    GradientStop { position: 0.92; color: "#ffb9d9e6" }
                    GradientStop { position: 1.00; color: "#00000000" }
                }

                startX: 50; startY: 100
                PathCubic {
                    x: 150; y: 100
                    control1X: cp1.x; control1Y: cp1.y
                    control2X: cp2.x; control2Y: cp2.y
                }
            }
        }

        Rectangle {
            id: cp1
            color: "red"
            width: 10; height: 10
            SequentialAnimation {
                loops: Animation.Infinite
                running: true
                NumberAnimation {
                    target: cp1
                    property: "x"
                    from: 0
                    to: shape.width - cp1.width
                    duration: 5000
                }
                NumberAnimation {
                    target: cp1
                    property: "x"
                    from: shape.width - cp1.width
                    to: 0
                    duration: 5000
                }
                NumberAnimation {
                    target: cp1
                    property: "y"
                    from: 0
                    to: shape.height - cp1.height
                    duration: 5000
                }
                NumberAnimation {
                    target: cp1
                    property: "y"
                    from: shape.height - cp1.height
                    to: 0
                    duration: 5000
                }
            }
        }

        Rectangle {
            id: cp2
            color: "blue"
            width: 10; height: 10
            x: shape.width - width
            SequentialAnimation {
                loops: Animation.Infinite
                running: true
                NumberAnimation {
                    target: cp2
                    property: "y"
                    from: 0
                    to: shape.height - cp2.height
                    duration: 5000
                }
                NumberAnimation {
                    target: cp2
                    property: "y"
                    from: shape.height - cp2.height
                    to: 0
                    duration: 5000
                }
                NumberAnimation {
                    target: cp2
                    property: "x"
                    from: shape.width - cp2.width
                    to: 0
                    duration: 5000
                }
                NumberAnimation {
                    target: cp2
                    property: "x"
                    from: 0
                    to: shape.width - cp2.width
                    duration: 5000
                }
            }
        }
    }

    Text {
        anchors.right: parent.right
        anchors.top: parent.top
        text: "Conical gradient angle: " + Math.round(conGrad.angle)
    }
}
