#version 150
in vec2 fUV1;

out vec4 oColor;

// Built-in uniforms
uniform float gAlphaTest;

// User-defined uniforms
uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
uniform sampler2D uDiffuse;

void main()
{
    vec4 texel = texture(uDiffuse, fUV1) * uColor;
    if(texel.a < gAlphaTest) { discard; }
    oColor = texel;
}
