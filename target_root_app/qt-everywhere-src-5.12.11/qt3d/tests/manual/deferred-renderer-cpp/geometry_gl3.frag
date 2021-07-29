#version 140

in vec4 color0;
in vec3 position0;
in vec3 normal0;

out vec4 color;
out vec4 position;
out vec4 normal;

void main()
{
    color = color0;
    position = vec4(position0, 0.0);
    normal = vec4(normal0, 0.0);
}
