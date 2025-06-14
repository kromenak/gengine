//
// Clark Kromenaker
//
// Tests for the Matrix4 class.
//
#include "catch.hh"
#include "Matrix4.h"

SCENARIO("Multiply Two Matrix4")
{
    GIVEN("Two Matrix4")
    {
        Matrix4 mat1(6, 10, 8, 12,
                     -5, 60, 80, 12,
                     12, 15, -20, -1,
                     25, 33, 42, 16);

        Matrix4 mat2(0.5f, -1.3f, 5.0f, 100.0f,
                     -256.0f, 34.5f, 2.0f, 32.0f,
                     -17.0f, 14.0f, -20.0f, -4.0f,
                     0.6f, 33.0f, 7.0f, -43.0f);

        WHEN("the matrices are multiplied")
        {
            Matrix4 result = mat1 * mat2;
            THEN("the result is correct")
            {
                REQUIRE(result(0,0) == -2685.8f);
                REQUIRE(result(0,1) == 845.2f);
                REQUIRE(result(0,2) == -26.0f);
                REQUIRE(result(0,3) == 372.0f);
                REQUIRE(result(1,0) == -16715.3f);
                REQUIRE(result(1,1) == 3592.5f);
                REQUIRE(result(1,2) == -1421.0f);
                REQUIRE(result(1,3) == 584.0f);
                REQUIRE(result(2,0) == -3494.6f);
                REQUIRE(result(2,1) == 188.9f);
                REQUIRE(result(2,2) == 483.0f);
                REQUIRE(result(2,3) == 1803.0f);
                REQUIRE(result(3,0) == -9139.9f);
                REQUIRE(result(3,1) == 2222.0f);
                REQUIRE(result(3,2) == -537.0f);
                REQUIRE(result(3,3) == 2700.0f);
            }
        }
    }
}

TEST_CASE("Test multiply Vector4 by translation Matrix4")
{
    Matrix4 translationMatrix = Matrix4::MakeTranslate(Vector3(5, 10, 20));
    Vector4 position(1.0f);

    Vector4 result = translationMatrix * position;
    REQUIRE(Math::AreEqual(result.x, 5.0f));
    REQUIRE(Math::AreEqual(result.y, 10.0f));
    REQUIRE(Math::AreEqual(result.z, 20.0f));
}

TEST_CASE("Test multiply Vector4 by rotation Matrix4")
{
    Matrix4 rotate90YMatrix = Matrix4::MakeRotateY(Math::kPiOver2);
    Vector4 xAxis = Vector4::UnitX;

    Vector4 result = rotate90YMatrix * xAxis;
    REQUIRE(Math::AreEqual(result.x, 0.0f));
    REQUIRE(Math::AreEqual(result.y, 0.0f));
    REQUIRE(Math::AreEqual(result.z, -1.0f));

    Matrix4 rotate90ZMatrix = Matrix4::MakeRotateZ(Math::kPiOver2);
    result = rotate90ZMatrix * xAxis;
    REQUIRE(Math::AreEqual(result.x, 0.0f));
    REQUIRE(Math::AreEqual(result.y, 1.0f));
    REQUIRE(Math::AreEqual(result.z, 0.0f));
}

TEST_CASE("Test multiply Vector4 by scale Matrix4")
{
    Matrix4 scaleUpMatrix = Matrix4::MakeScale(Vector3(2.0f, 2.0f, 2.0f));
    Vector4 pos(1.0f, 1.0f, 1.0f, 1.0f);

    Vector4 result = scaleUpMatrix * pos;
    REQUIRE(Math::AreEqual(result.x, 2.0f));
    REQUIRE(Math::AreEqual(result.y, 2.0f));
    REQUIRE(Math::AreEqual(result.z, 2.0f));
    REQUIRE(Math::AreEqual(result.w, 1.0f));

    Matrix4 scaleDownMatrix = Matrix4::MakeScale(Vector3(0.5f, 0.5f, 0.5f));
    result = scaleDownMatrix * pos;
    REQUIRE(Math::AreEqual(result.x, 0.5f));
    REQUIRE(Math::AreEqual(result.y, 0.5f));
    REQUIRE(Math::AreEqual(result.z, 0.5f));
    REQUIRE(Math::AreEqual(result.w, 1.0f));
}

