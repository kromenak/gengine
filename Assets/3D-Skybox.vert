#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 vPos;

out vec3 fTexCoords;

uniform mat4 uViewProj;

void main()
{
    fTexCoords = vPos;
    gl_Position = uViewProj * vec4(vPos, 1.0f);
}
