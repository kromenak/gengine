//
// Clark Kromenaker
//
// Tests for collision/intersection logic between geometric primitives.
//
#include "catch.hh"
#include "Collisions.h"
#include "Sphere.h"
#include "Triangle.h"

TEST_CASE("Sphere intersect triangle works")
{
    // Create a sphere at the origin.
    Sphere s(Vector3::Zero, 10.0f);

    // Triangle down the +x axis.
    Triangle t(Vector3(5.0f, -2.0f, -2.0f),
               Vector3(5.0f, -2.0f,  2.0f),
               Vector3(5.0f,  2.0f,  0.0f));

    Vector3 intersect;
    REQUIRE(Intersect::TestSphereTriangle(s, t, intersect));
    REQUIRE(intersect == Vector3(-5.0f, 0.0f, 0.0f));

    // Move sphere by intersection amount.
    Sphere s2(Vector3::Zero + intersect, 10.0f);
    REQUIRE(!Intersect::TestSphereTriangle(s2, t, intersect));
}
