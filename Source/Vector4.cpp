//
// Vector4.cpp
//
// Clark Kromenaker
// 

#include "Vector4.h"

Vector4::Vector4() : Vector4(0.0f, 0.0f, 0.0f, 0.0f)
{
    
}

Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{
    
}

void Vector4::Clear()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 0.0f;
}

ostream& operator<<(ostream& os, const Vector4& v)
{
    os << "(" << v.GetX() << ", " << v.GetY() << ", " << v.GetZ() << ", " << v.GetW() << ")";
    return os;
}
