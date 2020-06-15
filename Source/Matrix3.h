//
// Matrix3.h
//
// Clark Kromenaker
//
// Represents a 3x3 matrix.
//
#pragma once
#include <iostream>

class Quaternion;
class Vector3;

class Matrix3
{
public:
    static Matrix3 Identity;
    
    Matrix3() { ToIdentity(); }
    Matrix3(float vals[9]);
    explicit Matrix3(float vals[3][3]);
    explicit Matrix3(float vals[3][3], bool transpose);
    
    // Copy
    Matrix3(const Matrix3& other);
    Matrix3& operator=(const Matrix3& other);
    
    // Equality
    bool operator==(const Matrix3& other) const;
    bool operator!=(const Matrix3& other) const;
    
    // Retrieve an element using (row, col) notation.
    float& operator()(int row, int col) { return mVals[row + 3 * col]; }
    float operator()(int row, int col) const { return mVals[row + 3 * col]; }
    
    // Row and column getters/accessors.
    void SetRows(const Vector3& row1, const Vector3& row2, const Vector3& row3);
    void GetRows(Vector3& row1, Vector3& row2, Vector3& row3);
    
    void SetColumns(const Vector3& col1, const Vector3& col2, const Vector3& col3);
    void GetColumns(Vector3& col1, Vector3& col2, Vector3& col3);
    
    // Clear matrix to identity.
    void ToIdentity();
    
    // Transpose the matrix.
    Matrix3& Transpose();
    static Matrix3 Transpose(const Matrix3& matrix);
    
    float GetTrace() const;
    
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
    
    // Implicit float conversion - allows Matrix3 to be passed as a float* argument.
    operator float*() { return mVals; }
    operator const float*() const { return mVals; }
    
    // Factory methods for generating certain types of matrices.
    static Matrix3 MakeRotateX(float rotX);
    static Matrix3 MakeRotateY(float rotY);
    static Matrix3 MakeRotateZ(float rotZ);
    static Matrix3 MakeRotate(const Quaternion& quat);
    static Matrix3 MakeScale(Vector3 scale);
    static Matrix3 MakeBasis(Vector3 forward, Vector3 up, Vector3 right);
    
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
