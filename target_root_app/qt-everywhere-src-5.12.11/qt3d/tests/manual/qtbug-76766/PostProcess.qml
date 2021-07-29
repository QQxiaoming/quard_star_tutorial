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
import Qt3D.Extras 2.0

Entity {
    id: root

    property Texture colorTexture
    readonly property int samples : colorTexture ? colorTexture.samples : 1

    Effect {
        id: materialEffect
        techniques : [
            // OpenGL 3.2
            Technique {
                graphicsApiFilter {
                    api : GraphicsApiFilter.OpenGL
                    profile : GraphicsApiFilter.CoreProfile
                    minorVersion : 3
                    majorVersion : 2
                }

                renderPasses : RenderPass {
                    filterKeys : FilterKey { name : "pass"; value : "final" }

                    shaderProgram : ShaderProgram {
                        vertexShaderCode:
                            "#version 150 core

                            in vec3 vertexPosition;
                            out vec2 texCoords;

                            void main() {
                                texCoords = vec2(0.5) + vec2(0.5) * vertexPosition.xz;
                                gl_Position = vec4(vertexPosition.x, vertexPosition.z, 0.0, 1.0);
                            }"
                        fragmentShaderCode:
                            "#version 150 core

                            uniform sampler2DMS source;

                            in vec2 texCoords;
                            out vec4 fragColor;

                            void main() {
                                vec4 c = vec4(0.0);
                                c += texelFetch(source, ivec2(gl_FragCoord), 0);
                                fragColor = vec4(c.rgb / max(c.a, 0.01), c.a);
                            }"
                    }
                }
            }
        ]
    }

    Material {
        id: materialWithoutTexture

        parameters: [
            Parameter { name: "source"; value: colorTexture },
            Parameter { name: "samples"; value: root.samples }
        ]

        effect: materialEffect
    }

    PlaneMesh {
        id: planeMesh
        width: 2.0
        height: 2.0
        meshResolution: Qt.size(2, 2)
    }

    components : [
        planeMesh,
        materialWithoutTexture
    ]
}
