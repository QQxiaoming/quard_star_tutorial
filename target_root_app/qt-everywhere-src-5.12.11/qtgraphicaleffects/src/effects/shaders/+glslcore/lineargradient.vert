#version 150 core
in vec4 qt_Vertex;
in vec2 qt_MultiTexCoord0;
uniform mat4 qt_Matrix;
out vec2 qt_TexCoord0;
out vec2 qt_TexCoord1;
uniform vec2 startPoint;
uniform float l;
uniform vec2 matrixData;

void main() {
    mat2 rot = mat2(matrixData.y, -matrixData.x,
                          matrixData.x,  matrixData.y);

    qt_TexCoord0 = qt_MultiTexCoord0;

    qt_TexCoord1 = qt_MultiTexCoord0 * l;
    qt_TexCoord1 -= startPoint * l;
    qt_TexCoord1 *= rot;

    gl_Position = qt_Matrix * qt_Vertex;
}
