//
// Vector2.h
//
// Clark Kromenaker
// 
// A vector of 2 floating point values.
//
#pragma once
#include <iostream>
#include "Math.h"

class Vector2
{
public:
    static Vector2 Zero;
    static Vector2 One;
    static Vector2 UnitX;
    static Vector2 UnitY;
    
    Vector2();
    Vector2(float x, float y);
    
    // Copy some other Vector3 to this object.
    Vector2(const Vector2& other);
    Vector2& operator=(const Vector2& other);
    
    // Equality
    bool operator==(const Vector2& other) const;
    bool operator!=(const Vector2& other) const;
    
    // Accessors
    void SetX(float newX) { x = newX; }
    void SetY(float newY) { y = newY; }
    
    float GetX() const { return x; }
    float GetY() const { return y; }
    
    // Length
    float GetLength() const { return Math::Sqrt(x * x + y * y); }
    float GetLengthSq() const { return (x * x + y * y); }
    Vector2& Normalize();
    
    // Addition and subtraction
    Vector2 operator+(const Vector2& other) const;
    Vector2& operator+=(const Vector2& other);
    Vector2 operator-(const Vector2& other) const;
    Vector2& operator-=(const Vector2& other);
    
    Vector2 operator-() const;
    
    // Scalar multiplication
    Vector2 operator*(float scalar) const;
    friend Vector2 operator*(float scalar, const Vector2& vector);
    Vector2& operator*=(float scalar);
    Vector2 operator/(float scalar) const;
    Vector2& operator/=(float scalar);
    
    // Scalar product
    static float Dot(Vector2 lhs, Vector2 rhs);
    
private:
    float x = 0.0f;
    float y = 0.0f;
};

std::ostream& operator<<(std::ostream& os, const Vector2& v);

