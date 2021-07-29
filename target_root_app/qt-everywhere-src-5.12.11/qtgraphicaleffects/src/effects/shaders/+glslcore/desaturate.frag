#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform float desaturation;
out vec4 fragColor;
void main(void) {
    vec4 textureColor = texture(source, qt_TexCoord0.st);
    float grayColor = (textureColor.r + textureColor.g + textureColor.b) / 3.0;
    fragColor = mix(textureColor, vec4(vec3(grayColor), textureColor.a), desaturation) * qt_Opacity;
}
