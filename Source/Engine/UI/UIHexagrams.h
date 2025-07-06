//
// Clark Kromenaker
//
// A (very specialized) UI widget that allows you to render hexagrams.
//
#pragma once
#include "UIWidget.h"

#include "Vector2.h"
#include "Material.h"

class Color32;
class Mesh;

struct UIHexagram
{
    Vector2 center;
    float radius = 0.0f;
    float angle = 0.0f;
};

class UIHexagrams : public UIWidget
{
    TYPEINFO_SUB(UIHexagrams, UIWidget);
public:
    UIHexagrams(Actor* owner);
    ~UIHexagrams();

    void Render() override;

    void SetColor(const Color32& color);

    void AddHexagram(const Vector2& center, float radius, float angle);
    void ClearHexagrams();

    const UIHexagram& GetHexagram(size_t index) const { return mHexagrams[index]; }
    size_t GetCount() const { return mHexagrams.size(); }

private:
    // The hexagrams in the set.
    std::vector<UIHexagram> mHexagrams;

    // Material and mesh used for rendering.
    Material mMaterial;
    Mesh* mMesh = nullptr;
    bool mNeedMeshRegen = false;

    void GenerateMesh();
};