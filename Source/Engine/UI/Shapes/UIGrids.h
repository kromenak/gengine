//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of grids.
//
#pragma once
#include "UIShapes.h"

#include "Vector2.h"

struct UIGrid
{
    Vector2 center;
    Vector2 size;
    float angle = 0.0f;

    int divisions = 8;

    bool drawBorder = true;
};

class UIGrids : public UIShapes<UIGrid>
{
    TYPEINFO_SUB(UIGrids, UIShapes<UIGrid>);
public:
    UIGrids(Actor* owner);

    void Add(const Vector2& center, const Vector2& size, float angle, int divisions, bool drawBorder = true);

protected:
    void GenerateMesh(const std::vector<UIGrid>& shapes, Mesh* mesh) override;

private:
};