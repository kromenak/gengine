#version 150

in vec4 fColor;
in vec2 fUV1;

out vec4 oColor;

uniform sampler2D uDiffuse;

void main()
{
	//TODO: '+ fColor' is needed for debug aides to show up.
	// '* fColor' breaks rendering because default RGBA values, if not specified, are all (0,0,0,0).
	// Ideally, default color would be white.
    oColor = texture(uDiffuse, fUV1) + fColor;
}
