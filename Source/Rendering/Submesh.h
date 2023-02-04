//
// Clark Kromenaker
//
// A submesh represents a single piece of renderable geometry.
// For the most part, it is a wrapper around a VertexArray, with some extra functionality (raycasting, data querying, etc).
//
#pragma once
#include <string>

#include "Vector3.h"
#include "VertexArray.h"

struct RaycastHit;
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
	
    void SetRenderMode(RenderMode mode) { mRenderMode = mode; }
    
	void Render();
	void Render(unsigned int offset, unsigned int count);
	
	unsigned int GetVertexCount() const { return mVertexArray.GetVertexCount(); }
	Vector3 GetVertexPosition(int index) const;
    Vector3 GetVertexNormal(int index) const;
	
	int GetTriangleCount() const;
	bool GetTriangle(int index, Vector3& p0, Vector3& p1, Vector3& p2) const;
	
	bool Raycast(const Ray& ray, RaycastHit& hitInfo);
    
    void SetPositions(float* positions);
    float* GetPositions() { return mPositions; }
    
    void SetNormals(float* normals);
    float* GetNormals() { return mNormals; }
    
    void SetColors(float* colors);
    float* GetColors() { return mColors; }
    
    void SetUV1s(float* uvs);
    float* GetUV1s() { return mUV1; }
    
    void SetIndexes(unsigned short* indexes);
    int GetIndexCount() const { return mVertexArray.GetIndexCount(); }
    unsigned short* GetIndexes() { return mIndexes; }
    
    // Kind of a weird thing where GK3 submeshes hold the texture name.
    // Might make sense to move this to like a subclass or something?
    void SetTextureName(const std::string& textureName) { mTextureName = textureName; }
    const std::string& GetTextureName() const { return mTextureName; }
	
private:
    // Indicates how this mesh is rendered.
    // Dictates what rendering command we use in the underlying rendering system.
    RenderMode mRenderMode = RenderMode::Triangles;
	
	// Pointers to mesh data.
    // This data is owned by VertexArray, but efficient to cache pointers here for raycasting/querying.
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
