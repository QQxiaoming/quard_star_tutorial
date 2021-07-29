uniform sampler2D gradTabTexture;
uniform lowp float opacity;

uniform highp vec2 focalToCenter;
uniform highp float centerRadius;
uniform highp float focalRadius;

varying highp vec2 coord;

void main()
{
    highp float rd = centerRadius - focalRadius;
    highp float b = 2.0 * (rd * focalRadius + dot(coord, focalToCenter));
    highp float fmp2_m_radius2 = -focalToCenter.x * focalToCenter.x - focalToCenter.y * focalToCenter.y + rd * rd;
    highp float inverse_2_fmp2_m_radius2 = 1.0 / (2.0 * fmp2_m_radius2);
    highp float det = b * b - 4.0 * fmp2_m_radius2 * ((focalRadius * focalRadius) - dot(coord, coord));
    lowp vec4 result = vec4(0.0);
    if (det >= 0.0) {
        highp float detSqrt = sqrt(det);
        highp float w = max((-b - detSqrt) * inverse_2_fmp2_m_radius2, (-b + detSqrt) * inverse_2_fmp2_m_radius2);
        if (focalRadius + w * (centerRadius - focalRadius) >= 0.0)
            result = texture2D(gradTabTexture, vec2(w, 0.5)) * opacity;
    }
    gl_FragColor = result;
}
