uniform lowp sampler2D source;
uniform lowp sampler2D maskSource;
uniform lowp float qt_Opacity;
varying highp vec2 qt_TexCoord0;
varying highp vec2 qt_TexCoord1;

void main() {
    lowp vec4 gradientColor = texture2D(source, qt_TexCoord1);
    lowp float maskAlpha = texture2D(maskSource, qt_TexCoord0).a;
    gl_FragColor = gradientColor * maskAlpha * qt_Opacity;
}
