#version 150

in vec4 fColor;
in vec2 fUV1;

out vec4 oColor;

uniform sampler2D uDiffuse;
uniform vec4 uReplaceColor;

void main()
{
	// Grab texel.
	vec4 texel = texture(uDiffuse, fUV1);
	
    // See "how close" this texel color is to the replace color.
    vec3 diff = (texel - uReplaceColor).rgb;
    float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
    
    float t = clamp(distSq, 0, 1);
    texel.rgb = mix(texel.rgb, fColor.rgb, t);
    
    // Set final output color.
	oColor = texel;
}
