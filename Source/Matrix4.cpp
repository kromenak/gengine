//
// Matrix4.cpp
//
// Clark Kromenaker
// 

#include "Matrix4.h"
#include <cstring>

Matrix4::Matrix4()
{
    memset(mVals, 0, 16 * sizeof(float));
    mVals[0][0] = 1;
    mVals[1][1] = 1;
    mVals[2][2] = 1;
    mVals[3][3] = 1;
}

Matrix4::Matrix4(float vals[4][4])
{
    memcpy(mVals, vals, 16 * sizeof(float));
}

