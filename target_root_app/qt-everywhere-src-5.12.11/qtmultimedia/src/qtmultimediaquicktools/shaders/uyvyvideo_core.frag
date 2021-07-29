#version 150 core
// Reference: qgsvideonode_yuv.cpp:387 to 398
uniform sampler2D yTexture; // Y component passed as GL_RG, in uyvy Y = a
uniform sampler2D uvTexture; // UV component passed as RGBA macropixel, in uyvy U = r, V = b
uniform mat4 colorMatrix;
uniform float opacity;
in vec2 qt_TexCoord;
out vec4 fragColor;

void main()
{
    vec3 YUV = vec3(texture(yTexture, qt_TexCoord).a, texture2D(uvTexture, qt_TexCoord).rb);
    fragColor = colorMatrix * vec4(YUV, 1.0) * opacity;
}
