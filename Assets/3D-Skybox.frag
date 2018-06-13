#version 150

in vec3 fTexCoords;

out vec4 oColor;

uniform samplerCube uCubeMap;

void main()
{
    oColor = texture(uCubeMap, fTexCoords);
}
