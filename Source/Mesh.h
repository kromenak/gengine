//
// Mesh.h
//
// Clark Kromenaker
//
// Represents a renderable piece of 3D geometry.
// Vertex data - positions, colors, UVs, etc.
//
#pragma once
#include <GL/glew.h>

#include "Matrix4.h"
#include "Quaternion.h"
#include "Vector3.h"

class GLVertexArray;

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

class Mesh
{
public:
    Mesh(unsigned int vertexCount, unsigned int vertexSize, MeshUsage usage);
    ~Mesh();
    
    void SetPositions(float* positions);
    void SetColors(float* colors);
    void SetNormals(float* normals);
    void SetUV1(float* uvs);
    
    void SetIndexes(unsigned short* indexes, int count);
    
    void SetRenderMode(RenderMode mode) { mRenderMode = mode; }
    
    void Render();
    void Render(unsigned int offset, unsigned int count);
    
    void SetLocalTransformMatrix(const Matrix4& mat) { mLocalTransformMatrix = mat; }
    Matrix4& GetLocalTransformMatrix() { return mLocalTransformMatrix; }
	
    void SetTextureName(std::string textureName) { mTextureName = textureName; }
    std::string GetTextureName() { return mTextureName; }
    
	uint GetVertexCount() { return mVertexCount; }
    
private:
    // Number of vertices in the mesh.
    unsigned int mVertexCount = 0;
    
    // Indicates how this mesh is rendered.
    RenderMode mRenderMode = RenderMode::Triangles;
    
    // Matrix containing local translation/rotation.
    Matrix4 mLocalTransformMatrix;
	
    // Uniform color value applied to this mesh.
    // ???
    
    // Name of the texture to use for this mesh.
    std::string mTextureName;
    
    // Vertex data. This object owns this data.
    float* mPositions = nullptr;
    float* mColors = nullptr;
    float* mNormals = nullptr;
    float* mUV1 = nullptr;
    
    // Index data. This object owns this data.
    unsigned short* mIndexes = nullptr;
    
    // Vertex array object that actually renders the thing.
    GLVertexArray* mVertexArray = nullptr;
};
