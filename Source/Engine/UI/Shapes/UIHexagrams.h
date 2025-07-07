//
// Clark Kromenaker
//
// A (very specialized) UI widget that allows you to render hexagrams.
//
#pragma once
#include "UIShapes.h"

#include "Vector2.h"

struct UIHexagram
{
    Vector2 center;
    float radius = 0.0f;
    float angle = 0.0f;
};

class UIHexagrams : public UIShapes<UIHexagram>
{
    TYPEINFO_SUB(UIHexagrams, UIShapes<UIHexagram>);
public:
    UIHexagrams(Actor* owner);

    void Add(const Vector2& center, float radius, float angle);

protected:
    void GenerateMesh(const std::vector<UIHexagram>& shapes, Mesh* mesh) override;

private:

};