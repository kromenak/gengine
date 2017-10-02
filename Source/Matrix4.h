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
    Matrix4(float vals[4][4]);
    
    const float* GetFloatPtr() const
    {
        return reinterpret_cast<const float*>(&mVals[0][0]);
    }
    
    float& operator()(int col, int row);
    Matrix4 operator*(const Matrix& a, const Matrix& b);
    
    static Matrix4 MakeTranslateMatrix(Vector3 position);
    static Matrix4 MakeRotateXMatrix(float rotX);
    static Matrix4 MakeRotateYMatrix(float rotY);
    static Matrix4 MakeRotateZMatrix(float rotZ);
    static Matrix4 MakeScaleMatrix(Vector3 scale);
    
private:
    // Order is COLUMN, ROW. Think of it as column-major.
    float mVals[4][4];
};
