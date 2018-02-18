//
// Quaternion.h
//
// Clark Kromenaker
//
// For our purposes, a quaternion is a way to
// represent a rotation in 3D space.
//
#pragma once
#include <iostream>

class Vector3;

class Quaternion
{
public:
    static Quaternion Zero;
    static Quaternion Identity;
    
    Quaternion();
    Quaternion(float x, float y, float z, float w);
    Quaternion(const Vector3& axis, float angle);
    explicit Quaternion(const Vector3& vector);
    //explicit Quaternion(const Matrix33& matrix);
    ~Quaternion() { }
    
    // Copy
    Quaternion(const Quaternion& other);
    Quaternion& operator=(const Quaternion& other);
    
    // Equality
    bool operator==(const Quaternion& other) const;
    bool operator!=(const Quaternion& other) const;
    
    bool IsZero() const;
    bool IsUnit() const;
    bool IsIdentity() const;
    
    // Accessors
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
    float GetW() const { return w; }
    //TODO: [] overload?
    
    // Manipulators
    void SetX(float newX) { x = newX; }
    void SetY(float newY) { y = newY; }
    void SetZ(float newZ) { z = newZ; }
    void SetW(float newW) { w = newW; }
    void Set(float newX, float newY, float newZ, float newW)
    {
        x = newX;
        y = newY;
        z = newZ;
        w = newW;
    }
    void Set(const Vector3& axis, float angle);
    void Set(const Vector3& from, const Vector3& to);
    //void Set(const Matrix3& rotation);
    void Set(float xRadians, float yRadians, float zRadians);
    
    void GetAxisAngle(Vector3& axis, float& angle) const;
    
    // Modifiers
    void MakeZero() { x = y = z = w = 0.0f; }
    void MakeIdentity() { x = y = z = 0.0f; w = 1.0f; }
   
    float GetLength() const;
    void Normalize();
    
    static Quaternion Inverse(const Quaternion& quat);
    const Quaternion& Invert();
    
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
    
    //TODO: Multiply by a Vector.
    
    // Dot Product
    static float Dot(const Quaternion& quat1, const Quaternion& quat2);
    
    // Rotate Vector by this Quaternion
    Vector3 Rotate(const Vector3& vector) const;
    
    // Interpolate
    static void Lerp(Quaternion& result, const Quaternion& start, const Quaternion& end, float t);
    static void Slerp(Quaternion& result, const Quaternion& start, const Quaternion& end, float t);
    
private:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

std::ostream& operator<<(std::ostream& os, const Quaternion& q);
