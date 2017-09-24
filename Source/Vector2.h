//
// Vector2.h
//
// Clark Kromenaker
// 
// A vector of 2 floating point values.
//

#pragma once
#include <iostream>

using namespace std;

class Vector2
{
public:
    Vector2();
    Vector2(float x, float y);
    
    void Clear();
    
    void SetX(float newX) { x = newX; }
    void SetY(float newY) { y = newY; }
    
    float GetX() const { return x; }
    float GetY() const { return y; }
    
private:
    float x = 0.0f;
    float y = 0.0f;
};

ostream& operator<<(ostream& os, const Vector2& v);

