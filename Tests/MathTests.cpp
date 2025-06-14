//
// Clark Kromenaker
//
// Tests for general math functions.
//
#include "catch.hh"
#include "GMath.h"

TEST_CASE("Floating-point comparison works")
{
    float f1 = 4.7f;
    float f2 = 3.14f;
    float result = f1 / f2;
    REQUIRE(Math::AreEqual(result, 1.4968152866f));
}

TEST_CASE("Math::Abs works")
{
    REQUIRE(Math::Abs(-5.0f) == 5.0f);
}
