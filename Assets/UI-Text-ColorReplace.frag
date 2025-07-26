#version 150
in vec2 fUV1;

out vec4 oColor;

// Built-in uniforms
uniform vec4 gDiscardColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);
uniform float gDiscardColorTolerance = 0.1f;

// User-defined uniforms
uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
uniform sampler2D uDiffuse;
uniform vec4 uReplaceColor;

void main()
{
    // Grab texel.
    vec4 texel = texture(uDiffuse, fUV1);
    if(distance(texel.rgb, gDiscardColor.rgb) < gDiscardColorTolerance) { discard; }

    // If the texel's RGB matches the replace color's RGB, replace with main color.
    // Otherwise, just use texel color.
    oColor = texel;
    if(texel.rgb == uReplaceColor.rgb)
    {
        oColor.rgb = uColor.rgb;
    }

    //TODO: Above "if" logic can probably be replaced with a pure math approach.
    //TODO: Something like this (though this does not quite work):
    //vec3 diff = (texel - uReplaceColor).rgb;
    //float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

    //float t = clamp(distSq, 0, 1);
    //texel.rgb = mix(texel.rgb, fColor.rgb, t);

    // Set final output color.
    //oColor = texel;

    // Multiply output alpha by main color's alpha.
    // This is needed for "fading out".
    oColor.a *= uColor.a;
}
