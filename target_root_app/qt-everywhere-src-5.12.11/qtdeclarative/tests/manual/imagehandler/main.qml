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

import QtQuick 2.8
import QtQuick.Window 2.0

Window {
    id: root
    width: 480
    height: 480
    visible: true
    Image {
        id: svgImage
        source: "large.svg"
        height: parent.height - column.height
        width: parent.width
        sourceSize.height: height
        sourceSize.width: width
        MouseArea {
            anchors.fill: parent
            onClicked: svgImage.source = "embeddedimage.svg"
        }
    }
    ListModel {
        id: imageFillModeModel
        ListElement { text: "Stretch"; fillMode: Image.Stretch }
        ListElement { text: "Preserve Aspect Fit"; fillMode: Image.PreserveAspectFit }
        ListElement { text: "Preserve Aspect Crop"; fillMode: Image.PreserveAspectCrop }
        ListElement { text: "Tile"; fillMode: Image.Tile }
        ListElement { text: "Tile Vertically"; fillMode: Image.TileVertically }
        ListElement { text: "Tile Horizontally"; fillMode: Image.TileHorizontally }
        ListElement { text: "Pad"; fillMode: Image.Pad }
    }
    Column {
        id: column
        height: 75
        anchors.bottom: parent.bottom
        Text {
            text: "Click the options below to change the fill mode.<br>Click the image to change the used image."
            font.pointSize: 16
        }

        Row {
            id: checkBoxesRow
            width: parent.width
            Repeater {
                model: imageFillModeModel
                Rectangle {
                    color: "lightGreen"
                    height: 50
                    width: 100
                    Text {
                        text: model.text
                        wrapMode: Text.Wrap
                        font.pointSize: 16
                        anchors.fill: parent
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                svgImage.fillMode = model.fillMode
                                if (svgImage.fillMode === Image.Tile || svgImage.fillMode === Image.TileHorizontally ||
                                    svgImage.fillMode === Image.TileVertically)
                                    svgImage.sourceSize.height = svgImage.sourceSize.width = 300
                                else {
                                    svgImage.sourceSize.height = svgImage.height
                                    svgImage.sourceSize.width = svgImage.width
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
