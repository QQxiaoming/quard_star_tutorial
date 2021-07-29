#version 150 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec4 worldTangent;
in vec2 texCoord;
in vec2 waveTexCoord;
in vec2 movtexCoord;
in vec2 multexCoord;
in vec2 skyTexCoord;

in vec3 vpos;

in vec3 color;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;
uniform sampler2D waveTexture;
uniform sampler2D skyTexture;
uniform sampler2D foamTexture;

uniform float offsetx;
uniform float offsety;
uniform float specularity;
uniform float waveStrenght;
uniform vec4 ka;
uniform vec3 specularColor;
uniform float shininess;
uniform float normalAmount;
uniform vec3 eyePosition;

out vec4 fragColor;

#pragma include phong.inc.frag
#pragma include coordinatesystems.inc

void main()
{
    // Move waveTexCoords
    vec2 waveMovCoord = waveTexCoord;
    waveMovCoord.x += offsetx;
    waveMovCoord.y -= offsety;
    vec4 wave = texture(waveTexture, waveMovCoord);

    //Wiggle the newCoord by r and b colors of waveTexture
    vec2 newCoord = texCoord;
    newCoord.x += wave.r * waveStrenght;
    newCoord.y -= wave.b * waveStrenght;

    // Sample the textures at the interpolated texCoords
    // Use default texCoord for diffuse (it does not move on x or y, so it can be used as "ground under the water").
    vec4 diffuseTextureColor = texture(diffuseTexture, texCoord);
    // 2 Animated Layers of specularTexture mixed with the newCoord
    vec4 specularTextureColor = texture( specularTexture, multexCoord+newCoord) + (texture( specularTexture, movtexCoord+newCoord ));
    // 2 Animated Layers of normalTexture mixed with the newCoord
    vec3 tNormal = normalAmount * texture( normalTexture, movtexCoord+newCoord ).rgb - vec3( 1.0 )+(normalAmount * texture( normalTexture, multexCoord+newCoord ).rgb - vec3( 1.0 ));
    // Animated skyTexture layer
    vec4 skycolor = texture(skyTexture, skyTexCoord);
    skycolor = skycolor * 0.4;
    //Animated foamTexture layer
    vec4 foamTextureColor = texture(foamTexture, texCoord);

    mat3 tangentMatrix = calcWorldSpaceToTangentSpaceMatrix(worldNormal, worldTangent);
    mat3 invertTangentMatrix = transpose(tangentMatrix);

    vec3 wNormal = normalize(invertTangentMatrix * tNormal);
    vec3 worldView = normalize(eyePosition - worldPosition);

    vec4 diffuse = vec4(diffuseTextureColor.rgb, vpos.y);
    vec4 specular = vec4(specularTextureColor.a*specularity);
    vec4 outputColor = phongFunction(ka, diffuse, specular, shininess, worldPosition, worldView, wNormal);

    outputColor += vec4(skycolor.rgb, vpos.y);
    outputColor += (foamTextureColor.rgba*vpos.y);

    fragColor = vec4(outputColor.rgb,1.0);
}

