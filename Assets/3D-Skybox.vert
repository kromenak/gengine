#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 vPos;

out vec3 fTexCoords;

// Built-in uniforms
uniform mat4 gWorldToProjMatrix;

void main()
{
    fTexCoords = vPos;
    gl_Position = gWorldToProjMatrix * vec4(vPos, 1.0f);
}
