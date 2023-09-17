#include "VertexArray.h"

#include <cstring>
#include <iostream>

#include "ThreadUtil.h"

VertexArray::VertexArray(const MeshDefinition& data) :
    mData(data)
{

}

VertexArray::~VertexArray()
{
    // Delete data if owned.
    if(mData.ownsData)
    {
        // Delete vertex data.
        mData.ClearVertexData();
        
        // Delete index data.
        delete[] mData.indexData;
    }

    // Destroy GPU resources.
    BufferHandle vb = mVertexBuffer;
    BufferHandle ib = mIndexBuffer;
    ThreadUtil::RunOnMainThread([vb, ib]() {
        GAPI::Get()->DestroyVertexBuffer(vb);
        GAPI::Get()->DestroyIndexBuffer(ib);
    });
}

VertexArray::VertexArray(VertexArray&& other) noexcept
{
    *this = std::move(other);
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    mData = other.mData;
    mVertexBuffer = other.mVertexBuffer;
    mIndexBuffer = other.mIndexBuffer;

    other.mData = MeshDefinition();
    other.mVertexBuffer = nullptr;
    other.mIndexBuffer = nullptr;
    return *this;
}

void VertexArray::DrawTriangles()
{
    DrawTriangles(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangles(uint32_t offset, uint32_t count)
{
    Draw(GAPI::Primitive::Triangles, offset, count);
}

void VertexArray::DrawTriangleStrips()
{
    DrawTriangleStrips(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangleStrips(uint32_t offset, uint32_t count)
{
    Draw(GAPI::Primitive::TriangleStrip, offset, count);
}

void VertexArray::DrawTriangleFans()
{
    DrawTriangleFans(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangleFans(uint32_t offset, uint32_t count)
{
    Draw(GAPI::Primitive::TriangleFan, offset, count);
}

void VertexArray::DrawLines()
{
    DrawLines(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawLines(uint32_t offset, uint32_t count)
{
    Draw(GAPI::Primitive::Lines, offset, count);
}

void VertexArray::Draw(GAPI::Primitive mode)
{
    Draw(mode, 0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::Draw(GAPI::Primitive mode, uint32_t offset, uint32_t count)
{
    // Make sure vertex buffer and index buffer are ready to go.
    CreateVertexBuffer();
    CreateIndexBuffer();

    // Draw the thing!
    if(mIndexBuffer != nullptr)
    {
        GAPI::Get()->Draw(mode, mVertexBuffer, mIndexBuffer, offset, count);
    }
    else
    {
        GAPI::Get()->Draw(mode, mVertexBuffer, offset, count);
    }
}

void VertexArray::ChangeVertexData(void* data)
{
    // Save data locally.
    uint32_t size = mData.vertexCount * mData.vertexDefinition.CalculateSize();
    memcpy(mData.vertexData[0], data, size);

    // Send to GPU if buffer already exists.
    // Otherwise, it'll get sent when the vertex buffer is created.
    if(mVertexBuffer != nullptr)
    {
        // Assuming that the data is the correct size to fill the entire buffer.
        GAPI::Get()->SetVertexBufferData(mVertexBuffer, 0, size, data);
    }
}

void VertexArray::ChangeVertexData(VertexAttribute::Semantic semantic, void* data)
{
    // We can really only update a single attribute's data if attribute data is tightly packed.
    // If data is interleaved, we'll just fall back on overwriting all data.
    if(mData.vertexDefinition.layout == VertexLayout::Interleaved)
    {
        printf("WARNING: You can only update an individual vertex attribute's data when using non-interleaved data!\n");
        ChangeVertexData(data);
        return;
    }

    // For tightly packed data, we can determine the "sub data" and update just a portion.
    int offset = 0;
    for(size_t i = 0; i < mData.vertexDefinition.attributes.size(); ++i)
    {
        VertexAttribute& attribute = mData.vertexDefinition.attributes[i];

        // Determine size of this attribute's data.
        GLsizeiptr attributeSize = mData.vertexCount * attribute.GetSize();

        // Update sub-data, if semantic matches.
        if(attribute.semantic == semantic)
        {
            // Save data locally.
            memcpy(mData.vertexData[i], data, attributeSize);

            // Send to GPU if buffer already exists.
            // Otherwise, it'll get sent when the vertex buffer is created.
            if(mVertexBuffer != nullptr)
            {
                GAPI::Get()->SetVertexBufferData(mVertexBuffer, offset, attributeSize, data);
            }
            return;
        }

        // Next attribute's offset is calculated by adding this attribute's size.
        offset += attributeSize;
    }
}

void VertexArray::ChangeIndexData(uint16_t* indexes)
{
    // Just assume index count has not changed.
    ChangeIndexData(indexes, mData.indexCount);
}

void VertexArray::ChangeIndexData(uint16_t* indexes, uint32_t count)
{
    // If existing index buffer size doesn't match, we need to delete the old one.
    // It'll get recreated (with correct size) later on.
    if(mIndexBuffer != nullptr && mData.indexCount != count)
    {
        GAPI::Get()->DestroyIndexBuffer(mIndexBuffer);
        mIndexBuffer = nullptr;
    }

    // If our stored index data size doesn't match, we also need to delete this.
    if(mData.indexData != nullptr && mData.indexCount != count)
    {
        assert(mData.ownsData);
        delete[] mData.indexData;
        mData.indexData = nullptr;
    }

    // If index data is null, we need to create the memory.
    if(mData.indexData == nullptr)
    {
        mData.indexData = new unsigned short[count];
    }

    // Update index count and index data.
    mData.indexCount = count;
    memcpy(mData.indexData, indexes, count * sizeof(uint16_t));
    
    // If the index buffer exists, update the data in it.
    if(mIndexBuffer != nullptr)
    {
        GAPI::Get()->SetIndexBufferData(mIndexBuffer, mData.indexCount, mData.indexData);
    }
}

void VertexArray::CreateVertexBuffer()
{
    // Already got one? Don't need to create another one.
    if(mVertexBuffer != nullptr)
    {
        return;
    }

    // The way we create the vertex buffer depends on the layout of the data we will insert into the buffer.
    if(mData.vertexDefinition.layout == VertexLayout::Packed)
    {
        // With packed data, each vertex attribute has its own separate array of data.
        // So we can't set the data at the same time we create the buffer.
        mVertexBuffer = GAPI::Get()->CreateVertexBuffer(mData.vertexCount, mData.vertexDefinition, nullptr, mData.meshUsage);

        // We need to set the data in the vertex buffer separately for each attribute.
        // We assume attributes are specified in same order data is provided in.
        uint32_t offset = 0;
        size_t attributeIndex = 0;
        for(auto& attribute : mData.vertexDefinition.attributes)
        {
            // Determine size of this attribute's data.
            uint32_t attributeSize = mData.vertexCount * attribute.GetSize();
            GAPI::Get()->SetVertexBufferData(mVertexBuffer, offset, attributeSize, mData.vertexData[attributeIndex]);

            // Next attribute's offset is calculated by adding this attribute's size.
            offset += attributeSize;
            ++attributeIndex;
        }
    }
    else // Interleaved vertex layout.
    {
        // With interleaved data, we just have one big array of vertex data.
        // So we can create the buffer and set it's data in one command. 
        mVertexBuffer = GAPI::Get()->CreateVertexBuffer(mData.vertexCount, mData.vertexDefinition, mData.vertexData[0], mData.meshUsage);
    }
}

void VertexArray::CreateIndexBuffer()
{
    // Already got one? Don't need to create another one.
    if(mIndexBuffer != nullptr)
    {
        return;
    }

    // No need if index data is empty or count is zero.
    if(mData.indexData == nullptr || mData.indexCount <= 0)
    {
        return;
    }

    // Create the index buffer.
    mIndexBuffer = GAPI::Get()->CreateIndexBuffer(mData.indexCount, mData.indexData, mData.meshUsage);
}
