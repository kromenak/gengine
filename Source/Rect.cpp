//
// Rect.cpp
//
// Clark Kromenaker
//
#include "Rect.h"

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
	mX(min.GetX()), mY(min.GetY()),
	mWidth(max.GetX() - min.GetX()),
	mHeight(max.GetY() - min.GetY())
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
	if(vec.GetX() < mX) { return false; }
	if(vec.GetY() < mY) { return false; }
	
	// If x/y of vector are greater than rect's w/h, vector is not contained.
	if(vec.GetX() > mX + mWidth) { return false; }
	if(vec.GetY() > mY + mHeight) { return false; }
	
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
	bool noMinX = mX > otherMax.GetX();
	bool noMaxX = max.GetX() < other.GetX();
	bool noMinY = mY > otherMax.GetY();
	bool noMaxY = max.GetY() < other.GetY();
	return !noMinX && !noMaxX && !noMinY && !noMaxY;
}

Vector2 Rect::GetPoint(Vector2 normalizedPoint) const
{
	return Vector2(mX + (mWidth * normalizedPoint.GetX()), mY + (mHeight * normalizedPoint.GetY()));
}

Vector2 Rect::GetNormalizedPoint(Vector2 point) const
{
	float normalizedX = 0.0f;
	if(mWidth != 0.0f)
	{
		normalizedX = (point.GetX() - mX) / mWidth;
	}
	
	float normalizedY = 0.0f;
	if(mHeight != 0.0f)
	{
		normalizedY = (point.GetY() - mY) / mHeight;
	}
	return Vector2(normalizedX, normalizedY);
}

std::ostream& operator<<(std::ostream& os, const Rect& r)
{
	os << "(x:" << r.GetX() << ", y:" << r.GetY() << ", width:" << r.GetWidth() << ", height:" << r.GetHeight() << ")";
	return os;
}
