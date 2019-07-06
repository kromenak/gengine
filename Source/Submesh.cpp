//
// Submesh.cpp
//
// Clark Kromenaker
//
#include "Submesh.h"

#include "GLVertexArray.h"

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
	mVertexArray->SetIndexes(indexes, count);
}

