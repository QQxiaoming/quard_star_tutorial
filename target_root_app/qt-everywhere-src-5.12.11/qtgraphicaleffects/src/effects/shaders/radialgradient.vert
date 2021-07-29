attribute highp vec4 qt_Vertex;
attribute highp vec2 qt_MultiTexCoord0;
uniform highp mat4 qt_Matrix;
uniform highp vec2 matrixData;
uniform highp float horizontalRatio;
uniform highp float verticalRatio;
uniform highp vec2 center;
varying highp vec2 qt_TexCoord0;
varying highp vec2 qt_TexCoord1;
varying highp vec2 centerPoint;

void main() {
    highp vec2 ratio = vec2(horizontalRatio, verticalRatio);

    // Rotation matrix
    highp mat2 rot = mat2(matrixData.y, -matrixData.x,
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
