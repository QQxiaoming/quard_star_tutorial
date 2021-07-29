varying highp vec2 qt_TexCoord0;
uniform highp float qt_Opacity;
uniform lowp sampler2D source;
uniform highp vec3 minimumInputRGB;
uniform highp vec3 maximumInputRGB;
uniform highp float minimumInputAlpha;
uniform highp float maximumInputAlpha;
uniform highp vec3 minimumOutputRGB;
uniform highp vec3 maximumOutputRGB;
uniform highp float minimumOutputAlpha;
uniform highp float maximumOutputAlpha;
uniform highp vec3 gamma;

highp float linearstep(highp float e0, highp float e1, highp float x) {
    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main(void) {
    highp vec4 textureColor = texture2D(source, qt_TexCoord0.st);
    highp vec4 color = vec4(textureColor.rgb / max(1.0/256.0, textureColor.a), textureColor.a);

    color.r = linearstep(minimumInputRGB.r, maximumInputRGB.r, color.r);
    color.g = linearstep(minimumInputRGB.g, maximumInputRGB.g, color.g);
    color.b = linearstep(minimumInputRGB.b, maximumInputRGB.b, color.b);
    color.a = linearstep(minimumInputAlpha, maximumInputAlpha, color.a);

    color.rgb = pow(color.rgb, gamma);

    color.r = minimumOutputRGB.r + color.r * (maximumOutputRGB.r - minimumOutputRGB.r);
    color.g = minimumOutputRGB.g + color.g * (maximumOutputRGB.g - minimumOutputRGB.g);
    color.b = minimumOutputRGB.b + color.b * (maximumOutputRGB.b - minimumOutputRGB.b);
    color.a = minimumOutputAlpha + color.a * (maximumOutputAlpha - minimumOutputAlpha);

    gl_FragColor = vec4(color.rgb * color.a, color.a) * qt_Opacity;
}
