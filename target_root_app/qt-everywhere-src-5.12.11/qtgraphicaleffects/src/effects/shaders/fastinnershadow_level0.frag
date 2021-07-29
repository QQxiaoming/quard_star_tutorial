varying highp vec2 qt_TexCoord0;
uniform lowp float qt_Opacity;
uniform highp sampler2D source;
uniform lowp vec4 color;
uniform highp float horizontalOffset;
uniform highp float verticalOffset;

void main(void) {
    highp vec2 pos = qt_TexCoord0 - vec2(horizontalOffset, verticalOffset);
    lowp float ea = step(0.0, pos.x) * step(0.0, pos.y) * step(pos.x, 1.0) * step(pos.y, 1.0);
    lowp float eb = 1.0 - ea;
    gl_FragColor = (eb * color + ea * color * (1.0 - texture2D(source, pos).a)) * qt_Opacity;
}
