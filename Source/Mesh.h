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
#include "Vector3.h"
#include "Matrix4.h"
#include "Quaternion.h"

class GLVertexArray;

enum class RenderMode
{
    Triangles,
    TriangleFan,
    Lines
};

enum class MeshUsage
{
    Static,
    Dynamic
};

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
    Mesh(uint vertexCount, uint vertexSize, MeshUsage usage);
    ~Mesh();
    
    void SetPositions(const float* positions);
    void SetColors(const float* colors);
    void SetNormals(const float* normals);
    void SetUV1(const float* uvs);
    
    void SetIndexes(const ushort* indexes, int count);
    
    void SetRenderMode(RenderMode mode) { mRenderMode = mode; }
    
    void Render();
    void Render(uint offset, uint count);
    
    void SetLocalTransformMatrix(const Matrix4& mat) { mLocalTransformMatrix = mat; }
    Matrix4& GetLocalTransformMatrix() { return mLocalTransformMatrix; }
    
    void SetTextureName(std::string textureName) { mTextureName = textureName; }
    std::string GetTextureName() { return mTextureName; }
    
private:
    // Number of vertices in the mesh.
    uint mVertexCount = 0;
    
    // Indicates how this mesh is rendered.
    RenderMode mRenderMode = RenderMode::Triangles;
    
    // Matrix containing local translation/rotation.
    Matrix4 mLocalTransformMatrix;
    
    // Uniform color value applied to this mesh.
    // ???
    
    // Name of the texture to use for this mesh.
    std::string mTextureName;
    
    // Vertex data. This object owns this data.
    const float* mPositions = nullptr;
    const float* mColors = nullptr;
    const float* mNormals = nullptr;
    const float* mUV1 = nullptr;
    
    // Index data. This object owns this data.
    const ushort* mIndexes = nullptr;
    
    // Vertex array object that actually renders the thing.
    GLVertexArray* mVertexArray = nullptr;
};
