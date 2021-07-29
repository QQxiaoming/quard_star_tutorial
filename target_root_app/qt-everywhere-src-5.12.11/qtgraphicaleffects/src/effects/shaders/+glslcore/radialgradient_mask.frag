#version 150 core
uniform sampler2D gradientImage;
uniform sampler2D maskSource;
uniform float qt_Opacity;
in vec2 qt_TexCoord0;
in vec2 qt_TexCoord1;
in vec2 centerPoint;
out vec4 fragColor;

void main() {
    vec4 gradientColor = texture(gradientImage, vec2(0.0, 2.0 * distance(qt_TexCoord1, centerPoint)));
    float maskAlpha = texture(maskSource, qt_TexCoord0).a;
    fragColor = gradientColor * maskAlpha * qt_Opacity;
}
