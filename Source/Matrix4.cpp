//
// Matrix4.cpp
//
// Clark Kromenaker
// 
#include "Matrix4.h"

#include <cmath>
#include <cstring>

#include "Matrix3.h"

Matrix4 Matrix4::Identity;

Matrix4::Matrix4(float vals[16])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

Matrix4::Matrix4(float vals[4][4])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

Matrix4::Matrix4(float vals[4][4], bool transpose)
{
    memcpy(mVals, vals, 16 * sizeof(float));
    
    // Indicates that the matrix passed in needs to be transposed.
    if(transpose) { Transpose(); }
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

void Matrix4::ToIdentity()
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

Matrix4 Matrix4::Transpose(const Matrix4& matrix)
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

Matrix4 Matrix4::AffineInverse()
{
    Matrix4 m;
    
    // Compute determinant of the upper-left 3x3 matrix.
    float val1 = mVals[5] * mVals[10] - mVals[9] * mVals[6];
    float val2 = mVals[2] * mVals[9] - mVals[1] * mVals[10];
    float val3 = mVals[1] * mVals[6]  - mVals[5] * mVals[2];
    float det = mVals[0] * val1 + mVals[4] * val2 + mVals[8] * val3;
    
    // If determinant is zero, it means it's not possible to find an inverse, so we must return.
    if(Math::IsZero(det)) { return m; }
    
    // Create adjunct matrix, divided by inverse determinant, to get upper 3x3.
    float invDet = 1.0f / det;
    m.mVals[0] = invDet * val1;
    m.mVals[1] = invDet * val2;
    m.mVals[2] = invDet * val3;
    
    m.mVals[4] = invDet * (mVals[6] * mVals[8] - mVals[4] * mVals[10]);
    m.mVals[5] = invDet * (mVals[0] * mVals[10] - mVals[2] * mVals[8]);
    m.mVals[6] = invDet * (mVals[2] * mVals[4] - mVals[0] * mVals[6]);
    
    m.mVals[8] = invDet * (mVals[4] * mVals[9] - mVals[5] * mVals[8]);
    m.mVals[9] = invDet * (mVals[1] * mVals[8] - mVals[0] * mVals[9]);
    m.mVals[10] = invDet * (mVals[0] * mVals[5] - mVals[1] * mVals[4]);
    
    // Multiply negative translation by inverted 3x3 to get these values.
    m.mVals[12] = -m.mVals[0] * mVals[12] - m.mVals[4] * mVals[13] - m.mVals[8] * mVals[14];
    m.mVals[13] = -m.mVals[1] * mVals[12] - m.mVals[5] * mVals[13] - m.mVals[9] * mVals[14];
    m.mVals[14] = -m.mVals[2] * mVals[12] - m.mVals[6] * mVals[13] - m.mVals[10] * mVals[14];
    
    return m;
}

Matrix4 Matrix4::Inverse()
{
    Matrix4 m;
    double inv[16], det;
    int i;
    
    inv[0] = mVals[5]  * mVals[10] * mVals[15] -
    mVals[5]  * mVals[11] * mVals[14] -
    mVals[9]  * mVals[6]  * mVals[15] +
    mVals[9]  * mVals[7]  * mVals[14] +
    mVals[13] * mVals[6]  * mVals[11] -
    mVals[13] * mVals[7]  * mVals[10];
    
    inv[4] = -mVals[4]  * mVals[10] * mVals[15] +
    mVals[4]  * mVals[11] * mVals[14] +
    mVals[8]  * mVals[6]  * mVals[15] -
    mVals[8]  * mVals[7]  * mVals[14] -
    mVals[12] * mVals[6]  * mVals[11] +
    mVals[12] * mVals[7]  * mVals[10];
    
    inv[8] = mVals[4] * mVals[9] * mVals[15] -
    mVals[4]  * mVals[11] * mVals[13] -
    mVals[8]  * mVals[5] * mVals[15] +
    mVals[8]  * mVals[7] * mVals[13] +
    mVals[12] * mVals[5] * mVals[11] -
    mVals[12] * mVals[7] * mVals[9];
    
    inv[12] = -mVals[4]  * mVals[9] * mVals[14] +
    mVals[4]  * mVals[10] * mVals[13] +
    mVals[8]  * mVals[5] * mVals[14] -
    mVals[8]  * mVals[6] * mVals[13] -
    mVals[12] * mVals[5] * mVals[10] +
    mVals[12] * mVals[6] * mVals[9];
    
    inv[1] = -mVals[1]  * mVals[10] * mVals[15] +
    mVals[1]  * mVals[11] * mVals[14] +
    mVals[9]  * mVals[2] * mVals[15] -
    mVals[9]  * mVals[3] * mVals[14] -
    mVals[13] * mVals[2] * mVals[11] +
    mVals[13] * mVals[3] * mVals[10];
    
    inv[5] = mVals[0]  * mVals[10] * mVals[15] -
    mVals[0]  * mVals[11] * mVals[14] -
    mVals[8]  * mVals[2] * mVals[15] +
    mVals[8]  * mVals[3] * mVals[14] +
    mVals[12] * mVals[2] * mVals[11] -
    mVals[12] * mVals[3] * mVals[10];
    
    inv[9] = -mVals[0]  * mVals[9] * mVals[15] +
    mVals[0]  * mVals[11] * mVals[13] +
    mVals[8]  * mVals[1] * mVals[15] -
    mVals[8]  * mVals[3] * mVals[13] -
    mVals[12] * mVals[1] * mVals[11] +
    mVals[12] * mVals[3] * mVals[9];
    
    inv[13] = mVals[0]  * mVals[9] * mVals[14] -
    mVals[0]  * mVals[10] * mVals[13] -
    mVals[8]  * mVals[1] * mVals[14] +
    mVals[8]  * mVals[2] * mVals[13] +
    mVals[12] * mVals[1] * mVals[10] -
    mVals[12] * mVals[2] * mVals[9];
    
    inv[2] = mVals[1]  * mVals[6] * mVals[15] -
    mVals[1]  * mVals[7] * mVals[14] -
    mVals[5]  * mVals[2] * mVals[15] +
    mVals[5]  * mVals[3] * mVals[14] +
    mVals[13] * mVals[2] * mVals[7] -
    mVals[13] * mVals[3] * mVals[6];
    
    inv[6] = -mVals[0]  * mVals[6] * mVals[15] +
    mVals[0]  * mVals[7] * mVals[14] +
    mVals[4]  * mVals[2] * mVals[15] -
    mVals[4]  * mVals[3] * mVals[14] -
    mVals[12] * mVals[2] * mVals[7] +
    mVals[12] * mVals[3] * mVals[6];
    
    inv[10] = mVals[0]  * mVals[5] * mVals[15] -
    mVals[0]  * mVals[7] * mVals[13] -
    mVals[4]  * mVals[1] * mVals[15] +
    mVals[4]  * mVals[3] * mVals[13] +
    mVals[12] * mVals[1] * mVals[7] -
    mVals[12] * mVals[3] * mVals[5];
    
    inv[14] = -mVals[0]  * mVals[5] * mVals[14] +
    mVals[0]  * mVals[6] * mVals[13] +
    mVals[4]  * mVals[1] * mVals[14] -
    mVals[4]  * mVals[2] * mVals[13] -
    mVals[12] * mVals[1] * mVals[6] +
    mVals[12] * mVals[2] * mVals[5];
    
    inv[3] = -mVals[1] * mVals[6] * mVals[11] +
    mVals[1] * mVals[7] * mVals[10] +
    mVals[5] * mVals[2] * mVals[11] -
    mVals[5] * mVals[3] * mVals[10] -
    mVals[9] * mVals[2] * mVals[7] +
    mVals[9] * mVals[3] * mVals[6];
    
    inv[7] = mVals[0] * mVals[6] * mVals[11] -
    mVals[0] * mVals[7] * mVals[10] -
    mVals[4] * mVals[2] * mVals[11] +
    mVals[4] * mVals[3] * mVals[10] +
    mVals[8] * mVals[2] * mVals[7] -
    mVals[8] * mVals[3] * mVals[6];
    
    inv[11] = -mVals[0] * mVals[5] * mVals[11] +
    mVals[0] * mVals[7] * mVals[9] +
    mVals[4] * mVals[1] * mVals[11] -
    mVals[4] * mVals[3] * mVals[9] -
    mVals[8] * mVals[1] * mVals[7] +
    mVals[8] * mVals[3] * mVals[5];
    
    inv[15] = mVals[0] * mVals[5] * mVals[10] -
    mVals[0] * mVals[6] * mVals[9] -
    mVals[4] * mVals[1] * mVals[10] +
    mVals[4] * mVals[2] * mVals[9] +
    mVals[8] * mVals[1] * mVals[6] -
    mVals[8] * mVals[2] * mVals[5];
    
    det = mVals[0] * inv[0] + mVals[1] * inv[4] + mVals[2] * inv[8] + mVals[3] * inv[12];
    
    if(Math::IsZero(det))
    {
        return m;
    }
    det = 1.0 / det;
    
    for(i = 0; i < 16; i++)
    {
        m.mVals[i] = inv[i] * det;
    }
    return m;
}

void Matrix4::SetRows(const Vector4& row1, const Vector4& row2, const Vector4& row3, const Vector4& row4)
{
    mVals[0] = row1[0];
    mVals[4] = row1[1];
    mVals[8] = row1[2];
    mVals[12] = row1[3];
    
    mVals[1] = row2[0];
    mVals[5] = row2[1];
    mVals[9] = row2[2];
    mVals[13] = row2[3];
    
    mVals[2] = row3[0];
    mVals[6] = row3[1];
    mVals[10] = row3[2];
    mVals[14] = row3[3];
    
    mVals[3] = row4[0];
    mVals[7] = row4[1];
    mVals[11] = row4[2];
    mVals[15] = row4[3];
}

void Matrix4::GetRows(Vector4& row1, Vector4& row2, Vector4& row3, Vector4& row4)
{
    row1[0] = mVals[0];
    row1[1] = mVals[4];
    row1[2] = mVals[8];
    row1[3] = mVals[12];
    
    row2[0] = mVals[1];
    row2[1] = mVals[5];
    row2[2] = mVals[9];
    row2[3] = mVals[13];
    
    row3[0] = mVals[2];
    row3[1] = mVals[6];
    row3[2] = mVals[10];
    row3[3] = mVals[14];
    
    row4[0] = mVals[3];
    row4[1] = mVals[7];
    row4[2] = mVals[11];
    row4[3] = mVals[15];
}

void Matrix4::SetColumns(const Vector4& col1, const Vector4& col2, const Vector4& col3, const Vector4& col4)
{
    mVals[0] = col1[0];
    mVals[1] = col1[1];
    mVals[2] = col1[2];
    mVals[3] = col1[3];
    
    mVals[4] = col2[0];
    mVals[5] = col2[1];
    mVals[6] = col2[2];
    mVals[7] = col2[3];
    
    mVals[8] = col3[0];
    mVals[9] = col3[1];
    mVals[10] = col3[2];
    mVals[11] = col3[3];
    
    mVals[12] = col4[0];
    mVals[13] = col4[1];
    mVals[14] = col4[2];
    mVals[15] = col4[3];
}

void Matrix4::GetColumns(Vector4& col1, Vector4& col2, Vector4& col3, Vector4& col4)
{
    col1[0] = mVals[0];
    col1[1] = mVals[1];
    col1[2] = mVals[2];
    col1[3] = mVals[3];
    
    col2[0] = mVals[4];
    col2[1] = mVals[5];
    col2[2] = mVals[6];
    col2[3] = mVals[7];
    
    col3[0] = mVals[8];
    col3[1] = mVals[9];
    col3[2] = mVals[10];
    col4[3] = mVals[11];
    
    col4[0] = mVals[12];
    col4[1] = mVals[13];
    col4[2] = mVals[14];
    col4[3] = mVals[15];
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

/*
Vector3 Matrix4::operator*(const Vector3& rhs) const
{
    return Vector3(mVals[0] * rhs[0] + mVals[4] * rhs[1] + mVals[8]  * rhs[2] + mVals[12],
                   mVals[1] * rhs[0] + mVals[5] * rhs[1] + mVals[9]  * rhs[2] + mVals[13],
                   mVals[2] * rhs[0] + mVals[6] * rhs[1] + mVals[10] * rhs[2] + mVals[14]);
}

Vector3 operator*(const Vector3& lhs, const Matrix4& rhs)
{
    return Vector3(lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2]  + rhs[3],
                   lhs[0] * rhs[4] + lhs[1] * rhs[5] + lhs[2] * rhs[6]  + rhs[7],
                   lhs[0] * rhs[8] + lhs[1] * rhs[9] + lhs[2] * rhs[10] + rhs[11]);
}
*/

Vector3 Matrix4::Transform(const Vector3& rhs)
{
	// Assume Vector3 is not a point, so implicit w value for multiplication is "0".
	return Vector3(mVals[0] * rhs[0] + mVals[4] * rhs[1] + mVals[8]  * rhs[2],
				   mVals[1] * rhs[0] + mVals[5] * rhs[1] + mVals[9]  * rhs[2],
				   mVals[2] * rhs[0] + mVals[6] * rhs[1] + mVals[10] * rhs[2]);
}

Vector3 Matrix4::TransformPoint(const Vector3& rhs)
{
	// Assume Vector3 is a point, so implicit w value for multiplication is "1".
	return Vector3(mVals[0] * rhs[0] + mVals[4] * rhs[1] + mVals[8]  * rhs[2] + mVals[12],
				   mVals[1] * rhs[0] + mVals[5] * rhs[1] + mVals[9]  * rhs[2] + mVals[13],
				   mVals[2] * rhs[0] + mVals[6] * rhs[1] + mVals[10] * rhs[2] + mVals[14]);
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

Vector3 Matrix4::GetTranslation() const
{
	return Vector3(mVals[12], mVals[13], mVals[14]);
}

Matrix4 Matrix4::MakeTranslate(Vector3 translation)
{
    Matrix4 m;
    m.mVals[12] = translation.GetX();
    m.mVals[13] = translation.GetY();
    m.mVals[14] = translation.GetZ();
    return m;
}

Matrix4 Matrix4::MakeRotateX(float rotX)
{
    Matrix4 m;
    float cos = Math::Cos(rotX);
    float sin = Math::Sin(rotX);
    m[5] = cos;
    m[6] = sin;
    m[9] = -sin;
    m[10] = cos;
    return m;
}

Matrix4 Matrix4::MakeRotateY(float rotY)
{
    Matrix4 m;
    float cos = Math::Cos(rotY);
    float sin = Math::Sin(rotY);
    m[0] = cos;
    m[2] = -sin;
    m[8] = sin;
    m[10] = cos;
    return m;
}

Matrix4 Matrix4::MakeRotateZ(float rotZ)
{
    Matrix4 m;
    float cos = Math::Cos(rotZ);
    float sin = Math::Sin(rotZ);
    m[0] = cos;
    m[1] = sin;
    m[4] = -sin;
    m[5] = cos;
    return m;
}

Matrix4 Matrix4::MakeRotate(const Quaternion& quat)
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
    
    Matrix4 m;
    m.mVals[0] = 1.0f - (yy + zz);
    m.mVals[1] = xy + wz;
    m.mVals[2] = xz - wy;
    //m[3] = 0.0f;
    
    m.mVals[4] = xy - wz;
    m.mVals[5] = 1.0f - (xx + zz);
    m.mVals[6] = yz + wx;
    //m[7] = 0.0f;
    
    m.mVals[8] = xz + wy;
    m.mVals[9] = yz - wx;
    m.mVals[10] = 1.0f - (xx + yy);
    //m[11] = 0.0f;
    
    //m[12] = 0.0f;
    //m[13] = 0.0f;
    //m[14] = 0.0f;
    //m[15] = 1.0f;
    return m;
}

Matrix4 Matrix4::MakeRotate(const Matrix3& other)
{
    Matrix4 m;
    m[0] = other[0];
    m[1] = other[1];
    m[2] = other[2];
    //m[3] = 0;
    m[4] = other[3];
    m[5] = other[4];
    m[6] = other[5];
    //m[7] = 0;
    m[8] = other[6];
    m[9] = other[7];
    m[10] = other[8];
    //...
    return m;
}

Matrix4 Matrix4::MakeScale(Vector3 scale)
{
    Matrix4 m;
    m[0] = scale.GetX();
    m[5] = scale.GetY();
    m[10] = scale.GetZ();
    return m;
}

Matrix4 Matrix4::MakeScale(float xScale, float yScale, float zScale)
{
	Matrix4 m;
	m[0] = xScale;
	m[5] = yScale;
	m[10] = zScale;
	return m;
}

Matrix4 Matrix4::MakeLookAt(const Vector3& eye, const Vector3& lookAt, const Vector3& up)
{
	// Generate view space axes. First, the view direction.
	Vector3 viewDir = lookAt - eye;
	viewDir.Normalize();
	
	// We can use "Gram-Schmidt Orthogonalization" to ensure
	// that the up axis passed is orthogonal with our view direction.
	Vector3 viewUp = up - Vector3::Dot(up, viewDir) * viewDir;
	viewUp.Normalize();
	
	// Once we know the view direction and up, we can use cross product
	// to generate side axis. Order also has an effect on our view-space coordinate system.
	Vector3 viewSide = Vector3::Cross(viewDir, viewUp);
	
	// Generate rotation matrix. This is where we make some concrete
	// choices about our view-space coordinate system.
	// +X is right, +Y is up, +Z is forward (left-handed).
	Matrix3 rotate;
	rotate.SetColumns(-viewSide, viewUp, -viewDir);
	
	// This operation calculates wheter this is a left or right-handed system.
	//float result = Vector3::Dot(-viewSide, Vector3::Cross(viewUp, -viewDir));
	//std::cout << result << std::endl;
	
	// Make a 4x4 matrix based on the 3x3 rotation matrix.
	rotate.Transpose();
	Matrix4 m = Matrix4::MakeRotate(rotate);
	
	// Calculate inverse of eye vector and assign it to 4x4 matrix.
	Vector3 eyeInv = -(rotate * eye);
	m(0, 3) = eyeInv.GetX();
	m(1, 3) = eyeInv.GetY();
	m(2, 3) = eyeInv.GetZ();
	return m;
}

Matrix4 Matrix4::MakePerspective(float fovAngleRad, float aspectRatio, float near, float far)
{
    // This works for OpenGL, but needs to be different for DirectX.
    Matrix4 m;
    float d = 1 / Math::Tan(fovAngleRad / 2.0f);
    m[0] = d / aspectRatio;
    m[5] = d;
    m[10] = (near + far) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * near * far) / (near - far);
    m[15] = 0.0f;
    return m;
}

Matrix4 Matrix4::MakeOrthographic(float left, float right, float bottom, float top, float near, float far)
{
	// This works for OpenGL, but needs to be different for DirectX.
	Matrix4 m;
	m[0] = 2.0f / (right - left);
	m[5] = 2.0f / (top - bottom);
	m[10] = -2.0f / (far - near);
	m[12] = -((right + left) / (right - left));
	m[13] = -((top + bottom) / (top - bottom));
	m[14] = -((far + near) / (far - near));
	return m;
}

Matrix4 Matrix4::MakeSimpleScreenOrtho(float width, float height)
{
	// Zero values will cause divide-by-zeros.
	if(width == 0.0f || height == 0.0f) { return Matrix4(); }
	
	// The difference from MakeOrthographic is assumptions we can make.
	// Left/bottom are always zero, near is always -1, far is always 1, which simplifies the equations.
	Matrix4 m;
	m[0] = 2.0f / width; 		//2.0f / (right - left);
	m[5] = 2.0f / height; 		//2.0f / (top - bottom);
	m[10] = -1.0f; 				//-2.0f / (far - near);
	m[12] = -1.0f; 				//-((right + left) / (right - left));
	m[13] = -1.0f; 				//-((top + bottom) / (top - bottom));
	m[14] = 0.0f; 				//-((far + near) / (far - near));
	return m;
}

std::ostream& operator<<(std::ostream& os, const Matrix4& m)
{
    os << "[" << m(0,0) << ", " << m(0,1) << ", " << m(0,2) << ", " << m(0,3) << std::endl;
    os << " " << m(1,0) << ", " << m(1,1) << ", " << m(1,2) << ", " << m(1,3) << std::endl;
    os << " " << m(2,0) << ", " << m(2,1) << ", " << m(2,2) << ", " << m(2,3) << std::endl;
    os << " " << m(3,0) << ", " << m(3,1) << ", " << m(3,2) << ", " << m(3,3) << "]";
    return os;
}
