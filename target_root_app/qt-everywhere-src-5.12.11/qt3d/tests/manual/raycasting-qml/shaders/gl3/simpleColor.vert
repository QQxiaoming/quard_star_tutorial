#version 150 core

in vec3 vertexPosition;
uniform mat4 modelMatrix;
uniform mat4 mvp;

void main()
{
    // Calculate vertex position in clip coordinates
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
