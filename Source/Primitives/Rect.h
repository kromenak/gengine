//
// Clark Kromenaker
//
// A 2D rectangular area.
//
#pragma once
#include "Vector2.h"

class Rect
{
public:
    Rect() = default;
	Rect(float x, float y);
	Rect(float x, float y, float width, float height);
	Rect(const Vector2& p0, const Vector2& p1);
	
	bool operator==(const Rect& other) const;
	bool operator!=(const Rect& other) const;
	
	Vector2 GetMin() const { return Vector2(x, y); }
	Vector2 GetMax() const { return Vector2(x + width, y + height); }
    Vector2 GetCenter() const { return Vector2(x + (width * 0.5f), y + (height * 0.5f)); }

	Vector2 GetSize() const { return Vector2(width, height); }
	
    void Contain(const Rect& other);
	bool Contains(const Vector2& vec) const;
	bool Overlaps(const Rect& other) const;
	
	Vector2 GetPoint(const Vector2& normalizedPoint) const;
	Vector2 GetNormalizedPoint(const Vector2& point) const;
	
    // Coordinate of the minimum corner of the rect.
	float x = 0.0f;
	float y = 0.0f;
    
    // Width and height; add to x/y to get max point.
	float width = 0.0f;
	float height = 0.0f;
};

std::ostream& operator<<(std::ostream& os, const Rect& r);
