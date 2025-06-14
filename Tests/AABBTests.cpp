//
// Clark Kromenaker
//
// Tests for AABB class.
//
#include "catch.hh"
#include "AABB.h"

TEST_CASE("AABB creation works")
{
    // Most basic AABB should be valid and basically just be Vector3::Zero.
    AABB basic;
    REQUIRE(basic.GetMin() == Vector3::Zero);
    REQUIRE(basic.GetMax() == Vector3::Zero);
    REQUIRE(basic.GetCenter() == Vector3::Zero);
    REQUIRE(basic.GetExtents() == Vector3::Zero);
    REQUIRE(basic.IsValid());
    REQUIRE(basic.ContainsPoint(Vector3::Zero));
    REQUIRE(basic.GetClosestPoint(Vector3(20.0f, -10.0f, 5.5f)) == Vector3::Zero);

    // Try a more complex AABB with min/max points.
    Vector3 min(5.0f, 10.0f, -2.0f);
    Vector3 max(10.0f, 15.0f, 2.0f);
    AABB minMax(min, max);
    REQUIRE(minMax.GetMin() == min);
    REQUIRE(minMax.GetMax() == max);
    REQUIRE(minMax.GetCenter() == Vector3(7.5f, 12.5f, 0.0f));
    REQUIRE(minMax.GetExtents() == Vector3(2.5f, 2.5f, 2.0f));
    REQUIRE(minMax.IsValid());

    // Try creating an invalid AABB.
    AABB invalid(max, min);
    REQUIRE(invalid.GetMin() == max);
    REQUIRE(invalid.GetMax() == min);
    REQUIRE(!invalid.IsValid());
}

TEST_CASE("AABB contains point works")
{
    Vector3 min(5.0f, 10.0f, -2.0f);
    Vector3 max(10.0f, 15.0f, 2.0f);
    AABB aabb(min, max);
    REQUIRE(aabb.ContainsPoint(min));
    REQUIRE(aabb.ContainsPoint(max));
    REQUIRE(aabb.ContainsPoint(Vector3(9.4f, 12.1453f, 0.004412f)));
    REQUIRE(!aabb.ContainsPoint(Vector3(4.9f, 12.0f, 1.54f)));
}

TEST_CASE("AABB get closest point works")
{
    Vector3 min(64.0f, -30.0f, 8.5f);
    Vector3 max(90.0f, -10.0f, 12.0f);
    AABB aabb(min, max);

    // GetClosestPoint with a point contained in the AABB should just return the point.
    Vector3 contained(70.0f, -15.0f, 10.0f);
    REQUIRE(aabb.ContainsPoint(contained));
    REQUIRE(aabb.GetClosestPoint(contained) == contained);

    // Try something a bit more complex...
    REQUIRE(aabb.GetClosestPoint(Vector3(200.0f, 50.0f, 99.0f)) == max);
    REQUIRE(aabb.GetClosestPoint(Vector3(0.0f, -90.0f, 5.0f)) == min);
    REQUIRE(aabb.GetClosestPoint(Vector3(76.0f, 0.0f, 5.0f)) == Vector3(76.0f, -10.0f, 8.5f));
}
