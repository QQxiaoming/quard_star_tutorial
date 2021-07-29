#version 150 core

in vec4 vertexCoord;
in vec4 vertexColor;

uniform mat4 matrix;
uniform vec2 translationPoint;

out vec2 coord;

void main()
{
    coord = vertexCoord.xy - translationPoint;
    gl_Position = matrix * vertexCoord;
}
