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

Matrix3 Matrix3::Zero(0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f);

Matrix3 Matrix3::Identity(1.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 1.0f);

Matrix3::Matrix3(float vals[9])
{
    // Assumed to already be in column-major order.
    memcpy(mVals, vals, 9 * sizeof(float));
}

Matrix3::Matrix3(float v00, float v01, float v02,
                 float v10, float v11, float v12,
                 float v20, float v21, float v22)
{
    mVals[0] = v00;
    mVals[1] = v10;
    mVals[2] = v20;
    
    mVals[3] = v01;
    mVals[4] = v11;
    mVals[5] = v21;
    
    mVals[6] = v02;
    mVals[7] = v12;
    mVals[8] = v22;
}

Matrix3::Matrix3(const Vector3& col1, const Vector3& col2, const Vector3& col3)
{
    mVals[0] = col1.x;
    mVals[1] = col1.y;
    mVals[2] = col1.z;
    
    mVals[3] = col2.x;
    mVals[4] = col2.y;
    mVals[5] = col2.z;
    
    mVals[6] = col3.x;
    mVals[7] = col3.y;
    mVals[8] = col3.z;
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
    for(int i = 0; i < 9; i++)
    {
        mVals[i] = result.mVals[i];
    }
    return *this;
}

Vector3 Matrix3::operator*(const Vector3& rhs) const
{
    // Treating Vector3 as a column vector here (Mv).
    return Vector3(mVals[0] * rhs[0] + mVals[3] * rhs[1] + mVals[6] * rhs[2],
                   mVals[1] * rhs[0] + mVals[4] * rhs[1] + mVals[7] * rhs[2],
                   mVals[2] * rhs[0] + mVals[5] * rhs[1] + mVals[8] * rhs[2]);
}

