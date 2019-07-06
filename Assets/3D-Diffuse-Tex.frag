#version 150

in vec4 fColor;
in vec2 fUV1;

out vec4 oColor;

uniform sampler2D uDiffuse;
uniform float uAlphaTest;

void main()
{
	vec4 texel = texture(uDiffuse, fUV1) * fColor;
	if(texel.a < uAlphaTest) { discard; }
	oColor = texel;
}
