/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
    width: 600; height: 480; color: "#f0f0f0"

    property int globalGesturePolicy : TapHandler.DragThreshold

    Flickable {
        id: flick
        anchors.fill: parent
        contentHeight: 600
        contentWidth: 1000
        objectName: "Flick"

        Repeater {
            model: flick.contentHeight/200
            Rectangle {
                width: flick.contentWidth
                height: 101
                x: 0
                y: index * 200
                border.color: "#808080"
                border.width: 1
                color: "transparent"
            }
        }

        Repeater {
            model: flick.contentWidth/200
            Rectangle {
                width: 101
                height: flick.contentHeight
                x: index * 200
                y: 0
                border.color: "#808080"
                border.width: 1
                color: "transparent"
            }
        }

        TextBox {
            x: 0; y: 0
            width: 100; height: 100
            label: "DragHandler"
            objectName: "dragSquircle1"
            DragHandler {

            }
        }

        TextBox {
            x: 100; y: 0
            width: 100; height: 100
            label: "TapHandler"
            color: queryColor(tap1.pressed)

            TapHandler {
                id: tap1
                gesturePolicy: root.globalGesturePolicy
            }
        }

        TextBox {
            x: 200; y: 0
            width: 100; height: 100
            label: "TapHandler\nDragHandler"
            color: queryColor(tap2.pressed)
            TapHandler {
                id: tap2
                gesturePolicy: root.globalGesturePolicy
            }
            DragHandler { }
        }

        TextBox {
            x: 300; y: 0
            width: 100; height: 100
            label: "DragHandler\nTapHandler"
            color: queryColor(tap3.pressed)
            DragHandler { }
            TapHandler {
                id: tap3
                gesturePolicy: root.globalGesturePolicy
            }
        }

        TextBox {
            x: 400; y: 0
            width: 100; height: 100
            label: "DragHandler"
            DragHandler { }

            TextBox {
                label: "TapHandler"
                x: (parent.width - width)/2
                y: 60
                color: queryColor(tap4.pressed)
                TapHandler {
                    id: tap4
                    gesturePolicy: root.globalGesturePolicy
                }
            }
        }

        TextBox {
            objectName: "dragSquircle5"
            x: 500; y: 0
            width: 100; height: 100
            label: "TapHandler"
            color: queryColor(tap5.pressed)
            CheckBox {
                id: ckGreedyDrag
                x: 10
                anchors.bottom: dragRect5.top
                label: " Greedy ↓"
                checked: true
            }
            TapHandler {
                id: tap5
                gesturePolicy: root.globalGesturePolicy
            }

            TextBox {
                id: dragRect5
                objectName: "dragRect5"
                label: "DragHandler"
                x: (parent.width - width)/2
                y: 60
                DragHandler {
                    grabPermissions: ckGreedyDrag ? DragHandler.CanTakeOverFromAnything :
                        DragHandler.CanTakeOverFromItems | DragHandler.CanTakeOverFromHandlersOfDifferentType | DragHandler.ApprovesTakeOverByAnything
                }
            }
        }


        TextBox {
            x: 0; y: 100
            width: 100; height: 100
            label: "No MouseArea"

            TextBox {
                objectName: "dragRect01"
                label: "DragHandler"
                x: (parent.width - width)/2
                y: 60
                DragHandler { }
            }
        }

        TextBox {
            id: r2
            label: "MouseArea"
            x: 100; y: 100
            width: 100; height: 100

            MouseArea {
                id: ma
                enabled: ckEnabled.checked
                drag.target: ckDrag.checked ? r2 : undefined
                drag.threshold: ckExtendDragThreshold.checked ? 50 : undefined
                anchors.fill: parent
            }
            Column {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                x: 10
                CheckBox {
                    id: ckEnabled
                    label: " Enabled"
                    checked: true
                }

                CheckBox {
                    id: ckDrag
                    label: " Drag"
                    checked: true
                }

                CheckBox {
                    id: ckExtendDragThreshold
                    label: " Extend threshold"
                    checked: false
                }
            }
        }
    }
}
