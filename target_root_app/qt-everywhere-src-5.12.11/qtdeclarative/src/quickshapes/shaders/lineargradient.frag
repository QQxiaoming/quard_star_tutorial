uniform sampler2D gradTabTexture;
uniform highp float opacity;

varying highp float gradTabIndex;

void main()
{
    gl_FragColor = texture2D(gradTabTexture, vec2(gradTabIndex, 0.5)) * opacity;
}
