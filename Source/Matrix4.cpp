//
// Matrix4.cpp
//
// Clark Kromenaker
// 
#include "Matrix4.h"
#include <cstring>
#include <cmath>

Matrix4::Matrix4(float vals[16])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

Matrix4::Matrix4(float vals[16], bool transpose)
{
    // For a 1D array, we usually assume the values are in the right order, for efficiency.
    // But if desired, we can copy them over and then transpose.
    
    // This is helpful because array initialization in C++ looks like row-major order.
    // ex: float vals[16] = { 1, 2, 3, 4,
    //                        5, 6, 7, 8,
    //                        9, 10, 11, 12
    //                        13, 14, 15, 16 };
    // However, we actually store the transpose of what that looks like visually:
    // | 00 04 08 12 |
    // | 01 05 09 13 |
    // | 02 06 10 14 |
    // | 03 07 11 15 |
    // As a result, if we want the matrix to look like the passed in array, we must transpose.
    memcpy(mVals, vals, 16 * sizeof(float));
    Transpose();
}

Matrix4::Matrix4(float vals[4][4])
{
    // Even though values are stored in column-major order, passing 2D array "looks like" row-major order.
    // So, in this case, we do memcpy, but then we transpose, so the matrix is the same as it looks when you initialize a C++ 2D array.
    // See explanation in constructor (float[], bool) above.
    memcpy(mVals, vals, 16 * sizeof(float));
    Transpose();
}

Matrix4::Matrix4(const Matrix4& other)
{
    memcpy(mVals, other.mVals, 16 * sizeof(float));
}

Matrix4& Matrix4::operator=(const Matrix4 &other)
{
    memcpy(mVals, other.mVals, 16 * sizeof(float));
    return *this;
}

bool Matrix4::operator==(const Matrix4 &other) const
{
    for(int i = 0; i < 16; i++)
    {
        if(!Math::AreEqual(mVals[i], other.mVals[i]))
        {
            return false;
        }
    }
    return true;
}

bool Matrix4::operator!=(const Matrix4 &other) const
{
    for(int i = 0; i < 16; i++)
    {
        if(!Math::AreEqual(mVals[i], other.mVals[i]))
        {
            return true;
        }
    }
    return false;
}

void Matrix4::Identity()
{
    mVals[0] = 1.0f;
    mVals[1] = 0.0f;
    mVals[2] = 0.0f;
    mVals[3] = 0.0f;
    
    mVals[4] = 0.0f;
    mVals[5] = 1.0f;
    mVals[6] = 0.0f;
    mVals[7] = 0.0f;
    
    mVals[8] = 0.0f;
    mVals[9] = 0.0f;
    mVals[10] = 1.0f;
    mVals[11] = 0.0f;
    
    mVals[12] = 0.0f;
    mVals[13] = 0.0f;
    mVals[14] = 0.0f;
    mVals[15] = 1.0f;
}

Matrix4& Matrix4::Transpose()
{
    float temp = mVals[1];
    mVals[1] = mVals[4];
    mVals[4] = temp;
    
    temp = mVals[2];
    mVals[2] = mVals[8];
    mVals[8] = temp;
    
    temp = mVals[3];
    mVals[3] = mVals[12];
    mVals[12] = temp;
    
    temp = mVals[6];
    mVals[6] = mVals[9];
    mVals[9] = temp;
    
    temp = mVals[7];
    mVals[7] = mVals[13];
    mVals[13] = temp;
    
    temp = mVals[11];
    mVals[11] = mVals[14];
    mVals[14] = temp;
    
    return *this;
}

Matrix4 Transpose(const Matrix4& matrix)
{
    Matrix4 result;
    result.mVals[0] = matrix.mVals[0];
    result.mVals[1] = matrix.mVals[4];
    result.mVals[2] = matrix.mVals[8];
    result.mVals[3] = matrix.mVals[12];
    result.mVals[4] = matrix.mVals[1];
    result.mVals[5] = matrix.mVals[5];
    result.mVals[6] = matrix.mVals[9];
    result.mVals[7] = matrix.mVals[13];
    result.mVals[8] = matrix.mVals[2];
    result.mVals[9] = matrix.mVals[6];
    result.mVals[10] = matrix.mVals[10];
    result.mVals[11] = matrix.mVals[14];
    result.mVals[12] = matrix.mVals[3];
    result.mVals[13] = matrix.mVals[7];
    result.mVals[14] = matrix.mVals[11];
    result.mVals[15] = matrix.mVals[15];
    return result;
}

