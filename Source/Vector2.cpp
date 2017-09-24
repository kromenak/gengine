//
// Vector2.cpp
//
// Clark Kromenaker
// 

#include "Vector2.h"

Vector2::Vector2() : Vector2(0, 0)
{
    
}

Vector2::Vector2(float x, float y) : x(x), y(y)
{
    
}

void Vector2::Clear()
{
    x = 0.0f;
    y = 0.0f;
}

ostream& operator<<(ostream& os, const Vector2& v)
{
    os << "(" << v.GetX() << ", " << v.GetY() << ")";
    return os;
}
