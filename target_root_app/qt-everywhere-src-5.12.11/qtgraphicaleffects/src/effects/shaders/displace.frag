varying highp vec2 qt_TexCoord0;
uniform highp float qt_Opacity;
uniform lowp sampler2D source;
uniform lowp sampler2D displacementSource;
uniform highp float displacement;
uniform highp float xPixel;
uniform highp float yPixel;

highp float linearstep(highp float e0, highp float e1, highp float x) {
    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main() {
    lowp vec4 offset = texture2D(displacementSource, qt_TexCoord0);
    offset.xy -= vec2(0.5, 0.5);
    offset.xy = offset.xy * step(vec2(1.0/256.0), abs(offset.xy));
    highp vec2 tx = qt_TexCoord0 + (vec2(-offset.x, offset.y) * displacement);

    lowp float e1 = linearstep(0.0, xPixel, tx.x);
    lowp float e2 = linearstep(0.0, yPixel, tx.y);
    lowp float e3 = 1.0 - linearstep(1.0, 1.0 + xPixel, tx.x);
    lowp float e4 = 1.0 - linearstep(1.0, 1.0 + yPixel, tx.y);

    lowp vec4 sample = texture2D(source, tx);
    sample.rgb *= e1 * e2 * e3 * e4;
    gl_FragColor = sample * qt_Opacity * offset.a;
}
