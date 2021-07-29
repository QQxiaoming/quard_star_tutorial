uniform lowp sampler2D source1;
uniform lowp sampler2D source2;
uniform lowp sampler2D source3;
uniform lowp sampler2D source4;
uniform lowp sampler2D source5;
uniform mediump float weight1;
uniform mediump float weight2;
uniform mediump float weight3;
uniform mediump float weight4;
uniform mediump float weight5;
uniform lowp float qt_Opacity;
varying mediump vec2 qt_TexCoord0;

void main() {
    lowp vec4 sourceColor = texture2D(source1, qt_TexCoord0) * weight1;
    sourceColor += texture2D(source2, qt_TexCoord0) * weight2;
    sourceColor += texture2D(source3, qt_TexCoord0) * weight3;
    sourceColor += texture2D(source4, qt_TexCoord0) * weight4;
    sourceColor += texture2D(source5, qt_TexCoord0) * weight5;
    gl_FragColor = sourceColor * qt_Opacity;
}
