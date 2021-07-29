#version 150 core

uniform sampler2D gradTabTexture;
uniform float opacity;

in float gradTabIndex;
out vec4 fragColor;

void main()
{
    fragColor = texture(gradTabTexture, vec2(gradTabIndex, 0.5)) * opacity;
}
