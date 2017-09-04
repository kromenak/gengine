//
// Vector3.h
//
// Clark Kromenaker
//

#include "Vector3.h"

Vector3::Vector3() : Vector3(0, 0, 0)
{
    
}

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z)
{
    
}

void Vector3::Clear()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

ostream& operator<<(ostream& os, const Vector3& v3)
{
    os << "(" << v3.GetX() << ", " << v3.GetY() << ", " << v3.GetZ() << ")";
    return os;
}

