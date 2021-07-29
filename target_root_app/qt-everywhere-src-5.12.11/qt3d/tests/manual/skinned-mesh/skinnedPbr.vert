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

#version 150

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec4 vertexTangent;
in vec2 vertexTexCoord;
in ivec4 vertexJointIndices;
in vec4 vertexJointWeights;

out vec3 worldPosition;
out vec3 worldNormal;
out vec4 worldTangent;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

const int maxJoints = 100;
uniform mat4 skinningPalette[maxJoints];

void main()
{
    // Pass the texture coordinates through
    texCoord = vertexTexCoord;

    // Perform the skinning
    mat4 skinningMatrix = skinningPalette[vertexJointIndices[0]] * vertexJointWeights[0];
    skinningMatrix     += skinningPalette[vertexJointIndices[1]] * vertexJointWeights[1];
    skinningMatrix     += skinningPalette[vertexJointIndices[2]] * vertexJointWeights[2];
    skinningMatrix     += skinningPalette[vertexJointIndices[3]] * vertexJointWeights[3];

    vec4 skinnedPosition = skinningMatrix * vec4(vertexPosition, 1.0);
    vec3 skinnedNormal = vec3(skinningMatrix * vec4(vertexNormal, 0.0));
    vec3 skinnedTangent = vec3(skinningMatrix * vec4(vertexTangent.xyz, 0.0));

    // Transform position, normal, and tangent to world space
    worldPosition = vec3(modelMatrix * skinnedPosition);
    worldNormal = normalize(modelNormalMatrix * skinnedNormal);
    worldTangent.xyz = normalize(vec3(modelMatrix * vec4(skinnedTangent, 0.0)));
    worldTangent.w = vertexTangent.w;

    gl_Position = mvp * skinnedPosition;
}
