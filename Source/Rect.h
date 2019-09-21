//
// Rect.h
//
// Clark Kromenaker
//
// A rectangular area in 2D space.
//
#pragma once
#include "Vector2.h"

class Rect
{
public:
	Rect();
	Rect(float x, float y);
	Rect(float x, float y, float width, float height);
	Rect(const Vector2& min, const Vector2& max);
	
	float GetX() const { return mX; }
	float GetY() const { return mY; }
	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }
	
	void SetX(float x) { mX = x; }
	void SetY(float y) { mY = y; }
	void SetWidth(float width) { mWidth = width; }
	void SetHeight(float height) { mHeight = height; }
	
	Vector2 GetMin() const { return Vector2(mX, mY); }
	Vector2 GetMax() const { return Vector2(mX + mWidth, mY + mHeight); }
	
	Vector2 GetSize() const { return Vector2(mWidth, mHeight); }
	
	bool Contains(const Vector2& vec) const;
	bool Overlaps(const Rect& other) const;
	
	Vector2 GetPoint(Vector2 normalizedPoint) const;
	Vector2 GetNormalizedPoint(Vector2 point) const;
	
private:
	float mX = 0.0f;
	float mY = 0.0f;
	
	float mWidth = 0.0f;
	float mHeight = 0.0f;
};
