#version 150 core
uniform sampler2D original;
uniform sampler2D source1;
uniform sampler2D source2;
uniform sampler2D source3;
uniform sampler2D source4;
uniform sampler2D source5;
uniform float weight1;
uniform float weight2;
uniform float weight3;
uniform float weight4;
uniform float weight5;
uniform vec4 color;
uniform float spread;
uniform float qt_Opacity;
in vec2 qt_TexCoord0;
out vec4 fragColor;

float linearstep(float e0, float e1, float x) {
    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main() {
    vec4 shadowColor = texture(source1, qt_TexCoord0) * weight1;
    shadowColor += texture(source2, qt_TexCoord0) * weight2;
    shadowColor += texture(source3, qt_TexCoord0) * weight3;
    shadowColor += texture(source4, qt_TexCoord0) * weight4;
    shadowColor += texture(source5, qt_TexCoord0) * weight5;
    vec4 originalColor = texture(original, qt_TexCoord0);
    shadowColor.rgb = mix(originalColor.rgb, color.rgb * originalColor.a, linearstep(0.0, spread, shadowColor.a));
    fragColor = vec4(shadowColor.rgb, originalColor.a) * originalColor.a * qt_Opacity;
}
