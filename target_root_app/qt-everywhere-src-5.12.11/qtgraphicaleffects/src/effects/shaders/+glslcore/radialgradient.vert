#version 150 core
in vec4 qt_Vertex;
in vec2 qt_MultiTexCoord0;
uniform mat4 qt_Matrix;
uniform vec2 matrixData;
uniform float horizontalRatio;
uniform float verticalRatio;
uniform vec2 center;
out vec2 qt_TexCoord0;
out vec2 qt_TexCoord1;
out vec2 centerPoint;

void main() {
    vec2 ratio = vec2(horizontalRatio, verticalRatio);

    // Rotation matrix
    mat2 rot = mat2(matrixData.y, -matrixData.x,
                    matrixData.x,  matrixData.y);

    qt_TexCoord0 = qt_MultiTexCoord0;

    qt_TexCoord1 = qt_MultiTexCoord0;
    qt_TexCoord1 -= center;
    qt_TexCoord1 *= rot;
    qt_TexCoord1 += center;
    qt_TexCoord1 *= ratio;

    centerPoint = center * ratio;

    gl_Position = qt_Matrix * qt_Vertex;
}
