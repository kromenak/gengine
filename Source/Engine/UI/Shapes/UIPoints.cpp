#include "UIPoints.h"

#include "AssetManager.h"
#include "Mesh.h"

TYPEINFO_INIT(UIPoints, UIShapes<Vector2>, 24)
{

}

UIPoints::UIPoints(Actor* owner) : UIShapes<Vector2>(owner)
{
    mMaterial.SetShader(gAssetManager.LoadShader("3D-Tex", "UI-Point-Circle"));
    mMaterial.SetDiffuseTexture(&Texture::White);
}

void UIPoints::GenerateMesh(const std::vector<Vector2>& shapes, Mesh* mesh)
{
    // Populate vertex positions array.
    float* positions = new float[shapes.size() * 3];
    for(int i = 0; i < shapes.size(); ++i)
    {
        positions[i * 3] = shapes[i].x;
        positions[i * 3 + 1] = shapes[i].y;
        positions[i * 3 + 2] = 0.0f;
    }

    // Create mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, shapes.size());
    meshDefinition.SetVertexLayout(VertexLayout::Packed);
    meshDefinition.AddVertexData(VertexAttribute::Position, positions);

    // Create submesh from definition.
    Submesh* submesh = mesh->AddSubmesh(meshDefinition);

    // Render it in "points" mode, since this is a set of points!
    submesh->SetRenderMode(RenderMode::Points);
}