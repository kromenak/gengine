//
// Rect.cpp
//
// Clark Kromenaker
//
#include "Rect.h"

/*static*/ Rect Rect::FromTwoPoints(Vector2 p1, Vector2 p2)
{
	// Difference from min/max constructor: give any two points and a valid rect will be made!
	// Don't need to worry about whether min is actually min or max is actually max.
	
	// Make sure p1 has min values and p2 has max.
	if(p1.x > p2.x)
	{
		float temp = p1.x;
		p1.x = p2.x;
		p2.x = temp;
	}
	if(p1.y > p2.y)
	{
		float temp = p1.y;
		p1.y = p2.y;
		p2.y = temp;
	}
	
	// Use min/max constructor per usual.
	return Rect(p1, p2);
}

Rect::Rect()
{
	
}

Rect::Rect(float x, float y) : mX(x), mY(y)
{
	
}

Rect::Rect(float x, float y, float width, float height) : mX(x), mY(y),
	mWidth(width), mHeight(height)
{

}

Rect::Rect(const Vector2& min, const Vector2& max) :
	mX(min.x), mY(min.y),
	mWidth(max.x - min.x),
	mHeight(max.y - min.y)
{
	
}

bool Rect::operator==(const Rect& other) const
{
	return Math::AreEqual(mX, other.mX)
		&& Math::AreEqual(mY, other.mY)
		&& Math::AreEqual(mWidth, other.mWidth)
		&& Math::AreEqual(mHeight, other.mHeight);
}

bool Rect::operator!=(const Rect& other) const
{
	return !(Math::AreEqual(mX, other.mX)
		  && Math::AreEqual(mY, other.mY)
		  && Math::AreEqual(mWidth, other.mWidth)
		  && Math::AreEqual(mHeight, other.mHeight));
}

bool Rect::Contains(const Vector2& vec) const
{
	// If x/y of vector are less than rect's x/y, vector is not contained.
	if(vec.x < mX) { return false; }
	if(vec.y < mY) { return false; }
	
	// If x/y of vector are greater than rect's w/h, vector is not contained.
	if(vec.x > mX + mWidth) { return false; }
	if(vec.y > mY + mHeight) { return false; }
	
	// If x/y of vector are both greater than rect's x/y and less than w/h, it is contained!
	return true;
}

bool Rect::Overlaps(const Rect& other) const
{
	// Calculate maxes - just use x/y values for mins.
	Vector2 max = GetMax();
	Vector2 otherMax = other.GetMax();

	// Check for overlaps on each side.
	// As with an AABB, if no overlap on any side, we're not colliding.
	bool noMinX = mX > otherMax.x;
	bool noMaxX = max.x < other.GetX();
	bool noMinY = mY > otherMax.y;
	bool noMaxY = max.y < other.GetY();
	return !noMinX && !noMaxX && !noMinY && !noMaxY;
}

Vector2 Rect::GetPoint(Vector2 normalizedPoint) const
{
	return Vector2(mX + (mWidth * normalizedPoint.x), mY + (mHeight * normalizedPoint.y));
}

Vector2 Rect::GetNormalizedPoint(Vector2 point) const
{
	float normalizedX = 0.0f;
	if(mWidth != 0.0f)
	{
		normalizedX = (point.x - mX) / mWidth;
	}
	
	float normalizedY = 0.0f;
	if(mHeight != 0.0f)
	{
		normalizedY = (point.y - mY) / mHeight;
	}
	return Vector2(normalizedX, normalizedY);
}

std::ostream& operator<<(std::ostream& os, const Rect& r)
{
	os << "(x:" << r.GetX() << ", y:" << r.GetY() << ", width:" << r.GetWidth() << ", height:" << r.GetHeight() << ")";
	return os;
}
