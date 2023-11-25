#include "UILines.h"

#include "Mesh.h"

TYPE_DEF_CHILD(UIWidget, UILines);

UILines::UILines(Actor* owner) : UIWidget(owner)
{

}

UILines::~UILines()
{
    delete mMesh;
}

void UILines::Render()
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

void UILines::SetColor(const Color32& color)
{
    mMaterial.SetColor(color);
}

void UILines::AddLine(const Vector2& startPoint, const Vector2& endPoint)
{
    mLines.emplace_back(startPoint, endPoint);
    mNeedMeshRegen = true;
}

void UILines::GenerateMesh()
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

    // We don't need a mesh if we have no lines.
    if(mLines.empty()) { return; }

    // Create new mesh.
    mMesh = new Mesh();

    // We need two positions per line (2 * 3).
    float* positions = new float[mLines.size() * 2 * 3];
    for(int i = 0; i < mLines.size(); ++i)
    {
        // Start point
        positions[i * 6 + 0] = mLines[i].start.x;
        positions[i * 6 + 1] = mLines[i].start.y;
        positions[i * 6 + 2] = 0.0f;

        // End point
        positions[i * 6 + 3] = mLines[i].end.x;
        positions[i * 6 + 4] = mLines[i].end.y;
        positions[i * 6 + 5] = 0.0f;
    }

    // Create mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, mLines.size() * 2);
    meshDefinition.SetVertexLayout(VertexLayout::Packed);
    meshDefinition.AddVertexData(VertexAttribute::Position, positions);

    // Create submesh from definition. 
    Submesh* submesh = mMesh->AddSubmesh(meshDefinition);

    // Render it in "lines" mode, since this is a set of lines!
    submesh->SetRenderMode(RenderMode::Lines);

    // Mesh has been generated.
    mNeedMeshRegen = false;
}