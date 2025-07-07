#include "UIHexagrams.h"

#include "Mesh.h"

TYPEINFO_INIT(UIHexagrams, UIShapes<UIHexagram>, 34)
{

}

UIHexagrams::UIHexagrams(Actor* owner) : UIShapes<UIHexagram>(owner)
{

}

void UIHexagrams::Add(const Vector2& center, float radius, float angle)
{
    UIHexagram hexagram;
    hexagram.center = center;
    hexagram.radius = radius;
    hexagram.angle = angle;
    UIShapes<UIHexagram>::Add(hexagram);
}

void UIHexagrams::GenerateMesh(const std::vector<UIHexagram>& shapes, Mesh* mesh)
{
    // Each hexagram consists of 12 vertices.
    // This is because we need to double up each vertex since we're rendering with Lines approach.
    size_t vertexCount = shapes.size() * 12;
    float* positions = new float[vertexCount * 3];
    for(int i = 0; i < shapes.size(); ++i)
    {
        Matrix3 rotMat = Matrix3::MakeRotateZ(shapes[i].angle);
        const float kAngleInterval = Math::k2Pi / 6;
        Vector2 points[6];
        for(int j = 0; j < 6; ++j)
        {
            float angle = j * kAngleInterval;

            points[j].x = shapes[i].radius * Math::Sin(angle);
            points[j].y = shapes[i].radius * Math::Cos(angle);
            points[j] = rotMat.TransformVector(points[j]);
            points[j] += shapes[i].center;
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
    Submesh* submesh = mesh->AddSubmesh(meshDefinition);

    // Render it in "lines" mode, since this is a set of lines!
    submesh->SetRenderMode(RenderMode::Lines);
}