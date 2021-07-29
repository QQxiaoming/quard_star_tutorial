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

#pragma include light.inc.frag

void adsModel(const in vec3 worldPos,
              const in vec3 worldNormal,
              const in vec3 worldView,
              const in float shininess,
              out vec3 diffuseColor,
              out vec3 specularColor)
{
    diffuseColor = vec3(0.0);
    specularColor = vec3(0.0);

    // We perform all work in world space
    vec3 n = normalize(worldNormal);
    vec3 s = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        float att = 1.0;
        float sDotN = 0.0;

        if (lights[i].type != TYPE_DIRECTIONAL) {
            // Point and Spot lights

            // Light position is already in world space
            vec3 sUnnormalized = lights[i].position - worldPos;
            s = normalize(sUnnormalized); // Light direction

            // Calculate the attenuation factor
            sDotN = dot(s, n);
            if (sDotN > 0.0) {
                if (lights[i].constantAttenuation != 0.0
                 || lights[i].linearAttenuation != 0.0
                 || lights[i].quadraticAttenuation != 0.0) {
                    float dist = length(sUnnormalized);
                    att = 1.0 / (lights[i].constantAttenuation +
                                 lights[i].linearAttenuation * dist +
                                 lights[i].quadraticAttenuation * dist * dist);
                }

                // The light direction is in world space already
                if (lights[i].type == TYPE_SPOT) {
                    // Check if fragment is inside or outside of the spot light cone
                    if (degrees(acos(dot(-s, lights[i].direction))) > lights[i].cutOffAngle)
                        sDotN = 0.0;
                }
            }
        } else {
            // Directional lights
            // The light direction is in world space already
            s = normalize(-lights[i].direction);
            sDotN = dot(s, n);
        }

        // Calculate the diffuse factor
        float diffuse = max(sDotN, 0.0);

        // Calculate the specular factor
        float specular = 0.0;
        if (diffuse > 0.0 && shininess > 0.0) {
            float normFactor = (shininess + 2.0) / 2.0;
            vec3 r = reflect(-s, n);   // Reflection direction in world space
            specular = normFactor * pow(max(dot(r, worldView), 0.0), shininess);
        }

        // Accumulate the diffuse and specular contributions
        diffuseColor += att * lights[i].intensity * diffuse * lights[i].color;
        specularColor += att * lights[i].intensity * specular * lights[i].color;
    }
}

vec4 phongFunction(const in vec4 ambient,
                   const in vec4 diffuse,
                   const in vec4 specular,
                   const in float shininess,
                   const in vec3 worldPosition,
                   const in vec3 worldView,
                   const in vec3 worldNormal)
{
    // Calculate the lighting model, keeping the specular component separate
    vec3 diffuseColor, specularColor;
    adsModel(worldPosition, worldNormal, worldView, shininess, diffuseColor, specularColor);

    // Combine spec with ambient+diffuse for final fragment color
    vec3 color = (ambient.rgb + diffuseColor) * diffuse.rgb
               + specularColor * specular.rgb;

    return vec4(color, diffuse.a);
}
