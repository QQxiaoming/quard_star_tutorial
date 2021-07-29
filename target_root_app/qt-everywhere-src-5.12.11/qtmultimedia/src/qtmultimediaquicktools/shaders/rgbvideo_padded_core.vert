#version 150 core
uniform mat4 qt_Matrix;
uniform float width;
in vec4 qt_VertexPosition;
in vec2 qt_VertexTexCoord;
out vec2 qt_TexCoord;

void main() {
    qt_TexCoord = qt_VertexTexCoord * vec2(width, 1);
    gl_Position = qt_Matrix * qt_VertexPosition;
}
