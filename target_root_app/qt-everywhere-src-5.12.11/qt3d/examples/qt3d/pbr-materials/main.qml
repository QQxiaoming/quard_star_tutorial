/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
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

import QtQuick 2.1
import Qt3D.Core 2.0
import Qt3D.Render 2.9
import Qt3D.Input 2.0
import Qt3D.Extras 2.9

Entity {
    id: root
    objectName: "root"

    // Use the renderer configuration specified in ForwardRenderer.qml
    // and render from the mainCamera
    components: [
        RenderSettings {
            activeFrameGraph: ForwardRenderer {
                camera: mainCamera
            }
        },
        // Event Source will be set by the Qt3DQuickWindow
        InputSettings { }
    ]

    BasicCamera {
        id: mainCamera
        position: Qt.vector3d( -5.17253, 2.95727, 6.65948 )
        viewCenter: Qt.vector3d( 6.73978, -2.50545, -10.6525 )
    }

    FirstPersonCameraController { camera: mainCamera }

    Lights { }

    Entity {
        components: [
            PointLight {
                enabled: parent.enabled
                color: "black"
                intensity: 0
            },
            EnvironmentLight {
                enabled: parent.enabled

                irradiance: TextureLoader {
                    source: "assets/cubemaps/default/default_irradiance.dds"
                    wrapMode {
                        x: WrapMode.ClampToEdge
                        y: WrapMode.ClampToEdge
                    }
                    generateMipMaps: false
                }
                specular: TextureLoader {
                    source: "assets/cubemaps/default/default_specular.dds"
                    wrapMode {
                        x: WrapMode.ClampToEdge
                        y: WrapMode.ClampToEdge
                    }
                    generateMipMaps: false
                }
            }
        ]
    }

    Entity {
        id: floor

        components: [
            Mesh {
                source: "assets/obj/plane-10x10.obj"
            },
            TexturedMetalRoughMaterial {
                baseColor: TextureLoader {
                    source: "assets/textures/ceramic_small_diamond/ceramic_small_diamond_basecolor.png"
                    format: Texture.SRGB8_Alpha8
                    generateMipMaps: true
                }
                metalness: TextureLoader { source: "assets/textures/ceramic_small_diamond/ceramic_small_diamond_metallic.png"; generateMipMaps: true }
                roughness: TextureLoader { source: "assets/textures/ceramic_small_diamond/ceramic_small_diamond_roughness.png"; generateMipMaps: true }
                normal: TextureLoader { source: "assets/textures/ceramic_small_diamond/ceramic_small_diamond_normal.png"; generateMipMaps: true }
                ambientOcclusion: TextureLoader { source: "assets/textures/ceramic_small_diamond/ceramic_small_diamond_ambient_occlusion.png" }
            }
        ]
    }

    TrefoilKnot {
        id: trefoilKnot
        y: 1.5
        z: -2
        scale: 0.2

        ParallelAnimation {
            loops: Animation.Infinite
            running: true

            NumberAnimation {
                target: trefoilKnot
                property: "theta"
                from: 0; to: 360
                duration: 5000
            }

            NumberAnimation {
                target: trefoilKnot
                property: "phi"
                from: 0; to: 360
                duration: 5000
            }
        }
    }

    Mesh {
        id: matSphere
        source: "assets/obj/material-sphere.obj"
    }

    Entity {
        id: sphere1

        components: [
            matSphere1Transform,
            matSphere,
            matSphere1Material
        ]

        Transform {
            id: matSphere1Transform
            translation: Qt.vector3d(-3, 0, 0)
            rotationY: -90
        }

        TexturedMetalRoughMaterial {
            id: matSphere1Material
            baseColor:  TextureLoader {
                source: "assets/textures/aluminium_random_brushed/aluminium_random_brushed_basecolor.png"
                format: Texture.SRGB8_Alpha8
                generateMipMaps: true
            }
            metalness: TextureLoader { source: "assets/textures/aluminium_random_brushed/aluminium_random_brushed_metallic.png"; generateMipMaps: true }
            roughness: TextureLoader { source: "assets/textures/aluminium_random_brushed/aluminium_random_brushed_roughness.png"; generateMipMaps: true}
            normal: TextureLoader { source: "assets/textures/aluminium_random_brushed/aluminium_random_brushed_normal.png"; generateMipMaps: true }
            ambientOcclusion: TextureLoader { source: "assets/textures/no-ao.png" }
        }
    }

    Entity {
        id: sphere2

        components: [
            matSphere2Transform,
            matSphere,
            matSphere2Material
        ]

        Transform {
            id: matSphere2Transform
            translation: Qt.vector3d(-1.5, 0, 0)
            rotationY: -90
        }

        TexturedMetalRoughMaterial {
            id: matSphere2Material
            baseColor:  TextureLoader {
                source: "assets/textures/american_walnut_crown_cut/american_walnut_crown_cut_basecolor.png"
                format: Texture.SRGB8_Alpha8
                generateMipMaps: true
            }
            metalness: TextureLoader { source: "assets/textures/american_walnut_crown_cut/american_walnut_crown_cut_metallic.png"; generateMipMaps: true }
            roughness: TextureLoader { source: "assets/textures/american_walnut_crown_cut/american_walnut_crown_cut_roughness.png"; generateMipMaps: true }
            normal: TextureLoader { source: "assets/textures/american_walnut_crown_cut/american_walnut_crown_cut_normal.png"; generateMipMaps: true }
            ambientOcclusion: TextureLoader { source: "assets/textures/no-ao.png" }
        }
    }

    Entity {
        id: sphere3

        components: [
            matSphere3Transform,
            matSphere,
            matSphere3Material
        ]

        Transform {
            id: matSphere3Transform
            translation: Qt.vector3d(0, 0, 0)
            rotationY: -90
        }

        TexturedMetalRoughMaterial {
            id: matSphere3Material
            baseColor:  TextureLoader {
                source: "assets/textures/ceramic_tiles_brown_tomato/ceramic_tiles_brown_tomato_basecolor.png"
                format: Texture.SRGB8_Alpha8
                generateMipMaps: true
            }
            metalness: TextureLoader { source: "assets/textures/ceramic_tiles_brown_tomato/ceramic_tiles_brown_tomato_metallic.png"; generateMipMaps: true }
            roughness: TextureLoader { source: "assets/textures/ceramic_tiles_brown_tomato/ceramic_tiles_brown_tomato_roughness.png"; generateMipMaps: true }
            normal: TextureLoader { source: "assets/textures/ceramic_tiles_brown_tomato/ceramic_tiles_brown_tomato_normal.png"; generateMipMaps: true }
            ambientOcclusion: TextureLoader { source: "assets/textures/no-ao.png" }
        }
    }

    Entity {
        id: sphere4

        components: [
            matSphere4Transform,
            matSphere,
            matSphere4Material
        ]

        Transform {
            id: matSphere4Transform
            translation: Qt.vector3d(1.5, 0, 0)
            rotationY: -90
        }

        TexturedMetalRoughMaterial {
            id: matSphere4Material
            baseColor:  TextureLoader {
                source: "assets/textures/copper_brushed/copper_brushed_basecolor.png"
                format: Texture.SRGB8_Alpha8
                generateMipMaps: true
            }
            metalness: TextureLoader { source: "assets/textures/copper_brushed/copper_brushed_metallic.png"; generateMipMaps: true }
            roughness: TextureLoader { source: "assets/textures/copper_brushed/copper_brushed_roughness.png"; generateMipMaps: true }
            normal: TextureLoader { source: "assets/textures/copper_brushed/copper_brushed_normal.png"; generateMipMaps: true }
            ambientOcclusion: TextureLoader { source: "assets/textures/no-ao.png" }
        }
    }

    Entity {
        id: sphere5

        components: [
            matSphere5Transform,
            matSphere,
            matSphere5Material
        ]

        Transform {
            id: matSphere5Transform
            translation: Qt.vector3d(3, 0, 0)
            rotationY: -90
        }

        TexturedMetalRoughMaterial {
            id: matSphere5Material
            baseColor:  TextureLoader {
                source: "assets/textures/gold_leaf_waste/gold_leaf_waste_basecolor.png"
                format: Texture.SRGB8_Alpha8
                generateMipMaps: true
            }
            metalness: TextureLoader { source: "assets/textures/gold_leaf_waste/gold_leaf_waste_metallic.png"; generateMipMaps: true }
            roughness: TextureLoader { source: "assets/textures/gold_leaf_waste/gold_leaf_waste_roughness.png"; generateMipMaps: true }
            normal: TextureLoader { source: "assets/textures/gold_leaf_waste/gold_leaf_waste_normal.png"; generateMipMaps: true }
            ambientOcclusion: TextureLoader { source: "assets/textures/no-ao.png" }
        }
    }
}
