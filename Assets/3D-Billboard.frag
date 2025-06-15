#version 150
in vec4 fColor;
in vec2 fUV1;

out vec4 oColor;

// User-defined uniforms
uniform sampler2D uDiffuse;

void main()
{
    oColor = texture(uDiffuse, fUV1) * fColor;
}
