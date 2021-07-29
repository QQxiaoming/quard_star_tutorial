#version 150 core
in vec2 qt_TexCoord0;
uniform float qt_Opacity;
uniform sampler2D source;
uniform vec3 hsl;
out vec4 fragColor;

vec3 RGBtoHSL(vec3 color) {
    float cmin = min(color.r, min(color.g, color.b));
    float cmax = max(color.r, max(color.g, color.b));
    float h = 0.0;
    float s = 0.0;
    float l = (cmin + cmax) / 2.0;
    float diff = cmax - cmin;

    if (diff > 1.0 / 256.0) {
        if (l < 0.5)
            s = diff / (cmin + cmax);
        else
            s = diff / (2.0 - (cmin + cmax));

        if (color.r == cmax)
            h = (color.g - color.b) / diff;
        else if (color.g == cmax)
            h = 2.0 + (color.b - color.r) / diff;
        else
            h = 4.0 + (color.r - color.g) / diff;

        h /= 6.0;
    }
    return vec3(h, s, l);
}

float hueToIntensity(float v1, float v2, float h) {
    h = fract(h);
    if (h < 1.0 / 6.0)
        return v1 + (v2 - v1) * 6.0 * h;
    else if (h < 1.0 / 2.0)
        return v2;
    else if (h < 2.0 / 3.0)
        return v1 + (v2 - v1) * 6.0 * (2.0 / 3.0 - h);

    return v1;
}

vec3 HSLtoRGB(vec3 color) {
    float h = color.x;
    float l = color.z;
    float s = color.y;

    if (s < 1.0 / 256.0)
        return vec3(l);

    float v1;
    float v2;
    if (l < 0.5)
        v2 = l * (1.0 + s);
    else
        v2 = (l + s) - (s * l);

    v1 = 2.0 * l - v2;

    float d = 1.0 / 3.0;
    float r = hueToIntensity(v1, v2, h + d);
    float g = hueToIntensity(v1, v2, h);
    float b = hueToIntensity(v1, v2, h - d);
    return vec3(r, g, b);
}

void main() {
    vec4 sample = texture(source, qt_TexCoord0);
    sample = vec4(sample.rgb / max(1.0/256.0, sample.a), sample.a);
    sample.rgb = mix(vec3(dot(sample.rgb, vec3(0.2125, 0.7154, 0.0721))), sample.rgb, 1.0 + hsl.y);
    sample.xyz = RGBtoHSL(sample.rgb);
    sample.rgb = HSLtoRGB(vec3(sample.x + hsl.x, sample.y, sample.z));
    float c = step(0.0, hsl.z);
    sample.rgb = mix(sample.rgb, vec3(c), abs(hsl.z));
    fragColor = vec4(sample.rgb * sample.a, sample.a) * qt_Opacity;
}
