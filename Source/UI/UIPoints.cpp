#include "UIPoints.h"

#include "Mesh.h"

TYPE_DEF_CHILD(UIWidget, UIPoints);

UIPoints::UIPoints(Actor* owner) : UIWidget(owner)
{

}

UIPoints::~UIPoints()
{
    delete mMesh;
}

void UIPoints::Render()
{
    if(!IsActiveAndEnabled()) { return; }

    // Generate the mesh, if needed.
    GenerateMesh();

    // If mesh is still null for some reason, we can't render.
    if(mMesh == nullptr) { return; }

    // Activate material.
    mMaterial.Activate(GetRectTransform()->GetLocalToWorldMatrix());

    // Render the mesh!
    mMesh->Render();
}

void UIPoints::SetColor(const Color32& color)
{
    mMaterial.SetColor(color);
}

void UIPoints::AddPoint(const Vector2& point)
{
    mPoints.push_back(point);
    mNeedMeshRegen = true;
}

void UIPoints::RemovePoint(const Vector2& point)
{
    for(int i = 0; i < mPoints.size(); ++i)
    {
        if(mPoints[i] == point)
        {
            RemovePoint(i);
            return;
        }
    }
}

void UIPoints::RemovePoint(size_t index)
{
    mPoints.erase(mPoints.begin() + index);
    mNeedMeshRegen = true;
}

void UIPoints::ClearPoints()
{
    mPoints.clear();
    mNeedMeshRegen = true;
}

void UIPoints::GenerateMesh()
{
    // Don't need to generate mesh if we have one and not dirty.
    if(mMesh != nullptr && !mNeedMeshRegen)
    {
        return;
    }

    // If a previous mesh exists, get rid of it.
    if(mMesh != nullptr)
    {
        delete mMesh;
        mMesh = nullptr;
    }

    // We don't need a mesh if we have no points.
    if(mPoints.empty()) { return; }

    // Create new mesh.
    mMesh = new Mesh();

    // Populate vertex positions array.
    float* positions = new float[mPoints.size() * 3];
    for(int i = 0; i < mPoints.size(); ++i)
    {
        positions[i * 3] = mPoints[i].x;
        positions[i * 3 + 1] = mPoints[i].y;
        positions[i * 3 + 2] = 0.0f;
    }

    // Create mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, mPoints.size());
    meshDefinition.SetVertexLayout(VertexLayout::Packed);
    meshDefinition.AddVertexData(VertexAttribute::Position, positions);

    // Create submesh from definition. 
    Submesh* submesh = mMesh->AddSubmesh(meshDefinition);

    // Render it in "points" mode, since this is a set of points!
    submesh->SetRenderMode(RenderMode::Points);

    // Mesh has been generated.
    mNeedMeshRegen = false;
}