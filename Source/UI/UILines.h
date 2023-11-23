//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of lines.
//
#pragma once
#include "UIWidget.h"

#include <vector>

#include "LineSegment.h"
#include "Material.h"

class Color32;
class Mesh;
class Vector2;

class UILines : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UILines(Actor* owner);
    ~UILines();

    void Render() override;

    void AddLine(const Vector2& startPoint, const Vector2& endPoint);
    
    void SetColor(const Color32& color);

private:
    // The lines in the set.
    std::vector<LineSegment> mLines;

    // Material and mesh used for rendering.
    Material mMaterial;
    Mesh* mMesh = nullptr;
    bool mNeedMeshRegen = false;

    void GenerateMesh();
};