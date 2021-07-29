#version 150 core
in vec2 qt_TexCoord0;
uniform sampler2D gradientSource;
uniform float qt_Opacity;
uniform float startAngle;
uniform vec2 center;
out vec4 fragColor;

void main() {
    const float PI = 3.14159265;
    const float PIx2inv = 0.1591549;
    float a = (atan((center.y - qt_TexCoord0.t), (center.x - qt_TexCoord0.s)) + PI - startAngle) * PIx2inv;
    fragColor = texture(gradientSource, vec2(0.0, fract(a))) * qt_Opacity;
}
