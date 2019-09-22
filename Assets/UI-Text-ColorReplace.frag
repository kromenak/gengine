#version 150

in vec4 fColor;
in vec2 fUV1;

out vec4 oColor;

uniform sampler2D uDiffuse;
uniform float uAlphaTest;

void main()
{
	// Grab texel.
	vec4 texel = texture(uDiffuse, fUV1);
	
	// Completely replace color values with passed in color.
	// Basically only using texel for its alpha.
	texel.rgb = fColor.rgb;
	oColor = texel;
}
