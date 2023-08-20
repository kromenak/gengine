//
// RenderTransforms.h
//
// Clark Kromenaker
//
// Logic for generating transforms related to rendering.
// Includes look-at matrix, projection matrix, unproject matrix, etc.
//
#pragma once
#include "Matrix4.h"
#include "Vector2.h"
#include "Vector3.h"

// Defines whether view space is left-handed or right-handed.
#define VIEW_LH 0
#define VIEW_RH 1
#define VIEW_HAND VIEW_LH

namespace RenderTransforms
{
    // View Matrix
    Matrix4 MakeLookAt(const Vector3& eye, const Vector3& lookAt, const Vector3& up);
    
    // Projection Matrices
    Matrix4 MakePerspective(float fovAngle, float aspectRatio, float near, float far);
    Matrix4 MakeOrthographic(float left, float right, float bottom, float top, float near, float far);
    Matrix4 MakeOrthoBottomLeft(float width, float height);

    // Helpers
    Vector4 ScreenPointToNDCPoint(const Vector2& screenPoint, float distance, float screenWidth, float screenHeight);
}
