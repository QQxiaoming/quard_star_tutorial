#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform float gamma;
out vec4 fragColor;

void main(void) {
    vec4 originalColor = texture(source, qt_TexCoord0.st);
    originalColor.rgb = originalColor.rgb / max(1.0/256.0, originalColor.a);
    vec3 adjustedColor = pow(originalColor.rgb, vec3(gamma));
    fragColor = vec4(adjustedColor * originalColor.a, originalColor.a) * qt_Opacity;
}
