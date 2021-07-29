#version 150 core
uniform sampler2D source;
uniform float qt_Opacity;
in vec2 qt_TexCoord1;
out vec4 fragColor;

void main() {
    fragColor = texture(source, qt_TexCoord1) * qt_Opacity;
}
