//
// Clark Kromenaker
//
// Tests for Plane class
//
#include "catch.hh"
#include "Plane.h"

TEST_CASE("Plane construction")
{
    // Test default construction - should be distance zero, default UnitX normal.
    Plane p1;
    REQUIRE(p1.normal == Vector3::UnitX);
    REQUIRE(p1.distance == 0.0f);

    // Test normal/distance constructors.
    Vector3 normal = Vector3::UnitZ;
    float distance = -54.2345f;
    Plane p2(normal, distance);
    REQUIRE(p2.normal == normal);
    REQUIRE(p2.distance == distance);

    Plane p3(normal.x, normal.y, normal.z, distance);
    REQUIRE(p3.normal == normal);
    REQUIRE(p3.distance == distance);

    // Test triangle constructor (simple triangle on x/y plane, but 10 units out on the z-axis).
    Vector3 point0(1.0f, 0.0f, 10.0f);
    Vector3 point1(0.0f, 1.0f, 10.0f);
    Vector3 point2(-1.0f, 0.0f, 10.0f);
    Plane p4(point0, point1, point2);
    REQUIRE(p4.normal == Vector3::UnitZ);
    REQUIRE(p4.distance == -10.0f);
}

TEST_CASE("Plane distance")
{
    // A plane facing +X, but located at -1000.0f.
    // Check points in front of, on, and behind the plane.
    Plane p1(Vector3::UnitX, 1000.0f);
    REQUIRE(p1.GetSignedDistance(Vector3::Zero) == 1000.0f);
    REQUIRE(p1.GetSignedDistance(Vector3(-1000.0f, 0.0f, 0.0f)) == 0.0f);
    REQUIRE(p1.GetSignedDistance(Vector3(-1100.0f, 0.0f, 0.0f)) == -100.0f);

    // Regular distance should just be absolute values.
    REQUIRE(p1.GetDistance(Vector3(-1100.0f, 0.0f, 0.0f)) == 100.0f);
}

TEST_CASE("Plane contains point")
{
    // Simple case - default plane contains origin.
    Plane p1;
    REQUIRE(p1.ContainsPoint(Vector3::Zero));

    // A plane facing +Z and 90 units in positive Z direction.
    // Such a plane should contain any point with a Z value of 90.
    Plane p2(Vector3::UnitZ, -90.0f);
    REQUIRE(!p2.ContainsPoint(Vector3::Zero));
    REQUIRE(p2.ContainsPoint(Vector3(0.0f, 0.0f, 90.0f)));
    REQUIRE(p2.ContainsPoint(Vector3(100.0f, -302.32034f, 90.0f)));

    // Create a plane from a rando triangle.
    // The plane should contain all points.
    Vector3 point0(1.9f, -2.5f, 6.1f);
    Vector3 point1(0.0f, 8.4f, 4.3f);
    Vector3 point2(-10.143f, -1.4f, 9.54f);
    Plane p3(point0, point1, point2);
    REQUIRE(p3.ContainsPoint(point0));
    REQUIRE(p3.ContainsPoint(point1));
    REQUIRE(p3.ContainsPoint(point2));
}

TEST_CASE("Plane get closest point")
{
    // Simple case - plane through origin down +X.
    // For any point, the closest point on plane just squashes the X-component.
    Plane p1(Vector3::UnitX, 0.0f);
    REQUIRE(p1.GetClosestPoint(Vector3(100.0f, 0.0f, 0.0f)) == Vector3::Zero);
    REQUIRE(p1.GetClosestPoint(Vector3(600.0f, -34.5f, 10.8f)) == Vector3(0.0f, -34.5f, 10.8f));

    // More complex case.
    Plane p2(Vector3::UnitY, 1024.5f);
    REQUIRE(p2.GetClosestPoint(Vector3::Zero) == Vector3(0.0f, -1024.5f, 0.0f));
    REQUIRE(p2.GetClosestPoint(Vector3()) == Vector3(0.0f, -1024.5f, 0.0f));
}

