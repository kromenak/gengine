#version 150

in vec3 vPos;
in vec2 vUV1;

out vec2 fUV1;
out vec2 fUV2;

// Built-in uniforms
uniform mat4 gViewMatrix;
uniform mat4 gProjMatrix;
uniform mat4 gWorldToProjMatrix;
uniform mat4 gObjectToWorldMatrix;

// User-defined uniforms
uniform vec4 uLightmapScaleOffset;

void main()
{
    // Pass through the UV attribute.
    fUV1 = vUV1;
    
    // Calculate light map UV by applying offset/scale to texture UV.
    fUV2 = (vUV1 + uLightmapScaleOffset.zw) * uLightmapScaleOffset.xy;
    
    // Transform position obj->world->view->proj
    gl_Position = gWorldToProjMatrix * gObjectToWorldMatrix * vec4(vPos, 1.0f);
}
