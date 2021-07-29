#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform vec4 color;
uniform float horizontalOffset;
uniform float verticalOffset;
out vec4 fragColor;

void main(void) {
    vec2 pos = qt_TexCoord0 - vec2(horizontalOffset, verticalOffset);
    float ea = step(0.0, pos.x) * step(0.0, pos.y) * step(pos.x, 1.0) * step(pos.y, 1.0);
    float eb = 1.0 - ea;
    fragColor = (eb * color + ea * color * (1.0 - texture(source, pos).a)) * qt_Opacity;
}
