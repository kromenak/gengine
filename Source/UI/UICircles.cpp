#include "UICircles.h"

#include "Mesh.h"

TYPE_DEF_CHILD(UIWidget, UICircles);

UICircles::UICircles(Actor* owner) : UIWidget(owner)
{

}

UICircles::~UICircles()
{
    delete mMesh;
}

void UICircles::Render()
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

void UICircles::SetColor(const Color32& color)
{
    mMaterial.SetColor(color);
}

void UICircles::AddCircle(const Vector2& centerPoint, float radius)
{
    mCircles.emplace_back(centerPoint, radius);
    mNeedMeshRegen = true;
}

void UICircles::ClearCircles()
{
    mCircles.clear();
    mNeedMeshRegen = true;
}

void UICircles::GenerateMesh()
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

    // We don't need a mesh if we have no primitives.
    if(mCircles.empty()) { return; }

    // TThe circle's vertices are procedurally generated.
    // Smaller angle interval makes the circle smoother (at the cost of more vertices).
    const float kAngleInterval = 0.1f;
    const size_t kPointsPerCircle = Math::k2Pi / kAngleInterval;

    // Generate positions for each vertex.
    size_t vertexCount = mCircles.size() * kPointsPerCircle;
    float* positions = new float[vertexCount * 3];
    for(int i = 0; i < mCircles.size(); ++i)
    {
        for(int j = 0; j < kPointsPerCircle; ++j)
        {
            float angle = j * kAngleInterval;
            positions[(i * kPointsPerCircle) + (j * 3) + 0] = mCircles[i].center.x + mCircles[i].radius * Math::Sin(angle);
            positions[(i * kPointsPerCircle) + (j * 3) + 1] = mCircles[i].center.y + mCircles[i].radius * Math::Cos(angle);
            positions[(i * kPointsPerCircle) + (j * 3) + 2] = 0.0f;
        }
    }

    // Create new mesh.
    mMesh = new Mesh();

    // Create mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, vertexCount);
    meshDefinition.SetVertexLayout(VertexLayout::Packed);
    meshDefinition.AddVertexData(VertexAttribute::Position, positions);

    // Create submesh from definition. 
    Submesh* submesh = mMesh->AddSubmesh(meshDefinition);

    // A circle is actually rendered as a bunch of small lines, with the last line connecting to the first one.
    submesh->SetRenderMode(RenderMode::LineLoop);

    // Mesh has been generated.
    mNeedMeshRegen = false;
}