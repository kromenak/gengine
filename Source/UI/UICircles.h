//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set circles.
//
#pragma once
#include "UIWidget.h"

#include <vector>

#include "Circle.h"
#include "Material.h"

class Color32;
class Mesh;
class Vector2;

class UICircles : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UICircles(Actor* owner);
    ~UICircles();

    void Render() override;

    void SetColor(const Color32& color);

    void AddCircle(const Vector2& centerPoint, float radius);
    void ClearCircles();

    const Circle& GetCircle(size_t index) const { return mCircles[index]; }
    size_t GetCirclesCount() const { return mCircles.size(); }
    
private:
    // The circles in the set.
    std::vector<Circle> mCircles;

    // Material and mesh used for rendering.
    Material mMaterial;
    Mesh* mMesh = nullptr;
    bool mNeedMeshRegen = false;

    void GenerateMesh();
};