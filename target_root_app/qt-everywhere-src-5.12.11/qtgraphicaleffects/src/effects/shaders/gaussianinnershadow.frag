uniform highp sampler2D original;
uniform highp sampler2D shadow;
uniform lowp float qt_Opacity;
uniform highp float spread;
uniform lowp vec4 color;
varying highp vec2 qt_TexCoord0;

highp float linearstep(highp float e0, highp float e1, highp float x) {
    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main(void) {
    lowp vec4 originalColor = texture2D(original, qt_TexCoord0);
    lowp vec4 shadowColor = texture2D(shadow, qt_TexCoord0);
    shadowColor.rgb = mix(originalColor.rgb, color.rgb * originalColor.a, linearstep(0.0, spread, shadowColor.a));
    gl_FragColor = vec4(shadowColor.rgb, originalColor.a) * originalColor.a * qt_Opacity;
}
