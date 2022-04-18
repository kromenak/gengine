//
// Clark Kromenaker
// 
// A 2D vector.
//
#pragma once
#include <iostream>

#include "GMath.h"

class Vector3;

class Vector2
{
public:
    static Vector2 Zero;
    static Vector2 One;
    static Vector2 UnitX;
    static Vector2 UnitY;
    
    Vector2() = default;
    Vector2(float x, float y);
	
	// Conversion from Vector3.
	Vector2(const Vector3& other);
    
    // Copy
    Vector2(const Vector2& other);
    Vector2& operator=(const Vector2& other);
	
    // Equality
    bool operator==(const Vector2& other) const;
    bool operator!=(const Vector2& other) const;
    
    // Accessors
    float& operator[](int i)       { return (&x)[i]; }
    float  operator[](int i) const { return (&x)[i]; }
    
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
	
	// Component-wise multiplication
	Vector2 operator*(const Vector2& other) const;
    
    // Length
    float GetLength() const { return Math::Sqrt(x * x + y * y); }
    float GetLengthSq() const { return (x * x + y * y); }
    Vector2& Normalize();
    static Vector2 Normalize(const Vector2& v);
    
    // Scalar/dot product
    static float Dot(const Vector2& lhs, const Vector2& rhs);
	
	// Interpolation
	static Vector2 Lerp(const Vector2& from, const Vector2& to, float t);
    
    // Projection and rejection
    static Vector2 Project(const Vector2& a, const Vector2& b);
    static Vector2 Reject(const Vector2& a, const Vector2& b);
	
	// Vector elements. Order is important (memory layout is sometimes assumed).
    float x = 0.0f;
    float y = 0.0f;
};

std::ostream& operator<<(std::ostream& os, const Vector2& v);

struct Vector2Hash
{
	std::size_t operator()(const Vector2& v) const noexcept
	{
		std::size_t h1 = std::hash<float>{}(v.x);
		std::size_t h2 = std::hash<float>{}(v.y);
		return h1 ^ (h2 << 1);
	}
};

