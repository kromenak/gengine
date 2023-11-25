#include "Matrix4.h"

#include <cmath>
#include <cstring>

#include "Matrix3.h"

Matrix4 Matrix4::Zero(0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f);

Matrix4 Matrix4::Identity(1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 1.0f);

Matrix4::Matrix4(float vals[16])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

Matrix4::Matrix4(float v00, float v01, float v02, float v03,
                 float v10, float v11, float v12, float v13,
                 float v20, float v21, float v22, float v23,
                 float v30, float v31, float v32, float v33)
{
    mVals[0] = v00;
    mVals[1] = v10;
    mVals[2] = v20;
    mVals[3] = v30;
    
    mVals[4] = v01;
    mVals[5] = v11;
    mVals[6] = v21;
    mVals[7] = v31;
    
    mVals[8] = v02;
    mVals[9] = v12;
    mVals[10] = v22;
    mVals[11] = v32;
    
    mVals[12] = v03;
    mVals[13] = v13;
    mVals[14] = v23;
    mVals[15] = v33;
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

void Matrix4::GetRows(Vector4& row1, Vector4& row2, Vector4& row3, Vector4& row4) const
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

void Matrix4::GetColumns(Vector4& col1, Vector4& col2, Vector4& col3, Vector4& col4) const
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
    return Vector4(lhs[0] * rhs.mVals[0]  + lhs[1] * rhs.mVals[1]  + lhs[2] * rhs.mVals[2]  + lhs[3] * rhs.mVals[3],
                   lhs[0] * rhs.mVals[4]  + lhs[1] * rhs.mVals[5]  + lhs[2] * rhs.mVals[6]  + lhs[3] * rhs.mVals[7],
                   lhs[0] * rhs.mVals[8]  + lhs[1] * rhs.mVals[9]  + lhs[2] * rhs.mVals[10] + lhs[3] * rhs.mVals[11],
                   lhs[0] * rhs.mVals[12] + lhs[1] * rhs.mVals[13] + lhs[2] * rhs.mVals[14] + lhs[3] * rhs.mVals[15]);
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

void Matrix4::Transpose()
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

void Matrix4::Invert()
{
    // Math from Foundations of Game Engine Development.
    // We can split our 4x4 matrix into 4 3D column vectors and 1 4D row vector.
    // From there, those vectors can be used to calculate determinant and cofactor matrix fairly efficiently.
    // Then, we can use "inverse = adjugate matrix divided by determinant" method.

    // Grab 4 3D column vectors from the matrix.
    // matrix[x] returns reference to 4D column vector, but we only need first 3 values, so reinterpret to get that.
    const Vector3& col0 = reinterpret_cast<const Vector3&>((*this)[0]);
    const Vector3& col1 = reinterpret_cast<const Vector3&>((*this)[1]);
    const Vector3& col2 = reinterpret_cast<const Vector3&>((*this)[2]);
    const Vector3& col3 = reinterpret_cast<const Vector3&>((*this)[3]);
    
    // Grab 4D row vector values from last row of matrix.
    const float& x = mVals[3];
    const float& y = mVals[7];
    const float& z = mVals[11];
    const float& w = mVals[15];
    
    // Calculate intermediate vectors.
    Vector3 s = Vector3::Cross(col0, col1);
    Vector3 t = Vector3::Cross(col2, col3);
    Vector3 u = col0 * y - col1 * x;
    Vector3 v = col2 * w - col3 * z;
    
    // Calculate determinant from intermediate values.
    // If determinant is zero, no inverse exists!
    float determinant = Vector3::Dot(s, v) + Vector3::Dot(t, u);
    if(Math::IsZero(determinant))
    {
        return;
    }
    
    // Now we just need to calculate adjugate matrix and multiply by inverse determinant to get the inverse matrix!
    float invDet = 1.0f / determinant;
    
    // Pre-multiply each intermediate vector by inverse determinant.
    // By doing this now, we can avoid multiplying final result my inverse determinant.
    s *= invDet;
    t *= invDet;
    u *= invDet;
    v *= invDet;
    
    // Calculate left 4x3 rows of the inverse matrix.
    // Because we multiplied by invDet earlier, these are FINAL values.
    Vector3 row0 = Vector3::Cross(col1, v) + t * y;
    Vector3 row1 = Vector3::Cross(v, col0) - t * x;
    Vector3 row2 = Vector3::Cross(col3, u) + s * w;
    Vector3 row3 = Vector3::Cross(u, col2) - s * z;
    
    // Calculate right 4x1 column.
    float colX = -Vector3::Dot(col1, t);
    float colY =  Vector3::Dot(col0, t);
    float colZ = -Vector3::Dot(col3, s);
    float colW =  Vector3::Dot(col2, s);
    
    // Construct final 4x4 matrix.
    mVals[0] = row0.x;
    mVals[1] = row1.x;
    mVals[2] = row2.x;
    mVals[3] = row3.x;
    
    mVals[4] = row0.y;
    mVals[5] = row1.y;
    mVals[6] = row2.y;
    mVals[7] = row3.y;
    
    mVals[8] = row0.z;
    mVals[9] = row1.z;
    mVals[10] = row2.z;
    mVals[11] = row3.z;
    
    mVals[12] = colX;
    mVals[13] = colY;
    mVals[14] = colZ;
    mVals[15] = colW;
}

/*static*/ Matrix4 Matrix4::Inverse(const Matrix4& matrix)
{
    Matrix4 toInvert = matrix;
    toInvert.Invert();
    return toInvert;
}

void Matrix4::InvertOrthogonal()
{
    // If matrix is orthogonal, the inverse is equal to the transpose!
    //TODO: Actually check/assert this matrix is orthogonal.
    //TODO: Also, if the matrix has translation, even if it is technically orthogonal, this does not work!
    Transpose();
}

/*static*/ Matrix4 Matrix4::InverseOrthogonal(const Matrix4& matrix)
{
    // If matrix is orthogonal, the inverse is equal to the transpose!
    return Transpose(matrix);
}

/*static*/ Matrix4 Matrix4::Lerp(const Matrix4 &from, const Matrix4 &to, float t)
{
    Matrix4 ret;
    for(int i = 0; i < 16; ++i)
    {
        ret.mVals[i] = Math::Lerp(from.mVals[i], to.mVals[i], t);
    }
    return ret;
}

//*********************
// Transform Functions
//*********************
Quaternion Matrix4::GetRotation() const
{
    Quaternion q;
    q.w = (Math::Sqrt(Math::Max(0.0f, 1.0f + mVals[0] + mVals[5] + mVals[10])) / 2.0f);
    q.x = (Math::Sqrt(Math::Max(0.0f, 1.0f + mVals[0] - mVals[5] - mVals[10])) / 2.0f);
    q.y = (Math::Sqrt(Math::Max(0.0f, 1.0f - mVals[0] + mVals[5] - mVals[10])) / 2.0f);
    q.z = (Math::Sqrt(Math::Max(0.0f, 1.0f - mVals[0] - mVals[5] + mVals[10])) / 2.0f);
    
    q.x = (Math::MagnitudeSign(q.x, mVals[6] - mVals[9]));
    q.y = (Math::MagnitudeSign(q.y, mVals[8] - mVals[2]));
    q.z = (Math::MagnitudeSign(q.z, mVals[1] - mVals[4]));
    return q;
}

bool Matrix4::IsOrthogonal() const
{
    // A transform matrix is orthogonal if the upper-left 3x3 matrix meets these requirements:
    // 1) The axes are all orthogonal (perpendicular) to one another.
    // 2) The axes are all unit length.
    if(Math::AreEqual(GetXAxis().GetLengthSq(), 1.0f) &&
       Math::AreEqual(GetYAxis().GetLengthSq(), 1.0f) &&
       Math::AreEqual(GetZAxis().GetLengthSq(), 1.0f))
    {
        if(Math::IsZero(Vector3::Dot(GetXAxis(), GetYAxis())) &&
           Math::IsZero(Vector3::Dot(GetYAxis(), GetZAxis())) &&
           Math::IsZero(Vector3::Dot(GetZAxis(), GetXAxis())))
        {
            return true;
        }
    }
    return false;
}

Vector3 Matrix4::TransformVector(const Vector3& vector) const
{
    // Assume Vector3 is not a point, so w = 0.
    return Vector3(mVals[0] * vector[0] + mVals[4] * vector[1] + mVals[8]  * vector[2],
                   mVals[1] * vector[0] + mVals[5] * vector[1] + mVals[9]  * vector[2],
                   mVals[2] * vector[0] + mVals[6] * vector[1] + mVals[10] * vector[2]);
}

Vector3 Matrix4::TransformPoint(const Vector3& point) const
{
    // Assume Vector3 is a point, so w = 1.
    return Vector3(mVals[0] * point[0] + mVals[4] * point[1] + mVals[8]  * point[2] + mVals[12],
                   mVals[1] * point[0] + mVals[5] * point[1] + mVals[9]  * point[2] + mVals[13],
                   mVals[2] * point[0] + mVals[6] * point[1] + mVals[10] * point[2] + mVals[14]);
}

Vector3 Matrix4::TransformNormal(const Vector3& normal) const
{
    // Say this matrix transforms points from space A to B: (pB = M * pA)
    // That means it also transforms normals from space B to A: (nA = nB * M)
    // In other words, normals don't transform in the same way as points/vectors.

    // To have this matrix transform a normal from A to B, we must invert it: (nB = nA * invM)

    // HOWEVER, there is a trick: if the matrix is orthogonal, the inverse equals the transpose: (nB = nA * transM)
    // But in that case, it is equivalent to using the original matrix with a column vector: (nB = M * nA) - just like vectors!
    if(IsOrthogonal())
    {
        //TODO: If we transform a normalized vector against an orthogonal matrix, is the resulting vector normalized? If so, this Normalize call my be unneeded.
        return Vector3::Normalize(TransformVector(normal));
    }
       
    // But if not orthonormal, we must calculate inverse and perform (nA * InvM).
    Matrix4 inverse = Inverse(*this);
    Vector3 newNormal(normal.x * inverse.mVals[0] + normal.y * inverse.mVals[1] + normal.z * inverse.mVals[2],
                      normal.x * inverse.mVals[4] + normal.y * inverse.mVals[5] + normal.z * inverse.mVals[6],
                      normal.x * inverse.mVals[8] + normal.y * inverse.mVals[9] + normal.z * inverse.mVals[10]);
    
    // Transforming the normal often leaves it no longer unit length.
    // But a quick normalize puts it back in order...and it's pointing in the right direction!
    return Vector3::Normalize(newNormal);
}

void Matrix4::InvertTransform()
{
    // Math from Foundations of Game Engine Development.
    // When a matrix is a transform, the inverse math can be optimized.
    // A transform matrix's fourth row is always (0, 0, 0, 1).
    // See normal Inverse function for more in-depth explanation.
    
    // Grab 4 3D column vectors from the matrix.
    const Vector3& col0 = reinterpret_cast<const Vector3&>((*this)[0]);
    const Vector3& col1 = reinterpret_cast<const Vector3&>((*this)[1]);
    const Vector3& col2 = reinterpret_cast<const Vector3&>((*this)[2]);
    const Vector3& col3 = reinterpret_cast<const Vector3&>((*this)[3]);
    
    // Calculate intermediate vectors.
    // Because this is a transform, u cancels out entirely and v just equals col2!
    Vector3 s = Vector3::Cross(col0, col1);
    Vector3 t = Vector3::Cross(col2, col3);
    Vector3 v = col2;
    
    // Because this is a transform, determinant calc is simpler.
    // We will also assume that determinant is non-zero for transforms.
    float invDet = 1.0f / Vector3::Dot(s, col2);
    s *= invDet;
    t *= invDet;
    v *= invDet;
    
    // Calculate final result and return. row2 is not needed in this version
    Vector3 row0 = Vector3::Cross(col1, v);
    Vector3 row1 = Vector3::Cross(v, col0);
    
    // Calculate right 3x1 column.
    float colX = -Vector3::Dot(col1, t);
    float colY =  Vector3::Dot(col0, t);
    float colZ = -Vector3::Dot(col3, s);
    
    // Construct final 4x4 matrix.
    mVals[0] = row0.x;
    mVals[1] = row1.x;
    mVals[2] = s.x;
    mVals[3] = 0.0f;
    
    mVals[4] = row0.y;
    mVals[5] = row1.y;
    mVals[6] = s.y;
    mVals[7] = 0.0f;
    
    mVals[8] = row0.z;
    mVals[9] = row1.z;
    mVals[10] = s.z;
    mVals[11] = 0.0f;
    
    mVals[12] = colX;
    mVals[13] = colY;
    mVals[14] = colZ;
    mVals[15] = 1.0f;
}

/*static*/ Matrix4 Matrix4::InverseTransform(const Matrix4& matrix)
{
    Matrix4 toInvert = matrix;
    toInvert.InvertTransform();
    return toInvert;
}

//*********************
// Factory Methods
//*********************
/*static*/ Matrix4 Matrix4::MakeTranslate(const Vector3& translation)
{
    return Matrix4(1.0f, 0.0f, 0.0f, translation.x,
                   0.0f, 1.0f, 0.0f, translation.y,
                   0.0f, 0.0f, 1.0f, translation.z,
                   0.0f, 0.0f, 0.0f, 1.0f);
}

/*static*/ Matrix4 Matrix4::MakeRotateX(float rotX)
{
    float cos = Math::Cos(rotX);
    float sin = Math::Sin(rotX);
    return Matrix4(1.0f, 0.0f, 0.0f, 0.0f,
                   0.0f,  cos, -sin, 0.0f,
                   0.0f,  sin,  cos, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f);
}

/*static*/ Matrix4 Matrix4::MakeRotateY(float rotY)
{
    float cos = Math::Cos(rotY);
    float sin = Math::Sin(rotY);
    return Matrix4( cos, 0.0f,  sin, 0.0f,
                   0.0f, 1.0f, 0.0f, 0.0f,
                   -sin, 0.0f,  cos, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f);
}

/*static*/ Matrix4 Matrix4::MakeRotateZ(float rotZ)
{
    float cos = Math::Cos(rotZ);
    float sin = Math::Sin(rotZ);
    return Matrix4( cos, -sin, 0.0f, 0.0f,
                    sin,  cos, 0.0f, 0.0f,
                   0.0f, 0.0f, 1.0f, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f);
}

/*static*/ Matrix4 Matrix4::MakeRotate(const Quaternion& quat)
{
    // Handle bad input. TODO: Maybe assert instead?
    float dot = Quaternion::Dot(quat, quat);
    if(Math::IsZero(dot)) { return Matrix4::Identity; }

    float s = 2.0f / dot;
    
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
    
    Matrix4 m;
    m.mVals[0] = 1.0f - (yy + zz);
    m.mVals[1] = xy + wz;
    m.mVals[2] = xz - wy;
    m.mVals[3] = 0.0f;
    
    m.mVals[4] = xy - wz;
    m.mVals[5] = 1.0f - (xx + zz);
    m.mVals[6] = yz + wx;
    m.mVals[7] = 0.0f;
    
    m.mVals[8] = xz + wy;
    m.mVals[9] = yz - wx;
    m.mVals[10] = 1.0f - (xx + yy);
    m.mVals[11] = 0.0f;
    
    m.mVals[12] = 0.0f;
    m.mVals[13] = 0.0f;
    m.mVals[14] = 0.0f;
    m.mVals[15] = 1.0f;
    return m;
}

/*static*/ Matrix4 Matrix4::MakeRotate(const Matrix3& other)
{
    Matrix4 m = Identity;
    m.mVals[0] = other(0, 0);
    m.mVals[1] = other(1, 0);
    m.mVals[2] = other(2, 0);
    //m.mVals[3] = 0;
    m.mVals[4] = other(0, 1);
    m.mVals[5] = other(1, 1);
    m.mVals[6] = other(2, 1);
    //m.mVals[7] = 0;
    m.mVals[8] = other(0, 2);
    m.mVals[9] = other(1, 2);
    m.mVals[10] = other(2, 2);
    //...
    return m;
}

/*static*/ Matrix4 Matrix4::MakeScale(float scale)
{
    return Matrix4(scale, 0.0f, 0.0f, 0.0f,
                   0.0f, scale, 0.0f, 0.0f,
                   0.0f, 0.0f, scale, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f);
}

/*static*/ Matrix4 Matrix4::MakeScale(const Vector3& scale)
{
    return Matrix4(scale.x, 0.0f, 0.0f, 0.0f,
                   0.0f, scale.y, 0.0f, 0.0f,
                   0.0f, 0.0f, scale.z, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f);
}

std::ostream& operator<<(std::ostream& os, const Matrix4& m)
{
    os << "[" << m(0,0) << ", " << m(0,1) << ", " << m(0,2) << ", " << m(0,3) << std::endl;
    os << " " << m(1,0) << ", " << m(1,1) << ", " << m(1,2) << ", " << m(1,3) << std::endl;
    os << " " << m(2,0) << ", " << m(2,1) << ", " << m(2,2) << ", " << m(2,3) << std::endl;
    os << " " << m(3,0) << ", " << m(3,1) << ", " << m(3,2) << ", " << m(3,3) << "]";
    return os;
}
