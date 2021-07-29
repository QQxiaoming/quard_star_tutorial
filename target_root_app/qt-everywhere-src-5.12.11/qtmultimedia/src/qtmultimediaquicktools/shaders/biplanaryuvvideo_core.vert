#version 150 core
uniform mat4 qt_Matrix;
uniform float plane1Width;
uniform float plane2Width;
in vec4 qt_VertexPosition;
in vec2 qt_VertexTexCoord;
out vec2 plane1TexCoord;
out vec2 plane2TexCoord;

void main() {
    plane1TexCoord = qt_VertexTexCoord * vec2(plane1Width, 1);
    plane2TexCoord = qt_VertexTexCoord * vec2(plane2Width, 1);
    gl_Position = qt_Matrix * qt_VertexPosition;
}
