/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

import QtGraphicalEffects 1.12
import QtQuick 2.12

Item {
    id: root
    width: PathView.view.delegateSize
    height: width
    z: PathView.zOrder

    transform: [
        Rotation {
            angle: root.PathView.rotateY
            origin.x: sourceImage.width / 2
            origin.y: sourceImage.height * 0.3
            axis.x: 0
            axis.y: 1
            axis.z: 0
        },
        Scale {
            xScale: 1.0
            yScale: root.PathView.scale
            origin.x: sourceImage.width / 2
            origin.y: sourceImage.height * 0.4
        }
    ]

    Image {
        id: sourceImage
        width: root.PathView.view.delegateSize
        height: width
        fillMode: Image.PreserveAspectFit
        source: "file:/" + docImagesDir + model.source

        Rectangle {
            x: (sourceImage.width - sourceImage.paintedWidth) / 2
            width: sourceImage.paintedWidth + (index == 6 ? 2 : 1)
            height: sourceImage.height
            color: "transparent"
            border.color: "#f4f4f4"
            antialiasing: true
            visible: !model.dark
        }
    }

    ShaderEffectSource {
        id: reflection
        sourceItem: sourceImage
        y: sourceItem.height
        width: sourceItem.width
        height: sourceItem.height

        transform: [
            Rotation {
                origin.x: reflection.width / 2
                origin.y: reflection.height / 2
                axis.x: 1
                axis.y: 0
                axis.z: 0
                angle: 180
            }
        ]
    }

    Rectangle {
        objectName: sourceImage.source.toString().slice(-20)
        x: (parent.width - sourceImage.paintedWidth) / 2// + (paintedWidthDiff > 0 ? 1.0 - paintedWidthDiff : 0)
        y: reflection.y
        width: sourceImage.paintedWidth + (index == 6 ? 2 : 1)
        height: sourceImage.paintedHeight

        // TODO: figure out how to get perfect x/width without using the current width hack
//        readonly property real paintedWidthDiff: sourceImage.paintedWidth - Math.floor(sourceImage.paintedWidth)

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.rgba(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.33)
            }
            GradientStop {
                // This determines the point at which the reflection fades out.
                position: 1.0
                color: backgroundColor
            }
        }
    }
}
