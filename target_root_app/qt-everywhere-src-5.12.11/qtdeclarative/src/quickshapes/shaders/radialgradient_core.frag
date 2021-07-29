#version 150 core

uniform sampler2D gradTabTexture;
uniform float opacity;

uniform vec2 focalToCenter;
uniform float centerRadius;
uniform float focalRadius;

in vec2 coord;

out vec4 fragColor;

void main()
{
    float rd = centerRadius - focalRadius;
    float b = 2.0 * (rd * focalRadius + dot(coord, focalToCenter));
    float fmp2_m_radius2 = -focalToCenter.x * focalToCenter.x - focalToCenter.y * focalToCenter.y + rd * rd;
    float inverse_2_fmp2_m_radius2 = 1.0 / (2.0 * fmp2_m_radius2);
    float det = b * b - 4.0 * fmp2_m_radius2 * ((focalRadius * focalRadius) - dot(coord, coord));
    vec4 result = vec4(0.0);
    if (det >= 0.0) {
        float detSqrt = sqrt(det);
        float w = max((-b - detSqrt) * inverse_2_fmp2_m_radius2, (-b + detSqrt) * inverse_2_fmp2_m_radius2);
        if (focalRadius + w * (centerRadius - focalRadius) >= 0.0)
            result = texture(gradTabTexture, vec2(w, 0.5)) * opacity;
    }
    fragColor = result;
}
