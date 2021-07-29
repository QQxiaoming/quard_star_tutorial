varying highp vec2 qt_TexCoord0;
uniform highp float qt_Opacity;
uniform highp sampler2D source;
uniform highp vec3 hsl;

highp vec3 RGBtoHSL(highp vec3 color) {
    highp float cmin = min(color.r, min(color.g, color.b));
    highp float cmax = max(color.r, max(color.g, color.b));
    highp float h = 0.0;
    highp float s = 0.0;
    highp float l = (cmin + cmax) / 2.0;
    highp float diff = cmax - cmin;

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

highp float hueToIntensity(highp float v1, highp float v2, highp float h) {
    h = fract(h);
    if (h < 1.0 / 6.0)
        return v1 + (v2 - v1) * 6.0 * h;
    else if (h < 1.0 / 2.0)
        return v2;
    else if (h < 2.0 / 3.0)
        return v1 + (v2 - v1) * 6.0 * (2.0 / 3.0 - h);

    return v1;
}

highp vec3 HSLtoRGB(highp vec3 color) {
    highp float h = color.x;
    highp float l = color.z;
    highp float s = color.y;

    if (s < 1.0 / 256.0)
        return vec3(l);

    highp float v1;
    highp float v2;
    if (l < 0.5)
        v2 = l * (1.0 + s);
    else
        v2 = (l + s) - (s * l);

    v1 = 2.0 * l - v2;

    highp float d = 1.0 / 3.0;
    highp float r = hueToIntensity(v1, v2, h + d);
    highp float g = hueToIntensity(v1, v2, h);
    highp float b = hueToIntensity(v1, v2, h - d);
    return vec3(r, g, b);
}

void main() {
    lowp vec4 sample = texture2D(source, qt_TexCoord0);
    sample = vec4(sample.rgb / max(1.0/256.0, sample.a), sample.a);
    sample.rgb = mix(vec3(dot(sample.rgb, vec3(0.2125, 0.7154, 0.0721))), sample.rgb, 1.0 + hsl.y);
    sample.xyz = RGBtoHSL(sample.rgb);
    sample.rgb = HSLtoRGB(vec3(sample.x + hsl.x, sample.y, sample.z));
    highp float c = step(0.0, hsl.z);
    sample.rgb = mix(sample.rgb, vec3(c), abs(hsl.z));
    gl_FragColor = vec4(sample.rgb * sample.a, sample.a) * qt_Opacity;
}
