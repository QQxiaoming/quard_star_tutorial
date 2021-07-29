varying highp vec2 qt_TexCoord0;
uniform highp float qt_Opacity;
uniform lowp sampler2D source;
uniform lowp sampler2D maskSource;
uniform highp float threshold;
uniform highp float spread;
void main(void) {
    lowp vec4 colorFragment = texture2D(source, qt_TexCoord0.st);
    lowp vec4 maskFragment = texture2D(maskSource, qt_TexCoord0.st);
    gl_FragColor = colorFragment * smoothstep(threshold * (1.0 + spread) - spread, threshold * (1.0 + spread), maskFragment.a) * qt_Opacity;
}
