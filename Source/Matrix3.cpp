//
// Matrix3.cpp
//
// Clark Kromenaker
//
#include "Matrix3.h"

#include <cstring>
#include <cmath>

#include "Quaternion.h"
#include "Vector3.h"

Matrix3::Matrix3(float vals[9])
{
    memcpy(mVals, vals, 9 * sizeof(float));
}

Matrix3::Matrix3(float vals[3][3])
{
    memcpy(mVals, vals, 9 * sizeof(float));
}

Matrix3::Matrix3(float vals[3][3], bool transpose)
{
    memcpy(mVals, vals, 9 * sizeof(float));
    
    // Indicates that the matrix passed in needs to be transposed.
    if(transpose) { Transpose(); }
}

Matrix3::Matrix3(const Matrix3& other)
{
    memcpy(mVals, other.mVals, 9 * sizeof(float));
}

Matrix3& Matrix3::operator=(const Matrix3 &other)
{
    memcpy(mVals, other.mVals, 9 * sizeof(float));
    return *this;
}

bool Matrix3::operator==(const Matrix3 &other) const
{
    for(int i = 0; i < 9; i++)
    {
        if(!Math::AreEqual(mVals[i], other.mVals[i]))
        {
            return false;
        }
    }
    return true;
}

bool Matrix3::operator!=(const Matrix3 &other) const
{
    for(int i = 0; i < 9; i++)
    {
        if(!Math::AreEqual(mVals[i], other.mVals[i]))
        {
            return true;
        }
    }
    return false;
}

void Matrix3::ToIdentity()
{
    mVals[0] = 1.0f;
    mVals[1] = 0.0f;
    mVals[2] = 0.0f;
    
    mVals[3] = 0.0f;
    mVals[4] = 1.0f;
    mVals[5] = 0.0f;
    
    mVals[6] = 0.0f;
    mVals[7] = 0.0f;
    mVals[8] = 1.0f;
}

Matrix3& Matrix3::Transpose()
{
    float temp = mVals[1];
    mVals[1] = mVals[3];
    mVals[3] = temp;
    
    temp = mVals[2];
    mVals[2] = mVals[6];
    mVals[6] = temp;
    
    temp = mVals[5];
    mVals[5] = mVals[7];
    mVals[7] = temp;
    return *this;
}

Matrix3 Matrix3::Transpose(const Matrix3& matrix)
{
    Matrix3 result;
    result.mVals[0] = matrix.mVals[0];
    result.mVals[1] = matrix.mVals[3];
    result.mVals[2] = matrix.mVals[6];
    result.mVals[3] = matrix.mVals[1];
    result.mVals[4] = matrix.mVals[4];
    result.mVals[5] = matrix.mVals[7];
    result.mVals[6] = matrix.mVals[2];
    result.mVals[7] = matrix.mVals[5];
    result.mVals[8] = matrix.mVals[8];
    return result;
}

float Matrix3::GetTrace() const
{
    return mVals[0] + mVals[4] + mVals[8];
}

void Matrix3::SetRows(const Vector3& row1, const Vector3& row2, const Vector3& row3)
{
    mVals[0] = row1[0];
    mVals[3] = row1[1];
    mVals[6] = row1[2];
    
    mVals[1] = row2[0];
    mVals[4] = row2[1];
    mVals[7] = row2[2];
    
    mVals[2] = row3[0];
    mVals[5] = row3[1];
    mVals[8] = row3[2];
}

void Matrix3::GetRows(Vector3& row1, Vector3& row2, Vector3& row3)
{
    row1[0] = mVals[0];
    row1[1] = mVals[3];
    row1[2] = mVals[6];
    
    row2[0] = mVals[1];
    row2[1] = mVals[4];
    row2[2] = mVals[7];
    
    row3[0] = mVals[2];
    row3[1] = mVals[5];
    row3[2] = mVals[8];
}

