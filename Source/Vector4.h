//
// Vector4.h
//
// Clark Kromenaker
// 
// A vector of 4 floating point values.
//

#pragma once
#include <iostream>

using namespace std;

class Vector4
{
public:
    Vector4();
    Vector4(float x, float y, float z, float w);
    
    void Clear();
    
    void SetX(float newX) { x = newX; }
    void SetY(float newY) { y = newY; }
    void SetZ(float newZ) { z = newZ; }
    void SetW(float newW) { w = newW; }
    
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
    float GetW() const { return w; }
    
private:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

ostream& operator<<(ostream& os, const Vector4& v);
