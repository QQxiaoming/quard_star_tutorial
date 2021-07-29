/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
    width: image.implicitWidth; height: image.implicitHeight
    Image {
        id: image
        anchors.centerIn: parent
        source: "resources/joystick-outer-case-pov.jpg"
        property real margin: 50

        Image {
            id: knob
            source: "resources/redball.png"
            DragHandler {
                id: dragHandler
                xAxis {
                    minimum: image.margin
                    maximum: image.width - image.margin - knob.width
                }
                yAxis {
                    minimum: image.margin
                    maximum: image.height - image.margin - knob.height
                }
            }

            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.verticalCenter
            }
            states: [
                State {
                    when: dragHandler.active
                    AnchorChanges {
                        target: knob
                        anchors.horizontalCenter: undefined
                        anchors.verticalCenter: undefined
                    }
                }
            ]
            transitions: [
                Transition {
                    AnchorAnimation { easing.type: Easing.OutElastic }
                }
            ]
        }
    }
}
