//
// Submesh.cpp
//
// Clark Kromenaker
//
#include "Submesh.h"

#include "Collisions.h"
#include "GLVertexArray.h"
#include "Ray.h"

Submesh::Submesh(unsigned int vertexCount, unsigned int vertexSize, MeshUsage usage) : mVertexCount(vertexCount)
{
	mVertexArray = new GLVertexArray(vertexCount, vertexSize, usage);
}

Submesh::~Submesh()
{
	// Delete vertex attribute data.
	delete[] mPositions;
	delete[] mColors;
	delete[] mNormals;
	delete[] mUV1;
	
	// Delete indexes.
	delete[] mIndexes;
	
	delete mVertexArray;
}

void Submesh::Render() const
{
	switch(mRenderMode)
	{
    default:
    case RenderMode::Triangles:
        mVertexArray->DrawTriangles();
        break;
    case RenderMode::TriangleFan:
        mVertexArray->DrawTriangleFans();
        break;
    case RenderMode::Lines:
        mVertexArray->DrawLines();
        break;
    }
}

void Submesh::Render(unsigned int offset, unsigned int count) const
{
	switch(mRenderMode)
	{
    default:
    case RenderMode::Triangles:
        mVertexArray->DrawTriangles(offset, count);
        break;
    case RenderMode::TriangleFan:
        mVertexArray->DrawTriangleFans(offset, count);
        break;
    case RenderMode::Lines:
        mVertexArray->DrawLines(offset, count);
        break;
	}
}

void Submesh::SetPositions(float* positions)
{
	// For now, we'll assume the caller passes in vertex data for us to own.
	// In the future, maybe it'll be necessary for the caller to specify this somehow.
	mPositions = positions;
	mVertexArray->SetPositions(mPositions);
}

void Submesh::CopyPositions(float* positions)
{
	// We're assuming the positions passed in are the right size.
	// Hopefully the caller can guarantee that!
	if(mPositions == nullptr)
	{
		mPositions = new float[mVertexCount * 3];
	}
	memcpy(mPositions, positions, mVertexCount * 3 * sizeof(float));
	
	// Need to "re-set" the positions, to flag a change on the GPU.
	mVertexArray->SetPositions(mPositions);
}

void Submesh::SetColors(float* colors)
{
	mColors = colors;
	mVertexArray->SetColors(colors);
}

void Submesh::SetNormals(float* normals)
{
	mNormals = normals;
	mVertexArray->SetNormals(normals);
}

void Submesh::SetUV1(float* uvs)
{
	mUV1 = uvs;
	mVertexArray->SetUV1(uvs);
}

void Submesh::SetIndexes(unsigned short* indexes, int count)
{
	mIndexes = indexes;
	mIndexCount = count;
	mVertexArray->SetIndexes(indexes, count);
}

Vector3 Submesh::GetVertexPosition(int index) const
{
	// Handle error cases.
	if(mPositions == nullptr) { return Vector3::Zero; }
	if(index < 0 || index >= mVertexCount) { return Vector3::Zero; }
	
	int startOffset = index * 3;
	return Vector3(mPositions[startOffset], mPositions[startOffset + 1], mPositions[startOffset + 2]);
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

bool Submesh::GetNormal(int index, Vector3& n) const
{
	if(mRenderMode == RenderMode::Triangles)
	{
		int offset = index * 3;
		n.x = mNormals[offset];
		n.y = mNormals[offset + 1];
		n.z = mNormals[offset + 2];
		return true;
	}
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