const float& Matrix4::operator()(int row, int col) const
{
    return mVals[row + (4 * col)];
}

Matrix4 Matrix4::operator+(const Matrix4& rhs) const
{
    Matrix4 result;
    for(int i = 0; i < 16; i++)
    {
        result.mVals[i] = mVals[i] + rhs.mVals[i];
    }
    return result;
}

Matrix4& Matrix4::operator+=(const Matrix4& rhs)
{
    for(int i = 0; i < 16; i++)
    {
        mVals[i] += rhs.mVals[i];
    }
    return *this;
}

Matrix4 Matrix4::operator-(const Matrix4& rhs) const
{
    Matrix4 result;
    for(int i = 0; i < 16; i++)
    {
        result.mVals[i] = mVals[i] - rhs.mVals[i];
    }
    return result;
}

Matrix4& Matrix4::operator-=(const Matrix4& rhs)
{
    for(int i = 0; i < 16; i++)
    {
        mVals[i] -= rhs.mVals[i];
    }
    return *this;
}

Matrix4 Matrix4::operator-() const
{
    Matrix4 result;
    for(int i = 0; i < 16; i++)
    {
        result.mVals[i] = -mVals[i];
    }
    return result;
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
    Matrix4 result;
    // Column one
    result.mVals[0] = mVals[0] * rhs.mVals[0] + mVals[4] * rhs.mVals[1] + mVals[8] * rhs.mVals[2] + mVals[12] * rhs.mVals[3];
    result.mVals[1] = mVals[1] * rhs.mVals[0] + mVals[5] * rhs.mVals[1] + mVals[9] * rhs.mVals[2] + mVals[13] * rhs.mVals[3];
    result.mVals[2] = mVals[2] * rhs.mVals[0] + mVals[6] * rhs.mVals[1] + mVals[10] * rhs.mVals[2] + mVals[14] * rhs.mVals[3];
    result.mVals[3] = mVals[3] * rhs.mVals[0] + mVals[7] * rhs.mVals[1] + mVals[11] * rhs.mVals[2] + mVals[15] * rhs.mVals[3];
    
    // Column 2
    result.mVals[4] = mVals[0] * rhs.mVals[4] + mVals[4] * rhs.mVals[5] + mVals[8] * rhs.mVals[6] + mVals[12] * rhs.mVals[7];
    result.mVals[5] = mVals[1] * rhs.mVals[4] + mVals[5] * rhs.mVals[5] + mVals[9] * rhs.mVals[6] + mVals[13] * rhs.mVals[7];
    result.mVals[6] = mVals[2] * rhs.mVals[4] + mVals[6] * rhs.mVals[5] + mVals[10] * rhs.mVals[6] + mVals[14] * rhs.mVals[7];
    result.mVals[7] = mVals[3] * rhs.mVals[4] + mVals[7] * rhs.mVals[5] + mVals[11] * rhs.mVals[6] + mVals[15] * rhs.mVals[7];
    
    // Column 3
    result.mVals[8] = mVals[0] * rhs.mVals[8] + mVals[4] * rhs.mVals[9] + mVals[8] * rhs.mVals[10] + mVals[12] * rhs.mVals[11];
    result.mVals[9] = mVals[1] * rhs.mVals[8] + mVals[5] * rhs.mVals[9] + mVals[9] * rhs.mVals[10] + mVals[13] * rhs.mVals[11];
    result.mVals[10] = mVals[2] * rhs.mVals[8] + mVals[6] * rhs.mVals[9] + mVals[10] * rhs.mVals[10] + mVals[14] * rhs.mVals[11];
    result.mVals[11] = mVals[3] * rhs.mVals[8] + mVals[7] * rhs.mVals[9] + mVals[11] * rhs.mVals[10] + mVals[15] * rhs.mVals[11];
    
    // Column 4
    result.mVals[12] = mVals[0] * rhs.mVals[12] + mVals[4] * rhs.mVals[13] + mVals[8] * rhs.mVals[14] + mVals[12] * rhs.mVals[15];
    result.mVals[13] = mVals[1] * rhs.mVals[12] + mVals[5] * rhs.mVals[13] + mVals[9] * rhs.mVals[14] + mVals[13] * rhs.mVals[15];
    result.mVals[14] = mVals[2] * rhs.mVals[12] + mVals[6] * rhs.mVals[13] + mVals[10] * rhs.mVals[14] + mVals[14] * rhs.mVals[15];
    result.mVals[15] = mVals[3] * rhs.mVals[12] + mVals[7] * rhs.mVals[13] + mVals[11] * rhs.mVals[14] + mVals[15] * rhs.mVals[15];
    return result;
}

