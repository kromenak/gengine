//
// Matrix4.h
//
// Clark Kromenaker
//
// Represents a 4x4 matrix.
//

#pragma once

class Matrix4
{
public:
    Matrix4();
    Matrix4(float vals[4][4]);
    
    const float* GetFloatPtr() const
    {
        return reinterpret_cast<const float*>(&mVals[0][0]);
    }
    
private:
    // Order is COLUMN, ROW. Think of it as column-major.
    float mVals[4][4];
};
