attribute highp vec4 qt_Vertex;
attribute highp vec2 qt_MultiTexCoord0;
uniform highp mat4 qt_Matrix;
uniform highp float expandX;
uniform highp float expandY;
varying highp vec2 qt_TexCoord0;

void main() {
    mediump vec2 texCoord = qt_MultiTexCoord0;
    texCoord.s = (texCoord.s - expandX) / (1.0 - 2.0 * expandX);
    texCoord.t = (texCoord.t - expandY) / (1.0 - 2.0 * expandY);
    qt_TexCoord0 = texCoord;
    gl_Position = qt_Matrix * qt_Vertex;
}
