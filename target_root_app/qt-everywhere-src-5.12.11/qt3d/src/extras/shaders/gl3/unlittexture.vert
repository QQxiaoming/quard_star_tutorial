#version 150 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec3 position;
out vec2 texCoord;

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
