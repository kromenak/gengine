#include "UILines.h"

#include "Actor.h"
#include "Mesh.h"

TYPEINFO_INIT(UILines, UIShapes<LineSegment>, 22)
{

}

/*static*/ void UILines::GenerateMesh(const std::vector<LineSegment>& shapes, Mesh* mesh, float scale)
{
    // Each line segment requires 4 vertices and six indices representing 2 triangles.
    // Line segments aren't necessarily connected to one another, so we can't use triangle strips.
    Vector3* positions = new Vector3[shapes.size() * 4];
    uint16_t* indexes = new uint16_t[shapes.size() * 6];

    // When a line scales, the scale is applied to both the left and right sides equally.
    // So when calculating vertex positions, we use half the scale on each side of the line.
    float halfScale = scale * 0.5f;

    // Generate vertex positions and indices for each line segment.
    for(int i = 0; i < shapes.size(); ++i)
    {
        // The segment's start and end points represent the line, but it has no width!
        // To calculate width, we need to know the perpendicular direction to the line.
        // Fortunately, in 2D, this is easily done by swapping x/y and negating one of the components.
        Vector3 lineSegmentDir = Vector3::Normalize(shapes[i].end - shapes[i].start);
        Vector2 lineSegmentPerpDir(lineSegmentDir.y, -lineSegmentDir.x);
        lineSegmentPerpDir.Normalize();

        // Calculate the 4 points of the line segment's quad mesh.
        Vector2 p1 = shapes[i].start + lineSegmentPerpDir * halfScale;
        Vector2 p2 = shapes[i].start - lineSegmentPerpDir * halfScale;
        Vector2 p3 = shapes[i].end + lineSegmentPerpDir * halfScale;
        Vector2 p4 = shapes[i].end - lineSegmentPerpDir * halfScale;

        // We can also apply half scale in the direction of the line segment on both ends.
        // This has the effect of "capping" the line segment, applying the scale to the end points too.
        // This looks particularly good if the line segments form a square/rectangle or other shape.
        p1 -= lineSegmentDir * halfScale;
        p2 -= lineSegmentDir * halfScale;
        p3 += lineSegmentDir * halfScale;
        p4 += lineSegmentDir * halfScale;

        // Pack the positions.
        positions[i * 4 + 0] = p1;
        positions[i * 4 + 1] = p2;
        positions[i * 4 + 2] = p3;
        positions[i * 4 + 3] = p4;

        // Pack the indices.
        indexes[i * 6 + 0] = i * 4 + 0;
        indexes[i * 6 + 1] = i * 4 + 1;
        indexes[i * 6 + 2] = i * 4 + 2;
        indexes[i * 6 + 3] = i * 4 + 1;
        indexes[i * 6 + 4] = i * 4 + 2;
        indexes[i * 6 + 5] = i * 4 + 3;
    }

    // Create mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, shapes.size() * 4);
    meshDefinition.SetVertexLayout(VertexLayout::Packed);
    meshDefinition.AddVertexData(VertexAttribute::Position, positions);
    meshDefinition.SetIndexData(shapes.size() * 6, indexes);

    // Create submesh from definition.
    Submesh* submesh = mesh->AddSubmesh(meshDefinition);

    // Render it in "lines" mode, since this is a set of lines!
    submesh->SetRenderMode(RenderMode::Triangles);
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
    #if defined(NEW_SHAPE_RENDERING)
    GenerateMesh(shapes, mesh, Math::Max(GetUIScale() * 0.5f, 2.0f));
    #else
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
    #endif
}