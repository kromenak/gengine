#version 150

in vec4 fColor;
in vec2 fUV1;

out vec4 oColor;

// Built-in uniforms
uniform float gAlphaTest;

// User-defined uniforms
uniform sampler2D uDiffuse;
uniform vec4 uDiffuseScaleOffset;

uniform sampler2D uLightmap;

void main()
{
    vec2 uv = fUV1.xy * uDiffuseScaleOffset.xy + uDiffuseScaleOffset.zw;
    
	vec4 texel = texture(uDiffuse, fUV1) * fColor;
	if(texel.a < gAlphaTest) { discard; }
    
    vec4 lightmapTexel = texture(uLightmap, uv);
    texel.rgb *= lightmapTexel.rgb;
    
	oColor = texel;
}
