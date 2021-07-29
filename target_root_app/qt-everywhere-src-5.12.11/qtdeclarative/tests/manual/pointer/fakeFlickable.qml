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
import "content"

Rectangle {
    color: "#444"
    width: 480
    height: 480

    FakeFlickable {
        id: ff
        anchors.fill: parent
        anchors.rightMargin: rightSB.width
        Row {
            Item {
                width: 100
                height: 400
                Slider {
                    id: slider
                    label: "font size"
                    anchors.fill: parent
                    maximumValue: 36
                    value: 14
                }
            }
            Text {
                id: text
                color: "beige"
                font.family: "mono"
                font.pointSize: slider.value
                onTextChanged: console.log("text geom " + width + "x" + height +
                    ", parent " + parent + " geom " + parent.width + "x" + parent.height)
            }
        }


        onFlickStarted: console.log("flick started with velocity " + velocity)
        onFlickEnded: console.log("flick ended with velocity " + velocity)

        Component.onCompleted: {
            var request = new XMLHttpRequest()
            request.open('GET', 'content/FakeFlickable.qml')
            request.onreadystatechange = function(event) {
                if (request.readyState === XMLHttpRequest.DONE)
                    text.text = request.responseText
            }
            request.send()
        }
    }

    ScrollBar {
        id: rightSB
        objectName: "rightSB"
        flick: ff
        height: parent.height - width
        anchors.right: parent.right
    }

    ScrollBar {
        id: bottomSB
        objectName: "bottomSB"
        flick: ff
        width: parent.width - height
        anchors.bottom: parent.bottom
    }

    Rectangle {
        id: cornerCover
        color: "lightgray"
        width: rightSB.width
        height: bottomSB.height
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
    }
}
