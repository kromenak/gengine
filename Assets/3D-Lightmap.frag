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
    
    // Grab lightmap texel and multiply it with color texel.
    vec4 lightmapTexel = texture(uLightmap, fUV2);
    texel.rgb *= lightmapTexel.rgb;
    
    // Often, the result is too dark, so lighten it up!
    // (Maybe this is a surface property/flag?)
    texel.rgb *= 2.0f;
    
    // Finally, the output color.
    oColor = texel;
}
