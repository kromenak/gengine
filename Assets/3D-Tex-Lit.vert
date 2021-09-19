#version 150
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV1;

out vec3 fNormal;
out vec2 fUV1;
out vec3 fLightDir;

// Built-in uniforms
uniform mat4 gViewMatrix;
uniform mat4 gProjMatrix;
uniform mat4 gWorldToProjMatrix;
uniform mat4 gObjectToWorldMatrix;
uniform mat4 gWorldToObjectMatrix;

// User-defined uniforms
uniform vec4 uLightPos = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void main()
{
    // Pass through UV attribute.
    fUV1 = vUV1;

    // Pass through normal attribute.
    fNormal = vNormal;

    // Convert light position to object space, pass to pixel shader.
    mat4 worldToObjectMatrix = gWorldToObjectMatrix;
    vec4 localLightPos = worldToObjectMatrix * uLightPos;

    // Pass surface-to-light offset to pixel shader. Do not normalize here for proper interpolation.
    fLightDir = vec3((localLightPos - vec4(vPos, 1.0f)).xyz);

    // Transform position obj->world->view->proj.
    gl_Position = gWorldToProjMatrix * gObjectToWorldMatrix * vec4(vPos, 1.0f);
}
