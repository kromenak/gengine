//
// Clark Kromenaker
//
// Base class for a UI widget that can draw a shape.
//
#pragma once
#include "UIWidget.h"

#include <vector>

#include "Actor.h"
#include "Material.h"
#include "Mesh.h"

#define NEW_SHAPE_RENDERING

class Color32;

template<typename T>
class UIShapes : public UIWidget
{
    TYPEINFO_SUB(UIShapes, UIWidget);

public:
    UIShapes(Actor* owner) : UIWidget(owner)
    {
        // Defaulting to a white texture avoids undefined behavior.
        // If no texture is set in a material, the previously set texture is used, which can cause visual weirdness!
        mMaterial.SetDiffuseTexture(&Texture::White);
    }

    ~UIShapes()
    {
        delete mMesh;
    }

    void Render() override
    {
        // Don't render if owner is inactive or we are not enabled.
        if(!IsActiveAndEnabled()) { return; }

        // Nothing to render if shapes list is empty.
        if(mShapes.empty()) { return; }

        // Detect UI scale and regen mesh if needed.
        if(mRegenMeshOnUIScaleChange)
        {
            float currentUIScale = GetUIScale();
            if(currentUIScale != mLastUIScale)
            {
                mNeedMeshRegen = true;
                mLastUIScale = currentUIScale;
            }
        }

        // Generate the mesh, if needed.
        if(mNeedMeshRegen)
        {
            // If previously generated a mesh, delete the old one.
            if(mMesh != nullptr)
            {
                delete mMesh;
            }

            // Create a new one.
            mMesh = new Mesh();

            // Call subclass to actually generate a mesh.
            GenerateMesh(mShapes, mMesh);
            mNeedMeshRegen = false;
        }

        // Activate material.
        mMaterial.Activate(GetRectTransform()->GetLocalToWorldMatrix());

        // Render the mesh!
        mMesh->Render();
    }

    void Add(const T& shape)
    {
        mShapes.push_back(shape);
        mNeedMeshRegen = true;
    }

    void Set(size_t index, const T& shape)
    {
        mShapes[index] = shape;
        mNeedMeshRegen = true;
    }

    void Remove(const T& shape)
    {
        for(int i = 0; i < mShapes.size(); ++i)
        {
            if(mShapes[i] == shape)
            {
                Remove(i);
                return;
            }
        }
    }

    void Remove(size_t index)
    {
        mShapes.erase(mShapes.begin() + index);
        mNeedMeshRegen = true;
    }

    void Clear()
    {
        mShapes.clear();
        mNeedMeshRegen = true;
    }

    const T& Get(size_t index) const { return mShapes[index]; }
    size_t GetCount() const { return mShapes.size(); }

    void SetColor(const Color32& color) { mMaterial.SetColor(color); }

protected:
    // Material used to render the mesh.
    Material mMaterial;

    // If true, mesh will be regenerated if UI scale changes.
    bool mRegenMeshOnUIScaleChange = true;

    virtual void GenerateMesh(const std::vector<T>& shapes, Mesh* mesh) { }

    float GetUIScale() const { return GetOwner()->GetTransform()->GetWorldScale().x; }

private:
    // The shapes to be rendered.
    std::vector<T> mShapes;

    // Generated mesh containing the shapes to be rendered.
    Mesh* mMesh = nullptr;

    // If true, mesh needs to be regenerated.
    bool mNeedMeshRegen = true;

    // Last detected UI scale.
    float mLastUIScale = 1.0f;
};

TYPEINFO_INIT_TEMPLATE(UIShapes, UIWidget);