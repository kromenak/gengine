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
    // Grab color texel.
    vec4 texel = texture(uDiffuse, fUV1);
    
    // Discard if below alpha test value.
	if(texel.a < gAlphaTest) { discard; }
    
    // Grab lightmap texel.
    vec4 lightmapTexel = texture(uLightmap, fUV2);
    
    // Multiply into color texel - the multiplier seems needed to get correct results.
    texel.rgb *= (lightmapTexel.rgb * 2.0f);
    
    // Finally, the output color.
    oColor = texel;
}
