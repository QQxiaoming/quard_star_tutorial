#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform sampler2D displacementSource;
uniform float displacement;
uniform float xPixel;
uniform float yPixel;
out vec4 fragColor;

float linearstep(float e0, float e1, float x) {
    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main() {
    vec4 offset = texture(displacementSource, qt_TexCoord0);
    offset.xy -= vec2(0.5, 0.5);
    offset.xy = offset.xy * step(vec2(1.0/256.0), abs(offset.xy));
    vec2 tx = qt_TexCoord0 + (vec2(-offset.x, offset.y) * displacement);

    float e1 = linearstep(0.0, xPixel, tx.x);
    float e2 = linearstep(0.0, yPixel, tx.y);
    float e3 = 1.0 - linearstep(1.0, 1.0 + xPixel, tx.x);
    float e4 = 1.0 - linearstep(1.0, 1.0 + yPixel, tx.y);

    vec4 sample = texture(source, tx);
    sample.rgb *= e1 * e2 * e3 * e4;
    fragColor = sample * qt_Opacity * offset.a;
}
