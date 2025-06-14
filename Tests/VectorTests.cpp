//
// Clark Kromenaker
//
// Tests for the Vector2/Vector3/Vector4 class.
//
#include "catch.hh"
#include "Vector3.h"

TEST_CASE("Vector3 constructors are correct")
{
    // Check default vector is all zeros.
    Vector3 defVec3;
    REQUIRE(defVec3.x == 0.0f);
    REQUIRE(defVec3.y == 0.0f);
    REQUIRE(defVec3.z == 0.0f);

    // Check that vector using custom constructor has correct values.
    Vector3 customVec3(10.0f, 12.5f, -2.3456f);
    REQUIRE(customVec3.x == 10.0f);
    REQUIRE(customVec3.y == 12.5f);
    REQUIRE(customVec3.z == -2.3456f);
}

TEST_CASE("Vector3 copy operations and equality checks work correctly")
{
    Vector3 vec1(13.3f, -12.2f, 5.6f);

    // Check that we can use = to copy, and == != work as expected.
    Vector3 vec2 = Vector3::One;
    REQUIRE(vec2 == Vector3::One);
    REQUIRE(vec2 != Vector3::Zero);

    // Usage of copy constructor.
    Vector3 vec3(vec1);
    REQUIRE(vec3 == vec1);
    REQUIRE(vec3 != vec2);

    // Usage of = copying.
    vec2 = vec1;
    REQUIRE(vec3 == vec2);
    REQUIRE(vec1 == vec2);
    REQUIRE(vec1 == vec3);
}

TEST_CASE("Vector3 constants are correct")
{
    REQUIRE(Vector3::Zero == Vector3(0.0f, 0.0f, 0.0f));
    REQUIRE(Vector3::One == Vector3(1.0f, 1.0f, 1.0f));
    REQUIRE(Vector3::UnitX == Vector3(1.0f, 0.0f, 0.0f));
    REQUIRE(Vector3::UnitY == Vector3(0.0f, 1.0f, 0.0f));
    REQUIRE(Vector3::UnitZ == Vector3(0.0f, 0.0f, 1.0f));
}

TEST_CASE("Vector3 getters/setters work correctly")
{
    // Start out with Zero vector.
    Vector3 vec3;
    REQUIRE(vec3 == Vector3::Zero);

    // Set some values.
    vec3.x = 1000.0f;
    vec3.y = 12.5f - 10.2f; // This one might cause floating-point approximation issues!
    vec3.z = -345.0f;

    // Check that values are approximately correct.
    REQUIRE(Math::AreEqual(vec3.x, 1000.0f));
    REQUIRE(Math::AreEqual(vec3.y, 2.3f));
    REQUIRE(Math::AreEqual(vec3.z, -345.0f));
}

TEST_CASE("Vector3 length and normalization operations work correctly")
{
    // Length of zero vector should be zero.
    REQUIRE(Math::AreEqual(Vector3::Zero.GetLength(), 0.0f));
    REQUIRE(Vector3::Zero.Normalize() == Vector3::Zero);

    // Length of unit vectors should be one.
    REQUIRE(Math::AreEqual(Vector3::UnitX.GetLength(), 1.0f));
    REQUIRE(Math::AreEqual(Vector3::UnitY.GetLength(), 1.0f));
    REQUIRE(Math::AreEqual(Vector3::UnitZ.GetLength(), 1.0f));

    // Length of One vector should be...a complicated number.
    // Squared length should be 3 though.
    REQUIRE(Math::AreEqual(Vector3::One.GetLength(), 1.7320508075688772f));
    REQUIRE(Math::AreEqual(Vector3::One.GetLengthSq(), 3.0f));

    // And let's check the length of a fairly complicated vector.
    Vector3 vec3(24.0f, -3.0f, 16.5f);
    REQUIRE(Math::AreEqual(vec3.GetLength(), 29.278831943914703f));
    REQUIRE(Math::AreEqual(vec3.GetLengthSq(), 857.25f));

    // Check that normalization works as expected.
    vec3.Normalize();
    REQUIRE(Math::AreEqual(vec3.GetLength(), 1.0f));
}

