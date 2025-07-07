#include "UICircles.h"

#include "Mesh.h"

TYPEINFO_INIT(UICircles, UIShapes<Circle>, 17)
{

}

UICircles::UICircles(Actor* owner) : UIShapes(owner)
{

}

void UICircles::Add(const Vector2& center, float radius)
{
    Circle circle(center, radius);
    UIShapes<Circle>::Add(circle);
}

void UICircles::GenerateMesh(const std::vector<Circle>& shapes, Mesh* mesh)
{
    // TThe circle's vertices are procedurally generated.
    // Smaller angle interval makes the circle smoother (at the cost of more vertices).
    const float kAngleInterval = 0.1f;
    const size_t kPointsPerCircle = Math::k2Pi / kAngleInterval;

    // Generate positions for each vertex.
    size_t vertexCount = shapes.size() * kPointsPerCircle;
    float* positions = new float[vertexCount * 3];
    for(int i = 0; i < shapes.size(); ++i)
    {
        for(int j = 0; j < kPointsPerCircle; ++j)
        {
            float angle = j * kAngleInterval;
            positions[(i * kPointsPerCircle) + (j * 3) + 0] = shapes[i].center.x + shapes[i].radius * Math::Sin(angle);
            positions[(i * kPointsPerCircle) + (j * 3) + 1] = shapes[i].center.y + shapes[i].radius * Math::Cos(angle);
            positions[(i * kPointsPerCircle) + (j * 3) + 2] = 0.0f;
        }
    }

    // Create mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, vertexCount);
    meshDefinition.SetVertexLayout(VertexLayout::Packed);
    meshDefinition.AddVertexData(VertexAttribute::Position, positions);

    // Create submesh from definition.
    Submesh* submesh = mesh->AddSubmesh(meshDefinition);

    // A circle is actually rendered as a bunch of small lines, with the last line connecting to the first one.
    submesh->SetRenderMode(RenderMode::LineLoop);
}