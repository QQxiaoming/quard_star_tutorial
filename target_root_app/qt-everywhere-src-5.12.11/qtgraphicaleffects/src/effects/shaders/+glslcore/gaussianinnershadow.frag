#version 150 core
uniform sampler2D original;
uniform sampler2D shadow;
uniform float qt_Opacity;
uniform float spread;
uniform vec4 color;
in vec2 qt_TexCoord0;
out vec4 fragColor;

float linearstep(float e0, float e1, float x) {
    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main(void) {
    vec4 originalColor = texture(original, qt_TexCoord0);
    vec4 shadowColor = texture(shadow, qt_TexCoord0);
    shadowColor.rgb = mix(originalColor.rgb, color.rgb * originalColor.a, linearstep(0.0, spread, shadowColor.a));
    fragColor = vec4(shadowColor.rgb, originalColor.a) * originalColor.a * qt_Opacity;
}
