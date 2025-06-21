#version 150
in vec2 fUV1;

out vec4 oColor;

// User-defined uniforms
uniform vec4 uColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
uniform sampler2D uDiffuse;

void main()
{
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

    // Render pixel as you would for normal texture-mapped object: sample texture and multiple in color.
    vec4 texel = texture(uDiffuse, fUV1) * uColor;
    oColor = texel;
}
