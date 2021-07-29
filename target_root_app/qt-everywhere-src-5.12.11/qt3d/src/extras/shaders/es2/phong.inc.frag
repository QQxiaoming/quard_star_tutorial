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

void adsModel(const in FP vec3 vpos, const in FP vec3 vnormal, const in FP vec3 vview, const in FP float shininess,
              out FP vec3 diffuseColor, out FP vec3 specularColor)
{
    diffuseColor = vec3(0.0);
    specularColor = vec3(0.0);

    FP vec3 n = normalize( vnormal );

    FP vec3 s;
    Light light;
    for (int i = 0; i < lightCount; ++i) {
        if (i == 0)
            light = lights[0];
        else if (i == 1)
            light = lights[1];
        else if (i == 2)
            light = lights[2];
        else if (i == 3)
            light = lights[3];
        else if (i == 4)
            light = lights[4];
        else if (i == 5)
            light = lights[5];
        else if (i == 6)
            light = lights[6];
        else if (i == 7)
            light = lights[7];

        FP float att = 1.0;
        if ( light.type != TYPE_DIRECTIONAL ) {
            s = light.position - vpos;
            if (light.constantAttenuation != 0.0
              || light.linearAttenuation != 0.0
              || light.quadraticAttenuation != 0.0) {
                FP float dist = length(s);
                att = 1.0 / (light.constantAttenuation + light.linearAttenuation * dist + light.quadraticAttenuation * dist * dist);
            }
            s = normalize( s );
            if ( light.type == TYPE_SPOT ) {
                if ( degrees(acos(dot(-s, normalize(light.direction))) ) > light.cutOffAngle)
                    att = 0.0;
            }
        } else {
            s = normalize( -light.direction );
        }

        FP float diffuse = max( dot( s, n ), 0.0 );

        FP float specular = 0.0;
        if (diffuse > 0.0 && shininess > 0.0 && att > 0.0) {
            FP vec3 r = reflect( -s, n );
            FP float normFactor = ( shininess + 2.0 ) / 2.0;
            specular = normFactor * pow( max( dot( r, vview ), 0.0 ), shininess );
        }

        diffuseColor += att * light.intensity * diffuse * light.color;
        specularColor += att * light.intensity * specular * light.color;
    }
}

FP vec4 phongFunction(const in FP vec4 ambient,
                      const in FP vec4 diffuse,
                      const in FP vec4 specular,
                      const in FP float shininess,
                      const in FP vec3 worldPosition,
                      const in FP vec3 worldView,
                      const in FP vec3 worldNormal)
{
    // Calculate the lighting model, keeping the specular component separate
    FP vec3 diffuseColor, specularColor;
    adsModel(worldPosition, worldNormal, worldView, shininess, diffuseColor, specularColor);

    // Combine spec with ambient+diffuse for final fragment color
    FP vec3 color = (ambient.rgb + diffuseColor) * diffuse.rgb
                  + specularColor * specular.rgb;

    return vec4(color, diffuse.a);
}
