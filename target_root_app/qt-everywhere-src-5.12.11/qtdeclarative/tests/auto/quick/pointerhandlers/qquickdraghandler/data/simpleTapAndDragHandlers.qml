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

Rectangle {
    id: root
    width: 900
    height: 850
    objectName: "root"
    color: "#222222"

    Row {
        objectName: "row"
        anchors.fill: parent
        spacing: 10
        Rectangle {
            width: 50
            height: 50
            color: "aqua"
            objectName: "dragAndTap"
            DragHandler {
                objectName: "drag"
            }
            TapHandler {
                objectName: "tap"
                gesturePolicy: TapHandler.DragThreshold
            }
        }
        Rectangle {
            width: 50
            height: 50
            color: "aqua"
            objectName: "tapAndDrag"
            TapHandler {
                objectName: "tap"
                gesturePolicy: TapHandler.DragThreshold
            }
            DragHandler {
                objectName: "drag"
            }
        }

        Rectangle {
            color: "aqua"
            width: 50
            height: 50
            objectName: "dragAndTapNotSiblings"
            DragHandler {
                objectName: "drag"
            }
            Rectangle {
                color: "blue"
                width: 30
                height: 30
                anchors.centerIn: parent
                TapHandler {
                    objectName: "tap"
                    gesturePolicy: TapHandler.DragThreshold
                }
            }
        }
        Rectangle {
            color: "aqua"
            width: 50
            height: 50
            objectName: "tapAndDragNotSiblings"
            TapHandler {
                objectName: "tap"
                gesturePolicy: TapHandler.DragThreshold
            }
            Rectangle {
                color: "blue"
                x: 10
                y: 10
                width: 30
                height: 30
                DragHandler {
                    objectName: "drag"
                }
            }
        }


    }
}
