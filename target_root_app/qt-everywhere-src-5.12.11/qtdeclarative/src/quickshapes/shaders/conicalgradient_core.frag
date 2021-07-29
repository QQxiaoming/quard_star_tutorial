#version 150 core

#define INVERSE_2PI 0.1591549430918953358

uniform sampler2D gradTabTexture;
uniform float opacity;

uniform float angle;

in vec2 coord;

out vec4 fragColor;

void main()
{
    float t;
    if (abs(coord.y) == abs(coord.x))
        t = (atan(-coord.y + 0.002, coord.x) + angle) * INVERSE_2PI;
    else
        t = (atan(-coord.y, coord.x) + angle) * INVERSE_2PI;
    fragColor = texture(gradTabTexture, vec2(t - floor(t), 0.5)) * opacity;
}
