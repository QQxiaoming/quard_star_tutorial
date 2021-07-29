
uniform lowp sampler2D gradientImage;
uniform lowp float qt_Opacity;
varying highp vec2 qt_TexCoord1;
varying highp vec2 centerPoint;

void main() {
    lowp vec4 gradientColor = texture2D(gradientImage, vec2(0.0, 2.0 * distance(qt_TexCoord1, centerPoint)));
    gl_FragColor = gradientColor * qt_Opacity;
}
