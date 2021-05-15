#version 150
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV1;

out vec2 fUV1;
out vec4 fLight;

// Built-in uniforms
uniform mat4 gViewMatrix;
uniform mat4 gProjMatrix;
uniform mat4 gWorldToProjMatrix;
uniform mat4 gObjectToWorldMatrix;

// User-defined uniforms
uniform vec4 uLightPos = vec4(-35.900139f ,98.740967f, 205.638931f, 1.0f);

void main()
{
    // Pass through the UV attribute.
    fUV1 = vUV1;
    
    // Transform to world space.
    vec4 worldPos = gObjectToWorldMatrix * vec4(vPos, 1.0f);
    vec3 lightDir = normalize(vec3((uLightPos - worldPos).xyz));
    
    // Convert normal to world space.
    //TODO: Do inverse calc in C++ and pass down as uniform.
    mat4 inv = inverse(gObjectToWorldMatrix);
    vec3 worldNormal = (vec4(vNormal, 0.0f) * inv).xyz;
    
    // Determine how much light affects vertex based on facing to light.
    float dotResult = dot(lightDir.xyz, worldNormal);
    fLight = vec4((dotResult * 0.2f), (dotResult * 0.2f), (dotResult * 0.2f), 1.0f);
    //fLight = vec4(worldNormal.xyz, 1.0f);
    
    // Transform position obj->world->view->proj
    gl_Position = gWorldToProjMatrix * worldPos;
}
