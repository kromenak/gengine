//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of points.
//
#pragma once
#include "UIWidget.h"

#include <vector>

#include "Material.h"

class Color32;
class Mesh;
class Vector2;

class UIPoints : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UIPoints(Actor* owner);
    ~UIPoints();

    void Render() override;

    void AddPoint(const Vector2& point);
    void ClearPoints();

    void SetColor(const Color32& color);

private:
    // The points in the set.
    std::vector<Vector2> mPoints;

    // Material and mesh used for rendering.
    Material mMaterial;
    Mesh* mMesh = nullptr;
    bool mNeedMeshRegen = false;

    void GenerateMesh();
};