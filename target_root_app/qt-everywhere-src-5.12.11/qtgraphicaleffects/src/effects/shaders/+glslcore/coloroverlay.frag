#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform vec4 color;
out vec4 fragColor;
void main() {
    vec4 pixelColor = texture(source, qt_TexCoord0);
    fragColor = vec4(mix(pixelColor.rgb/max(pixelColor.a, 0.00390625), color.rgb/max(color.a, 0.00390625), color.a) * pixelColor.a, pixelColor.a) * qt_Opacity;
}
