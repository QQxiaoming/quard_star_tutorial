/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

Item {
    id: root

    property url iconUrl: ""
    property int iconSize: 128
    property int iconMinimumSize: 8
    property int iconMaximumSize: 256

    RowLayout {
        anchors.fill: parent
        spacing: 2

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                anchors.fill: parent
                spacing: 2

                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: (parent.width - 2 * parent.spacing) / 3

                    border.color: "black"
                    border.width: 1
                    clip: true

                    Text {
                        z: parent.z + 1
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        font.bold: true

                        text: "faviconImage"
                    }

                    Image {
                        id: faviconImage
                        anchors.centerIn: parent

                        width: root.iconSize
                        height: width
                        sourceSize: Qt.size(width, height)

                        source: root.iconUrl

                        onStatusChanged: {
                            if (status == Image.Ready) {
                                grabToImage(function(result) {
                                    grabImage.source = result.url;
                                });
                            }

                            if (status == Image.Null)
                                grabImage.source = "";
                        }
                    }
                }

                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: (parent.width - 2 * parent.spacing) / 3

                    border.color: "black"
                    border.width: 1
                    clip: true

                    Text {
                        z: parent.z + 1
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        font.bold: true

                        text: "grabImage"
                    }

                    Image {
                        id: grabImage
                        anchors.centerIn: parent

                        width: root.iconSize
                        height: width

                        onStatusChanged: {
                            if (status == Image.Ready || status == Image.Null)
                                faviconCanvas.requestPaint();
                        }
                    }
                }

                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: (parent.width - 2 * parent.spacing) / 3

                    border.color: "black"
                    border.width: 1
                    clip: true

                    Text {
                        z: parent.z + 1
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        font.bold: true

                        text: "faviconCanvas"
                    }

                    Canvas {
                        id: faviconCanvas
                        anchors.centerIn: parent

                        width: root.iconSize
                        height: width

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.clearRect(0, 0, width, height);

                            if (grabImage.source == "")
                                return;

                            ctx.drawImage(grabImage, 0, 0, width, height);

                            var imageData = ctx.getImageData(width/2, height/2, width/2, height/2);
                            var pixel = imageData.data;

                            verbose.append("pixel(" + width/2 + ", " + height/2 + "): " + pixel[0] + ", " + pixel[1] + ", " + pixel[2]);
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 100

            Slider {
                id: faviconSizeSlider
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.bottom: faviconSizeSpin.top

                orientation: Qt.Vertical
                minimumValue: root.iconMinimumSize
                maximumValue: root.iconMaximumSize
                stepSize: 1
                tickmarksEnabled: true
                value: root.iconSize

                onValueChanged: {
                    if (pressed && value != root.iconSize)
                        root.iconSize = value;
                }
            }

            SpinBox {
                id: faviconSizeSpin
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom

                minimumValue: root.iconMinimumSize
                maximumValue: root.iconMaximumSize
                value: root.iconSize

                onEditingFinished: {
                    if (value != root.iconSize)
                        root.iconSize = value;
                }
            }
        }

        TextArea {
            id: verbose

            Layout.fillHeight: true
            Layout.preferredWidth: 310

            readOnly: true
            tabChangesFocus: true

            font.family: "Monospace"
            font.pointSize: 12

            textFormat: TextEdit.RichText
            frameVisible: false

            style: TextAreaStyle {
                backgroundColor: "lightgray"
            }
        }
    }
}
