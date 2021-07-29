#version 150 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec4 color;

out vec4 fragColor;

uniform vec3 eyePosition;

#pragma include phong.inc.frag

void main()
{
    vec3 worldView = normalize(eyePosition - worldPosition);
    fragColor = phongFunction(color, color, vec4(0.0), 0.0, worldPosition, worldView, worldNormal);
}
