#include "Submesh.h"

#include "Collisions.h"
#include "Ray.h"

Submesh::Submesh(const MeshDefinition& meshDefinition) :
    mIndexes(meshDefinition.indexData),
    mVertexArray(meshDefinition)
{
    // Cache pointers to mesh data needed for submesh calculations.
    if(meshDefinition.vertexDefinition.layout == VertexLayout::Packed)
    {
        mPositions = meshDefinition.GetVertexData<float>(VertexAttribute::Position);
        mColors = meshDefinition.GetVertexData<float>(VertexAttribute::Color);
        mNormals = meshDefinition.GetVertexData<float>(VertexAttribute::Normal);
        mUV1 = meshDefinition.GetVertexData<float>(VertexAttribute::UV1);
    }
}

void Submesh::Render()
{
    switch(mRenderMode)
    {
    default:
    case RenderMode::Triangles:
        mVertexArray.DrawTriangles();
        break;
    case RenderMode::TriangleFan:
        mVertexArray.DrawTriangleFans();
        break;
    case RenderMode::Lines:
        mVertexArray.DrawLines();
        break;
    case RenderMode::LineLoop:
        mVertexArray.DrawLineLoop();
        break;
    case RenderMode::Points:
        mVertexArray.DrawPoints();
        break;
    }
}

void Submesh::Render(unsigned int offset, unsigned int count)
{
    switch(mRenderMode)
    {
    default:
    case RenderMode::Triangles:
        mVertexArray.DrawTriangles(offset, count);
        break;
    case RenderMode::TriangleFan:
        mVertexArray.DrawTriangleFans(offset, count);
        break;
    case RenderMode::Lines:
        mVertexArray.DrawLines(offset, count);
        break;
    case RenderMode::LineLoop:
        mVertexArray.DrawLineLoop(offset, count);
        break;
    case RenderMode::Points:
        mVertexArray.DrawPoints(offset, count);
        break;
    }
}

Vector3 Submesh::GetVertexPosition(int index) const
{
    if(mPositions == nullptr) { return Vector3::Zero; }
    if(index < 0 || index >= mVertexArray.GetVertexCount()) { return Vector3::Zero; }

    int offset = index * 3;
    return Vector3(mPositions[offset], mPositions[offset + 1], mPositions[offset + 2]);
}

void Submesh::SetVertexPosition(int index, const Vector3& position)
{
    if(mPositions != nullptr && index >= 0 && index < mVertexArray.GetVertexCount())
    {
        int offset = index * 3;
        mPositions[offset] = position.x;
        mPositions[offset + 1] = position.y;
        mPositions[offset + 2] = position.z;
    }
}

Vector3 Submesh::GetVertexNormal(int index) const
{
    if(mNormals == nullptr) { return Vector3::Zero; }
    if(index < 0 || index >= mVertexArray.GetVertexCount()) { return Vector3::Zero; }

    int offset = index * 3;
    return Vector3(mNormals[offset], mNormals[offset + 1], mNormals[offset + 2]);
}

Vector2 Submesh::GetVertexUV(int index) const
{
    if(mUV1 == nullptr) { return Vector2::Zero; }
    if(index < 0 || index >= mVertexArray.GetVertexCount()) { return Vector2::Zero; }

    int offset = index * 2;
    return Vector2(mUV1[offset], mUV1[offset + 1]);
}

int Submesh::GetTriangleCount() const
{
    if(mRenderMode == RenderMode::Triangles)
    {
        return mIndexes != nullptr ? (mVertexArray.GetIndexCount() / 3) : (mVertexArray.GetVertexCount() / 3);
    }
    //TODO: Add support for TriangleFan/TriangleStrip modes.

    // Can't compute triangle count!
    return 0;
}

