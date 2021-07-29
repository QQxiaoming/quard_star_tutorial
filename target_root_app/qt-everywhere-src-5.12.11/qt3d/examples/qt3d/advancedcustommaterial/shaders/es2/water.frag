#define FP highp

varying FP vec3 worldPosition;
varying FP vec3 worldNormal;
varying FP vec4 worldTangent;
varying FP vec2 texCoord;
varying FP vec2 waveTexCoord;
varying FP vec2 movtexCoord;
varying FP vec2 multexCoord;
varying FP vec2 skyTexCoord;

varying FP vec3 vpos;

varying FP vec3 color;

uniform FP sampler2D diffuseTexture;
uniform FP sampler2D specularTexture;
uniform FP sampler2D normalTexture;
uniform FP sampler2D waveTexture;
uniform FP sampler2D skyTexture;
uniform FP sampler2D foamTexture;

uniform FP float offsetx;
uniform FP float offsety;
uniform FP float specularity;
uniform FP float waveStrenght;
uniform FP vec4 ka;
uniform FP float shininess;
uniform FP float normalAmount;
uniform FP vec3 eyePosition;

#pragma include phong.inc.frag
#pragma include coordinatesystems.inc

void main()
{
    // Move waveTexCoords
    FP vec2 waveMovCoord = waveTexCoord;
    waveMovCoord.x += offsetx;
    waveMovCoord.y -= offsety;
    FP vec4 wave = texture2D(waveTexture, waveMovCoord);

    //Wiggle the newCoord by r and b colors of waveTexture
    FP vec2 newCoord = texCoord;
    newCoord.x += wave.r * waveStrenght;
    newCoord.y -= wave.b * waveStrenght;

    // Sample the textures at the interpolated texCoords
    // Use default texCoord for diffuse (it does not move on x or y, so it can be used as "ground under the water").
    FP vec4 diffuseTextureColor = texture2D(diffuseTexture, texCoord);
    // 2 Animated Layers of specularTexture mixed with the newCoord
    FP vec4 specularTextureColor = texture2D( specularTexture, multexCoord+newCoord) + (texture2D( specularTexture, movtexCoord+newCoord ));
    // 2 Animated Layers of normalTexture mixed with the newCoord
    FP vec3 tNormal = normalAmount * texture2D( normalTexture, movtexCoord+newCoord ).rgb - vec3( 1.0 )+(normalAmount * texture2D( normalTexture, multexCoord+newCoord ).rgb - vec3( 1.0 ));
    // Animated skyTexture layer
    FP vec4 skycolor = texture2D(skyTexture, skyTexCoord);
    skycolor = skycolor * 0.4;
    //Animated foamTexture layer
    FP vec4 foamTextureColor = texture2D(foamTexture, texCoord);

    FP mat3 tangentMatrix = calcWorldSpaceToTangentSpaceMatrix(worldNormal, worldTangent);
    FP mat3 invertTangentMatrix = transpose(tangentMatrix);

    FP vec3 wNormal = normalize(invertTangentMatrix * tNormal);
    FP vec3 worldView = normalize(eyePosition - worldPosition);

    FP vec4 diffuse = vec4(diffuseTextureColor.rgb, vpos.y);
    FP vec4 specular = vec4(specularTextureColor.a*specularity);
    FP vec4 outputColor = phongFunction(ka, diffuse, specular, shininess, worldPosition, worldView, wNormal);

    outputColor += vec4(skycolor.rgb, vpos.y);
    outputColor += (foamTextureColor.rgba*vpos.y);

    gl_FragColor = vec4(outputColor.rgb,1.0);
}

