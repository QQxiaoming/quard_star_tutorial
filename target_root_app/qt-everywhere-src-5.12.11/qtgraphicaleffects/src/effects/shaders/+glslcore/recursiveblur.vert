#version 150 core
in vec4 qt_Vertex;
in vec2 qt_MultiTexCoord0;
uniform mat4 qt_Matrix;
uniform float expandX;
uniform float expandY;
out vec2 qt_TexCoord0;

void main() {
    vec2 texCoord = qt_MultiTexCoord0;
    texCoord.s = (texCoord.s - expandX) / (1.0 - 2.0 * expandX);
    texCoord.t = (texCoord.t - expandY) / (1.0 - 2.0 * expandY);
    qt_TexCoord0 = texCoord;
    gl_Position = qt_Matrix * qt_Vertex;
}
