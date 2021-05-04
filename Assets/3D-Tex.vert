#version 150
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV1;

out vec4 fColor;
out vec2 fUV1;

// Built-in uniforms
uniform mat4 gWorldToProjMatrix;
uniform mat4 gObjectToWorldMatrix;

void main()
{
    // Pass through the UV attribute.
    fUV1 = vUV1;
    
    // Transform position obj->world->view->proj.
    gl_Position = gWorldToProjMatrix * gObjectToWorldMatrix * vec4(vPos, 1.0f);
}
