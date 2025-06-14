//
// Clark Kromenaker
//
// Tests for the Quaternion class.
//
#include "catch.hh"
#include "Quaternion.h"
#include "Vector3.h"

TEST_CASE("Test quaternion constructors")
{
    Quaternion defaultQuat;
    REQUIRE(defaultQuat.x == 0.0f);
    REQUIRE(defaultQuat.y == 0.0f);
    REQUIRE(defaultQuat.z == 0.0f);
    REQUIRE(defaultQuat.w == 1.0f);

    Quaternion specificQuat(2.0f, 3.2f, 1.1f, 0.0f);
    REQUIRE(specificQuat.x == 2.0f);
    REQUIRE(specificQuat.y == 3.2f);
    REQUIRE(specificQuat.z == 1.1f);
    REQUIRE(specificQuat.w == 0.0f);

    Quaternion axisAngleQuat(Vector3(0.8804509f, 0.1760902f, 0.4402255f), 1.57f);
    REQUIRE(Math::AreEqual(axisAngleQuat.x, 0.6223249f));
    REQUIRE(Math::AreEqual(axisAngleQuat.y, 0.124465f));
    REQUIRE(Math::AreEqual(axisAngleQuat.z, 0.3111624f));
    REQUIRE(Math::AreEqual(axisAngleQuat.w, 0.7073883f));
}

TEST_CASE("Test quaternion copy and assign")
{
    Quaternion specificQuat(2.0f, 3.2f, 1.1f, 0.0f);
    Quaternion copyQuat(specificQuat);
    REQUIRE(copyQuat.x == specificQuat.x);
    REQUIRE(copyQuat.y == specificQuat.y);
    REQUIRE(copyQuat.z == specificQuat.z);
    REQUIRE(copyQuat.w == specificQuat.w);

    Quaternion assignQuat = specificQuat;
    REQUIRE(assignQuat.x == specificQuat.x);
    REQUIRE(assignQuat.y == specificQuat.y);
    REQUIRE(assignQuat.z == specificQuat.z);
    REQUIRE(assignQuat.w == specificQuat.w);
}

TEST_CASE("Test quaternion equality")
{
    Quaternion quat1(2.0f, 3.2f, 1.1f, 0.0f);
    Quaternion quat2(1.22f, 0.342f, 1.0f, 1.0f);
    REQUIRE(quat1 != quat2);

    Quaternion equalQuat = quat2;
    REQUIRE(equalQuat == quat2);
    REQUIRE(equalQuat != quat1);
}

TEST_CASE("Test quaternion zero, identity, and unit")
{
    Quaternion zeroQuat(0.0f, 0.0f, 0.0f, 0.0f);
    REQUIRE(zeroQuat == Quaternion::Zero);
    REQUIRE(Math::IsZero(zeroQuat.GetLength()));

    Quaternion identity(0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(identity == Quaternion::Identity);
    REQUIRE(Math::AreEqual(identity.GetLength(), 1.0f));

    Quaternion unit(0.6223249f, 0.124465f, 0.3111624f, 0.7073883f);
    REQUIRE(Math::AreEqual(identity.GetLength(), 1.0f));
}

TEST_CASE("Test quaternion unit and normalization")
{
    Quaternion randQuat(2.0f, 3.2f, 1.1f, 0.0f);
    REQUIRE(!randQuat.IsUnit());
    REQUIRE(randQuat.GetLength() > 1.0f);
    randQuat.Normalize();
    REQUIRE(randQuat.IsUnit());
    REQUIRE(randQuat.GetLength() == 1.0f);
}

TEST_CASE("Test quaternion addition and subtraction")
{

}



