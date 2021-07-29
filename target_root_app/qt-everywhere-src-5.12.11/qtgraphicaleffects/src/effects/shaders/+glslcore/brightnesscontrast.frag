#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform float brightness;
uniform float contrast;
out vec4 fragColor;

void main() {
    vec4 pixelColor = texture(source, qt_TexCoord0);
    pixelColor.rgb /= max(1.0/256.0, pixelColor.a);
    float c = 1.0 + contrast;
    float contrastGainFactor = 1.0 + c * c * c * c * step(0.0, contrast);
    pixelColor.rgb = ((pixelColor.rgb - 0.5) * (contrastGainFactor * contrast + 1.0)) + 0.5;
    pixelColor.rgb = mix(pixelColor.rgb, vec3(step(0.0, brightness)), abs(brightness));
    fragColor = vec4(pixelColor.rgb * pixelColor.a, pixelColor.a) * qt_Opacity;
}
