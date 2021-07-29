attribute vec4 vertexCoord;
attribute vec4 vertexColor;

uniform mat4 matrix;
uniform vec2 translationPoint;

varying vec2 coord;

void main()
{
    coord = vertexCoord.xy - translationPoint;
    gl_Position = matrix * vertexCoord;
}
