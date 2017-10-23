//
// Matrix4.h
//
// Clark Kromenaker
//
// Represents a 4x4 matrix.
//
#pragma once
#include "Vector3.h"
#include "Vector4.h"

class Matrix4
{
public:
    Matrix4() { Identity(); }
    Matrix4(float vals[16]);
    Matrix4(float vals[16], bool transpose);
    explicit Matrix4(float vals[4][4]);
    ~Matrix4() { }
    
    // Copy
    Matrix4(const Matrix4& other);
    Matrix4& operator=(const Matrix4& other);
    
    // Equality
    bool operator==(const Matrix4& other) const;
    bool operator!=(const Matrix4& other) const;
    
    // Retrieve an element using (row, col) notation.
    const float& operator()(int row, int col) const;
    
    // Clear matrix to identity.
    void Identity();
    
    // Transpose the matrix.
    Matrix4& Transpose();
    friend Matrix4 Transpose(const Matrix4& matrix);
    
    // Addition and subtraction
    Matrix4 operator+(const Matrix4& rhs) const;
    Matrix4& operator+=(const Matrix4& rhs);
    Matrix4 operator-(const Matrix4& rhs) const;
    Matrix4& operator-=(const Matrix4& rhs);
    Matrix4 operator-() const;
    
    // Matrix multiplication
    Matrix4 operator*(const Matrix4& rhs) const;
    Matrix4& operator*=(const Matrix4& rhs);
    
    // Vector multiplication
    // Matrix times column vector
    Vector4 operator*(const Vector4& rhs) const;
    // Row vector times matrix
    friend Vector4 operator*(const Vector4& lhs, const Matrix4& rhs);
    
    // Scalar multiplication
    Matrix4 operator*(float scalar) const;
    Matrix4& operator*=(float scalar);
    friend Matrix4 operator*(float scalar, const Matrix4& matrix);
    
    // Data accessors for graphics APIs.
    operator float*() { return mVals; }
    operator const float*() const { return mVals; }
    const float* GetFloatPtr() { return mVals; }
    
    static Matrix4 MakeTranslateMatrix(Vector3 position);
    static Matrix4 MakeRotateXMatrix(float rotX);
    static Matrix4 MakeRotateYMatrix(float rotY);
    static Matrix4 MakeRotateZMatrix(float rotZ);
    static Matrix4 MakeScaleMatrix(Vector3 scale);
    
private:
    // Elements are stored in a 1D array internally.
    // Our storage is column-major, meaning that [0] = m(0,0), [1] = m(1,0), [2] = m(2,0), and so on.
    // The indexes of each matrix element are:
    // | 00 04 08 12 |
    // | 01 05 09 13 |
    // | 02 06 10 14 |
    // | 03 07 11 15 |
    float mVals[16];
};

std::ostream& operator<<(std::ostream& os, const Matrix4& v);
