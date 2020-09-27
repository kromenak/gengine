#version 150

in vec4 fColor;
in vec2 fUV1;
in vec2 fUV2;

out vec4 oColor;

// Built-in uniforms
uniform float gAlphaTest;

// User-defined uniforms
uniform sampler2D uDiffuse;
uniform sampler2D uLightmap;

void main()
{
    vec4 texel = texture(uDiffuse, fUV1);
	if(texel.a < gAlphaTest) { discard; }
    
    vec4 lightmapTexel = texture(uLightmap, fUV2);
    texel.rgb *= lightmapTexel.rgb;
    texel.rgb *= 2.0f;
    
    oColor = texel;
}
