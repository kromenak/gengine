#include "Frustum.h"

#include "Matrix4.h"

Frustum::Frustum(const Matrix4& matrix)
{
    // Usually, a frustum is generated from a matrix (passed in).
    // One very cool thing is that the matrix passed in can dictate what "space" the frustum is in!
    //
    // Pass in a projection matrix => the generated frustum is in view space.
    // Pass in a (proj * view) matrix => the generated frustum is in world space.
    // Pass in a (proj * view * world) matrix => the generated frustum is in model space.

    // This code is surprisingly simple, and is taken from "the Gribb/Hartmann paper" (http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf).
    //TODO: Note that the code below needs to change slightly if we're using DirectX (because the near/far is [0, 1] instead of [-1, 1]).
    Vector4 rows[4];
    matrix.GetRows(rows[0], rows[1], rows[2], rows[3]);
    near = rows[3] - rows[2];
    far = rows[3] + rows[2];
    left = rows[3] + rows[0];
    right = rows[3] - rows[0];
    bottom = rows[3] + rows[1];
    top = rows[3] - rows[1];

    // The generated planes are not normalized, and probably need to be to be useful.
    near.Normalize();
    far.Normalize();
    left.Normalize();
    right.Normalize();
    top.Normalize();
    bottom.Normalize();
}

bool Frustum::ContainsPoint(const Vector3& point) const
{
    // All frustum planes have their normals pointing inwards.
    // Therefore, a point is contained within the frustum if signed distance from ALL planes is positive.
    return near.GetSignedDistance(point) >= 0.0f &&
        far.GetSignedDistance(point) >= 0.0f &&
        left.GetSignedDistance(point) >= 0.0f &&
        right.GetSignedDistance(point) >= 0.0f &&
        bottom.GetSignedDistance(point) >= 0.0f &&
        top.GetSignedDistance(point) >= 0.0f;
}

//TODO: Problem with this code: it gets nearest point to *planes* rather than the actual box of the frustum. Not correct.
//TODO: Need to research correct algorithm for this.
/*
Vector3 Frustum::GetClosestPoint(const Vector3& point) const
{
    // First, find the closest plane.
    float closestDist = near.GetDistance(point);
    const Plane* closestPlane = &near;

    float dist = far.GetDistance(point);
    if(dist < closestDist)
    {
        closestPlane = &far;
        closestDist = dist;
    }

    dist = left.GetDistance(point);
    if(dist < closestDist)
    {
        closestPlane = &left;
        closestDist = dist;
    }

    dist = right.GetDistance(point);
    if(dist < closestDist)
    {
        closestPlane = &right;
        closestDist = dist;
    }

    dist = bottom.GetDistance(point);
    if(dist < closestDist)
    {
        closestPlane = &bottom;
        closestDist = dist;
    }

    dist = top.GetDistance(point);
    if(dist < closestDist)
    {
        closestPlane = &top;
        closestDist = dist;
    }

    // Get closest point to the closest plane.
    return closestPlane->GetClosestPoint(point);
}
*/