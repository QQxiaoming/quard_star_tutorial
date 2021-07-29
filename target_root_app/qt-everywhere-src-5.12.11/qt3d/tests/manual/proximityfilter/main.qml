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

import QtQuick 2.9
import Qt3D.Core 2.10
import Qt3D.Render 2.10
import Qt3D.Input 2.0
import Qt3D.Extras 2.9

Entity {
    id: sceneRoot

    Camera {
        id: camera
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 45
        aspectRatio: 16/9
        nearPlane : 0.1
        farPlane : 1000.0
        position: Qt.vector3d( 0.0, 0.0, 150.0 )
        upVector: Qt.vector3d( 0.0, 1.0, 0.0 )
        viewCenter: Qt.vector3d( 0.0, 0.0, 0.0 )
    }

    OrbitCameraController { camera: camera }

    components: [
        RenderSettings {
            activeFrameGraph: RenderSurfaceSelector {
                Viewport {
                    normalizedRect: Qt.rect(0, 0, 1, 1)

                    ClearBuffers {
                        buffers: ClearBuffers.ColorDepthBuffer
                        clearColor: Qt.rgba(0.6, 0.6, 0.6, 1.0)
                        NoDraw {}
                    }

                    FrustumCulling {
                        CameraSelector {
                            camera: camera
                            ProximityFilter {
                                entity: proximityTarget
                                distanceThreshold: 30
                            }
                        }
                    }
                }
            }
        },
        // Event Source will be set by the Qt3DQuickWindow
        InputSettings { }
    ]

    SphereMesh {
        id: sphereMesh
    }

    PhongMaterial {
        id: phongMaterial
        diffuse: "orange"
    }

    NodeInstantiator {
        id: instantiator
        model: 64
        Entity {
            readonly property real angle: Math.PI * 2.0 * model.index % 8
            readonly property real radius: 20
            readonly property real verticalStep: 10
            readonly property color meshColor: Qt.hsla(model.index / instantiator.count, 0.5, 0.5, 1.0);

            readonly property Transform transform: Transform {
                translation: Qt.vector3d(radius * Math.cos(angle),
                                         (-(instantiator.count / (8 * 2)) + model.index / 8) * verticalStep,
                                         radius * Math.sin(angle))
            }
            readonly property Material material: Material {
                effect: phongMaterial.effect
                parameters: Parameter { name: "kd"; value: meshColor }
            }
            readonly property SphereMesh mesh: sphereMesh
            components: [ transform, mesh, material ]
        }
    }

    Entity {
        id: proximityTarget
        readonly property Transform transform: Transform
        {
            property real y: 0;
            SequentialAnimation on y {
                NumberAnimation { from: -50; to: 50; duration: 2000; easing.type: Easing.InOutQuart }
                NumberAnimation { from: 50; to: -50; duration: 2000; easing.type: Easing.InOutQuart }
                loops: Animation.Infinite
            }
            translation: Qt.vector3d(0.0, y, 0.0)
        }
        components: [ sphereMesh, phongMaterial, transform ]
    }
}
