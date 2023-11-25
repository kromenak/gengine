#include "UIRectangles.h"

#include "Matrix3.h"
#include "Mesh.h"

TYPE_DEF_CHILD(UIWidget, UIRectangles);

UIRectangles::UIRectangles(Actor* owner) : UIWidget(owner)
{

}

UIRectangles::~UIRectangles()
{
    delete mMesh;
}

void UIRectangles::Render()
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

void UIRectangles::SetColor(const Color32& color)
{
    mMaterial.SetColor(color);
}

void UIRectangles::AddRectangle(const Vector2& center, const Vector2& size, float angle)
{
    mRectangles.emplace_back();
    mRectangles.back().center = center;
    mRectangles.back().size = size;
    mRectangles.back().angle = angle;
    mNeedMeshRegen = true;
}

void UIRectangles::ClearRectangles()
{
    mRectangles.clear();
    mNeedMeshRegen = true;
}

void UIRectangles::GenerateMesh()
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
    if(mRectangles.empty()) { return; }

    // Create new mesh.
    mMesh = new Mesh();

    // Each rectangle consists of eight vertices.
    // This is because we need to double up each vertex since we're rendering with Lines approach.
    size_t vertexCount = mRectangles.size() * 8;

    float* positions = new float[vertexCount * 3];
    for(int i = 0; i < mRectangles.size(); ++i)
    {
        float halfWidth = mRectangles[i].size.x * 0.5f;
        float halfHeight = mRectangles[i].size.y * 0.5f;

        Matrix3 rotMat = Matrix3::MakeRotateZ(mRectangles[i].angle);
        Vector2 topLeftDir = rotMat.TransformVector(Vector2(-halfWidth, halfHeight));
        Vector2 topRightDir = rotMat.TransformVector(Vector2(halfWidth, halfHeight));
        Vector2 botLeftDir = rotMat.TransformVector(Vector2(-halfWidth, -halfHeight));
        Vector2 botRightDir = rotMat.TransformVector(Vector2(halfWidth, -halfHeight));

        Vector2 topLeft = mRectangles[i].center + topLeftDir;
        Vector2 topRight = mRectangles[i].center + topRightDir;
        Vector2 bottomLeft = mRectangles[i].center + botLeftDir;
        Vector2 bottomRight = mRectangles[i].center + botRightDir;

        // Left Side
        positions[i * 24 + 0] = topLeft.x;
        positions[i * 24 + 1] = topLeft.y;
        positions[i * 24 + 2] = 0.0f;
        positions[i * 24 + 3] = bottomLeft.x;
        positions[i * 24 + 4] = bottomLeft.y;
        positions[i * 24 + 5] = 0.0f;

        // Bottom Side
        positions[i * 24 + 6] = bottomLeft.x;
        positions[i * 24 + 7] = bottomLeft.y;
        positions[i * 24 + 8] = 0.0f;
        positions[i * 24 + 9] = bottomRight.x;
        positions[i * 24 + 10] = bottomRight.y;
        positions[i * 24 + 11] = 0.0f;

        // Right Side
        positions[i * 24 + 12] = bottomRight.x;
        positions[i * 24 + 13] = bottomRight.y;
        positions[i * 24 + 14] = 0.0f;
        positions[i * 24 + 15] = topRight.x;
        positions[i * 24 + 16] = topRight.y;
        positions[i * 24 + 17] = 0.0f;

        // Top Side
        positions[i * 24 + 18] = topRight.x;
        positions[i * 24 + 19] = topRight.y;
        positions[i * 24 + 20] = 0.0f;
        positions[i * 24 + 21] = topLeft.x;
        positions[i * 24 + 22] = topLeft.y;
        positions[i * 24 + 23] = 0.0f;
    }

    // Create mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, vertexCount);
    meshDefinition.SetVertexLayout(VertexLayout::Packed);
    meshDefinition.AddVertexData(VertexAttribute::Position, positions);

    // Create submesh from definition. 
    Submesh* submesh = mMesh->AddSubmesh(meshDefinition);

    // Render it in "lines" mode, since this is a set of lines!
    submesh->SetRenderMode(RenderMode::Lines);

    // Mesh has been generated.
    mNeedMeshRegen = false;
}