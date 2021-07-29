#version 150 core
uniform sampler2D source;
uniform sampler2D maskSource;
uniform float qt_Opacity;
in vec2 qt_TexCoord0;
in vec2 qt_TexCoord1;
out vec4 fragColor;

void main() {
    vec4 gradientColor = texture(source, qt_TexCoord1);
    float maskAlpha = texture(maskSource, qt_TexCoord0).a;
    fragColor = gradientColor * maskAlpha * qt_Opacity;
}
