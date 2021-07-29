#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform sampler2D maskSource;
out vec4 fragColor;
void main(void) {
    fragColor = texture(source, qt_TexCoord0.st) * (1.0 - texture(maskSource, qt_TexCoord0.st).a) * qt_Opacity;
}
