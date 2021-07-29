#version 150 core
uniform sampler2D original;
uniform float qt_Opacity;
uniform vec4 color;
uniform float horizontalOffset;
uniform float verticalOffset;
in vec2 qt_TexCoord0;
out vec4 fragColor;

void main(void) {
    vec2 pos = qt_TexCoord0 - vec2(horizontalOffset, verticalOffset);
    float ea = step(0.0, pos.x) * step(0.0, pos.y) * step(pos.x, 1.0) * step(pos.y, 1.0);
    float eb = 1.0 - ea;
    fragColor = eb * color + ea * color * (1.0 - texture(original, pos).a) * qt_Opacity;
}
