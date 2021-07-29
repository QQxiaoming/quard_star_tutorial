/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

Item {
    id: root
    width: 640
    height: 480
    objectName: "qtbug86402Container"

    property bool finished

    Item {
        id: popup
        objectName: "popup"
        width: 200
        height: 200

        Rectangle {
            id: rect
            objectName: "rect"
            implicitWidth: 100
            implicitHeight: 100
            color: "blue"

            Item {
                id: ripple
                objectName: "ripple"
                anchors.fill: parent
                visible: false

                Rectangle {
                    id: rippleBox
                    objectName: "rippleBox"
                    property real cx
                    property real cy
                    x: cx - width / 2
                    y: cy - height / 2
                    width: 0
                    height: width
                    radius: width / 2
                    color: Qt.darker("red", 1.8)
                }
                layer.effect: ShaderEffect {
                    id: mask
                    objectName: "shaderEffect"
                    property variant source
                    property variant mask: rect

                    fragmentShader: "
                        uniform lowp sampler2D source;
                        uniform lowp sampler2D mask;
                        varying highp vec2 qt_TexCoord0;
                        void main() {
                            gl_FragColor = texture2D(source, qt_TexCoord0) * texture2D(mask, qt_TexCoord0).a;
                        }"
                }
            }

            SequentialAnimation {
                id: rippleStartAnimation
                running: popup.visible
                onFinished: {
                    popup.parent = null
                    rippleEndAnimation.start()
                }

                ScriptAction {
                    script: {
                        rippleBox.width = 0
                        rippleBox.opacity = 0.3
                        ripple.visible = true
                        ripple.layer.enabled = true
                    }
                }
                NumberAnimation {
                    target: rippleBox
                    property: "width"
                    from: 0
                    to: Math.max(rect.width,
                                 rect.height) * 2.2
                    duration: 100
                }
            }
            SequentialAnimation {
                id: rippleEndAnimation

                onFinished: root.finished = true

                //Causes Crash on QT Versions > 5.12.5
                NumberAnimation {
                    target: rippleBox
                    property: "opacity"
                    to: 0
                    duration: 100
                }
                ScriptAction {
                    script: {
                        rippleBox.opacity = 0
                        ripple.layer.enabled = false
                        ripple.visible = false
                    }
                }
            }
        }
    }
}
