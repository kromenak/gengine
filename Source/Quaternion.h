//
// Quaternion.h
//
// Clark Kromenaker
//
// A quaternion represents a rotation in 3D space.
// Think of it as an axis of rotation and an angle of rotation.
//
#pragma once
#include <iostream>

class Vector3;
class Matrix3;

class Quaternion
{
public:
    static Quaternion Zero;
    static Quaternion Identity;
    
    Quaternion() = default;
    Quaternion(float x, float y, float z, float w);
    Quaternion(const Vector3& axis, float angle);
	
	// Conversion from Vector3 (component-wise copy)
    explicit Quaternion(const Vector3& vector);
	
	// Conversion from Matrix - rotation that matches the matrix axes
    explicit Quaternion(const Matrix3& matrix);
    
    // Copy
    Quaternion(const Quaternion& other);
    Quaternion& operator=(const Quaternion& other);
    
    // Equality
    bool operator==(const Quaternion& other) const;
    bool operator!=(const Quaternion& other) const;
    
    // Accessors - using [index] notation to get x/y/z/w
    float& operator[](int i)       { return (&x)[i]; }
    float  operator[](int i) const { return (&x)[i]; }
    
    // Addition and Subtraction
    Quaternion operator+(const Quaternion& other) const;
    Quaternion& operator+=(const Quaternion& other);
    Quaternion operator-(const Quaternion& other) const;
    Quaternion& operator-=(const Quaternion& other);
    Quaternion operator-() const;
    
    // Scalar Multiplication
    friend Quaternion operator*(float scalar, const Quaternion& quat);
    Quaternion& operator*=(float scalar);
    
    // Quaternion Multiplication
    Quaternion operator*(const Quaternion& other) const;
    Quaternion& operator*=(const Quaternion& other);
    
    // Rotate Vector
    Vector3 Rotate(const Vector3& vector) const;
	
	// Conversions To
    void Set(const Vector3& axis, float angle); // From axis/angle
    void Set(const Vector3& from, const Vector3& to); // From from/to facing
    void Set(const Matrix3& rotation); // From matrix representation
    void Set(float xRadians, float yRadians, float zRadians); // From euler angles
    
	// Conversions From
    void GetAxisAngle(Vector3& axis, float& angle) const; // To axis/angle
	Vector3 GetEulerAngles() const; // To euler angles
    
	// Length
	bool IsUnit() const;
    float GetLength() const;
    void Normalize();
    
    // Dot Product
    static float Dot(const Quaternion& quat1, const Quaternion& quat2);
    
	// Inversion
    void Invert();
    static Quaternion Inverse(const Quaternion& quat);
    

    
    // Interpolate
    static void Lerp(Quaternion& result, const Quaternion& start, const Quaternion& end, float t);
    static void Slerp(Quaternion& result, const Quaternion& start, const Quaternion& end, float t);
    
    // Quaternion elements. Order is important (memory layout is sometimes assumed).
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

std::ostream& operator<<(std::ostream& os, const Quaternion& q);
