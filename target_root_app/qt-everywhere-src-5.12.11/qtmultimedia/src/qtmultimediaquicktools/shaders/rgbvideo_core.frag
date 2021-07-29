#version 150 core
uniform sampler2D rgbTexture;
uniform float opacity;
in vec2 qt_TexCoord;
out vec4 fragColor;

void main()
{
    fragColor = texture(rgbTexture, qt_TexCoord) * opacity;
}
