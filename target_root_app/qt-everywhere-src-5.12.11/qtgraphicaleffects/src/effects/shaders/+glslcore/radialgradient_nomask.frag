#version 150 core
uniform sampler2D gradientImage;
uniform float qt_Opacity;
in vec2 qt_TexCoord1;
in vec2 centerPoint;
out vec4 fragColor;

void main() {
    vec4 gradientColor = texture(gradientImage, vec2(0.0, 2.0 * distance(qt_TexCoord1, centerPoint)));
    fragColor = gradientColor * qt_Opacity;
}
