/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qardboard 1.0

Entity {
    id: root
    property real convergence: 2000.0
    property real eyeSeparation: 35.0
    property real aspectRatio: _window.width / _window.height
    property real fieldOfView: 60.0
    property real nearPlane: 1.
    property real farPlane: 1000.0

    property vector3d position: Qt.vector3d(0.0, 0.0, 0.0)
    property vector3d viewDirection: Qt.vector3d(0.0, 0.0, 1.0)
    property vector3d viewUp: Qt.vector3d(0.0, 1.0, 0.0)
    property alias viewMatrix: eyeTransform.matrix

    readonly property real _fov2: Math.tan(fieldOfView * Math.PI / 180 * 0.5)
    readonly property real top: nearPlane * _fov2
    readonly property real a: aspectRatio * _fov2 * convergence

    DeviceOrientation {
        id: orientation
        enabled: true
    }

    CameraLens {
        id: leftEyeLens
        projectionType: CameraLens.FrustumProjection
        nearPlane : root.nearPlane
        farPlane : root.farPlane
        left: -(a - eyeSeparation * 0.5) * nearPlane / convergence
        right: (a + eyeSeparation * 0.5) * nearPlane / convergence
        top: root.top
        bottom: -root.top
    }

    CameraLens {
        id: rightEyeLens
        projectionType: CameraLens.FrustumProjection
        nearPlane : root.nearPlane
        farPlane : root.farPlane
        left: -(a + eyeSeparation * 0.5) * nearPlane / convergence
        right: (a - eyeSeparation * 0.5) * nearPlane / convergence
        top: root.top
        bottom: -root.top
    }

    Transform {
        id: eyeTransform

        function computeMatrix(roll, pitch, yaw) {
            var n = Qt.matrix4x4()
            n.rotate(-roll, Qt.vector3d(1, 0, 0))
            n.rotate(yaw, Qt.vector3d(0, 1, 0))
            var vd = n.times(stereoCamera.viewDirection)
            var vu = n.times(stereoCamera.viewUp)
            var vc = stereoCamera.position.minus(vd)

            var m = Qt.matrix4x4();
            m.translate(stereoCamera.position)
            var zAxis = stereoCamera.position.minus(vc).normalized()
            var xAxis = vu.crossProduct(zAxis).normalized();
            var yAxis = zAxis.crossProduct(xAxis);
            var r = Qt.matrix4x4(xAxis.x, yAxis.x, zAxis.x, 0,
                                 xAxis.y, yAxis.y, zAxis.y, 0,
                                 xAxis.z, yAxis.z, zAxis.z, 0,
                                 0, 0, 0, 1)
            return m.times(r);
        }

        matrix: computeMatrix(orientation.roll, orientation.pitch, orientation.yaw)
    }

    components: [ eyeTransform ]

    property Entity leftCamera: Entity {
        components: [ leftEyeLens ]
    }

    property Entity rightCamera: Entity {
        id: rightCameraEntity
        components: [ rightEyeLens ]
    }
}
