//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of points.
//
#pragma once
#include "UIShapes.h"

#include <vector>

#include "Vector2.h"

class UIPoints : public UIShapes<Vector2>
{
    TYPEINFO_SUB(UIPoints, UIShapes<Vector2>);
public:
    UIPoints(Actor* owner);

    void Render() override;

protected:
    void GenerateMesh(const std::vector<Vector2>& shapes, Mesh* mesh) override;

private:
};