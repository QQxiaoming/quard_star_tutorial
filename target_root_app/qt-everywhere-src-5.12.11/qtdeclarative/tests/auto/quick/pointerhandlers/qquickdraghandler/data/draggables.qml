/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

Item {
    id: root
    objectName: "Draggables"
    width: 640
    height: 480

    Repeater {
        model: 2

        Rectangle {
            id: ball
            objectName: "Ball " + (index + 1)
            color: dragHandler.active ? "blue" : "lightsteelblue"
            width: 80; height: 80; x: 200 + index * 200; y: 200; radius: width / 2
            onParentChanged: console.log(this + " parent " + parent)

            DragHandler {
                id: dragHandler
                objectName: "DragHandler " + (index + 1)
            }

            Text {
                color: "white"
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: ball.objectName + "\n" + dragHandler.centroid.position.x.toFixed(1) + "," + dragHandler.centroid.position.y.toFixed(1)
            }
        }
    }
}
