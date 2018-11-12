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
	
	Vector2 GetMin() const;
	Vector2 GetMax() const;
	
	bool Contains(const Vector2& vec) const;
	
private:
	float mX = 0.0f;
	float mY = 0.0f;
	
	float mWidth = 0.0f;
	float mHeight = 0.0f;
};
