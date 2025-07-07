//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of lines.
//
#pragma once
#include "UIShapes.h"

#include "LineSegment.h"

class Vector2;

class UILines : public UIShapes<LineSegment>
{
    TYPEINFO_SUB(UILines, UIShapes<LineSegment>);
public:
    UILines(Actor* owner);

    void Add(const Vector2& startPoint, const Vector2& endPoint);
    void Set(size_t index, const Vector2& startPoint, const Vector2& endPoint);

protected:
    void GenerateMesh(const std::vector<LineSegment>& shapes, Mesh* mesh);

private:
};