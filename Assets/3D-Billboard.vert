#version 150
in vec3 vPos;
in vec4 vColor;
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
    fColor = vColor * uColor;

    // Pass through the UV attribute.
    fUV1 = vUV1;

    mat4 modelView = gViewMatrix * gObjectToWorldMatrix;
    modelView[0][0] = -1.0;
    modelView[0][1] = 0.0;
    modelView[0][2] = 0.0;

    //modelView[1][0] = 0.0;
    //modelView[1][1] = 1.0;
    //modelView[1][2] = 0.0;

    modelView[2][0] = 0.0;
    modelView[2][1] = 0.0;
    modelView[2][2] = -1.0;

    // Transform position obj->world->view->proj
    gl_Position = gProjMatrix * modelView * vec4(vPos, 1.0f);
}
