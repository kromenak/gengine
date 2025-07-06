//
// Clark Kromenaker
//
// A chunk of geometry that can be rendered by the GPU.
// The lowest-level rendering object you can use.
//
// The VA creates/owns GPU resources.
//
// It also allocates and manages memory for vertex/index data.
// You can assume any data passed to VA is copied internally - so buffers do not need to be dynamically allocated.
//
#pragma once
#include "GAPI.h"
#include "MeshDefinition.h"

class VertexArray
{
public:
    VertexArray() = default;
    VertexArray(const MeshDefinition& data);
    ~VertexArray();

    // VertexArrays contain handles to GPU resources, so don't allow copying!
    VertexArray(const VertexArray& other) = delete;
    VertexArray& operator=(const VertexArray& other) = delete;

    // VertexArrays can be moved to/from.
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void DrawTriangles();
    void DrawTriangles(uint32_t offset, uint32_t count);

    void DrawTriangleStrips();
    void DrawTriangleStrips(uint32_t offset, uint32_t count);

    void DrawTriangleFans();
    void DrawTriangleFans(uint32_t offset, uint32_t count);

    void DrawLines();
    void DrawLines(uint32_t offset, uint32_t count);

    void DrawLineLoop();
    void DrawLineLoop(uint32_t offset, uint32_t count);

    void DrawPoints();
    void DrawPoints(uint32_t offset, uint32_t count);

    void Draw(GAPI::Primitive mode);
    void Draw(GAPI::Primitive mode, uint32_t offset, uint32_t count);

    unsigned int GetVertexCount() const { return mData.vertexCount; }
    unsigned int GetIndexCount() const { return mData.indexCount; }

    void ChangeVertexData(void* data);
    void ChangeVertexData(VertexAttribute::Semantic semantic, void* data);

    void ChangeIndexData(uint16_t* indexes);
    void ChangeIndexData(uint16_t* indexes, uint32_t count);

private:
    // Mesh data (vertices, normals, indexes, etc).
    // This is maintained and holds CPU-side copies of all mesh data.
    MeshDefinition mData;

    // Handle to the vertex buffer in the graphics system.
    BufferHandle mVertexBuffer = nullptr;

    // Handle to the (optional) index buffer in the graphics system.
    BufferHandle mIndexBuffer = nullptr;

    void CreateVertexBuffer();
    void CreateIndexBuffer();
};
