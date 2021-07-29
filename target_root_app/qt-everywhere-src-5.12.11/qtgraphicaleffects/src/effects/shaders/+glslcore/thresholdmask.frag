#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform sampler2D maskSource;
uniform float threshold;
uniform float spread;
out vec4 fragColor;

void main(void) {
    vec4 colorFragment = texture(source, qt_TexCoord0.st);
    vec4 maskFragment = texture(maskSource, qt_TexCoord0.st);
    fragColor = colorFragment * smoothstep(threshold * (1.0 + spread) - spread, threshold * (1.0 + spread), maskFragment.a) * qt_Opacity;
}