void Matrix3::SetColumns(const Vector3& col1, const Vector3& col2, const Vector3& col3)
{
    mVals[0] = col1[0];
    mVals[1] = col1[1];
    mVals[2] = col1[2];
    
    mVals[3] = col2[0];
    mVals[4] = col2[1];
    mVals[5] = col2[2];
    
    mVals[6] = col3[0];
    mVals[7] = col3[1];
    mVals[8] = col3[2];
}

void Matrix3::GetColumns(Vector3& col1, Vector3& col2, Vector3& col3)
{
    col1[0] = mVals[0];
    col1[1] = mVals[1];
    col1[2] = mVals[2];
    
    col2[0] = mVals[3];
    col2[1] = mVals[4];
    col2[2] = mVals[5];
    
    col3[0] = mVals[6];
    col3[1] = mVals[7];
    col3[2] = mVals[8];
}

Matrix3 Matrix3::operator+(const Matrix3& rhs) const
{
    Matrix3 result;
    for(int i = 0; i < 9; i++)
    {
        result.mVals[i] = mVals[i] + rhs.mVals[i];
    }
    return result;
}

Matrix3& Matrix3::operator+=(const Matrix3& rhs)
{
    for(int i = 0; i < 9; i++)
    {
        mVals[i] += rhs.mVals[i];
    }
    return *this;
}

Matrix3 Matrix3::operator-(const Matrix3& rhs) const
{
    Matrix3 result;
    for(int i = 0; i < 9; i++)
    {
        result.mVals[i] = mVals[i] - rhs.mVals[i];
    }
    return result;
}

Matrix3& Matrix3::operator-=(const Matrix3& rhs)
{
    for(int i = 0; i < 9; i++)
    {
        mVals[i] -= rhs.mVals[i];
    }
    return *this;
}

Matrix3 Matrix3::operator-() const
{
    Matrix3 result;
    for(int i = 0; i < 9; i++)
    {
        result.mVals[i] = -mVals[i];
    }
    return result;
}

Matrix3 Matrix3::operator*(const Matrix3& rhs) const
{
    Matrix3 result;
    // Column one
    result.mVals[0] = mVals[0] * rhs.mVals[0] + mVals[3] * rhs.mVals[1] + mVals[6] * rhs.mVals[2];
    result.mVals[1] = mVals[1] * rhs.mVals[0] + mVals[4] * rhs.mVals[1] + mVals[7] * rhs.mVals[2];
    result.mVals[2] = mVals[2] * rhs.mVals[0] + mVals[5] * rhs.mVals[1] + mVals[8] * rhs.mVals[2];
    
    // Column 2
    result.mVals[3] = mVals[0] * rhs.mVals[3] + mVals[3] * rhs.mVals[4] + mVals[6] * rhs.mVals[5];
    result.mVals[4] = mVals[1] * rhs.mVals[3] + mVals[4] * rhs.mVals[4] + mVals[7] * rhs.mVals[5];
    result.mVals[5] = mVals[2] * rhs.mVals[3] + mVals[5] * rhs.mVals[4] + mVals[8] * rhs.mVals[5];
    
    // Column 3
    result.mVals[6] = mVals[0] * rhs.mVals[6] + mVals[3] * rhs.mVals[7] + mVals[6] * rhs.mVals[8];
    result.mVals[7] = mVals[1] * rhs.mVals[6] + mVals[4] * rhs.mVals[7] + mVals[7] * rhs.mVals[8];
    result.mVals[8] = mVals[2] * rhs.mVals[6] + mVals[5] * rhs.mVals[7] + mVals[8] * rhs.mVals[8];
    return result;
}

Matrix3& Matrix3::operator*=(const Matrix3& rhs)
{
    Matrix3 result = *this * rhs;
    for(int i = 0; i < 16; i++)
    {
        mVals[i] = result.mVals[i];
    }
    return *this;
}

Vector3 Matrix3::operator*(const Vector3& rhs) const
{
    return Vector3(mVals[0] * rhs[0] + mVals[3] * rhs[1] + mVals[6] * rhs[2],
                   mVals[1] * rhs[0] + mVals[4] * rhs[1] + mVals[7] * rhs[2],
                   mVals[2] * rhs[0] + mVals[5] * rhs[1] + mVals[8] * rhs[2]);
}

