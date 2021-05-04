//
// Submesh.h
//
// Clark Kromenaker
//
// Whereas a vertex array ONLY knows how to upload to the GPU and render,
// a Submesh owns vertex data and provides some other functionality (e.g getting triangles, raycasting).
//
#pragma once
#include <string>

#include "Vector3.h"
#include "VertexArray.h"

class Ray;

enum class RenderMode
{
	Triangles,			// Every 3 vertices is one triangle.
	TriangleStrip,		// A triangle is the current vertex and previous two vertices.
	TriangleFan,		// A triangle is the first vertex and subsequent groups of two vertices.
	Lines				// Every 2 vertices is one line.
};

class Submesh
{
public:
    Submesh(const MeshDefinition& meshDefinition);
	~Submesh();
	
    void SetRenderMode(RenderMode mode) { mRenderMode = mode; }
    
	void Render() const;
	void Render(unsigned int offset, unsigned int count) const;
	
	unsigned int GetVertexCount() const { return mVertexCount; }
	Vector3 GetVertexPosition(int index) const;
    Vector3 GetVertexNormal(int index) const;
	
	int GetTriangleCount() const;
	bool GetTriangle(int index, Vector3& p0, Vector3& p1, Vector3& p2) const;
	
	bool Raycast(const Ray& ray);
    
    void SetPositions(float* positions, bool createCopy = false);
    float* GetPositions() { return mPositions; }
    
    void SetNormals(float* normals, bool createCopy = false);
    float* GetNormals() { return mNormals; }
    
    void SetColors(float* colors, bool createCopy = false);
    float* GetColors() { return mColors; }
    
    void SetUV1s(float* uvs, bool createCopy = false);
    float* GetUV1s() { return mUV1; }
    
    void SetIndexes(unsigned short* indexes, bool createCopy = false);
    int GetIndexCount() const { return mIndexCount; }
    unsigned short* GetIndexes() { return mIndexes; }
    
    // Kind of a weird thing where GK3 submeshes hold the texture name.
    // Might make sense to move this to like a subclass or something?
    void SetTextureName(const std::string& textureName) { mTextureName = textureName; }
    const std::string& GetTextureName() const { return mTextureName; }
	
private:
    // Indicates how this mesh is rendered.
    // Dictates what rendering command we use in the underlying rendering system.
    RenderMode mRenderMode = RenderMode::Triangles;
    
	// Number of vertices and indexes in the mesh.
	unsigned int mVertexCount = 0;
    unsigned int mIndexCount = 0;
	
	// Vertex data. The submesh owns this data.
	float* mPositions = nullptr;
	float* mColors = nullptr;
	float* mNormals = nullptr;
	float* mUV1 = nullptr;
    unsigned short* mIndexes = nullptr;
	
    // Vertex array that actually renders using the underlying rendering system.
    VertexArray mVertexArray;
    
	// Name of the default texture to use for this submesh.
	std::string mTextureName;
};
