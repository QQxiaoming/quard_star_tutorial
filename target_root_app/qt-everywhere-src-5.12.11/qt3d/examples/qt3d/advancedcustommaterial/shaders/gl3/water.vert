#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexTangent;

out vec3 worldPosition;
out vec3 worldNormal;
out vec4 worldTangent;
out vec2 texCoord;
out vec2 movtexCoord;
out vec2 multexCoord;
out vec2 waveTexCoord;
out vec2 skyTexCoord;
out vec3 vpos;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

uniform float offsetx;
uniform float offsety;
uniform float vertYpos;
uniform float texCoordScale;
uniform float waveheight;
uniform float waveRandom;


void main()
{
    // Scale texture coordinates for for fragment shader
    texCoord = vertexTexCoord * texCoordScale;
    movtexCoord = vertexTexCoord * texCoordScale;
    multexCoord = vertexTexCoord * (texCoordScale*0.5);
    waveTexCoord = vertexTexCoord * (texCoordScale * 6);
    skyTexCoord = vertexTexCoord * (texCoordScale * 0.2);

    // Add Animated x and y Offset to SKY, MOV and MUL texCoords
    movtexCoord = vec2(texCoord.x+offsetx,texCoord.y+offsety);
    multexCoord = vec2(texCoord.x-offsetx,texCoord.y+offsety);
    skyTexCoord = vec2(texCoord.x-(offsetx/2),texCoord.y-(offsety/2));

    // Transform position, normal, and tangent to world coords
    worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    worldNormal = normalize(modelNormalMatrix * vertexNormal);
    worldTangent.xyz = normalize(vec3(modelMatrix * vec4(vertexTangent.xyz, 0.0)));
    worldTangent.w = vertexTangent.w;

    // Calculate animated vertex positions

    float sinPos = (vertexPosition.z)+(vertexPosition.x);
    float sinPos2 = (vertexPosition.y/2)+(vertexPosition.z);
    vec3 vertMod = vec3(vertexPosition.x,vertexPosition.y,vertexPosition.z);

    vertMod = vec3(vertMod.x+=sin(vertYpos*2.2-sinPos2)*waveheight,
                   vertMod.y=sin(vertYpos*2.2+sinPos)*waveheight,
                   vertMod.z-=sin(vertYpos*2.2-cos(sinPos2))*waveheight);

    vec3 vertModCom = vec3(vertMod.x+=cos(vertYpos*2.2-cos(sinPos2))*waveheight,
                           vertMod.y=sin(vertYpos*2.2+cos(sinPos))*waveheight,
                           vertMod.z-=cos(vertYpos*2.2-cos(sinPos))*waveheight);


    // Add wave animation only to vertices above world pos.y zero
    if(vertexPosition.y < 0.0){vertModCom = vertexPosition;}
    else{vertModCom = vertModCom;}

    vpos = vertModCom;

    // Calculate vertex position in clip coordinates
    gl_Position = mvp * vec4(vertModCom, 1.0);
}
