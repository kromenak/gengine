//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of points.
//
#pragma once
#include "UIWidget.h"

#include <vector>

#include "Material.h"
#include "Vector2.h"

class Color32;
class Mesh;

class UIPoints : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UIPoints(Actor* owner);
    ~UIPoints();

    void Render() override;

    void SetColor(const Color32& color);

    void AddPoint(const Vector2& point);
    void RemovePoint(const Vector2& point);
    void RemovePoint(size_t index);
    void ClearPoints();

    const Vector2& GetPoint(size_t index) const { return mPoints[index]; }
    size_t GetPointsCount() const { return mPoints.size(); }
    
private:
    // The points in the set.
    std::vector<Vector2> mPoints;

    // Material and mesh used for rendering.
    Material mMaterial;
    Mesh* mMesh = nullptr;
    bool mNeedMeshRegen = false;

    void GenerateMesh();
};