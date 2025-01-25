#include "UIHexagrams.h"

#include "Mesh.h"

TYPEINFO_INIT(UIHexagrams, UIWidget, 34)
{

}

UIHexagrams::UIHexagrams(Actor* owner) : UIWidget(owner)
{

}

UIHexagrams::~UIHexagrams()
{
    delete mMesh;
}

void UIHexagrams::Render()
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

void UIHexagrams::SetColor(const Color32& color)
{
    mMaterial.SetColor(color);
}

void UIHexagrams::AddHexagram(const Vector2& center, float radius, float angle)
{
    mHexagrams.emplace_back();
    mHexagrams.back().center = center;
    mHexagrams.back().radius = radius;
    mHexagrams.back().angle = angle;
    mNeedMeshRegen = true;
}

void UIHexagrams::ClearHexagrams()
{
    mHexagrams.clear();
    mNeedMeshRegen = true;
}

void UIHexagrams::GenerateMesh()
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
    if(mHexagrams.empty()) { return; }

    // Create new mesh.
    mMesh = new Mesh();

    // Each hexagram consists of 12 vertices.
    // This is because we need to double up each vertex since we're rendering with Lines approach.
    size_t vertexCount = mHexagrams.size() * 12;
    float* positions = new float[vertexCount * 3];
    for(int i = 0; i < mHexagrams.size(); ++i)
    {
        Matrix3 rotMat = Matrix3::MakeRotateZ(mHexagrams[i].angle);
        const float kAngleInterval = Math::k2Pi / 6;
        Vector2 points[6];
        for(int j = 0; j < 6; ++j)
        {
            float angle = j * kAngleInterval;

            points[j].x = mHexagrams[i].radius * Math::Sin(angle);
            points[j].y = mHexagrams[i].radius * Math::Cos(angle);
            points[j] = rotMat.TransformVector(points[j]);
            points[j] += mHexagrams[i].center;
        }

        // Triangle 1, side 1.
        positions[i * 36 + 0] = points[0].x;
        positions[i * 36 + 1] = points[0].y;
        positions[i * 36 + 2] = 0.0f;
        positions[i * 36 + 3] = points[2].x;
        positions[i * 36 + 4] = points[2].y;
        positions[i * 36 + 5] = 0.0f;

        // Triangle 1, side 2.
        positions[i * 36 + 6] = points[2].x;
        positions[i * 36 + 7] = points[2].y;
        positions[i * 36 + 8] = 0.0f;
        positions[i * 36 + 9] = points[4].x;
        positions[i * 36 + 10] = points[4].y;
        positions[i * 36 + 11] = 0.0f;

        // Triangle 1, side 3.
        positions[i * 36 + 12] = points[4].x;
        positions[i * 36 + 13] = points[4].y;
        positions[i * 36 + 14] = 0.0f;
        positions[i * 36 + 15] = points[0].x;
        positions[i * 36 + 16] = points[0].y;
        positions[i * 36 + 17] = 0.0f;

        // Triangle 2, side 1.
        positions[i * 36 + 18] = points[1].x;
        positions[i * 36 + 19] = points[1].y;
        positions[i * 36 + 20] = 0.0f;
        positions[i * 36 + 21] = points[3].x;
        positions[i * 36 + 22] = points[3].y;
        positions[i * 36 + 23] = 0.0f;

        // Triangle 2, side 2.
        positions[i * 36 + 24] = points[3].x;
        positions[i * 36 + 25] = points[3].y;
        positions[i * 36 + 26] = 0.0f;
        positions[i * 36 + 27] = points[5].x;
        positions[i * 36 + 28] = points[5].y;
        positions[i * 36 + 29] = 0.0f;

        // Triangle 2, side 3.
        positions[i * 36 + 30] = points[5].x;
        positions[i * 36 + 31] = points[5].y;
        positions[i * 36 + 32] = 0.0f;
        positions[i * 36 + 33] = points[1].x;
        positions[i * 36 + 34] = points[1].y;
        positions[i * 36 + 35] = 0.0f;
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