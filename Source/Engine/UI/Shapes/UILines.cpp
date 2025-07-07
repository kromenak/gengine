#include "UILines.h"

#include "Mesh.h"

TYPEINFO_INIT(UILines, UIShapes<LineSegment>, 22)
{

}

UILines::UILines(Actor* owner) : UIShapes<LineSegment>(owner)
{

}

void UILines::Add(const Vector2& startPoint, const Vector2& endPoint)
{
    UIShapes<LineSegment>::Add(LineSegment(startPoint, endPoint));
}

void UILines::Set(size_t index, const Vector2& startPoint, const Vector2& endPoint)
{
    UIShapes<LineSegment>::Set(index, LineSegment(startPoint, endPoint));
}

void UILines::GenerateMesh(const std::vector<LineSegment>& shapes, Mesh* mesh)
{
    // We need two positions per line (2 * 3).
    float* positions = new float[shapes.size() * 2 * 3];
    for(int i = 0; i < shapes.size(); ++i)
    {
        // Start point
        positions[i * 6 + 0] = shapes[i].start.x;
        positions[i * 6 + 1] = shapes[i].start.y;
        positions[i * 6 + 2] = 0.0f;

        // End point
        positions[i * 6 + 3] = shapes[i].end.x;
        positions[i * 6 + 4] = shapes[i].end.y;
        positions[i * 6 + 5] = 0.0f;
    }

    // Create mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, shapes.size() * 2);
    meshDefinition.SetVertexLayout(VertexLayout::Packed);
    meshDefinition.AddVertexData(VertexAttribute::Position, positions);

    // Create submesh from definition.
    Submesh* submesh = mesh->AddSubmesh(meshDefinition);

    // Render it in "lines" mode, since this is a set of lines!
    submesh->SetRenderMode(RenderMode::Lines);
}