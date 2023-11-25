//
// Clark Kromenaker
//
// A 3x3 matrix.
//
#pragma once
#include <iostream>

#include "Vector2.h"

class Quaternion;
class Vector3;

class Matrix3
{
public:
    static Matrix3 Zero;
    static Matrix3 Identity;
    
    Matrix3() = default;
    Matrix3(float vals[9]);
    Matrix3(float v00, float v01, float v02,
            float v10, float v11, float v12,
            float v20, float v21, float v22);
    Matrix3(const Vector3& col1, const Vector3& col2, const Vector3& col3);
    
    // Copy
    Matrix3(const Matrix3& other);
    Matrix3& operator=(const Matrix3& other);
    
    // Equality
    bool operator==(const Matrix3& other) const;
    bool operator!=(const Matrix3& other) const;
    
    // Accessors - get entry using (row, col) notation.
    float& operator()(int row, int col) { return mVals[row + 3 * col]; }
    float operator()(int row, int col) const { return mVals[row + 3 * col]; }
    
    // Accessors - get column using [index] notation.
    Vector3& operator[](int col) { return (*reinterpret_cast<Vector3*>(&mVals[col * 3])); }
    const Vector3& operator[](int col) const { return (*reinterpret_cast<const Vector3*>(&mVals[col * 3])); }
    
    // Implicit float conversion - allows Matrix3 to be passed as a float* argument.
    operator float*() { return mVals; }
    operator const float*() const { return mVals; }
    
    // More explicit Row/Column getters/setters.
    void SetRows(const Vector3& row1, const Vector3& row2, const Vector3& row3);
    void GetRows(Vector3& row1, Vector3& row2, Vector3& row3);
    void SetColumns(const Vector3& col1, const Vector3& col2, const Vector3& col3);
    void GetColumns(Vector3& col1, Vector3& col2, Vector3& col3);
    
    // Addition and subtraction
    Matrix3 operator+(const Matrix3& rhs) const;
    Matrix3& operator+=(const Matrix3& rhs);
    Matrix3 operator-(const Matrix3& rhs) const;
    Matrix3& operator-=(const Matrix3& rhs);
    Matrix3 operator-() const;
    
    // Matrix multiplication
    Matrix3 operator*(const Matrix3& rhs) const;
    Matrix3& operator*=(const Matrix3& rhs);
    
    // Vector3 multiplication - column-vector (rhs) and row-vector (lhs)
    Vector3 operator*(const Vector3& rhs) const;
    friend Vector3 operator*(const Vector3& lhs, const Matrix3& rhs);
    
    // Scalar multiplication
    Matrix3 operator*(float scalar) const;
    Matrix3& operator*=(float scalar);
    friend Matrix3 operator*(float scalar, const Matrix3& matrix);
    
    // Transpose
    void Transpose();
    static Matrix3 Transpose(const Matrix3& matrix);
    
    // Trace (sum of diagonal entries)
    float GetTrace() const;
    
    // Inverse
    void Invert();
    static Matrix3 Inverse(const Matrix3& matrix);

    //*********************
    // Transform Functions
    //*********************
    Vector2 TransformVector(const Vector2& vector) const;
    Vector2 TransformPoint(const Vector2& point) const;
    
    //*********************
    // Factory Methods
    //*********************
    static Matrix3 MakeRotateX(float rotX);
    static Matrix3 MakeRotateY(float rotY);
    static Matrix3 MakeRotateZ(float rotZ);
    static Matrix3 MakeRotate(const Vector3& axis, float angle);
    static Matrix3 MakeRotate(const Quaternion& quat);
    
    static Matrix3 MakeScale(float scale);
    static Matrix3 MakeScale(const Vector3& scale);
    
    static Matrix3 MakeBasis(const Vector3& forward, const Vector3& up, const Vector3& right);
    
    //TODO: Others...
    // Reflection
    // Skew
    
private:
    // Elements are stored in a 1D array internally.
    // Our storage is column-major, meaning that [0] = m(0,0), [1] = m(1,0), [2] = m(2,0), and so on.
    // The indexes of each matrix element are:
    // | 00 03 06 |
    // | 01 04 07 |
    // | 02 05 08 |
    float mVals[9];
};

std::ostream& operator<<(std::ostream& os, const Matrix3& v);
