uniform sampler2D rgbTexture;
uniform lowp float opacity;
varying highp vec2 qt_TexCoord;
uniform bool hasAlpha;

void main()
{
    lowp vec4 v = texture2D(rgbTexture, qt_TexCoord);
    gl_FragColor = vec4(v.bgr, hasAlpha ? v.a : 1.0) * opacity;
}
