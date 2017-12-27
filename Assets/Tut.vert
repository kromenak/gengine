#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec4 vUV1;

out vec4 fColor;

uniform mat4 uViewProj;
uniform mat4 uWorldTransform;

void main(void)
{
    // Pass through color attribute.
    fColor = vec4(vPos.x, vPos.y, vPos.z, 1.0f);
    //fColor = vColor;
    
    // Transform position into world space
    vec4 newPos =  uWorldTransform * vec4(vPos, 1.0f);
    newPos = uViewProj * newPos;
    gl_Position = newPos;
}
