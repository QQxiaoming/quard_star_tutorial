#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexTangent;

out vec3 worldPosition;
out vec3 worldNormal;
out vec4 worldTangent;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

void main()
{
    // Scale texture coordinates for for fragment shader
    texCoord = vertexTexCoord;

    // Transform position, normal, and tangent to world coords
    worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    worldNormal = normalize(modelNormalMatrix * vertexNormal);
    worldTangent.xyz = normalize(vec3(modelMatrix * vec4(vertexTangent.xyz, 0.0)));
    worldTangent.w = vertexTangent.w;

    // Calculate animated vertex positions

    float sinPos = (vertexPosition.z)+(vertexPosition.x);
    float sinPos2 = (vertexPosition.y/2)+(vertexPosition.z);
    vec3 vertMod = vec3(vertexPosition.x,vertexPosition.y,vertexPosition.z);

    // Calculate vertex position in clip coordinates
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
