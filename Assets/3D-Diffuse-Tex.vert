#version 150

in vec3 vPos;
in vec3 vNormal;
in vec2 vUV1;

out vec4 fColor;
out vec2 fUV1;

// Built-in uniforms
uniform mat4 gViewMatrix;
uniform mat4 gProjMatrix;
uniform mat4 gWorldToProjMatrix;
uniform mat4 gObjectToWorldMatrix;

// User-defined uniforms
uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main()
{
    // Pass through color attribute.
	fColor = uColor;
    
    // Pass through the UV attribute.
    fUV1 = vUV1;
    
    // Transform position obj->world->view->proj
    gl_Position = gWorldToProjMatrix * gObjectToWorldMatrix * vec4(vPos, 1.0f);
}
