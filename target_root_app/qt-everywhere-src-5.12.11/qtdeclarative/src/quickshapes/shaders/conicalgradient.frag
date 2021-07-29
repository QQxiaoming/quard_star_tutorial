#define INVERSE_2PI 0.1591549430918953358

uniform sampler2D gradTabTexture;
uniform lowp float opacity;

uniform highp float angle;

varying highp vec2 coord;

void main()
{
    highp float t;
    if (abs(coord.y) == abs(coord.x))
        t = (atan(-coord.y + 0.002, coord.x) + angle) * INVERSE_2PI;
    else
        t = (atan(-coord.y, coord.x) + angle) * INVERSE_2PI;
    gl_FragColor = texture2D(gradTabTexture, vec2(t - floor(t), 0.5)) * opacity;

}
