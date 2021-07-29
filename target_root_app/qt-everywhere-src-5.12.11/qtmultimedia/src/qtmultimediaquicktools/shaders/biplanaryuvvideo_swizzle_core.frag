#version 150 core
uniform sampler2D plane1Texture;
uniform sampler2D plane2Texture;
uniform mat4 colorMatrix;
uniform float opacity;
in vec2 plane1TexCoord;
in vec2 plane2TexCoord;
out vec4 fragColor;

void main()
{
    float Y = texture(plane1Texture, plane1TexCoord).r;
    vec2 UV = texture(plane2Texture, plane2TexCoord).ar;
    vec4 color = vec4(Y, UV.x, UV.y, 1.);
    fragColor = colorMatrix * color * opacity;
}
