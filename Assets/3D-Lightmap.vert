#version 150

in vec3 vPos;
in vec4 vColor;
in vec2 vUV1;
in vec2 vUV2;

out vec4 fColor;
out vec2 fUV1;
out vec2 fUV2;

// Built-in uniforms
uniform mat4 gViewMatrix;
uniform mat4 gProjMatrix;
uniform mat4 gWorldToProjMatrix;
uniform mat4 gObjectToWorldMatrix;

// User-defined uniforms
uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main()
{
    // Pass through the UV attribute.
    fUV1 = vUV1;
    fUV2 = vUV2;
    
    fColor = vColor;
    
    // Transform position obj->world->view->proj
    gl_Position = gWorldToProjMatrix * gObjectToWorldMatrix * vec4(vPos, 1.0f);
}