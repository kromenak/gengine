//
// Vector4.h
//
// Clark Kromenaker
// 
// A vector of 4 floating point values.
//
#pragma once
#include <iostream>
#include "Math.h"
#include "Vector3.h"

class Vector4
{
public:
    static Vector4 Zero;
    static Vector4 One;
    static Vector4 UnitX;
    static Vector4 UnitY;
    static Vector4 UnitZ;
    static Vector4 UnitW;
    
    Vector4();
    Vector4(float x, float y, float z, float w);
    Vector4(bool isPos);
    
    // Copy
    Vector4(const Vector4& other);
    Vector4& operator=(const Vector4& other);
    
    // Equality
    bool operator==(const Vector4& other) const;
    bool operator!=(const Vector4& other) const;
    
    // Accessors
    void SetX(float newX) { x = newX; }
    void SetY(float newY) { y = newY; }
    void SetZ(float newZ) { z = newZ; }
    void SetW(float newW) { w = newW; }
    
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
    float GetW() const { return w; }
    float& operator[](unsigned int i)       { return (&x)[i]; }
    float  operator[](unsigned int i) const { return (&x)[i]; }
    
    // Length
    float GetLength() const { return Math::Sqrt(x * x + y * y + z * z + w * w); }
    float GetLengthSq() const { return (x * x + y * y + z * z + w * w); }
    Vector4& Normalize();
    
    // Addition and subtraction
    Vector4 operator+(const Vector4& other) const;
    Vector4& operator+=(const Vector4& other);
    Vector4 operator-(const Vector4& other) const;
    Vector4& operator-=(const Vector4& other);
    
    Vector4 operator-() const;
    
    // Scalar multiplication
    Vector4 operator*(float scalar) const;
    friend Vector4 operator*(float scalar, const Vector4& vector);
    Vector4& operator*=(float scalar);
    Vector4 operator/(float scalar) const;
    Vector4& operator/=(float scalar);
    
    static float Dot(Vector4 lhs, Vector4 rhs);
    static Vector4 Cross(Vector4 lhs, Vector4 rhs);
    
private:
    // Vector elements - important that they are in this order. We assume some memory layout stuff.
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

std::ostream& operator<<(std::ostream& os, const Vector4& v);
