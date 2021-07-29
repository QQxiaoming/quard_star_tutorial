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
import "qrc:/quick/shared/" as Examples
import "content"

Rectangle {
    id: root
    width: 400
    height: 400
    objectName: "root"
    color: "#222222"

    Flickable {
        anchors.fill: parent
        anchors.margins: 10
        anchors.topMargin: 40
        contentHeight: 600
        contentWidth: 600
        pressDelay: pressDelayCB.checked ? 1000 : 0

        Column {
            spacing: 6
            Rectangle {
                radius: 5
                width: parent.width - 12
                height: pressDelayCB.implicitHeight + 12
                x: 6
                color: "lightgray"
                Examples.CheckBox {
                    x: 6; y: 6
                    id: pressDelayCB
                    text: "press delay"
                }
            }


            Row {
                spacing: 6
                Slider {
                    label: "DragHandler"
                    value: 49; width: 100; height: 400
                }
                MouseAreaSlider {
                    label: "MouseArea"
                    value: 49; width: 100; height: 400
                }
                Column {
                    spacing: 6
                    MouseAreaButton {
                        text: "MouseArea"
                    }
                    MptaButton {
                        text: "MultiPointTouchArea"
                    }
                    MptaButton {
                        text: "MultiPointTouchArea"
                    }
                    TapHandlerButton {
                        text: "TapHandler"
                    }
                    TapHandlerButton {
                        text: "TapHandler"
                    }
                }
            }
        }
    }
}
