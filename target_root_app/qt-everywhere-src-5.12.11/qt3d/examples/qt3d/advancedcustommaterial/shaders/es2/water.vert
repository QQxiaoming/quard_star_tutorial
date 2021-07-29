#define FP highp

attribute FP vec3 vertexPosition;
attribute FP vec3 vertexNormal;
attribute FP vec2 vertexTexCoord;
attribute FP vec4 vertexTangent;

varying FP vec3 worldPosition;
varying FP vec3 worldNormal;
varying FP vec4 worldTangent;
varying FP vec2 texCoord;
varying FP vec2 movtexCoord;
varying FP vec2 multexCoord;
varying FP vec2 waveTexCoord;
varying FP vec2 skyTexCoord;
varying FP vec3 vpos;

uniform FP mat4 modelMatrix;
uniform FP mat3 modelNormalMatrix;
uniform FP mat4 mvp;

uniform FP float offsetx;
uniform FP float offsety;
uniform FP float vertYpos;
uniform FP float texCoordScale;
uniform FP float waveheight;
uniform FP float waveRandom;


void main()
{
    // Scale texture coordinates for for fragment shader
    texCoord = vertexTexCoord * texCoordScale;
    movtexCoord = vertexTexCoord * texCoordScale;
    multexCoord = vertexTexCoord * (texCoordScale*0.5);
    waveTexCoord = vertexTexCoord * (texCoordScale * 6.0);
    skyTexCoord = vertexTexCoord * (texCoordScale * 0.2);

    // Add Animated x and y Offset to SKY, MOV and MUL texCoords
    movtexCoord = vec2(texCoord.x+offsetx,texCoord.y+offsety);
    multexCoord = vec2(texCoord.x-offsetx,texCoord.y+offsety);
    skyTexCoord = vec2(texCoord.x-(offsetx/2.0),texCoord.y-(offsety/2.0));

    // Transform position, normal, and tangent to world coords
    worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    worldNormal = normalize(modelNormalMatrix * vertexNormal);
    worldTangent.xyz = normalize(vec3(modelMatrix * vec4(vertexTangent.xyz, 0.0)));
    worldTangent.w = vertexTangent.w;

    // Calculate animated vertex positions

    FP float sinPos = (vertexPosition.z)+(vertexPosition.x);
    FP float sinPos2 = (vertexPosition.y/2.0)+(vertexPosition.z);
    FP vec3 vertMod = vec3(vertexPosition.x,vertexPosition.y,vertexPosition.z);

    vertMod = vec3(vertMod.x+=sin(vertYpos*2.2-sinPos2)*waveheight,
                   vertMod.y=sin(vertYpos*2.2+sinPos)*waveheight,
                   vertMod.z-=sin(vertYpos*2.2-cos(sinPos2))*waveheight);

    FP vec3 vertModCom = vec3(vertMod.x+=cos(vertYpos*2.2-cos(sinPos2))*waveheight,
                           vertMod.y=sin(vertYpos*2.2+cos(sinPos))*waveheight,
                           vertMod.z-=cos(vertYpos*2.2-cos(sinPos))*waveheight);


    // Add wave animation only to vertices above world pos.y zero
    if(vertexPosition.y < 0.0){vertModCom = vertexPosition;}
    else{vertModCom = vertModCom;}

    vpos = vertModCom;

    // Calculate vertex position in clip coordinates
    gl_Position = mvp * vec4(vertModCom, 1.0);
}
