varying highp vec2 qt_TexCoord0;
uniform highp float qt_Opacity;
uniform lowp sampler2D source;
uniform highp float gamma;
void main(void) {
    highp vec4 originalColor = texture2D(source, qt_TexCoord0.st);
    originalColor.rgb = originalColor.rgb / max(1.0/256.0, originalColor.a);
    highp vec3 adjustedColor = pow(originalColor.rgb, vec3(gamma));
    gl_FragColor = vec4(adjustedColor * originalColor.a, originalColor.a) * qt_Opacity;
}
