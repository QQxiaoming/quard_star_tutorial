#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
out vec4 fragColor;

void main() {
    fragColor = texture(source, qt_TexCoord0) * qt_Opacity;
}
