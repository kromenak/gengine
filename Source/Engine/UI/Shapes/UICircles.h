//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set circles.
//
#pragma once
#include "UIShapes.h"

#include "Circle.h"

class Vector2;

class UICircles : public UIShapes<Circle>
{
    TYPEINFO_SUB(UICircles, UIShapes<Circle>);
public:
    UICircles(Actor* owner);

    void Add(const Vector2& center, float radius);

protected:
    void GenerateMesh(const std::vector<Circle>& shapes, Mesh* mesh) override;

private:

};