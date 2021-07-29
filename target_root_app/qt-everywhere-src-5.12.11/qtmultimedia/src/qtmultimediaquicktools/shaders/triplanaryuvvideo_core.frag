#version 150 core
uniform sampler2D plane1Texture;
uniform sampler2D plane2Texture;
uniform sampler2D plane3Texture;
uniform mat4 colorMatrix;
uniform float opacity;
in vec2 plane1TexCoord;
in vec2 plane2TexCoord;
in vec2 plane3TexCoord;
out vec4 fragColor;

void main()
{
    float Y = texture(plane1Texture, plane1TexCoord).r;
    float U = texture(plane2Texture, plane2TexCoord).r;
    float V = texture(plane3Texture, plane3TexCoord).r;
    vec4 color = vec4(Y, U, V, 1.);
    fragColor = colorMatrix * color * opacity;
}
