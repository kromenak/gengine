#include "RenderTransforms.h"

Matrix4 RenderTransforms::MakeLookAt(const Vector3& eye, const Vector3& lookAt, const Vector3& up)
{
    // Using look-at point and eye position, calculate forward (view) direction.
    Vector3 viewFwd = lookAt - eye;
    viewFwd.Normalize();

    // Using forward and up, calculate side vector (using left-hand rule, this is pointing right).
    Vector3 viewSide = Vector3::Cross(up, viewFwd);
    viewSide.Normalize();

    // Calculate up from forward and side.
    // No need to normalize because cross product between perpendicular unit vectors is also unit.
    Vector3 viewUp = Vector3::Cross(viewFwd, viewSide);

    // Traditionally, OpenGL has +Z point away from view forward (RH system).
    // However, we can ignore this and use LH if we account for it in the projection matrix math.
    #if VIEW_HAND == VIEW_RH
    viewFwd *= -1;
    #endif

    // Negating the side axis is one way we can reflect the view space, which has the effect of making world space appear right-handed!
    //viewSide *= -1;

    // This commented out code explains the full algorithm for calculating the "world to view" matrix.
    // But the actual code below this does the same thing in a more compact (and less clear) way.
    /*
    // This is where we define the view space coordinate system.
    // Using X as side, Y as up, and Z as forward is pretty much undisputed in graphics at this point.
    Matrix3 rotate;
    rotate[0] = viewSide; // X axis is side
    rotate[1] = viewUp;   // Y axis is up
    rotate[2] = viewFwd;  // Z axis is forward

    // The matrix defines a "view to world" rotation matrix. But we actually need a "world to view" rotation matrix!
    // To fix this, invert the rotation matrix. Since the rotation matrix is orthoganal, we can simply transpose to get the same effect.
    rotate.Transpose();

    // We have the rotation part of the "world to view" matrix, now we need the translation part.
    // The eye is in world space, but it defines the origin of view space - we need to use it for the translation part of the "world to view" matrix.
    // Multiply eye by the "world to view" rotation to properly rotate it.
    Vector3 eyeInv = -(rotate * eye);

    // Make a 4x4 transform matrix based on the 3x3 rotation matrix.
    // Then manually fill in the translation bits.
    Matrix4 worldToViewMatrix = Matrix4::MakeRotate(rotate);
    worldToViewMatrix(0, 3) = eyeInv.x;
    worldToViewMatrix(1, 3) = eyeInv.y;
    worldToViewMatrix(2, 3) = eyeInv.z;
    return worldToViewMatrix;
    */

    // This does exactly what the above commented out code is doing, just more compactly/efficiently.
    Matrix4 lookAtMatrix(viewSide.x, viewSide.y, viewSide.z, -Vector3::Dot(viewSide, eye),
                         viewUp.x,   viewUp.y,   viewUp.z,   -Vector3::Dot(viewUp, eye),
                         viewFwd.x,  viewFwd.y,  viewFwd.z,  -Vector3::Dot(viewFwd, eye),
                         0.0f,       0.0f,     0.0f,      1.0f);
    return lookAtMatrix;
}

Matrix4 RenderTransforms::MakePerspective(float fovAngle, float aspectRatio, float near, float far)
{
    // Perspective projection matrix converts a point from view space to clip space.
    // Once in clip space, dividing by w-component (perspective divide) puts point in normalized device coordinates (NDC).

    // If view space is left-handed (+Z is view dir), near and far are positive in view space.
    // If view space is right-handed (-Z is view dir), near and far are negative in view space.
    // That difference causes some math between LH and RH calculations.

    // Also, the coordinate system in NDC differs depending on graphics system, which affects the math.
    // Everyone maps X/Y to [-1, 1], but OpenGL maps Z to [-1, 1] and DirectX maps Z to [0, 1].

    // Start with zeroed out matrix.
    Matrix4 m = Matrix4::Zero;

    // Calculate width and height of the projection, which converts x and y components from view space to clip space.
    // One way to do this is with fov and aspect ratio. Another option is use near/top/bottom/left/right values to calc this.
    // This is consistent between DirectX, OpenGL, LH, and RH - nice and easy. And all map this to [-1, 1] range in NDC.
    float height = 1.0f / Math::Tan(fovAngle * 0.5f);
    float width = height / aspectRatio;
    m(0, 0) = width;
    m(1, 1) = height;

    // NDC uses a LH coordinate system, so if view space is RH, the matrix needs to flip that.
    // To do this, we can force the w-component in clip space to be either positive or negative, depending on our needs.
    // We want it to be negative (resulting in an axis flip) if view space is RH.
    #if VIEW_HAND == VIEW_LH
    m(3, 2) = 1.0f;  // If view space is LH
    #else
    m(3, 2) = -1.0f; // If view space is RH
    #endif

    // Converting z component from view to clip space is most complex.
    // It depends on LH/RH (because Z-axis direction differs) and graphics system (GL maps to [-1, 1] while DX maps to [0, 1]).
    // These equations are derived from known values (e.g. "near" equals -1 or 0). See http://www.songho.ca/opengl/gl_projectionmatrix.html for a good explanation.
    #if VIEW_HAND == VIEW_LH
    // OpenGL, LH view space
    m(2, 2) = -(far + near) / (far - near);
    m(2, 3) = (2.0f * near * far) / (far - near);
    #else
    // OpenGL, RH view space
    m(2, 2) = -(far + near) / (far - near);
    m(2, 3) = -(2.0f * near * far) / (far - near);
    #endif

    // DirectX, LH view space
    //m(2, 2) = far / (far - near);
    //m(2, 3) = -near * (far / (far - near));

    // DirectX, RH view space
    //m(2, 2) = far / (far - near);
    //m(2, 3) = near * (far / (far - near));
    return m;
}

