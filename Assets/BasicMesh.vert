// Sprite.vert
// Vertex shader for texture mapped meshes

#version 150
#extension GL_ARB_explicit_attrib_location : require

uniform mat4 uViewProj;
uniform mat4 uWorldTransform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

out vec3 outNormal;
out vec2 outTexCoord;

void main()
{
	// Just propagate the texture coordinates
	outTexCoord = inTexCoord;

	// Transform normal into world space
	outNormal = (uWorldTransform * vec4(inNormal, 0.0f)).xyz;
	
	// Transform position into world space
	vec4 newPos =  uWorldTransform * vec4(inPosition, 1.0f);
	// Now into projection space
	newPos = uViewProj * newPos;
    gl_Position = newPos;
}
