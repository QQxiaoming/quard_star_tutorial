uniform lowp sampler2D source;
uniform lowp float qt_Opacity;
varying highp vec2 qt_TexCoord1;

void main() {
    gl_FragColor = texture2D(source, qt_TexCoord1) * qt_Opacity;
}
