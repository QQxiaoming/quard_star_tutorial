#define FP highp

varying FP vec3 worldPosition;
varying FP vec3 worldNormal;
varying FP vec4 color;

uniform FP vec3 eyePosition;

#pragma include phong.inc.frag

void main()
{
    FP vec3 worldView = normalize(eyePosition - worldPosition);
    gl_FragColor = phongFunction(color, color, vec4(0.0), 0.0, worldPosition, worldView, worldNormal);
}
