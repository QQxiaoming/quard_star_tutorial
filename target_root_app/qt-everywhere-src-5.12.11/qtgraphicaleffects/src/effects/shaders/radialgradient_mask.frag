uniform lowp sampler2D gradientImage;
uniform lowp sampler2D maskSource;
uniform lowp float qt_Opacity;
varying highp vec2 qt_TexCoord0;
varying highp vec2 qt_TexCoord1;
varying highp vec2 centerPoint;

void main() {
    lowp vec4 gradientColor = texture2D(gradientImage, vec2(0.0, 2.0 * distance(qt_TexCoord1, centerPoint)));
    lowp float maskAlpha = texture2D(maskSource, qt_TexCoord0).a;
    gl_FragColor = gradientColor * maskAlpha * qt_Opacity;
}
