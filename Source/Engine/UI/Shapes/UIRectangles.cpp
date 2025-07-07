#include "UIRectangles.h"

#include "Matrix3.h"
#include "Mesh.h"

TYPEINFO_INIT(UIRectangles, UIShapes<UIRectangle>, 25)
{

}

UIRectangles::UIRectangles(Actor* owner) : UIShapes<UIRectangle>(owner)
{

}

void UIRectangles::Add(const Vector2& center, const Vector2& size, float angle)
{
    UIRectangle rect;
    rect.center = center;
    rect.size = size;
    rect.angle = angle;
    UIShapes<UIRectangle>::Add(rect);
}

void UIRectangles::GenerateMesh(const std::vector<UIRectangle>& shapes, Mesh* mesh)
{
    // Each rectangle consists of eight vertices.
    // This is because we need to double up each vertex since we're rendering with Lines approach.
    size_t vertexCount = shapes.size() * 8;

    float* positions = new float[vertexCount * 3];
    for(int i = 0; i < shapes.size(); ++i)
    {
        float halfWidth = shapes[i].size.x * 0.5f;
        float halfHeight = shapes[i].size.y * 0.5f;

        Matrix3 rotMat = Matrix3::MakeRotateZ(shapes[i].angle);
        Vector2 topLeftDir = rotMat.TransformVector(Vector2(-halfWidth, halfHeight));
        Vector2 topRightDir = rotMat.TransformVector(Vector2(halfWidth, halfHeight));
        Vector2 botLeftDir = rotMat.TransformVector(Vector2(-halfWidth, -halfHeight));
        Vector2 botRightDir = rotMat.TransformVector(Vector2(halfWidth, -halfHeight));

        Vector2 topLeft = shapes[i].center + topLeftDir;
        Vector2 topRight = shapes[i].center + topRightDir;
        Vector2 bottomLeft = shapes[i].center + botLeftDir;
        Vector2 bottomRight = shapes[i].center + botRightDir;

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
    Submesh* submesh = mesh->AddSubmesh(meshDefinition);

    // Render it in "lines" mode, since this is a set of lines!
    submesh->SetRenderMode(RenderMode::Lines);
}