//
// Clark Kromenaker
//
// Tests for Sphere class.
//
#include "catch.hh"
#include "Sphere.h"

TEST_CASE("Sphere contains point check works")
{
    // Sphere at (0, 0, 0) with radius 10.
    Sphere s(Vector3::Zero, 10.0f);

    // Test a few points that should be within the sphere.
    REQUIRE(s.ContainsPoint(Vector3::Zero));
    REQUIRE(s.ContainsPoint(Vector3(5.0f, 5.0f, 5.0f)));
    REQUIRE(s.ContainsPoint(Vector3(-8.0f, 3.0f, -4.5f)));

    // Test a point on the surface of the sphere. We expect this to be true.
    REQUIRE(s.ContainsPoint(Vector3(0.0f, 0.0f, 10.0f)));

    // Test some points definitely outside the sphere.
    REQUIRE(!s.ContainsPoint(Vector3(10.0f, 10.0f, 10.0f)));
    REQUIRE(!s.ContainsPoint(Vector3(8.0f, 9.0f, 5.0f)));

    // Sphere not at origin with unusual radius.
    Sphere s2(Vector3(1000.0f, 95.0f, -100.0f), 42.0f);

    // Test some points that should be within sphere.
    REQUIRE(s2.ContainsPoint(Vector3(1004.9492f, 90.0f, -70.0f)));
    REQUIRE(s2.ContainsPoint(Vector3(1039.0f, 96.0f, -90.0f)));
    REQUIRE(s2.ContainsPoint(Vector3(1000.0f, 95.0f, -100.0f)));

    // Test point on the surface of the sphere.
    REQUIRE(s2.ContainsPoint(Vector3(1042.0f, 95.0f, -100.0f)));

    // Test some points not in the sphere.
    REQUIRE(!s2.ContainsPoint(Vector3::Zero));
    REQUIRE(!s2.ContainsPoint(Vector3(65.0f, -10.0f, 200.0f)));
}

TEST_CASE("Sphere closest surface point works")
{
    // Simple case check.
    Sphere s(Vector3::Zero, 20.0f);
    REQUIRE(s.GetClosestSurfacePoint(Vector3(100.0f, 0.0f, 0.0f)) == Vector3(20.0f, 0.0f, 0.0f));
    REQUIRE(s.GetClosestSurfacePoint(Vector3(-62.0542f, 0.0f, 0.0f)) == Vector3(-20.0f, 0.0f, 0.0f));
    REQUIRE(s.GetClosestSurfacePoint(Vector3(0.0f, 95.443f, 0.0f)) == Vector3(0.0f, 20.0f, 0.0f));
}
