//
// Matrix4Tests.cpp
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
        float vals1[4][4] = {
            { 6, 10, 8, 12 },
            { -5, 60, 80, 12 },
            { 12, 15, -20, -1 },
            { 25, 33, 42, 16 }
        };
        Matrix4 mat1(vals1);
        
        float vals2[4][4] = {
            { 0.5f, -1.3f, 5, 100 },
            { -256, 34.5f, 2, 32 },
            { -17, 14, -20, -4 },
            { 0.6f, 33.0f, 7, -43 }
        };
        Matrix4 mat2(vals2);
        
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
