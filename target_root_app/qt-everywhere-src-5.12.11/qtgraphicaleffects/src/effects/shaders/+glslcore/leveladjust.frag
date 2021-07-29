#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform vec3 minimumInputRGB;
uniform vec3 maximumInputRGB;
uniform float minimumInputAlpha;
uniform float maximumInputAlpha;
uniform vec3 minimumOutputRGB;
uniform vec3 maximumOutputRGB;
uniform float minimumOutputAlpha;
uniform float maximumOutputAlpha;
uniform vec3 gamma;
out vec4 fragColor;

float linearstep(float e0, float e1, float x) {
    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main(void) {
    vec4 textureColor = texture(source, qt_TexCoord0.st);
    vec4 color = vec4(textureColor.rgb / max(1.0/256.0, textureColor.a), textureColor.a);

    color.r = linearstep(minimumInputRGB.r, maximumInputRGB.r, color.r);
    color.g = linearstep(minimumInputRGB.g, maximumInputRGB.g, color.g);
    color.b = linearstep(minimumInputRGB.b, maximumInputRGB.b, color.b);
    color.a = linearstep(minimumInputAlpha, maximumInputAlpha, color.a);

    color.rgb = pow(color.rgb, gamma);

    color.r = minimumOutputRGB.r + color.r * (maximumOutputRGB.r - minimumOutputRGB.r);
    color.g = minimumOutputRGB.g + color.g * (maximumOutputRGB.g - minimumOutputRGB.g);
    color.b = minimumOutputRGB.b + color.b * (maximumOutputRGB.b - minimumOutputRGB.b);
    color.a = minimumOutputAlpha + color.a * (maximumOutputAlpha - minimumOutputAlpha);

    fragColor = vec4(color.rgb * color.a, color.a) * qt_Opacity;
}
