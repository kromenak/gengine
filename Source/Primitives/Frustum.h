//
// Clark Kromenaker
//
// Represents a view frustum. Could be any type of frustum - perspective, orthographic, etc.
//
// A frustum is a pyramid shape with the top part cut off. This shape is usually used to
// represent the camera's view area.
//
#pragma once
#include "Plane.h"

class Matrix4;
class Vector3;

class Frustum
{
public:
    Frustum() = default;
    Frustum(const Matrix4& matrix);

    bool ContainsPoint(const Vector3& point) const;
    //Vector3 GetClosestPoint(const Vector3& point) const;

    // The frustum just consists of 6 planes forming the bounding area.
    // Plane normals are facing inwards.
    Plane near;
    Plane far;
    Plane left;
    Plane right;
    Plane bottom;
    Plane top;
};
