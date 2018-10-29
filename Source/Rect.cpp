//
// Rect.cpp
//
// Clark Kromenaker
//
#include "Rect.h"

Rect::Rect(float x, float y) : mX(x), mY(y)
{
	
}

Rect::Rect(float x, float y, float width, float height) : mX(x), mY(y),
	mWidth(width), mHeight(height)
{
	
}

bool Rect::Contains(Vector2 vec)
{
	// If x/y of vector are less than rect's x/y, vector is not contained.
	if(vec.GetX() < mX) { return false; }
	if(vec.GetY() < mY) { return false; }
	
	// If x/y of vector are greater than rect's w/h, vector is not contained.
	if(vec.GetX() > mX + mWidth) { return false; }
	if(vec.GetY() > mY + mHeight) { return false; }
	
	// If x/y of vector are both greater than rect's x/y and less than w/h, it is contained!
	return true;
}
