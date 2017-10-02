//
// Matrix4.h
//
// Clark Kromenaker
//
// Represents a 4x4 matrix.
//

#pragma once
#include "Vector3.h"

class Matrix4
{
public:
    Matrix4();
    Matrix4(float vals[16]);
    Matrix4(float vals[4][4]);
    
    const float* GetFloatPtr() const
    {
        return reinterpret_cast<const float*>(&mVals[0][0]);
    }
    
    // Retrieve an element using (row, col) notation.
    const float& operator()(int row, int col) const;
    
    // Multiply by another 4x4 matrix.
    Matrix4 operator*(const Matrix4& rhs) const;
    
    static Matrix4 MakeTranslateMatrix(Vector3 position);
    static Matrix4 MakeRotateXMatrix(float rotX);
    static Matrix4 MakeRotateYMatrix(float rotY);
    static Matrix4 MakeRotateZMatrix(float rotZ);
    static Matrix4 MakeScaleMatrix(Vector3 scale);
    
private:
    // Order is ROW, COLUMN - as you'd address any matrix.
    // Note this is opposite of a coordinate system (ROW, COLUMN) == (y, x)
    float mVals[4][4];
};

ostream& operator<<(ostream& os, const Matrix4& v);
