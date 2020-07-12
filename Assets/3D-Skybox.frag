#version 150

in vec3 fTexCoords;

out vec4 oColor;

// User-defined uniforms
uniform samplerCube uCubeMap;

void main()
{
    oColor = texture(uCubeMap, fTexCoords);
}
