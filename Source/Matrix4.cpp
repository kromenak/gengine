//
// Matrix4.cpp
//
// Clark Kromenaker
// 

#include "Matrix4.h"
#include <cstring>
#include <cmath>

Matrix4::Matrix4()
{
    memset(mVals, 0, 16 * sizeof(float));
    mVals[0][0] = 1;
    mVals[1][1] = 1;
    mVals[2][2] = 1;
    mVals[3][3] = 1;
}

Matrix4::Matrix4(float vals[16])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

Matrix4::Matrix4(float vals[4][4])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

const float& Matrix4::operator()(int row, int col) const
{
    return mVals[row][col];
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
    Matrix4 out;
    for(int row = 0; row < 4; row++)
    {
        for(int col = 0; col < 4; col++)
        {
            out.mVals[row][col] = mVals[row][0] * rhs.mVals[0][col] +
                                  mVals[row][1] * rhs.mVals[1][col] +
                                  mVals[row][2] * rhs.mVals[2][col] +
                                  mVals[row][3] * rhs.mVals[3][col];
        }
    }
    return out;
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

ostream& operator<<(ostream& os, const Matrix4& m)
{
    os << "[" << m(0,0) << ", " << m(0,1) << ", " << m(0,2) << ", " << m(0,3) << endl;
    os << " " << m(1,0) << ", " << m(1,1) << ", " << m(1,2) << ", " << m(1,3) << endl;
    os << " " << m(2,0) << ", " << m(2,1) << ", " << m(2,2) << ", " << m(2,3) << endl;
    os << " " << m(3,0) << ", " << m(3,1) << ", " << m(3,2) << ", " << m(3,3) << "]";
    return os;
}