Vector3 operator*(const Vector3& lhs, const Matrix3& rhs)
{
    // Treating Vector3 as a row vector here (vM).
    return Vector3(lhs[0] * rhs.mVals[0] + lhs[1] * rhs.mVals[1]  + lhs[2] * rhs.mVals[2],
                   lhs[0] * rhs.mVals[3] + lhs[1] * rhs.mVals[4]  + lhs[2] * rhs.mVals[5],
                   lhs[0] * rhs.mVals[6] + lhs[1] * rhs.mVals[7]  + lhs[2] * rhs.mVals[8]);
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

void Matrix3::Transpose()
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

void Matrix3::Invert()
{
    // Math from Foundations of Game Engine Development.
    // Uses "inverse = adjugate divided by determinant" method.
    
    // For a 3x3 matrix:
    // The adjugate matrix can be calculated using a series of cross products.
    // The determinant is equal to scalar triple produce ((a x b) dot c).
    
    // Extract column vectors from matrix - we can calculate entire inverse with these.
    const Vector3& col0 = (*this)[0];
    const Vector3& col1 = (*this)[1];
    const Vector3& col2 = (*this)[2];
    
    // Cross products of the column vectors give us the rows of the adjugate matrix.
    Vector3 row0 = Vector3::Cross(col1, col2);
    Vector3 row1 = Vector3::Cross(col2, col0);
    Vector3 row2 = Vector3::Cross(col0, col1);
    
    // Determinant is calculated using scalar triple product.
    // row2 = (col0 x col1) and we dot product with col2.
    float determinant = Vector3::Dot(row2, col2);
    
    // Determinant of zero indicates that an inverse doesn't exist.
    // Further reinforced by the "divide by determinant" below.
    if(Math::IsZero(determinant))
    {
        return;
    }
    
    // Multiply adjugate matrix by (1/det) to get the inverse.
    float invDet = 1.0f / determinant;
    mVals[0] = row0.x * invDet;
    mVals[1] = row1.x * invDet;
    mVals[2] = row2.x * invDet;
    
    mVals[3] = row0.y * invDet;
    mVals[4] = row1.y * invDet;
    mVals[5] = row2.y * invDet;
    
    mVals[6] = row0.z * invDet;
    mVals[7] = row1.z * invDet;
    mVals[8] = row2.z * invDet;
}

/*static*/ Matrix3 Matrix3::Inverse(const Matrix3& matrix)
{
    Matrix3 toInvert = matrix;
    toInvert.Invert();
    return toInvert;
}

//*********************
// Factory Methods
//*********************
/*static*/ Matrix3 Matrix3::MakeRotateX(float rotX)
{
    float cos = Math::Cos(rotX);
    float sin = Math::Sin(rotX);
    return Matrix3(1.0f, 0.0f, 0.0f,
                   0.0f, cos,  -sin,
                   0.0f, sin,  cos);
}

/*static*/ Matrix3 Matrix3::MakeRotateY(float rotY)
{
    float cos = Math::Cos(rotY);
    float sin = Math::Sin(rotY);
    return Matrix3(cos, 0.0f, sin,
                   0.0f, 1.0f, 0.0f,
                   -sin, 0.0f, cos);
}

/*static*/ Matrix3 Matrix3::MakeRotateZ(float rotZ)
{
    float cos = Math::Cos(rotZ);
    float sin = Math::Sin(rotZ);
    return Matrix3(cos, -sin, 0.0f,
                   sin,  cos, 0.0f,
                   0.0f, 0.0f, 1.0f);
}

/*static*/ Matrix3 Matrix3::MakeRotate(const Vector3 &axis, float angle)
{
    // Math from Foundations of Game Engine Development.
    // Multiply by a vector (v) to rotate the vector by angle about axis.
    
    // How it works: create a set of coordinate axes as [axis, projection of v onto axis, a x v].
    // Result = v.proj(a) + v.reject(a) * cos(theta) + (a x v) * sin(theta)
    // Representing this project/reject/cross as 3x3 matrices (and factoring out v),
    // we can generate a final 3x3 matrix to multiply by v to get the result.
    
    // Pre-calculate terms that are used many times in the math.
    float cos = Math::Cos(angle);
    float cosComplement = 1.0f - cos;
    float sin = Math::Sin(angle);
    
    float x = axis.x * cosComplement; // (1 - c) * x
    float y = axis.y * cosComplement; // (1 - c) * y
    float z = axis.z * cosComplement; // (1 - c) * z
    float axay = x * axis.y;
    float axaz = x * axis.z;
    float ayaz = y * axis.z;
    
    // Generate final matrix.
    return Matrix3(cos + x * axis.x, axay - sin * axis.z, axaz + sin * axis.y,
                   axay + sin * axis.z, cos + y * axis.y, ayaz - sin * axis.x,
                   axaz - sin * axis.y, ayaz + sin * axis.x, cos + z * axis.z);
}

/*static*/ Matrix3 Matrix3::MakeRotate(const Quaternion& quat)
{
    float s = 2.0f / Quaternion::Dot(quat, quat);
    
    float xs = s * quat.x;
    float ys = s * quat.y;
    float zs = s * quat.z;
    float wx = quat.w * xs;
    float wy = quat.w * ys;
    float wz = quat.w * zs;
    float xx = quat.x * xs;
    float xy = quat.x * ys;
    float xz = quat.x * zs;
    float yy = quat.y * ys;
    float yz = quat.y * zs;
    float zz = quat.z * zs;
    
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

/*static*/ Matrix3 Matrix3::MakeScale(float scale)
{
    return Matrix3(scale, 0.0f, 0.0f,
                   0.0f, scale, 0.0f,
                   0.0f, 0.0f, scale);
}

/*static*/ Matrix3 Matrix3::MakeScale(const Vector3& scale)
{
    return Matrix3(scale.x, 0.0f, 0.0f,
                   0.0f, scale.y, 0.0f,
                   0.0f, 0.0f, scale.z);
}

/*static*/ Matrix3 Matrix3::MakeBasis(const Vector3& forward, const Vector3& up, const Vector3& right)
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