Matrix4& Matrix4::operator*=(const Matrix4& rhs)
{
    Matrix4 result = *this * rhs;
    for(int i = 0; i < 16; i++)
    {
        mVals[i] = result.mVals[i];
    }
    return *this;
}

Vector4 Matrix4::operator*(const Vector4& rhs) const
{
    return Vector4(mVals[0] * rhs[0] + mVals[4] * rhs[1] + mVals[8]  * rhs[2] + mVals[12] * rhs[3],
                   mVals[1] * rhs[0] + mVals[5] * rhs[1] + mVals[9]  * rhs[2] + mVals[13] * rhs[3],
                   mVals[2] * rhs[0] + mVals[6] * rhs[1] + mVals[10] * rhs[2] + mVals[14] * rhs[3],
                   mVals[3] * rhs[0] + mVals[7] * rhs[1] + mVals[11] * rhs[2] + mVals[15] * rhs[3]);
}

Vector4 operator*(const Vector4& lhs, const Matrix4& rhs)
{
    return Vector4(lhs[0] * rhs[0]  + lhs[1] * rhs[1]  + lhs[2] * rhs[2]  + lhs[3] * rhs[3],
                   lhs[0] * rhs[4]  + lhs[1] * rhs[5]  + lhs[2] * rhs[6]  + lhs[3] * rhs[7],
                   lhs[0] * rhs[8]  + lhs[1] * rhs[9]  + lhs[2] * rhs[10] + lhs[3] * rhs[11],
                   lhs[0] * rhs[12] + lhs[1] * rhs[13] + lhs[2] * rhs[14] + lhs[3] * rhs[15]);
}

Matrix4 Matrix4::operator*(float scalar) const
{
    Matrix4 result;
    for(int i = 0; i < 16; i++)
    {
        result.mVals[i] = mVals[i] * scalar;
    }
    return result;
}

Matrix4& Matrix4::operator*=(float scalar)
{
    for(int i = 0; i < 16; i++)
    {
        mVals[i] *= scalar;
    }
    return *this;
}

Matrix4 operator*(float scalar, const Matrix4& matrix)
{
    return matrix * scalar;
}

Matrix4 Matrix4::MakeTranslateMatrix(Vector3 position)
{
    float vals[4][4] =
    {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { position.GetX(), position.GetY(), position.GetZ(), 1 }
    };
    return Matrix4(vals);
}

Matrix4 Matrix4::MakeRotateXMatrix(float rotX)
{
    float vals[4][4] =
    {
        { 1, 0, 0, 0 },
        { 0, cosf(rotX), sinf(rotX), 0 },
        { 0, -sinf(rotX), cosf(rotX), 0 },
        { 0, 0, 0, 1 }
    };
    return Matrix4(vals);
}

Matrix4 Matrix4::MakeRotateYMatrix(float rotY)
{
    float vals[4][4] =
    {
        { cosf(rotY), 0, -sinf(rotY), 0 },
        { 0, 1, 0, 0 },
        { sinf(rotY), 0, cosf(rotY), 0 },
        { 0, 0, 0, 1 }
    };
    return Matrix4(vals);
}

Matrix4 Matrix4::MakeRotateZMatrix(float rotZ)
{
    float vals[4][4] =
    {
        { cosf(rotZ), sinf(rotZ), 0, 0 },
        { -sinf(rotZ), cosf(rotZ), 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 }
    };
    return Matrix4(vals);
}

Matrix4 Matrix4::MakeScaleMatrix(Vector3 scale)
{
    float vals[4][4] =
    {
        { scale.GetX(), 0, 0, 0 },
        { 0, scale.GetY(), 0, 0 },
        { 0, 0, scale.GetZ(), 0 },
        { 0, 0, 0, 1 }
    };
    return Matrix4(vals);
}

std::ostream& operator<<(std::ostream& os, const Matrix4& m)
{
    os << "[" << m(0,0) << ", " << m(0,1) << ", " << m(0,2) << ", " << m(0,3) << std::endl;
    os << " " << m(1,0) << ", " << m(1,1) << ", " << m(1,2) << ", " << m(1,3) << std::endl;
    os << " " << m(2,0) << ", " << m(2,1) << ", " << m(2,2) << ", " << m(2,3) << std::endl;
    os << " " << m(3,0) << ", " << m(3,1) << ", " << m(3,2) << ", " << m(3,3) << "]";
    return os;
}
