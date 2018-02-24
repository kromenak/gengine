#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vUV1;

out vec4 fColor;
out vec2 fUV1;

//uniform vec3 uOffset;
//uniform mat4 uRotation;

uniform mat4 uViewProj;
uniform mat4 uWorldTransform;

void main()
{
    // Pass through color attribute.
    fColor = vColor;
    
    // Pass through the UV attribute.
    fUV1 = vUV1;
    
    // Transform position into world space.
    vec4 newPos =  uWorldTransform * vec4(vPos, 1.0f);
    newPos = uViewProj * newPos;
    gl_Position = newPos;
}
