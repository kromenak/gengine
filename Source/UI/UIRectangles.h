//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of rectangles.
//
#pragma once
#include "UIWidget.h"

#include <vector>

#include "Vector2.h"
#include "Material.h"

class Color32;
class Mesh;

struct UIRectangle
{
    Vector2 center;
    Vector2 size;
    float angle = 0.0f;
};

class UIRectangles : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UIRectangles(Actor* owner);
    ~UIRectangles();

    void Render() override;

    void SetColor(const Color32& color);

    void AddRectangle(const Vector2& center, const Vector2& size, float angle);
    void ClearRectangles();

    const UIRectangle& GetRectangle(size_t index) const { return mRectangles[index]; }
    size_t GetCount() const { return mRectangles.size(); }
    
private:
    // The lines in the set.
    std::vector<UIRectangle> mRectangles;

    // Material and mesh used for rendering.
    Material mMaterial;
    Mesh* mMesh = nullptr;
    bool mNeedMeshRegen = false;

    void GenerateMesh();
};