#version 150
in vec3 vPos;
in vec4 vColor;

out vec4 fColor;

// Built-in uniforms
uniform mat4 gWorldToProjMatrix;
uniform mat4 gObjectToWorldMatrix;

// User-defined uniforms
uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main()
{
    // Pass through color attribute.
	fColor = vColor * uColor;
    
    // Transform position obj->world->view->proj
    gl_Position = gWorldToProjMatrix * gObjectToWorldMatrix * vec4(vPos, 1.0f);
}
