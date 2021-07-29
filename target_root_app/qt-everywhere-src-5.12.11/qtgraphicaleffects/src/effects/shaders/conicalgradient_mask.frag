varying mediump vec2 qt_TexCoord0;
uniform lowp sampler2D gradientSource;
uniform lowp sampler2D maskSource;
uniform highp float qt_Opacity;
uniform highp float startAngle;
uniform highp vec2 center;

void main() {
    lowp float maskAlpha = texture2D(maskSource, qt_TexCoord0).a;
    const highp float PI = 3.14159265;
    const highp float PIx2inv = 0.1591549;
    highp float a = (atan((center.y - qt_TexCoord0.t), (center.x - qt_TexCoord0.s)) + PI - startAngle) * PIx2inv;
    gl_FragColor = texture2D(gradientSource, vec2(0.0, fract(a))) * maskAlpha * qt_Opacity;
}