TEST_CASE("Test calculate the inverse of Matrix4")
{
    Matrix4 matrix(1, 1, 1, 23,
                   0, 3, 1, 10,
                   2, 3, 1, 14,
                   0, 0, 0, 1);
    Matrix4 inverse = Matrix4::Inverse(matrix);

    // Make sure values are correct in the inverse matrix.
    REQUIRE(Math::AreEqual(inverse(0,0), 0.0f));
    REQUIRE(Math::AreEqual(inverse(1,0), -0.5f));
    REQUIRE(Math::AreEqual(inverse(2,0), 1.5f));
    REQUIRE(Math::AreEqual(inverse(3,0), 0.0f));

    REQUIRE(Math::AreEqual(inverse(0,1), -0.5f));
    REQUIRE(Math::AreEqual(inverse(1,1), 0.25f));
    REQUIRE(Math::AreEqual(inverse(2,1), 0.25f));
    REQUIRE(Math::AreEqual(inverse(3,1), 0.0f));

    REQUIRE(Math::AreEqual(inverse(0,2), 0.5f));
    REQUIRE(Math::AreEqual(inverse(1,2), 0.25f));
    REQUIRE(Math::AreEqual(inverse(2,2), -0.75f));
    REQUIRE(Math::AreEqual(inverse(3,2), 0.0f));

    REQUIRE(Math::AreEqual(inverse(0,3), -2.0f));
    REQUIRE(Math::AreEqual(inverse(1,3), 5.5f));
    REQUIRE(Math::AreEqual(inverse(2,3), -26.5f));
    REQUIRE(Math::AreEqual(inverse(3,3), 1.0f));

    // Actually do the math and make sure we get identity.
    Matrix4 result = matrix * inverse;
    REQUIRE(result == Matrix4::Identity);
}

TEST_CASE("Extract Translation/Rotation from Matrix4")
{
    Vector3 translation = Vector3(5.0f, 20.0f, -4.0f);
    Quaternion rotation = Quaternion(Vector3::UnitY, Math::kPiOver2);

    Matrix4 translationMatrix = Matrix4::MakeTranslate(translation);
    Matrix4 rotationMatrix = Matrix4::MakeRotate(rotation);
    Matrix4 combinedMatrix = translationMatrix * rotationMatrix;

    Vector3 extractedTranslation = combinedMatrix.GetTranslation();
    Quaternion extractedRotation = combinedMatrix.GetRotation();

    REQUIRE(extractedTranslation == translation);
    REQUIRE(extractedRotation == rotation);
}

TEST_CASE("Transform a normal vector")
{
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Matrix4 rotationMatrix = Matrix4::MakeRotateZ(Math::kPiOver2);
    Vector3 transformedVector = rotationMatrix.TransformVector(normal);
    Vector3 transformedNormal = rotationMatrix.TransformNormal(normal);
    REQUIRE(transformedNormal == Vector3(0.0f, 1.0f, 0.0f));

    Matrix4 scaleMatrix = Matrix4::MakeScale(10);
    transformedVector = scaleMatrix.TransformVector(normal);
    transformedNormal = scaleMatrix.TransformNormal(normal);
    REQUIRE(transformedNormal == Vector3(1.0f, 0.0f, 0.0f));

    Matrix4 scaleAndRotateMatrix = rotationMatrix * scaleMatrix;
    transformedVector = scaleAndRotateMatrix.TransformVector(normal);
    transformedNormal = scaleAndRotateMatrix.TransformNormal(normal);
    REQUIRE(transformedNormal == Vector3(0.0f, 1.0f, 0.0f));
}
