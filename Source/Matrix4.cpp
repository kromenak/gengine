//
// Matrix4.cpp
//
// Clark Kromenaker
// 
#include "Matrix4.h"
#include "Matrix3.h"
#include <cstring>
#include <cmath>

Matrix4 Matrix4::Identity;

Matrix4::Matrix4(float vals[16])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

Matrix4::Matrix4(float vals[4][4])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

Matrix4::Matrix4(float vals[4][4], bool convert)
{
    memcpy(mVals, vals, 16 * sizeof(float));
    
    // Indicates that the matrix passed in needs to be transposed.
    if(convert) { Transpose(); }
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

Matrix4 Matrix4::MakeLookAt(const Vector3 &eye, const Vector3 &lookAt, const Vector3 &up)
{
    // Generate view space axes. First, the view direction.
    Vector3 viewDir = lookAt - eye;
    viewDir.Normalize();
    
    // We can use "Gram-Schmidt Orthogonalization" equation to ensure
    // that the up axis passed is orthogonal (at a right angle) with our view direction.
    Vector3 viewUp = up - Vector3::Dot(up, viewDir) * viewDir;
    viewUp.Normalize();
    
    // Once we know the view direction and up, we can use cross product
    // to generate side axis. Order also has an effect on our view-space coordinate system.
    Vector3 viewSide = Vector3::Cross(viewDir, viewUp);
    
    // Generate rotation matrix. This is where we make some concrete
    // choices about our view-space coordinate system.
    // +X is right, +Y is up, -Z is forward.
    Matrix3 rotate;
    rotate.SetRows(viewSide, viewUp, -viewDir);
    
    // Make a 4x4 matrix based on the 3x3 rotation matrix.
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
    // Note that this doesn't work for all view systems.
    // I think different values would be required for Direct3D.
    // This should work in OpenGL though.
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

std::ostream& operator<<(std::ostream& os, const Matrix4& m)
{
    os << "[" << m(0,0) << ", " << m(0,1) << ", " << m(0,2) << ", " << m(0,3) << std::endl;
    os << " " << m(1,0) << ", " << m(1,1) << ", " << m(1,2) << ", " << m(1,3) << std::endl;
    os << " " << m(2,0) << ", " << m(2,1) << ", " << m(2,2) << ", " << m(2,3) << std::endl;
    os << " " << m(3,0) << ", " << m(3,1) << ", " << m(3,2) << ", " << m(3,3) << "]";
    return os;
}