TEST_CASE("Vector3 can be added and subtracted")
{
    Vector3 vec1(10.0f, 12.3f, -3.5f);
    Vector3 vec2(-19.543f, 6.25f, -2.00004f);

    // Check addition.
    Vector3 vecAdd = vec1 + vec2;
    REQUIRE(vecAdd == Vector3(-9.543f, 18.55f, -5.50004));

    // Check subtraction.
    Vector3 vecSub = vec1 - vec2;
    REQUIRE(vecSub == Vector3(29.543f, 6.05f, -1.49996f));

    // Check addition and assignment.
    Vector3 vec3 = vec1;
    vec3 += Vector3::One;
    REQUIRE(vec3 == Vector3(11.0f, 13.3f, -2.5f));

    // Check subtraction and assignment.
    vec2 -= vec1;
    REQUIRE(vec2 == Vector3(-29.543f, -6.05f, 1.49996f));
}

TEST_CASE("Vector3 can be multiplied and divided by a scalar")
{
    Vector3 vec1(10.0f, 12.3f, -3.5f);
    Vector3 vec2(-19.543f, 6.25f, -2.00004f);

    // Check scalar multiplication. Both pre and post should work.
    Vector3 vecMult = vec1 * 2.0f;
    REQUIRE(vecMult == Vector3(20.0f, 24.6f, -7.0f));
    vecMult = 2.0f * vec1;
    REQUIRE(vecMult == Vector3(20.0f, 24.6f, -7.0f));

    // Check scalar division. Only as numerator should work.
    Vector3 vecDiv = vec1 / 2.0f;
    REQUIRE(vecDiv == Vector3(5.0f, 6.15f, -1.75f));

    vec1 *= 2.0f;
    REQUIRE(vec1 == vecMult);

    vec2 /= 5.0f;
    REQUIRE(vec2 == Vector3(-3.9086f, 1.25f, -0.400008f));
}

TEST_CASE("Vector3 Dot and Cross products work correctly")
{
    Vector3 vec1(10.0f, 12.3f, -3.5f);
    Vector3 vec2(-19.543f, 6.25f, -2.0f);

    // Test dot product with two random vectors.
    float dotProduct = Vector3::Dot(vec1, vec2);
    REQUIRE(Math::AreEqual(dotProduct, -111.554993f));

    // Dot product with zero should be zero.
    dotProduct = Vector3::Dot(vec1, Vector3::Zero);
    REQUIRE(Math::AreEqual(dotProduct, 0.0f));

    // Dot product of perpendicular vectors should be zero.
    dotProduct = Vector3::Dot(Vector3::UnitX, Vector3::UnitY);
    REQUIRE(Math::AreEqual(dotProduct, 0.0f));

    // Dot product of vectors facing opposite directions should be -1.
    dotProduct = Vector3::Dot(Vector3::UnitX, -Vector3::UnitX);
    REQUIRE(Math::AreEqual(dotProduct, -1.0f));

    // Dot product of vectors facing the same direction should be 1.
    dotProduct = Vector3::Dot(Vector3::UnitZ, Vector3::UnitZ);
    REQUIRE(Math::AreEqual(dotProduct, 1.0f));

    // Test cross product with two random vectors.
    Vector3 crossProduct = Vector3::Cross(vec1, vec2);
    REQUIRE(crossProduct == Vector3(-2.725f, 88.4005f, 302.8789f));
    crossProduct = Vector3::Cross(vec2, vec1);
    REQUIRE(crossProduct == Vector3(2.725f, -88.4005f, -302.8789f));

    // Cross product of of X/Y unit vectors should be the Z unit vector.
    crossProduct = Vector3::Cross(Vector3::UnitX, Vector3::UnitY);
    REQUIRE(crossProduct == Vector3::UnitZ);
}

TEST_CASE("Vector3 LERP works correctly")
{
    Vector3 vec1(0.0f, 0.0f, 0.0f);
    Vector3 vec2(10.0f, 10.0f, 10.0f);

    Vector3 middle = Vector3::Lerp(vec1, vec2, 0.5f);
    REQUIRE(middle == Vector3(5.0f, 5.0f, 5.0f));
}
