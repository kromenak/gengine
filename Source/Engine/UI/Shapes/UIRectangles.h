//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of rectangles.
//
#pragma once
#include "UIShapes.h"

#include "Vector2.h"

struct UIRectangle
{
    Vector2 center;
    Vector2 size;
    float angle = 0.0f;
};

class UIRectangles : public UIShapes<UIRectangle>
{
    TYPEINFO_SUB(UIRectangles, UIShapes<UIRectangle>);
public:
    UIRectangles(Actor* owner);

    void Add(const Vector2& center, const Vector2& size, float angle);

protected:
    void GenerateMesh(const std::vector<UIRectangle>& shapes, Mesh* mesh) override;

private:

};