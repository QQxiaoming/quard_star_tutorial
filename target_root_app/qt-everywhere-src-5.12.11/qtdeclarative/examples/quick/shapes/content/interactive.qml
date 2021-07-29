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

    property int mode: 0
    property bool showResizers: true
    property bool fill: false

    Row {
        x: 20
        y: 10
        spacing: 20
        Rectangle {
            border.color: "black"
            color: root.mode === 0 ? "red" : "transparent"
            width: 100
            height: 40
            Text {
                anchors.centerIn: parent
                text: "Line"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.mode = 0
            }
        }
        Rectangle {
            border.color: "black"
            color: root.mode === 1 ? "red" : "transparent"
            width: 100
            height: 40
            Text {
                anchors.centerIn: parent
                text: "Cubic"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.mode = 1
            }
        }
        Rectangle {
            border.color: "black"
            color: root.mode === 2 ? "red" : "transparent"
            width: 100
            height: 40
            Text {
                anchors.centerIn: parent
                text: "Quadratic"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.mode = 2
            }
        }

        Slider {
            id: widthSlider
            name: "Width"
            min: 1
            max: 60
            init: 4
        }

        Rectangle {
            border.color: "black"
            color: root.showResizers ? "yellow" : "transparent"
            width: 50
            height: 40
            Text {
                anchors.centerIn: parent
                text: "Manip"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    root.showResizers = !root.showResizers;
                    for (var i = 0; i < canvas.resizers.length; ++i)
                        canvas.resizers[i].visible = root.showResizers;
                }
            }
        }

        Rectangle {
            border.color: "black"
            color: root.fill ? "yellow" : "transparent"
            width: 50
            height: 40
            Text {
                anchors.centerIn: parent
                text: "Fill"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.fill = !root.fill
            }
        }
    }

    Rectangle {
        id: canvas
        width: root.width - 40
        height: root.height - 120
        x: 20
        y: 100

        property variant activePath: null

        property variant resizers: []
        property variant funcs

        function genResizer(obj, x, y, xprop, yprop, color) {
            var ma = Qt.createQmlObject('import QtQuick 2.9; import QtQuick.Shapes 1.0; Rectangle { id: rr; property variant obj; color: "' + color + '"; width: 20; height: 20;'+
                                         'MouseArea { anchors.fill: parent; hoverEnabled: true;' +
                                         'onEntered: color = "yellow"; onExited: color = "' + color + '";' +
                                         'property bool a: false; onPressed: a = true; onReleased: a = false; ' +
                                         'onPositionChanged: if (a) { var pt = mapToItem(rr.parent, mouse.x, mouse.y);' +
                                         'obj.' + xprop + ' = pt.x; obj.' + yprop + ' = pt.y; rr.x = pt.x - 10; rr.y = pt.y - 10; } } }',
                                         canvas, "resizer_item");
            ma.visible = root.showResizers;
            ma.obj = obj;
            ma.x = x - 10;
            ma.y = y - 10;
            resizers.push(ma);
            return ma;
        }

        Component.onCompleted: {
            funcs = [
                        { "start": function(x, y) {
                            var p = Qt.createQmlObject('import QtQuick 2.9; import QtQuick.Shapes 1.0; ShapePath {' +
                                                       'strokeColor: "black"; fillColor: "transparent";'+
                                                       'strokeWidth: ' + widthSlider.value + ';' +
                                                       'startX: ' + x + '; startY: ' + y + ';' +
                                                       'PathLine { x: ' + x + ' + 1; y: ' + y + ' + 1 } }',
                                                       root, "dynamic_visual_path");
                            shape.data.push(p);
                            activePath = p;
                        }, "move": function(x, y) {
                            if (!activePath)
                                return;
                            var pathObj = activePath.pathElements[0];
                            pathObj.x = x;
                            pathObj.y = y;
                        }, "end": function() {
                            canvas.genResizer(activePath, activePath.startX, activePath.startY, "startX", "startY", "red");
                            var pathObj = activePath.pathElements[0];
                            canvas.genResizer(pathObj, pathObj.x, pathObj.y, "x", "y", "red");
                            activePath = null;
                        }
                        },
                        { "start": function(x, y) {
                            var p = Qt.createQmlObject('import QtQuick 2.9; import QtQuick.Shapes 1.0; ShapePath {' +
                                                       'strokeColor: "black"; fillColor: "' + (root.fill ? 'green' : 'transparent') + '";'+
                                                       'strokeWidth: ' + widthSlider.value + ';' +
                                                       'startX: ' + x + '; startY: ' + y + ';' +
                                                       'PathCubic { x: ' + x + ' + 1; y: ' + y + ' + 1;' +
                                                       'control1X: ' + x + ' + 50; control1Y: ' + y + ' + 50; control2X: ' + x + ' + 150; control2Y: ' + y + ' + 50; } }',
                                                       root, "dynamic_visual_path");
                            shape.data.push(p);
                            activePath = p;
                        }, "move": function(x, y) {
                            if (!activePath)
                                return;
                            var pathObj = activePath.pathElements[0];
                            pathObj.x = x;
                            pathObj.y = y;
                        }, "end": function() {
                            canvas.genResizer(activePath, activePath.startX, activePath.startY, "startX", "startY", "red");
                            var pathObj = activePath.pathElements[0];
                            canvas.genResizer(pathObj, pathObj.x, pathObj.y, "x", "y", "red");
                            canvas.genResizer(pathObj, pathObj.control1X, pathObj.control1Y, "control1X", "control1Y", "blue");
                            canvas.genResizer(pathObj, pathObj.control2X, pathObj.control2Y, "control2X", "control2Y", "lightBlue");
                            activePath = null;
                        }
                        },
                        { "start": function(x, y) {
                            var p = Qt.createQmlObject('import QtQuick 2.9; import QtQuick.Shapes 1.0; ShapePath {' +
                                                       'strokeColor: "black"; fillColor: "' + (root.fill ? 'green' : 'transparent') + '";'+
                                                       'strokeWidth: ' + widthSlider.value + ';' +
                                                       'startX: ' + x + '; startY: ' + y + ';' +
                                                       'PathQuad { x: ' + x + ' + 1; y: ' + y + ' + 1;' +
                                                       'controlX: ' + x + ' + 50; controlY: ' + y + ' + 50 } }',
                                                       root, "dynamic_visual_path");
                            shape.data.push(p);
                            activePath = p;
                        }, "move": function(x, y) {
                            if (!activePath)
                                return;
                            var pathObj = activePath.pathElements[0];
                            pathObj.x = x;
                            pathObj.y = y;
                        }, "end": function() {
                            canvas.genResizer(activePath, activePath.startX, activePath.startY, "startX", "startY", "red");
                            var pathObj = activePath.pathElements[0];
                            canvas.genResizer(pathObj, pathObj.x, pathObj.y, "x", "y", "red");
                            canvas.genResizer(pathObj, pathObj.controlX, pathObj.controlY, "controlX", "controlY", "blue");
                            activePath = null;
                        }
                        }
                    ];
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                canvas.funcs[root.mode].start(mouse.x, mouse.y);
            }
            onPositionChanged: {
                canvas.funcs[root.mode].move(mouse.x, mouse.y);
            }
            onReleased: {
                canvas.funcs[root.mode].end();
            }
        }

        Shape {
            id: shape
            anchors.fill: parent

            data: []
        }
    }
}
