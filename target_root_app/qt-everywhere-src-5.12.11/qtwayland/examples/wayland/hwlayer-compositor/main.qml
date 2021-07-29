/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
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

import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtWayland.Compositor 1.3

WaylandCompositor {
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            color: "tomato"
            id: win
            width: 1024
            height: 768
            visible: true
            Rectangle {
                color: "lightgreen"
                anchors.centerIn: parent
                width: parent.width / 3
                height: parent.width / 3
                NumberAnimation on rotation {
                    id: rotationAnimation
                    running: false
                    from: 0
                    to: 90
                    loops: Animation.Infinite
                    duration: 1000
                }
            }
                Repeater {
                    model: shellSurfaces
                    ShellSurfaceItem {
                        id: waylandItem
                        onSurfaceDestroyed: shellSurfaces.remove(index)
                        shellSurface: shSurface
                        WaylandHardwareLayer {
                            stackingLevel: level
                            Component.onCompleted: console.log("Added hardware layer with stacking level", stackingLevel);
                        }
                        Component.onCompleted: console.log("Added wayland quick item");
                        Behavior on x {
                            PropertyAnimation {
                                easing.type: Easing.OutBounce
                                duration: 1000
                            }
                        }
                        Timer {
                            interval: 2000; running: animatePosition; repeat: true
                            onTriggered: waylandItem.x = waylandItem.x === 0 ? win.width - waylandItem.width : 0
                        }
                        Behavior on opacity {
                            PropertyAnimation {
                                duration: 1000
                            }
                        }
                        Timer {
                            interval: 2000; running: animateOpacity; repeat: true
                            onTriggered: waylandItem.opacity = waylandItem.opacity === 1 ? 0 : 1
                        }
                    }
            }
            Column {
                anchors.bottom: parent.bottom
                Repeater {
                    model: shellSurfaces
                    Row {
                        Label {
                            anchors.verticalCenter: parent.verticalCenter
                            leftPadding: 15
                            rightPadding: 15
                            text: "Surface " + index
                        }
                        CheckBox {
                            text: "Animate position"
                            checked: animatePosition
                            onClicked: animatePosition = !animatePosition
                        }
                        CheckBox {
                            text: "Animate Opacity"
                            checked: animateOpacity
                            onClicked: animateOpacity = !animateOpacity
                        }
                        Label {
                            text: "Stacking level"
                        }
                        SpinBox {
                            value: level
                            onValueModified: level = value;
                        }
                        Button {
                            text: "Kill"
                            onClicked: shSurface.surface.client.close()
                        }
                    }
                }
                CheckBox {
                    text: "Rotation"
                    checked: rotationAnimation.running
                    onClicked: rotationAnimation.running = !rotationAnimation.running
                    padding: 30
                }
            }
        }
    }
    ListModel { id: shellSurfaces }
    function addShellSurface(shellSurface) {
        shellSurfaces.append({shSurface: shellSurface, animatePosition: false, animateOpacity: false, level: 0});
    }
    XdgShell { onToplevelCreated: addShellSurface(xdgSurface) }
    XdgShellV6 { onToplevelCreated: addShellSurface(xdgSurface) }
    WlShell { onWlShellSurfaceCreated: addShellSurface(shellSurface) }
}
