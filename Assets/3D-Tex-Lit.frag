#version 150
in vec3 fNormal;
in vec2 fUV1;
in vec3 fLightDir;

out vec4 oColor;

// Built-in uniforms
uniform float gAlphaTest;

// User-defined uniforms
uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
uniform sampler2D uDiffuse;

uniform vec4 uLightColor = vec4(0.6f, 0.6f, 0.6f, 1.0f);
uniform vec4 uAmbientColor = vec4(0.06f, 0.08f, 0.06f, 1.0f);

void main()
{
    // Sample surface color. Discard if invisible (for alpha test-style transparency).
    vec4 texel = texture(uDiffuse, fUV1) * uColor;
    if(texel.a < gAlphaTest) { discard; }

    // Normal must be renormalized - interpolation from vertex shader may have changed length.
    vec3 normalDir = normalize(fNormal);

    // Light direction was (purposely) not normalized in vertex shader (so it'd interpolate correctly).
    // Must be normalized now.
    vec3 lightDir = normalize(fLightDir);

    // Lighting calculations!
    float saturate = clamp(dot(normalDir, lightDir), 0.0f, 1.0f);
    vec4 directColor = uLightColor * saturate;
    vec4 allLightColors = uAmbientColor + directColor;
    oColor = allLightColors * texel;
}
