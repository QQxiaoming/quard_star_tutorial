/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

/*
    A testbench for source items (and effects) that set smooth to false.

    Setting smooth to false doesn't make sense for all effects (like blurs).
*/

Window {
    id: window
    width: 1200
    height: 1000
    visible: true

    Flickable {
        anchors.fill: parent
        contentWidth: rowLayout.width
        contentHeight: rowLayout.implicitHeight

        GridLayout {
            id: rowLayout
            width: window.width
            columns: 4

            // Header
            Text {
                text: "Plain Image"
                Layout.alignment: Qt.AlignHCenter
                Layout.column: 1
            }

            Text {
                text: "... with standalone effect"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "... with layer effect"
                Layout.alignment: Qt.AlignHCenter
            }

            // ColorOverlay
            Text {
                text: "ColorOverlay"
                Layout.fillWidth: true
            }

            CellContainer {
                SourceImage {}
            }

            CellContainer {
                SourceImage {
                    visible: false
                }

                ColorOverlay {
                    width: parent.children[0].width
                    height: parent.children[0].height
                    source: parent.children[0]
                    color: "#aa0ff000"
                    scale: parent.children[0].scale
                    transformOrigin: Item.TopLeft
                }
            }

            CellContainer {
                SourceImage {
                    layer.enabled: true
                    layer.effect: ColorOverlay {
                        width: parent.children[0].width
                        height: parent.children[0].height
                        source: parent.children[0]
                        color: "#aa0ff000"
                    }
                }
            }

            // Colorize
            Text {
                text: "Colorize"
                Layout.fillWidth: true
            }

            CellContainer {
                SourceImage {}
            }

            CellContainer {
                SourceImage {
                    visible: false
                }

                Colorize {
                    width: parent.children[0].width
                    height: parent.children[0].height
                    source: parent.children[0]
                    hue: 0.5
                    scale: parent.children[0].scale
                    transformOrigin: Item.TopLeft
                }
            }

            CellContainer {
                SourceImage {
                    layer.enabled: true
                    layer.effect: Colorize {
                        width: parent.children[0].width
                        height: parent.children[0].height
                        source: parent.children[0]
                        hue: 0.5
                    }
                }
            }

            // BrightnessContrast
            Text {
                text: "BrightnessContrast"
                Layout.fillWidth: true
            }

            CellContainer {
                SourceImage {}
            }

            CellContainer {
                SourceImage {
                    visible: false
                }

                BrightnessContrast {
                    width: parent.children[0].width
                    height: parent.children[0].height
                    source: parent.children[0]
                    brightness: 0.5
                    scale: parent.children[0].scale
                    transformOrigin: Item.TopLeft
                }
            }

            CellContainer {
                SourceImage {
                    layer.enabled: true
                    layer.effect: BrightnessContrast {
                        width: parent.children[0].width
                        height: parent.children[0].height
                        source: parent.children[0]
                        brightness: 0.5
                    }
                }
            }

            // HueSaturation
            Text {
                text: "HueSaturation"
                Layout.fillWidth: true
            }

            CellContainer {
                SourceImage {}
            }

            CellContainer {
                SourceImage {
                    visible: false
                }

                HueSaturation {
                    width: parent.children[0].width
                    height: parent.children[0].height
                    source: parent.children[0]
                    saturation: 1.0
                    scale: parent.children[0].scale
                    transformOrigin: Item.TopLeft
                }
            }

            CellContainer {
                SourceImage {
                    layer.enabled: true
                    layer.effect: HueSaturation {
                        width: parent.children[0].width
                        height: parent.children[0].height
                        source: parent.children[0]
                        saturation: 1.0
                    }
                }
            }

            // Desaturate
            Text {
                text: "Desaturate"
                Layout.fillWidth: true
            }

            CellContainer {
                SourceImage {}
            }

            CellContainer {
                SourceImage {
                    visible: false
                }

                Desaturate {
                    width: parent.children[0].width
                    height: parent.children[0].height
                    source: parent.children[0]
                    desaturation: 0.85
                    scale: parent.children[0].scale
                    transformOrigin: Item.TopLeft
                }
            }

            CellContainer {
                SourceImage {
                    layer.enabled: true
                    layer.effect: Desaturate {
                        width: parent.children[0].width
                        height: parent.children[0].height
                        source: parent.children[0]
                        desaturation: 0.85
                    }
                }
            }

            // GammaAdjust
            Text {
                text: "GammaAdjust"
                Layout.fillWidth: true
            }

            CellContainer {
                SourceImage {}
            }

            CellContainer {
                SourceImage {
                    visible: false
                }

                // doesn't seem to do anything...
                GammaAdjust {
                    width: parent.children[0].width
                    height: parent.children[0].height
                    source: parent.children[0]
                    gamma: 10
                    scale: parent.children[0].scale
                    transformOrigin: Item.TopLeft
                }
            }

            CellContainer {
                SourceImage {
                    layer.enabled: true
                    layer.effect: GammaAdjust {
                        width: parent.children[0].width
                        height: parent.children[0].height
                        source: parent.children[0]
                        gamma: 10
                    }
                }
            }

            // LevelAdjust
            Text {
                text: "LevelAdjust"
                Layout.fillWidth: true
            }

            CellContainer {
                SourceImage {}
            }

            CellContainer {
                SourceImage {
                    visible: false
                }

                LevelAdjust {
                    width: parent.children[0].width
                    height: parent.children[0].height
                    source: parent.children[0]
                    minimumOutput: "#00ffffff"
                    maximumOutput: "#ff000000"
                    scale: parent.children[0].scale
                    transformOrigin: Item.TopLeft
                }
            }

            CellContainer {
                SourceImage {
                    layer.enabled: true
                    layer.effect: LevelAdjust {
                        width: parent.children[0].width
                        height: parent.children[0].height
                        source: parent.children[0]
                        minimumOutput: "#00ffffff"
                        maximumOutput: "#ff000000"
                    }
                }
            }
        }
    }
}
