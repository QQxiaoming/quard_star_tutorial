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

import QtQuick 2.7
import QtWayland.Compositor 1.3
import QtQuick.Window 2.3
import QtQuick.Controls 2.0

WaylandCompositor {
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            id: win

            property int pixelWidth: width * screen.devicePixelRatio
            property int pixelHeight: height * screen.devicePixelRatio

            visible: true
            width: 1280
            height: 720

            Grid {
                id: grid

                property bool overview: true
                property int selected: 0
                property int selectedColumn: selected % columns
                property int selectedRow: selected / columns

                anchors.fill: parent
                columns: Math.ceil(Math.sqrt(toplevels.count))
                transform: [
                    Scale {
                        xScale: grid.overview ? (1.0/grid.columns) : 1
                        yScale: grid.overview ? (1.0/grid.columns) : 1
                        Behavior on xScale { PropertyAnimation { easing.type: Easing.InOutQuad; duration: 200 } }
                        Behavior on yScale { PropertyAnimation { easing.type: Easing.InOutQuad; duration: 200 } }
                    },
                    Translate {
                        x: grid.overview ? 0 : win.width * -grid.selectedColumn
                        y: grid.overview ? 0 : win.height * -grid.selectedRow
                        Behavior on x { PropertyAnimation { easing.type: Easing.InOutQuad; duration: 200 } }
                        Behavior on y { PropertyAnimation { easing.type: Easing.InOutQuad; duration: 200 } }
                    }
                ]

                Repeater {
                    model: toplevels
                    Item {
                        width: win.width
                        height: win.height
                        ShellSurfaceItem {
                            anchors.fill: parent
                            shellSurface: xdgSurface
                            autoCreatePopupItems: true
                            sizeFollowsSurface: false
                            onSurfaceDestroyed: toplevels.remove(index)
                        }
                        MouseArea {
                            enabled: grid.overview
                            anchors.fill: parent
                            onClicked: {
                                grid.selected = index;
                                grid.overview = false;
                            }
                        }
                    }
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                text: "Toggle overview";
                onClicked: grid.overview = !grid.overview
            }

            Shortcut { sequence: "space"; onActivated: grid.overview = !grid.overview }
            Shortcut { sequence: "right"; onActivated: grid.selected = Math.min(grid.selected+1, toplevels.count-1) }
            Shortcut { sequence: "left"; onActivated: grid.selected = Math.max(grid.selected-1, 0) }
            Shortcut { sequence: "up"; onActivated: grid.selected = Math.max(grid.selected-grid.columns, 0) }
            Shortcut { sequence: "down"; onActivated: grid.selected = Math.min(grid.selected+grid.columns, toplevels.count-1) }
        }
    }

    ListModel { id: toplevels }

    XdgShell {
        onToplevelCreated: {
            toplevels.append({xdgSurface});
            toplevel.sendFullscreen(Qt.size(win.pixelWidth, win.pixelHeight));
        }
    }
}
