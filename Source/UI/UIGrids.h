//
// Clark Kromenaker
//
// A UI widget that allows you to render an arbitrary set of grids.
//
#pragma once
#include "UIWidget.h"

#include <vector>

#include "Vector2.h"
#include "Material.h"

class Color32;
class Mesh;

struct UIGrid
{
    Vector2 center;
    Vector2 size;
    float angle = 0.0f;

    int divisions = 8;

    bool drawBorder = true;
};

class UIGrids : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UIGrids(Actor* owner);
    ~UIGrids();

    void Render() override;
    void SetColor(const Color32& color);

    void Add(const Vector2& center, const Vector2& size, float angle, int divisions, bool drawBorder = true);
    void Clear();

    const UIGrid& GetGrid(size_t index) const { return mGrids[index]; }
    size_t GetCount() const { return mGrids.size(); }
    
private:
    // The lines in the set.
    std::vector<UIGrid> mGrids;

    // Material and mesh used for rendering.
    Material mMaterial;
    Mesh* mMesh = nullptr;
    bool mNeedMeshRegen = false;

    void GenerateMesh();
};