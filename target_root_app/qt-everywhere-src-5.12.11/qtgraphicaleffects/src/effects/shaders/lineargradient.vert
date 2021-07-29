attribute highp vec4 qt_Vertex;
attribute highp vec2 qt_MultiTexCoord0;
uniform highp mat4 qt_Matrix;
varying highp vec2 qt_TexCoord0;
varying highp vec2 qt_TexCoord1;
uniform highp vec2 startPoint;
uniform highp float l;
uniform highp vec2 matrixData;

void main() {
    highp mat2 rot = mat2(matrixData.y, -matrixData.x,
                          matrixData.x,  matrixData.y);

    qt_TexCoord0 = qt_MultiTexCoord0;

    qt_TexCoord1 = qt_MultiTexCoord0 * l;
    qt_TexCoord1 -= startPoint * l;
    qt_TexCoord1 *= rot;

    gl_Position = qt_Matrix * qt_Vertex;
}
