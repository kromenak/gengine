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
	
	float GetX() { return mX; }
	float GetY() { return mY; }
	float GetWidth() { return mWidth; }
	float GetHeight() { return mHeight; }
	
	void SetX(float x) { mX = x; }
	void SetY(float y) { mY = y; }
	void SetWidth(float width) { mWidth = width; }
	void SetHeight(float height) { mHeight = height; }
	
	bool Contains(Vector2 vec);
	
private:
	float mX = 0.0f;
	float mY = 0.0f;
	
	float mWidth = 0.0f;
	float mHeight = 0.0f;
};
