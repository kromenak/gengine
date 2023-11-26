#include "UIGrids.h"

#include "Matrix3.h"
#include "Mesh.h"

TYPE_DEF_CHILD(UIWidget, UIGrids);

UIGrids::UIGrids(Actor* owner) : UIWidget(owner)
{

}

UIGrids::~UIGrids()
{
    delete mMesh;
}

void UIGrids::Render()
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

void UIGrids::SetColor(const Color32& color)
{
    mMaterial.SetColor(color);
}

void UIGrids::Add(const Vector2& center, const Vector2& size, float angle, int divisions, bool drawBorder)
{
    mGrids.emplace_back();
    mGrids.back().center = center;
    mGrids.back().size = size;
    mGrids.back().angle = angle;
    mGrids.back().divisions = divisions;
    mGrids.back().drawBorder = drawBorder;
    mNeedMeshRegen = true;
}

void UIGrids::Clear()
{
    mGrids.clear();
    mNeedMeshRegen = true;
}

void UIGrids::GenerateMesh()
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

    // We don't need a mesh if we have no grids.
    if(mGrids.empty()) { return; }

    // Create new mesh.
    mMesh = new Mesh();

    // Calculating then number of vertices is a bit more involved - it depends on grid divisions and other options.
    size_t vertexCount = 0;
    for(auto& grid : mGrids)
    {
        // A 1x1 grid needs no lines...
        // Each size of grid after that requires 2 more lines to be added (vertical and horizontal).
        // 2x2 = 2 lines, 4 verts
        // 3x3 = 4 lines, 8 verts
        // And so on...

        // We need two lines per division, and 2 vertices per line.
        vertexCount += (grid.divisions - 1) * 2 * 2;
        
        // If we're drawing the border, we need 8 vertices (4 lines with 2 vertices each).
        if(grid.drawBorder)
        {
            vertexCount += 8;
        }
    }

    // Generate the grid vertex positions.
    size_t posOffset = 0;
    float* positions = new float[vertexCount * 3];
    for(auto& grid : mGrids)
    {
        float halfWidth = grid.size.x * 0.5f;
        float halfHeight = grid.size.y * 0.5f;

        Matrix3 rotMat = Matrix3::MakeRotateZ(grid.angle);
        Vector2 topLeftDir = rotMat.TransformVector(Vector2(-halfWidth, halfHeight));
        Vector2 topRightDir = rotMat.TransformVector(Vector2(halfWidth, halfHeight));
        Vector2 botLeftDir = rotMat.TransformVector(Vector2(-halfWidth, -halfHeight));
        Vector2 botRightDir = rotMat.TransformVector(Vector2(halfWidth, -halfHeight));

        Vector2 topLeft = grid.center + topLeftDir;
        Vector2 topRight = grid.center + topRightDir;
        Vector2 bottomLeft = grid.center + botLeftDir;
        Vector2 bottomRight = grid.center + botRightDir;

        if(grid.drawBorder)
        {
            // Left Side
            positions[posOffset + 0] = topLeft.x;
            positions[posOffset + 1] = topLeft.y;
            positions[posOffset + 2] = 0.0f;
            positions[posOffset + 3] = bottomLeft.x;
            positions[posOffset + 4] = bottomLeft.y;
            positions[posOffset + 5] = 0.0f;

            // Bottom Side
            positions[posOffset + 6] = bottomLeft.x;
            positions[posOffset + 7] = bottomLeft.y;
            positions[posOffset + 8] = 0.0f;
            positions[posOffset + 9] = bottomRight.x;
            positions[posOffset + 10] = bottomRight.y;
            positions[posOffset + 11] = 0.0f;

            // Right Side
            positions[posOffset + 12] = bottomRight.x;
            positions[posOffset + 13] = bottomRight.y;
            positions[posOffset + 14] = 0.0f;
            positions[posOffset + 15] = topRight.x;
            positions[posOffset + 16] = topRight.y;
            positions[posOffset + 17] = 0.0f;

            // Top Side
            positions[posOffset + 18] = topRight.x;
            positions[posOffset + 19] = topRight.y;
            positions[posOffset + 20] = 0.0f;
            positions[posOffset + 21] = topLeft.x;
            positions[posOffset + 22] = topLeft.y;
            positions[posOffset + 23] = 0.0f;
            posOffset += 24;
        }

        if(grid.divisions > 1)
        {
            Vector2 tlToTr = Vector2::Normalize(topRight - topLeft);
            Vector2 tlToBl = Vector2::Normalize(bottomLeft - topLeft);
            float intervalX = grid.size.x / grid.divisions;
            float intervalY = grid.size.y / grid.divisions;

            Vector2 currentTop = topLeft;
            Vector2 currentBot = bottomLeft;
            Vector2 currentLeft = topLeft;
            Vector2 currentRight = topRight;
            for(int i = 1; i < grid.divisions; ++i)
            {
                // Increment current values to next grid line spots.
                currentTop += (tlToTr * intervalX);
                currentBot += (tlToTr * intervalX);
                currentLeft += (tlToBl * intervalY);
                currentRight += (tlToBl * intervalY);

                // Vertical line.
                positions[posOffset + 0] = currentTop.x;
                positions[posOffset + 1] = currentTop.y;
                positions[posOffset + 2] = 0.0f;

                positions[posOffset + 3] = currentBot.x;
                positions[posOffset + 4] = currentBot.y;
                positions[posOffset + 5] = 0.0f;

                // Horizontal line.
                positions[posOffset + 6] = currentLeft.x;
                positions[posOffset + 7] = currentLeft.y;
                positions[posOffset + 8] = 0.0f;

                positions[posOffset + 9] = currentRight.x;
                positions[posOffset + 10] = currentRight.y;
                positions[posOffset + 11] = 0.0f;

                // Increment offset.
                posOffset += 12;
            }
        }
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