#version 150
in vec2 fUV1;

out vec4 oColor;

// Built-in uniforms
uniform float gAlphaTest;

uniform vec4 gDiscardColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);
uniform float gDiscardColorTolerance = 0.1f;

// User-defined uniforms
uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
uniform sampler2D uDiffuse;

void main()
{
    vec4 texel = texture(uDiffuse, fUV1) * uColor;
    if(texel.a < gAlphaTest || distance(texel.rgb, gDiscardColor.rgb) < gDiscardColorTolerance) { discard; }
    oColor = texel;
}
