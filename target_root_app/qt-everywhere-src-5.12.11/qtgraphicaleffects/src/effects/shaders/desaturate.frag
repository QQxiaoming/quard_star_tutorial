varying highp vec2 qt_TexCoord0;
uniform highp float qt_Opacity;
uniform lowp sampler2D source;
uniform highp float desaturation;
void main(void) {
    lowp vec4 textureColor = texture2D(source, qt_TexCoord0.st);
    lowp float grayColor = (textureColor.r + textureColor.g + textureColor.b) / 3.0;
    gl_FragColor = mix(textureColor, vec4(vec3(grayColor), textureColor.a), desaturation) * qt_Opacity;
}
