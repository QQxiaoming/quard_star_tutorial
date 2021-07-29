#version 150 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 texCoord;
out float zValue;

uniform mat4 modelView;
uniform mat4 mvp;

void main()
{
    texCoord = vertexTexCoord;
    zValue = vertexPosition.z;

    gl_Position = mvp * vec4(vertexPosition.xy, 0.0, 1.0);
}

