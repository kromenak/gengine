#version 150

in vec3 fTexCoords;

out vec4 oColor;

uniform samplerCube uCubeMap;

void main()
{
    //oColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    oColor = texture(uCubeMap, fTexCoords);
}
