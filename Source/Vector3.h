//
// Vector3.h
//
// Clark Kromenaker
//
// A vector of 3 floating point values.
// Often represents a 3D position/velocity/etc.
//

#pragma once
#include <iostream>

using namespace std;

class Vector3
{
public:
    Vector3();
    Vector3(float x, float y, float z);
    
    void Clear();
    
    void SetX(float newX) { x = newX; }
    void SetY(float newY) { y = newY; }
    void SetZ(float newZ) { z = newZ; }
    
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
    
private:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

ostream& operator<<(ostream& os, const Vector3& v3);
