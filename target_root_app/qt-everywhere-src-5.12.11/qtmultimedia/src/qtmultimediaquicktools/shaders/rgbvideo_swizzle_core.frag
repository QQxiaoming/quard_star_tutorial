#version 150 core
uniform sampler2D rgbTexture;
uniform float opacity;
uniform bool hasAlpha;
in vec2 qt_TexCoord;
out vec4 fragColor;

void main()
{
    vec4 v = texture(rgbTexture, qt_TexCoord);
    fragColor = vec4(v.bgr, hasAlpha ? v.a : 1.0) * opacity;
}
