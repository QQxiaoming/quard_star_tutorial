attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;

varying vec3 position;
varying vec2 texCoord;

uniform mat4 modelView;
uniform mat4 mvp;
uniform mat3 texCoordTransform;

void main()
{
    vec3 tt = texCoordTransform * vec3(vertexTexCoord, 1.0);
    texCoord = (tt / tt.z).xy;
    position = vec3( modelView * vec4( vertexPosition, 1.0 ) );

    gl_Position = mvp * vec4( vertexPosition, 1.0 );
}
