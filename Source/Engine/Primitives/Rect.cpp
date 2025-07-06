#include "Rect.h"

Rect::Rect(float x, float y) : x(x), y(y)
{

}

Rect::Rect(float x, float y, float width, float height) :
    x(x),
    y(y),
    width(width),
    height(height)
{

}

Rect::Rect(const Vector2& p0, const Vector2& p1)
{
    // Make sure p0 has min and p1 has max.
    Vector2 min = p0;
    Vector2 max = p1;
    if(min.x > max.x)
    {
        float temp = min.x;
        min.x = max.x;
        max.x = temp;
    }
    if(min.y > max.y)
    {
        float temp = min.y;
        min.y = max.y;
        max.y = temp;
    }

    // Assign values.
    x = min.x;
    y = min.y;
    width = max.x - min.x;
    height = max.y - min.y;
}

bool Rect::operator==(const Rect& other) const
{
    return Math::AreEqual(x, other.x)
        && Math::AreEqual(y, other.y)
        && Math::AreEqual(width, other.width)
        && Math::AreEqual(height, other.height);
}

bool Rect::operator!=(const Rect& other) const
{
    return !(Math::AreEqual(x, other.x)
          && Math::AreEqual(y, other.y)
          && Math::AreEqual(width, other.width)
          && Math::AreEqual(height, other.height));
}

void Rect::Contain(const Rect& other)
{
    // Figure out new max based on two max points.
    Vector2 max = GetMax();
    Vector2 otherMax = other.GetMax();
    float biggestMaxX = Math::Max(max.x, otherMax.x);
    float biggestMaxY = Math::Max(max.y, otherMax.y);

    // Expand mins.
    x = Math::Min(other.x, x);
    y = Math::Min(other.y, y);

    // Calculate new width/height.
    width = biggestMaxX - x;
    height = biggestMaxY - y;
}

bool Rect::Contains(const Vector2& vec) const
{
    // If x/y of vector are less than rect's x/y, vector is not contained.
    if(vec.x < x) { return false; }
    if(vec.y < y) { return false; }

    // If x/y of vector are greater than rect's w/h, vector is not contained.
    if(vec.x > x + width) { return false; }
    if(vec.y > y + height) { return false; }

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
    bool noMinX = x > otherMax.x;
    bool noMaxX = max.x < other.x;
    bool noMinY = y > otherMax.y;
    bool noMaxY = max.y < other.y;
    return !noMinX && !noMaxX && !noMinY && !noMaxY;
}

Vector2 Rect::GetPoint(const Vector2& normalizedPoint) const
{
    return Vector2(x + (width * normalizedPoint.x), y + (height * normalizedPoint.y));
}

Vector2 Rect::GetNormalizedPoint(const Vector2& point) const
{
    float normalizedX = 0.0f;
    if(width != 0.0f)
    {
        normalizedX = (point.x - x) / width;
    }

    float normalizedY = 0.0f;
    if(height != 0.0f)
    {
        normalizedY = (point.y - y) / height;
    }
    return Vector2(normalizedX, normalizedY);
}

std::ostream& operator<<(std::ostream& os, const Rect& r)
{
    os << "(x:" << r.x << ", y:" << r.y << ", width:" << r.width << ", height:" << r.height << ")";
    return os;
}
