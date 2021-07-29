uniform lowp sampler2D mask;
uniform lowp sampler2D source1;
uniform lowp sampler2D source2;
uniform lowp sampler2D source3;
uniform lowp sampler2D source4;
uniform lowp sampler2D source5;
uniform lowp sampler2D source6;
uniform lowp float lod;
uniform lowp float qt_Opacity;
varying mediump vec2 qt_TexCoord0;

mediump float weight(mediump float v) {
    if (v <= 0.0)
        return 1.0;

    if (v >= 0.5)
        return 0.0;

    return 1.0 - v * 2.0;
}

void main() {

    lowp vec4 maskColor = texture2D(mask, qt_TexCoord0);
    mediump float l = lod * maskColor.a;

    mediump float w1 = weight(abs(l - 0.100));
    mediump float w2 = weight(abs(l - 0.300));
    mediump float w3 = weight(abs(l - 0.500));
    mediump float w4 = weight(abs(l - 0.700));
    mediump float w5 = weight(abs(l - 0.900));
    mediump float w6 = weight(abs(l - 1.100));

    mediump float sum = w1 + w2 + w3 + w4 + w5 + w6;
    mediump float weight1 = w1 / sum;
    mediump float weight2 = w2 / sum;
    mediump float weight3 = w3 / sum;
    mediump float weight4 = w4 / sum;
    mediump float weight5 = w5 / sum;
    mediump float weight6 = w6 / sum;

    lowp vec4 sourceColor = texture2D(source1, qt_TexCoord0) * weight1;
    sourceColor += texture2D(source2, qt_TexCoord0) * weight2;
    sourceColor += texture2D(source3, qt_TexCoord0) * weight3;
    sourceColor += texture2D(source4, qt_TexCoord0) * weight4;
    sourceColor += texture2D(source5, qt_TexCoord0) * weight5;
    sourceColor += texture2D(source6, qt_TexCoord0) * weight6;

    gl_FragColor = sourceColor * qt_Opacity;

}
