attribute vec4 vertexCoord;
attribute vec4 vertexColor;

uniform mat4 matrix;
uniform vec2 gradStart;
uniform vec2 gradEnd;

varying float gradTabIndex;

void main()
{
    vec2 gradVec = gradEnd - gradStart;
    gradTabIndex = dot(gradVec, vertexCoord.xy - gradStart) / (gradVec.x * gradVec.x + gradVec.y * gradVec.y);
    gl_Position = matrix * vertexCoord;
}
