//
// Matrix4.h
//
// Clark Kromenaker
//
// A 4x4 matrix.
//
#pragma once
#include <iostream>

#include "Matrix3.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "Vector4.h"

class Matrix4
{
public:
    static Matrix4 Identity;
    
    Matrix4() { ToIdentity(); }
    Matrix4(float vals[16]);
    explicit Matrix4(float vals[4][4]);
    explicit Matrix4(float vals[4][4], bool transpose);
    
    // Copy
    Matrix4(const Matrix4& other);
    Matrix4& operator=(const Matrix4& other);
    
    // Equality
    bool operator==(const Matrix4& other) const;
    bool operator!=(const Matrix4& other) const;
    
    // Retrieve or set an element using (row, col) notation.
    float& operator()(int row, int col) { return mVals[row + 4 * col]; }
    float operator()(int row, int col) const { return mVals[row + 4 * col]; }
    
    // Row and column getters/accessors.
    void SetRows(const Vector4& row1, const Vector4& row2, const Vector4& row3, const Vector4& row4);
    void GetRows(Vector4& row1, Vector4& row2, Vector4& row3, Vector4& row4);
    
    void SetColumns(const Vector4& col1, const Vector4& col2, const Vector4& col3, const Vector4& col4);
    void GetColumns(Vector4& col1, Vector4& col2, Vector4& col3, Vector4& col4);
    
    // Clear matrix to identity.
    void ToIdentity();
    
    // Transpose the matrix.
    Matrix4& Transpose();
    static Matrix4 Transpose(const Matrix4& matrix);
    
    // Inverse and affine inverse.
    // Affine inverse is maybe faster, but only works with affine matrices!
    Matrix4 AffineInverse() const;
    Matrix4 Inverse() const;
    
    // Addition and subtraction
    Matrix4 operator+(const Matrix4& rhs) const;
    Matrix4& operator+=(const Matrix4& rhs);
    Matrix4 operator-(const Matrix4& rhs) const;
    Matrix4& operator-=(const Matrix4& rhs);
    Matrix4 operator-() const;
    
    // Matrix multiplication
    Matrix4 operator*(const Matrix4& rhs) const;
    Matrix4& operator*=(const Matrix4& rhs);
    
	// Vector3 multiplication - column-vector only!
	Vector3 TransformPoint(const Vector3& rhs) const;
	Vector3 Transform(const Vector3& rhs) const;
	
    // Vector4 multiplication - column-vector (rhs) and row-vector (lhs)
    Vector4 operator*(const Vector4& rhs) const;
    friend Vector4 operator*(const Vector4& lhs, const Matrix4& rhs);
	
    // Scalar multiplication
    Matrix4 operator*(float scalar) const;
    Matrix4& operator*=(float scalar);
    friend Matrix4 operator*(float scalar, const Matrix4& matrix);
    
    // Implicit float conversion - allows Matrix3 to be passed as a float* argument.
    operator float*() { return mVals; }
    operator const float*() const { return mVals; }
	
	// A Matrix4 is often used to represent a transformation matrix.
	// In that case, we can extract certain meaningful data if needed.
	const Vector3& GetXAxis() const { return reinterpret_cast<const Vector3&>(mVals[0]); }
	const Vector3& GetYAxis() const { return reinterpret_cast<const Vector3&>(mVals[4]); }
	const Vector3& GetZAxis() const { return reinterpret_cast<const Vector3&>(mVals[8]); }
	const Vector3& GetTranslation() const { return reinterpret_cast<const Vector3&>(mVals[12]); }
	Quaternion GetRotation() const;
	
    // Factory methods for generating certain types of matrices.
    static Matrix4 MakeTranslate(Vector3 translation);
	
    static Matrix4 MakeRotateX(float rotX);
    static Matrix4 MakeRotateY(float rotY);
    static Matrix4 MakeRotateZ(float rotZ);
	
    static Matrix4 MakeRotate(const Quaternion& quat);
    static Matrix4 MakeRotate(const Matrix3& matrix3);
	
    static Matrix4 MakeScale(Vector3 scale);
	static Matrix4 MakeScale(float xScale, float yScale, float zScale);
	
	static Matrix4 MakeLookAt(const Vector3& eye, const Vector3& lookAt, const Vector3& up);
	
    static Matrix4 MakePerspective(float fovAngleRad, float aspectRatio, float near, float far);
	static Matrix4 MakeOrthographic(float left, float right, float bottom, float top, float near, float far);
	static Matrix4 MakeSimpleScreenOrtho(float width, float height);
	
	static Matrix4 MakeSimpleViewProj(float width, float height);
	
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
