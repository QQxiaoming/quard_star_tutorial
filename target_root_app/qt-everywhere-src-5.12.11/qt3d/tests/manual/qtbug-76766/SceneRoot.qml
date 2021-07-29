/****************************************************************************
**
** Copyright (C) 2019 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Extras 2.0
import Qt3D.Input 2.0
import QtQuick 2.0 as QQ2
import QtQuick 2.12


Entity {
    id: sceneRoot

    Camera {
        id: camera
        projectionType: CameraLens.OrthographicProjection
        left: -2
        right: 2
        top: 2
        bottom:  -2
        nearPlane: 0.1
        farPlane: 1000.0
        position: Qt.vector3d(0.0, 0.0, 1.0)
        upVector: Qt.vector3d(0.0, 1.0, 0.0)
        viewCenter: Qt.vector3d(0.0, 0.0, 0.0)
    }

    RenderTarget {
        id: renderTargetFBO
        attachments : [
            RenderTargetOutput {
                attachmentPoint : RenderTargetOutput.Color0
                texture : Texture2DMultisample {
                    id : colorAttachment
                    width : scene3d.width
                    height : scene3d.height
                    format : Texture.RGBA8_UNorm
                    generateMipMaps : false
                    samples: 1
                }
            },
            RenderTargetOutput {
                attachmentPoint : RenderTargetOutput.Depth
                texture : Texture2DMultisample {
                    width : scene3d.width
                    height : scene3d.height
                    format : Texture.D32F
                    generateMipMaps : false
                    samples: 1
                }
            }
        ]
    }

    components: [
        RenderSettings {
            activeFrameGraph: FrameGraph {
                id: framegraph
                camera: camera
                fbo: renderTargetFBO
            }
            renderPolicy: RenderSettings.Always
        }
    ]

    Entity {
        id: firstPassEntities

        Entity {
            components: [
                Transform {
                    translation: Qt.vector3d(0,0,-2)
                },
                Material1 {
                    color: "red"
                },
                SphereMesh {
                }
            ]
        }


        Entity {
            components: [
                Transform {
                    translation: Qt.vector3d(1,0,-1)
                },
                Material2 {
                    color: "green"
                },
                SphereMesh {
                }
            ]
        }
    }

    Entity {
        id: secondPassEntities
        components: [framegraph.layer]

        Entity {
            components: [
                Transform {
                    translation: Qt.vector3d(0,0,-4)
                    scale: 0.5
                },
                Material1 {
                    ColorAnimation on color {
                        from: "black"
                        to: "purple"
                        duration: 2000
                        loops: Animation.Infinite
                    }
                },
                SphereMesh {
                }
            ]
        }


        Entity {
            components: [
                Transform {
                    translation: Qt.vector3d(1,0,-3)
                    scale: 0.5
                },
                Material2 {
                    color: "orange"
                },
                SphereMesh {
                }
            ]
        }
    }

    PostProcess {
        colorTexture: colorAttachment
    }
}
