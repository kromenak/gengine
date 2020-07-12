#version 150

in vec4 fColor;
in vec2 fUV1;

out vec4 oColor;

// Built-in uniforms
uniform float gAlphaTest;

// User-defined uniforms
uniform sampler2D uDiffuse;

void main()
{
	vec4 texel = texture(uDiffuse, fUV1) * fColor;
	if(texel.a < gAlphaTest) { discard; }
	oColor = texel;
}
