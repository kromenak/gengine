#include "Plane.h"

#include "Matrix4.h"
#include "Vector4.h"

Plane::Plane(const Vector3& normal, float distance) :
    normal(normal),
    distance(distance)
{

}

Plane::Plane(const Vector3& normal, const Vector3& p0) :
    normal(normal),
    distance(-Vector3::Dot(p0, normal)) // (p dot n + d = 0), therefore (d = -(p dot n))
{

}

Plane::Plane(float normalX, float normalY, float normalZ, float distance) :
    normal(normalX, normalY, normalZ),
    distance(distance)
{

}

Plane::Plane(const Vector4& normalAndDist) :
    normal(normalAndDist.x, normalAndDist.y, normalAndDist.z),
    distance(normalAndDist.w)
{

}

Plane::Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
    // Calculate normal using cross product.
    // NOTE: assuming a clockwise winding order for the points.
    normal = Vector3::Cross(p1 - p0, p2 - p0);
    normal.Normalize();

    // Plane equation is "n dot p + d = 0", therefore "n dot p = -d" or "-(n dot p) = d".
    // Any of the three points works here.
    distance = -Vector3::Dot(normal, p0);
}

void Plane::Normalize()
{
    float invLength = 1.0f / normal.GetLength();
    normal *= invLength;
    distance *= invLength;
}

/*static*/ Plane Plane::Normalize(const Plane& plane)
{
    Plane p = plane;
    p.Normalize();
    return p;
}

void Plane::Transform(const Matrix4& aToBTransform, bool preInverted)
{
    // Plane transformation is similar to normal transformation.
    // Passed in matrix transforms points from system A to system B: (pB = M * pA)
    // But it would do the opposite for planes and normals: (pA = pB * M)

    // To get the matrix to take a plane from A to B, we must invert it: (pB = pA * invM)
    // Unlike normals, however, we cannot avoid inversion if the matrix is orthogonal (because the translation in column 3 is used).

    // Let's say the caller needs to transform a dozen planes - to improve performance, they may invert the matrix once and pass it in.
    // If caller says the passed in matrix is already inverted, don't invert it here - just use it directly.
    Matrix4 inverse = preInverted ? aToBTransform : Matrix4::Inverse(aToBTransform);

    // Perform (pB = pA * invM).
    float newX = normal.x * inverse(0, 0) + normal.y * inverse(1, 0) + normal.z * inverse(2, 0);
    float newY = normal.x * inverse(0, 1) + normal.y * inverse(1, 1) + normal.z * inverse(2, 1);
    float newZ = normal.x * inverse(0, 2) + normal.y * inverse(1, 2) + normal.z * inverse(2, 2);
    float newD = normal.x * inverse(0, 3) + normal.y * inverse(1, 3) + normal.z * inverse(2, 3) + distance;
    normal.x = newX;
    normal.y = newY;
    normal.z = newZ;
    distance = newD;

    // Make sure plane is still normalized.
    Normalize();
}

float Plane::GetSignedDistance(const Vector3& point) const
{
    // A point must be in front of, behind, or on the plane.
    // Signed distance (also sometimes called "the plane test") is the distance from the plane,
    // but it is positive if point is in front of the plane, negative if behind, and zero if on the plane.
    return Vector3::Dot(normal, point) + distance;
}

float Plane::GetDistance(const Vector3& point) const
{
    // Absolute value of signed distance does the trick.
    return Math::Abs(GetSignedDistance(point));
}

bool Plane::ContainsPoint(const Vector3& point) const
{
    return Math::IsZero(GetSignedDistance(point));
}

Vector3 Plane::GetClosestPoint(const Vector3& point) const
{
    // Get a vector from plane surface to the point.
    Vector3 planeToPoint = GetSignedDistance(point) * normal;

    // Negate to get vector from point to plane.
    // Add to point to get a point on the plane!
    return point - planeToPoint; // Equivalent to "point + (-planeToPoint)"
}

/*static*/ bool Plane::GetIntersectionPoint(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& outPoint)
{
    // Taken from https://gdbooks.gitbooks.io/3dcollisions/content/Chapter1/three_plane_intersection.html
    Vector3 p1Normal = p1.normal;
    Vector3 p2Normal = p2.normal;
    Vector3 p3Normal = p3.normal;

    Vector3 m1(p1Normal.x, p2Normal.x, p3Normal.x);
    Vector3 m2(p1Normal.y, p2Normal.y, p3Normal.y);
    Vector3 m3(p1Normal.z, p2Normal.z, p3Normal.z);

    Vector3 u = Vector3::Cross(m2, m3);

    // Calculate denominator.
    // If zero, planes don't actually intersect.
    float denom = Vector3::Dot(m1, u);
    if(Math::IsZero(denom))
    {
        // Return false; don't touch outPoint.
        return false;
    }

    Vector3 d(p1.distance,
              p2.distance,
              p3.distance);
    Vector3 v = Vector3::Cross(m1, d);

    // Calculate point.
    outPoint = Vector3(Vector3::Dot(d, u) / denom,
                   Vector3::Dot(m3, v) / denom,
                   -Vector3::Dot(m2, v) / denom);
    return true;
}
