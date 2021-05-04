//
// Submesh.cpp
//
// Clark Kromenaker
//
#include "Submesh.h"

#include "Collisions.h"
#include "Ray.h"

Submesh::Submesh(const MeshDefinition& meshDefinition) :
    mVertexCount(meshDefinition.vertexCount),
    mIndexCount(meshDefinition.indexCount),
    mVertexArray(meshDefinition)
{
    
}

Submesh::~Submesh()
{
	// Delete vertex data.
	delete[] mPositions;
	delete[] mColors;
	delete[] mNormals;
	delete[] mUV1;
	
	// Delete indexes.
	delete[] mIndexes;
}

void Submesh::Render() const
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
    }
}

void Submesh::Render(unsigned int offset, unsigned int count) const
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
	}
}

Vector3 Submesh::GetVertexPosition(int index) const
{
	// Handle error cases.
	if(mPositions == nullptr) { return Vector3::Zero; }
	if(index < 0 || index >= mVertexCount) { return Vector3::Zero; }
	
	int offset = index * 3;
	return Vector3(mPositions[offset], mPositions[offset + 1], mPositions[offset + 2]);
}

Vector3 Submesh::GetVertexNormal(int index) const
{
    int offset = index * 3;
    return Vector3(mNormals[offset], mNormals[offset + 1], mNormals[offset + 2]);
}

int Submesh::GetTriangleCount() const
{
	if(mRenderMode == RenderMode::Triangles)
	{
		return mIndexes != nullptr ? (mIndexCount / 3) : (mVertexCount / 3);
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

bool Submesh::Raycast(const Ray& ray)
{
	if(mRenderMode != RenderMode::Triangles || mIndexes == nullptr)
	{
		std::cout << "Submesh::Raycast only supports Triangle meshes with indexes for now - aborting." << std::endl;
		return false;
	}
	
    RaycastHit hitInfo;
	for(int i = 0; i < mIndexCount; i += 3)
	{
		Vector3 vert1 = GetVertexPosition(mIndexes[i]);
		Vector3 vert2 = GetVertexPosition(mIndexes[i + 1]);
		Vector3 vert3 = GetVertexPosition(mIndexes[i + 2]);
		
		if(Collisions::TestRayTriangle(ray, vert1, vert2, vert3, hitInfo))
		{
			return true;
		}
	}
	
	// Ray did not hit any triangles.
	return false;
}

void Submesh::SetPositions(float* positions, bool createCopy)
{
    // Size of array is assumed to be correct based on vertex count.
    if(createCopy)
    {
        if(mPositions == nullptr)
        {
            mPositions = new float[mVertexCount * 3];
        }
        memcpy(mPositions, positions, mVertexCount * 3 * sizeof(float));
    }
    else
    {
        mPositions = positions;
    }
    mVertexArray.ChangeVertexData(VertexAttribute::Semantic::Position, mPositions);
}

void Submesh::SetColors(float* colors, bool createCopy)
{
    // Size of array is assumed to be correct based on vertex count.
    if(createCopy)
    {
        if(mColors == nullptr)
        {
            mColors = new float[mVertexCount * 4];
        }
        memcpy(mColors, colors, mVertexCount * 4 * sizeof(float));
    }
    else
    {
        mColors = colors;
    }
    mVertexArray.ChangeVertexData(VertexAttribute::Semantic::Color, mColors);
}

void Submesh::SetNormals(float* normals, bool createCopy)
{
    // Size of array is assumed to be correct based on vertex count.
    if(createCopy)
    {
        if(mNormals == nullptr)
        {
            mNormals = new float[mVertexCount * 3];
        }
        memcpy(mNormals, normals, mVertexCount * 3 * sizeof(float));
    }
    else
    {
        mNormals = normals;
    }
    mVertexArray.ChangeVertexData(VertexAttribute::Semantic::Normal, mNormals);
}

void Submesh::SetUV1s(float* uvs, bool createCopy)
{
    // Size of array is assumed to be correct based on vertex count.
    if(createCopy)
    {
        if(mUV1 == nullptr)
        {
            mUV1 = new float[mVertexCount * 2];
        }
        memcpy(mUV1, uvs, mVertexCount * 3 * sizeof(float));
    }
    else
    {
        mUV1 = uvs;
    }
    mVertexArray.ChangeVertexData(VertexAttribute::Semantic::UV1, mUV1);
}

void Submesh::SetIndexes(unsigned short* indexes, bool createCopy)
{
    // Size of array is assumed to be correct based on vertex count.
    if(createCopy)
    {
        if(mIndexes == nullptr)
        {
            mIndexes = new unsigned short[mIndexCount * 2];
        }
        memcpy(mIndexes, indexes, mIndexCount * 2 * sizeof(unsigned short));
    }
    else
    {
        mIndexes = indexes;
    }
    mVertexArray.ChangeIndexData(mIndexes, mIndexCount);
}
