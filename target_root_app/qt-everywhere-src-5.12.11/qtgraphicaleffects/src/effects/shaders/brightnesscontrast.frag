varying mediump vec2 qt_TexCoord0;
uniform highp float qt_Opacity;
uniform lowp sampler2D source;
uniform highp float brightness;
uniform highp float contrast;
void main() {
    highp vec4 pixelColor = texture2D(source, qt_TexCoord0);
    pixelColor.rgb /= max(1.0/256.0, pixelColor.a);
    highp float c = 1.0 + contrast;
    highp float contrastGainFactor = 1.0 + c * c * c * c * step(0.0, contrast);
    pixelColor.rgb = ((pixelColor.rgb - 0.5) * (contrastGainFactor * contrast + 1.0)) + 0.5;
    pixelColor.rgb = mix(pixelColor.rgb, vec3(step(0.0, brightness)), abs(brightness));
    gl_FragColor = vec4(pixelColor.rgb * pixelColor.a, pixelColor.a) * qt_Opacity;
}
