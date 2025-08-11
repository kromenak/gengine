// This single "Uber" GLSL contains vertex & fragment shaders for G-Engine rendering.
// Benefits of this approach: fewer shader files to wrangle, less shader code duplication.
// Downsides to this approach: can be harder to maintain interleaving shader features!

#ifdef VERTEX_SHADER
    // INPUT VERTEX DATA
    // If a mesh doesn't include certain vertex data, defaults are used.
    in vec3 vPos;       // Required
    in vec4 vColor;     // Defaults to (0, 0, 0, 1)
    in vec3 vNormal;    // Defaults to (0, 0, 1)
    in vec2 vUV1;       // Defaults to (0, 1)

    // OUTPUTS
    // Values passed to the fragment shader.
    out vec4 fColor;
    out vec3 fNormal;
    out vec2 fUV1;

    #ifdef FEATURE_LIGHTING
    // Calculated direction from this vertex to the light source, in local space.
    out vec3 fLightDir;
    #endif

    #ifdef FEATURE_LIGHTMAPS
    // Secondary UV for sampling a lightmap texture.
    out vec2 fUV2;
    #endif

    #ifdef FEATURE_SKYBOX
    // A direction vector "UV" used to sample the skybox cubemap in fragment shader.
    out vec3 fCubemapUV;
    #endif

    // UNIFORMS
    // Matrices converting from object space to projection space.
    // Absolutely vital for the vertex shader to function.
    uniform mat4 gObjectToWorldMatrix;
    uniform mat4 gWorldToProjMatrix;

    #ifdef FEATURE_LIGHTING
    // The position of the light source, in world space.
    uniform vec4 uLightPos = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // The light pos needs to be converted to object space.
    // Passing this as a uniform avoids having to recalculate it per-vertex.
    uniform mat4 gWorldToObjectMatrix;
    #endif

    #ifdef FEATURE_LIGHTMAPS
    // Scale (xy) and offset (zw) from diffuse UVs to lightmap UVs.
    uniform vec4 uLightmapScaleOffset;
    #endif

    #ifdef FEATURE_DRAW_POINTS_AS_CIRCLES
    // When drawing point primitives, the size of a point in pixels.
    uniform float gPointSize = 6.0f;
    #endif

    void main()
    {
        // Transform vertex position obj->world->view->proj.
        // Every shader variant needs to do this.
        gl_Position = gWorldToProjMatrix * gObjectToWorldMatrix * vec4(vPos, 1.0f);

        // Certain vertex attributes aren't needed in the vertex shader.
        // But we need them in the fragment shader, so pass them through!
        fColor = vColor;
        fNormal = vNormal;
        fUV1 = vUV1;

        #ifdef FEATURE_SKYBOX
        // For skybox, sample cubemap with dir vector from origin to vertex.
        // We can use vPos directly since (vPos - (0,0,0) = vPos).
        fCubemapUV = vPos;
        #endif

        #ifdef FEATURE_LIGHTMAPS
        // Calculate light map UV by applying offset/scale to texture UV.
        fUV2 = (vUV1 + uLightmapScaleOffset.zw) * uLightmapScaleOffset.xy;
        #endif

        #ifdef FEATURE_LIGHTING
        // Convert light position to object space, pass to pixel shader.
        mat4 worldToObjectMatrix = gWorldToObjectMatrix;
        vec4 localLightPos = worldToObjectMatrix * uLightPos;

        // Pass surface-to-light offset to pixel shader.
        // Do not normalize for proper interpolation.
        fLightDir = (localLightPos - vec4(vPos, 1.0f)).xyz;
        #endif

        #ifdef FEATURE_DRAW_POINTS_AS_CIRCLES
        // Set the built-in variable that tells OpenGL the pixel size of the point.
        gl_PointSize = gPointSize;
        #endif
    }