Matrix4 RenderTransforms::MakeOrthographic(float left, float right, float bottom, float top, float near, float far)
{
    // As with perspective projection, the math can differ somewhat depending on
    // whether view space is LH or RH, and whether Z-axis is mapped to [-1, 1] or [0, 1].

    // Start with identity.
    Matrix4 m = Matrix4::Identity;

    // Calculate conversion of x component to clip space (row 0).
    m(0, 0) = 2.0f / (right - left);
    m(0, 3) = -(right + left) / (right - left);

    // Calculate conversion of x component to clip space (row 1).
    m(1, 1) = 2.0f / (top - bottom);
    m(1, 3) = -(top + bottom) / (top - bottom);

    // Converting z component is more complex (row 3).
    // If view space is RH, the z-axis must flip during conversion.
    // And again, GL uses [-1, 1] while DX uses [0, 1].
    #if VIEW_HAND == VIEW_LH
    // OpenGL, LH view space
    m(2, 2) = 2.0f / (far - near);
    m(2, 3) = -(far + near) / (far - near);
    #else
    // OpenGL, RH view space
    m(2, 2) = -2.0f / (far - near);
    m(2, 3) = -(far + near) / (far - near);
    #endif

    // DirectX, LH view space
    //m(2, 2) = 1.0f / (far - near);
    //m(2, 3) = -near / (far - near);

    // DirectX, RH view space
    //m(2, 2) = -1.0f / (far - near);
    //m(2, 3) = -near / (far - near);
    return m;
}

Matrix4 RenderTransforms::MakeOrthoBottomLeft(float width, float height)
{
    // An orthographic projection with origin in bottom-left corner; useful for UI rendering.
    // This assumes left/bottom are 0, right/top are width/height, near/far are defaults.
    Matrix4 m = Matrix4::Identity;

    // Convert x component to clip space (row 0).
    m(0, 0) = 2.0f / width;
    m(0, 3) = -1.0f;        // -(width + 0) / (width - 0)

    // Convert y component to clip space (row 1).
    m(1, 1) = 2.0f / height;
    m(1, 3) = -1.0f;        // -(height + 0) / (height - 0)

    // Convert z component to clip space (row 2).
    // Again, differs by LH/RH and graphics API
    #if VIEW_HAND == VIEW_LH
    // OpenGL, LH view space
    m(2, 2) = 1.0f;         // 2.0f / (far - near)
    //m(2, 3) = 0.0f;       // -(far + near) / (far - near)
    #else
    // OpenGL, RH view space
    m(2, 2) = -1.0f;
    //m(2, 3) = 0.0f;
    #endif

    // DirectX, LH view space
    //m(2, 2) = 1.0f;
    //m(2, 3) = 0.0f;

    // DirectX, RH view space
    //m(2, 2) = -1.0f;
    //m(2, 3) = 0.0f;
    return m;
}

Vector4 RenderTransforms::ScreenPointToNDCPoint(const Vector2& screenPoint, float distance, float screenWidth, float screenHeight)
{
    // We need the point to be in normalized device coordinates.
    // Normalized device coordinates vary from (-1, -1) in lower-left and (1, 1) in upper right.
    //
    //                  |-----------| (screenWidth, screenHeight) (1, 1)
    //                  |           |
    //                  |           |
    //  (0, 0) (-1, -1) |-----------|
    //

    // X and Y are similar and consistent.
    // Both should map [-1, 1] in NDC, regardless of LH/RH and regardless of GL/DX.
    float ndcX = (2.0f * (screenPoint.x / screenWidth)) - 1.0f;
    float ndcY = (2.0f * (screenPoint.y / screenHeight)) - 1.0f;

    // As we say with the projection matrix calculations, Z is more complex.
    // For GL, map to [-1, 1]. For DX, map [0, 1].
    // If LH, the value must also be negated.
    float ndcZ = (2.0f * distance) - 1.0f;
    //float ndcZ = distance; // DirectX
    #if VIEW_HAND == VIEW_LH
    ndcZ *= -1.0f;
    #endif

    // Return NDC point.
    return Vector4(ndcX, ndcY, ndcZ, 1.0f);
}