Vector3 operator*(const Vector3& lhs, const Matrix3& rhs)
{
    return Vector3(lhs[0] * rhs[0] + lhs[1] * rhs[1]  + lhs[2] * rhs[2],
                   lhs[0] * rhs[3] + lhs[1] * rhs[4]  + lhs[2] * rhs[5],
                   lhs[0] * rhs[6] + lhs[1] * rhs[7]  + lhs[2] * rhs[8]);
}

Matrix3 Matrix3::operator*(float scalar) const
{
    Matrix3 result;
    for(int i = 0; i < 9; i++)
    {
        result.mVals[i] = mVals[i] * scalar;
    }
    return result;
}

Matrix3& Matrix3::operator*=(float scalar)
{
    for(int i = 0; i < 9; i++)
    {
        mVals[i] *= scalar;
    }
    return *this;
}

Matrix3 operator*(float scalar, const Matrix3& matrix)
{
    return matrix * scalar;
}

Matrix3 Matrix3::MakeRotateX(float rotX)
{
    Matrix3 m;
    float cos = Math::Cos(rotX);
    float sin = Math::Sin(rotX);
    m[5] = cos;
    m[6] = sin;
    m[9] = -sin;
    m[10] = cos;
    return m;
}

Matrix3 Matrix3::MakeRotateY(float rotY)
{
    Matrix3 m;
    float cos = Math::Cos(rotY);
    float sin = Math::Sin(rotY);
    m[0] = cos;
    m[2] = -sin;
    m[8] = sin;
    m[10] = cos;
    return m;
}

Matrix3 Matrix3::MakeRotateZ(float rotZ)
{
    Matrix3 m;
    float cos = Math::Cos(rotZ);
    float sin = Math::Sin(rotZ);
    m[0] = cos;
    m[1] = sin;
    m[4] = -sin;
    m[5] = cos;
    return m;
}

Matrix3 Matrix3::MakeRotate(const Quaternion& quat)
{
    float s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
    s = 2.0f / Quaternion::Dot(quat, quat);
    
    xs = s * quat.GetX();
    ys = s * quat.GetY();
    zs = s * quat.GetZ();
    wx = quat.GetW() * xs;
    wy = quat.GetW() * ys;
    wz = quat.GetW() * zs;
    xx = quat.GetX() * xs;
    xy = quat.GetX() * ys;
    xz = quat.GetX() * zs;
    yy = quat.GetY() * ys;
    yz = quat.GetY() * zs;
    zz = quat.GetZ() * zs;
    
    Matrix3 m;
    m.mVals[0] = 1.0f - (yy + zz);
    m.mVals[1] = xy + wz;
    m.mVals[2] = xz - wy;
    
    m.mVals[3] = xy - wz;
    m.mVals[4] = 1.0f - (xx + zz);
    m.mVals[5] = yz + wx;
    
    m.mVals[6] = xz + wy;
    m.mVals[7] = yz - wx;
    m.mVals[8] = 1.0f - (xx + yy);
    return m;
}

Matrix3 Matrix3::MakeScale(Vector3 scale)
{
    Matrix3 m;
    m[0] = scale.x;
    m[4] = scale.y;
    m[8] = scale.z;
    return m;
}

Matrix3 Matrix3::MakeBasis(Vector3 forward, Vector3 up, Vector3 right)
{
    Matrix3 m;
    m.SetColumns(forward, up, right);
    return m;
}

std::ostream& operator<<(std::ostream& os, const Matrix3& m)
{
    os << "[" << m(0,0) << ", " << m(0,1) << ", " << m(0,2) << std::endl;
    os << " " << m(1,0) << ", " << m(1,1) << ", " << m(1,2) << std::endl;
    os << " " << m(2,0) << ", " << m(2,1) << ", " << m(2,2) << "]";
    return os;
}
