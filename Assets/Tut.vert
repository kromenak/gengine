#version 150

in vec3 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec4 vUV1;

out vec4 fColor;

uniform mat4 uViewProj;
uniform mat4 uWorldTransform;

void main(void)
{
    fColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    
    //gl_Position = vec4(vPos, 1.0);
    
    // Transform position into world space
    vec4 newPos =  uWorldTransform * vec4(vPos, 1.0f);
    newPos = uViewProj * newPos;
    gl_Position = newPos;
}
