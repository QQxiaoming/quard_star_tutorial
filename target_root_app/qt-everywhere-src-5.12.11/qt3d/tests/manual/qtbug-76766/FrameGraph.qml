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

import Qt3D.Core 2.9
import Qt3D.Render 2.9

RenderSurfaceSelector {
    id: surfaceSelector

    readonly property Layer layer: Layer {
        recursive: true
    }

    property alias camera: cameraSelector.camera
    property alias clearColor: clearBuffers.clearColor
    property alias fbo : renderToFboSelector.target

    Viewport {
        normalizedRect: Qt.rect(0.0, 0.0, 1.0, 1.0)

        // 1st: render scene to texture
        RenderTargetSelector {
            id : renderToFboSelector

            RenderStateSet {
                renderStates: [
                    MultiSampleAntiAliasing {},
                    DepthTest {
                        depthFunction: DepthTest.Less
                    },
                    CullFace{
                        mode: CullFace.Back
                    }
                ]

                ClearBuffers {
                    id: clearBuffers
                    buffers: ClearBuffers.ColorDepthBuffer
                    NoDraw {}
                }

                CameraSelector {
                    id: cameraSelector

                    LayerFilter {
                        layers: [layer]
                        filterMode: LayerFilter.DiscardAllMatchingLayers
                        TechniqueFilter {
                            RenderPassFilter {
                                matchAny: FilterKey { name: "pass"; value: 0 }
                            }
                            SortPolicy {
                                sortTypes: [SortPolicy.BackToFront]
                                RenderPassFilter {
                                    matchAny: FilterKey { name: "pass"; value: 1 }
                                }
                            }
                        }
                    }

                    RenderStateSet {
                        renderStates: [
                            DepthTest {
                                depthFunction: DepthTest.Always
                            }
                        ]
                        LayerFilter {
                            layers: [layer]
                            filterMode: LayerFilter.AcceptAnyMatchingLayers
                            TechniqueFilter {
                                RenderPassFilter {
                                    matchAny: FilterKey { name: "pass"; value: 0 }
                                }
                                SortPolicy {
                                    sortTypes: [SortPolicy.BackToFront]
                                    RenderPassFilter {
                                        matchAny: FilterKey { name: "pass"; value: 1 }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        RenderStateSet {
            renderStates: [
                DepthTest {depthFunction: DepthTest.Always},
                BlendEquation {},
                BlendEquationArguments {
                    sourceRgb: BlendEquationArguments.One
                    sourceAlpha: BlendEquationArguments.One
                    destinationRgb: BlendEquationArguments.Zero
                    destinationAlpha: BlendEquationArguments.Zero
                }
            ]

            RenderPassFilter {
                matchAny : FilterKey { name : "pass"; value : "final" }
            }
        }
    }
}