bool Submesh::GetTriangle(int index, Vector3& p0, Vector3& p1, Vector3& p2) const
{
    if(mRenderMode == RenderMode::Triangles)
    {
        if(mIndexes != nullptr)
        {
            int offset = index * 3;
            p0 = GetVertexPosition(mIndexes[offset]);
            p1 = GetVertexPosition(mIndexes[offset + 1]);
            p2 = GetVertexPosition(mIndexes[offset + 2]);
            return true;
        }
        //TODO: Add support if no indexes present
    }
    //TODO: Add support for triangle fans/strips.

    return false;
}

bool Submesh::Raycast(const Ray& ray, float& outRayT, Vector2& outUV)
{
    // Ensure out value is at default.
    outRayT = FLT_MAX;

    // This function doesn't support certain submesh configurations yet.
    if(mRenderMode != RenderMode::Triangles)
    {
        std::cout << "Submesh::Raycast only supports 'Triangles' RenderMode - aborting." << std::endl;
        return false;
    }

    // Check whether the ray hits any triangles in this submesh.
    // Even if hit occurs, can't early out! Must check all triangles in case a closer one (lower t value) is found.
    int elementCount = mIndexes != nullptr ? mVertexArray.GetIndexCount() : mVertexArray.GetVertexCount();
    Vector3 vert1;
    Vector3 vert2;
    Vector3 vert3;
    for(uint32_t i = 0; i < elementCount; i += 3)
    {
        if(mIndexes != nullptr)
        {
            vert1 = GetVertexPosition(mIndexes[i]);
            vert2 = GetVertexPosition(mIndexes[i + 1]);
            vert3 = GetVertexPosition(mIndexes[i + 2]);
        }
        else
        {
            vert1 = GetVertexPosition(i);
            vert2 = GetVertexPosition(i + 1);
            vert3 = GetVertexPosition(i + 2);
        }

        float t = FLT_MAX;
        float u = 0.0f;
        float v = 0.0f;
        if(Intersect::TestRayTriangle(ray, vert1, vert2, vert3, t, u, v))
        {
            if(t < outRayT)
            {
                outRayT = t;

                // The calling code sometimes needs to know the UV coordinate where the ray hit.
                // First, get the three UVs that correspond to these three triangle vertices.
                //TODO: There is similar code to this here, in skybox, and in BSP. Probably they can be consolidated!
                Vector2 uv0;
                Vector2 uv1;
                Vector2 uv2;
                if(mIndexes != nullptr)
                {
                    uv0 = GetVertexUV(mIndexes[i]);
                    uv1 = GetVertexUV(mIndexes[i + 1]);
                    uv2 = GetVertexUV(mIndexes[i + 2]);
                }
                else
                {
                    uv0 = GetVertexUV(i);
                    uv1 = GetVertexUV(i + 1);
                    uv2 = GetVertexUV(i + 2);
                }

                // Calculate the point UV.
                //TODO: This math doesn't totally make sense to me, and I think it needs more scrutinizing.
                //TODO: Why do u/v/w not correlate to uv0/uv1/uv2 here? Why do we need to negate and flop the UVs?
                Vector2 pointUV = uv1 * u + uv2 * v + uv0 * (1.0f - u - v);
                pointUV.y *= -1.0f;
                pointUV.y = 1.0f - pointUV.y;
                outUV = pointUV;
            }
        }
    }

    // Return whether we hit anything.
    return outRayT < FLT_MAX;
}

void Submesh::SetPositions(float* positions)
{
    mVertexArray.ChangeVertexData(VertexAttribute::Semantic::Position, positions);
}

void Submesh::SetNormals(float* normals)
{
    mVertexArray.ChangeVertexData(VertexAttribute::Semantic::Normal, normals);
}

void Submesh::SetColors(float* colors)
{
    mVertexArray.ChangeVertexData(VertexAttribute::Semantic::Color, colors);
}

void Submesh::SetUV1s(float* uvs)
{
    mVertexArray.ChangeVertexData(VertexAttribute::Semantic::UV1, uvs);
}

void Submesh::SetIndexes(unsigned short* indexes)
{
    mVertexArray.ChangeIndexData(indexes);
}
