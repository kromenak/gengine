//
// Submesh.h
//
// Clark Kromenaker
//
// Actual 3D mesh data in a particular format that is fit to be rendered.
// Vertex data - positions, colors, UVs, etc.
//
#pragma once

#include <string>

#include "Vector3.h"

class GLVertexArray;
class Ray;

enum class RenderMode
{
	Triangles,			// Every 3 vertices is one triangle.
	TriangleStrip,		// A triangle is the current vertex and previous two vertices.
	TriangleFan,		// A triangle is the first vertex and subsequent groups of two vertices.
	Lines				// Every 2 vertices is one line.
};

enum class MeshUsage
{
	Static,
	Dynamic
};

//TODO: Figure out how to best define this stuff (semantic/type/format).
enum class VertexAttributeSemantic
{
	Position,
	Color,
	Normal,
	UV
};

enum class VertexAttributeType
{
	Float,
	Float2,
	Float3,
	Float4
};

struct VertexFormat
{
	bool interleaved = false;
};

class Submesh
{
public:
	Submesh(unsigned int vertexCount, unsigned int vertexSize, MeshUsage usage);
	~Submesh();
	
	void Render() const;
	void Render(unsigned int offset, unsigned int count) const;
	
	void SetPositions(float* positions);
	void CopyPositions(float* positions);
	
	void SetColors(float* colors);
	void SetNormals(float* normals);
	void SetUV1(float* uvs);
	
	void SetIndexes(unsigned short* indexes, int count);
	
	void SetRenderMode(RenderMode mode) { mRenderMode = mode; }
	
	void SetTextureName(const std::string& textureName) { mTextureName = textureName; }
	const std::string& GetTextureName() const { return mTextureName; }
	
	unsigned int GetVertexCount() const { return mVertexCount; }
	Vector3 GetVertexPosition(int index) const;
	
	int GetIndexCount() const { return mIndexCount; }
	unsigned short* GetIndexes() { return mIndexes; }
	
	int GetTriangleCount() const;
	bool GetTriangle(int index, Vector3& p0, Vector3& p1, Vector3& p2) const;
	
	bool Raycast(const Ray& ray);
	
	float* GetPositions() { return mPositions; }
	float* GetColors() { return mColors; }
	float* GetNormals() { return mNormals; }
	float* GetUV1s() { return mUV1; }
	
private:
	// Number of vertices in the mesh.
	unsigned int mVertexCount = 0;
	
	// Vertex data. The submesh owns this data.
	float* mPositions = nullptr;
	float* mColors = nullptr;
	float* mNormals = nullptr;
	float* mUV1 = nullptr;
	
	// Index data. If present, the submesh draws using indexed methods.
	// The submesh owns this data.
	unsigned int mIndexCount = 0;
	unsigned short* mIndexes = nullptr;
	
	// Name of the default texture to use for this submesh.
	std::string mTextureName;
	
	// Uniform color value applied to this mesh.
	// ???
	
	// Indicates how this mesh is rendered.
	// Dictates what rendering command we use in the underlying rendering system.
	RenderMode mRenderMode = RenderMode::Triangles;
	
	// Vertex array object that actually renders using the underlying rendering system.
	GLVertexArray* mVertexArray = nullptr;
};
