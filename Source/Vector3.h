//
// Vector3.h
//
// Clark Kromenaker
//
// A vector of 3 floating point values.
// Often represents a 3D position/velocity/etc.
//
#pragma once
#include <iomanip>
#include <iostream>

#include "Math.h"

class Vector2;

class Vector3
{
public:
    static Vector3 Zero;
    static Vector3 One;
    static Vector3 UnitX;
    static Vector3 UnitY;
    static Vector3 UnitZ;
    
    Vector3();
    Vector3(float x, float y, float z);
	
	// Conversion from Vector2
	Vector3(float x, float y);
	Vector3(const Vector2& other);
	
    // Copy
    Vector3(const Vector3& other);
    Vector3& operator=(const Vector3& other);
	
    // Equality
    bool operator==(const Vector3& other) const;
    bool operator!=(const Vector3& other) const;
    
    // Accessors
    void SetX(float newX) { x = newX; }
    void SetY(float newY) { y = newY; }
    void SetZ(float newZ) { z = newZ; }
    
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
	
    float& operator[](unsigned int i)       { return (&x)[i]; }
    float  operator[](unsigned int i) const { return (&x)[i]; }
    
    // Length
    float GetLength() const { return Math::Sqrt(x * x + y * y + z * z); }
    float GetLengthSq() const { return (x * x + y * y + z * z); }
    Vector3& Normalize();
    
    // Addition and subtraction
    Vector3 operator+(const Vector3& other) const;
    Vector3& operator+=(const Vector3& other);
    Vector3 operator-(const Vector3& other) const;
	Vector3& operator-=(const Vector3& other);
    Vector3 operator-() const;

    // Scalar multiplication
    Vector3 operator*(float scalar) const;
    friend Vector3 operator*(float scalar, const Vector3& vector);
    Vector3& operator*=(float scalar);
    Vector3 operator/(float scalar) const;
    Vector3& operator/=(float scalar);
	
	// Component-wise multiplication
	Vector3 operator*(const Vector3& other) const;
    
    // Scalar and vector products
    static float Dot(Vector3 lhs, Vector3 rhs);
    static Vector3 Cross(Vector3 lhs, Vector3 rhs);
    
    // Interpolation
    static Vector3 Lerp(Vector3 from, Vector3 to, float t);
    
private:
    // Vector elements - important that they are in this order. We assume some memory layout stuff.
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

std::ostream& operator<<(std::ostream& os, const Vector3& v);