#elif defined(FRAGMENT_SHADER)
    // INPUTS (should match vertex shader outputs above)
    in vec4 fColor;
    in vec3 fNormal;
    in vec2 fUV1;

    #ifdef FEATURE_LIGHTING
    in vec3 fLightDir;
    #endif

    #ifdef FEATURE_LIGHTMAPS
    in vec2 fUV2;
    #endif

    #ifdef FEATURE_SKYBOX
    in vec3 fCubemapUV;
    #endif

    // OUTPUTS (must always be a vec4 color)
    out vec4 oColor;

    // UNIFORMS
    // A (tint) color. Multiplied in right after sampling.
    uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // Texels with alpha below this value will be discarded.
    // Typically, 0 => "alpha test disabled", greater than 0 => "alpha test enabled".
    uniform float gAlphaTest;

    // Acts like a "chroma key" to discard a certain texel color (typically magenta).
    // The tolerance allows some error tolerance for close-but-not-exact colors.
    uniform vec4 gDiscardColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);
    uniform float gDiscardColorTolerance = 0.1f;

    #ifdef FEATURE_TEXTURING
    // Samples a diffuse texture.
    uniform sampler2D uDiffuse;
    #endif

    #ifdef FEATURE_LIGHTING
    // Base level of lighting in the scene.
    // Even if no light source is touching the surface, it will be this bright.
    uniform vec4 uAmbientColor = vec4(0.06f, 0.08f, 0.06f, 1.0f);

    // Color of light source in the scene.
    uniform vec4 uLightColor = vec4(0.6f, 0.6f, 0.6f, 1.0f);
    #endif

    #ifdef FEATURE_LIGHTMAPS
    // The lightmap texture that is overlaid on (multiplied into) the color texture.
    uniform sampler2D uLightmap;

    // For debugging, can be set to disable diffuse texture, leaving only lightmap visible.
    uniform float uDiffuseVisible = 1.0f; // 1 = diffuse visible, 0 = only lightmap visible

    // A multiplier for the lightmap's RGB.
    // Because lightmaps use "multiply" blend type, a higher multiplier actually makes the shadows weaker/fainter.
    uniform float uLightmapMultiplier = 2.0f;
    #endif

    #ifdef FEATURE_SKYBOX
    uniform samplerCube uCubeMap;
    #endif

    #ifdef FEATURE_COLOR_REPLACE
    uniform vec4 uReplaceColor;
    #endif

    void main()
    {
        // If using textures, sample the texture and tint by uColor uniform.
        // If not using textures, use the input interpolated color from the vertex shader.
        #ifdef FEATURE_SKYBOX
        vec4 texel = texture(uCubeMap, fCubemapUV);
        #elif defined(FEATURE_COLOR_REPLACE)
        vec4 texel = texture(uDiffuse, fUV1);
        #elif defined(FEATURE_TEXTURING)
        vec4 texel = texture(uDiffuse, fUV1) * uColor;
        #else
        vec4 texel = fColor * uColor;
        #endif

        // Check for discard due to alpha test or discard color.
        if(texel.a < gAlphaTest || distance(texel.rgb, gDiscardColor.rgb) < gDiscardColorTolerance) { discard; }

        #ifdef FEATURE_DRAW_POINTS_AS_CIRCLES
        // By default, points render as squares.
        // When rendering points, "gl_PointCoord" varies from 0.0-1.0 from bottom-left to top-right of the point.
        // We can do a bit of math to discard some pixels and make it look like a circle.

        // Convert the coordinate from range "0 to 1" to instead be "-1 to 1".
        vec2 circCoord = 2.0 * gl_PointCoord - 1.0;

        // If the length squared of this vector is greater than one, discard the pixel.
        // This gets rid of pixels in a circular way around the center of the point.
        if(dot(circCoord, circCoord) > 1.0) {
            discard;
        }
        #endif

        // Apply lighting if desired.
        #ifdef FEATURE_LIGHTING
        // Normal must be renormalized - interpolation from vertex shader may have changed length.
        vec3 normalDir = normalize(fNormal);

        // Light direction was (purposely) not normalized in vertex shader (so it'd interpolate correctly).
        // Must be normalized now.
        vec3 lightDir = normalize(fLightDir);

        // Lighting calculations!
        float saturate = clamp(dot(normalDir, lightDir), 0.0f, 1.0f);
        vec4 directColor = uLightColor * saturate;
        vec4 allLightColors = uAmbientColor + directColor;
        texel = allLightColors * texel;
        #endif

        #ifdef FEATURE_LIGHTMAPS
        // Show/hide diffuse texture. May want to show just lightmap texture for debugging.
        texel = (uDiffuseVisible * texel) + ((1.0f - uDiffuseVisible) * vec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Grab lightmap texel; apply multiplier.
        vec4 lightmapTexel = texture(uLightmap, fUV2);
        lightmapTexel.rgb *= uLightmapMultiplier;

        // Create final output color.
        texel = texel * lightmapTexel;
        #endif

        #ifdef FEATURE_COLOR_REPLACE
        // If the texel's RGB matches the replace color's RGB, replace with main color.
        // Otherwise, just use texel color.
        if(texel.rgb == uReplaceColor.rgb)
        {
            //texel.rgb = uReplaceColor.rgb;
            texel.rgb = uColor.rgb;
        }

        // Multiply output alpha by main color's alpha.
        // This is needed for "fading out".
        texel.a *= uColor.a;
        #endif

        oColor = texel;
    }
#endif