#include "VertexArray.h"

#include <cstring>
#include <iostream>

#include "ThreadUtil.h"

// Some OpenGL calls take in array indexes/offsets as pointers.
// This macro just makes the syntax clearer for the reader.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

VertexArray::VertexArray(const MeshDefinition& data) :
    mData(data)
{

}

VertexArray::~VertexArray()
{
    // Delete data if owned.
    if(mData.ownsData)
    {
        for(auto& data : mData.vertexData)
        {
            data.DeleteArray();
        }
        mData.vertexData.clear();
        
        // Delete index data.
        delete[] mData.indexData;
    }

    GLuint vbo = mVBO;
    GLuint vao = mVAO;
    GLuint ibo = mIBO;
    ThreadUtil::RunOnMainThread([vbo, vao, ibo]() {
        // Delete GPU resources.
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ibo);
    });
}

VertexArray::VertexArray(VertexArray&& other)
{
    *this = std::move(other);
}

VertexArray& VertexArray::operator=(VertexArray&& other)
{
    mData = other.mData;
    mVBO = other.mVBO;
    mVAO = other.mVAO;
    mIBO = other.mIBO;

    other.mData = MeshDefinition();
    other.mVBO = GL_NONE;
    other.mVAO = GL_NONE;
    other.mIBO = GL_NONE;
    return *this;
}

void VertexArray::DrawTriangles()
{
    DrawTriangles(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangles(unsigned int offset, unsigned int count)
{
    Draw(GL_TRIANGLES, offset, count);
}

void VertexArray::DrawTriangleStrips()
{
    DrawTriangleStrips(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangleStrips(unsigned int offset, unsigned int count)
{
    Draw(GL_TRIANGLE_STRIP, offset, count);
}

void VertexArray::DrawTriangleFans()
{
    DrawTriangleFans(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangleFans(unsigned int offset, unsigned int count)
{
    Draw(GL_TRIANGLE_FAN, offset, count);
}

void VertexArray::DrawLines()
{
    DrawLines(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawLines(unsigned int offset, unsigned int count)
{
    Draw(GL_LINES, offset, count);
}

void VertexArray::Draw(GLenum mode)
{
    Draw(mode, 0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::Draw(GLenum mode, unsigned int offset, unsigned int count)
{
    // Create VBO/VAO/IBO if not yet created.
    if(mVBO == GL_NONE)
    {
        CreateVBO();
        CreateVAO();
        RefreshIBOContents();
    }

    // Bind vertex array object and index buffer objects.
    // Only do this if not already bound (small performance gain).
    static GLuint lastBoundVAO = GL_NONE;
    if(lastBoundVAO != mVAO)
    {
        glBindVertexArray(mVAO);
        lastBoundVAO = mVAO;

        // Assuming we always want to bind the IBO when binding VAO...
        if(mIBO != GL_NONE)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        }
    }
    
    // Draw method depends on whether we have indexes or not.
    if(mIBO != GL_NONE)
    {
        // Draw "count" indices at offset.
        glDrawElements(mode, count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(offset * sizeof(GLushort)));
    }
    else
    {
        // Draw "count" triangles at offset.
        glDrawArrays(mode, offset, count);
    }
}

void VertexArray::ChangeVertexData(void* data)
{
    // Save data locally.
    int size = mData.vertexCount * mData.vertexDefinition.CalculateSize();
    memcpy(mData.vertexData[0].data, data, size);

    // Assuming that the data is the correct size to fill the entire buffer.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

void VertexArray::ChangeVertexData(VertexAttribute::Semantic semantic, void* data)
{
    // We can really only update a single attribute's data if attribute data is tightly packed.
    // If data is interleaved, we'll just fall back on overwriting all data.
    if(mData.vertexDefinition.layout == VertexLayout::Interleaved)
    {
        std::cout << "WARNING: You can only update an individual vertex attribute's data when using non-interleaved data!" << std::endl;
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
            memcpy(mData.vertexData[i].data, data, attributeSize);

            // Send to GPU.
            glBindBuffer(GL_ARRAY_BUFFER, mVBO);
            glBufferSubData(GL_ARRAY_BUFFER, offset, attributeSize, data);
            return;
        }

        // Next attribute's offset is calculated by adding this attribute's size.
        offset += attributeSize;
    }
}

void VertexArray::ChangeIndexData(unsigned short* indexes)
{
    // Just assume index count has not changed.
    ChangeIndexData(indexes, mData.indexCount);
}

void VertexArray::ChangeIndexData(unsigned short* indexes, unsigned int count)
{
    // If changing existing buffer contents, but the count is different, we must create delete old buffer and make a new one.
    if(mIBO != GL_NONE && mData.indexCount != count)
    {
        glDeleteBuffers(1, &mIBO);
        mIBO = GL_NONE;
    }

    // Same thing with index buffer. If size changed, need to recreate it!
    if(mData.indexData != nullptr && mData.indexCount != count)
    {
        assert(mData.ownsData);
        delete[] mData.indexData;
        mData.indexData = new unsigned short[count];
    }

    // Update index count and index data.
    mData.indexCount = count;
    memcpy(mData.indexData, indexes, count * sizeof(unsigned short));

    // Refresh IBO contents with new data.
    RefreshIBOContents();
}

void VertexArray::CreateVBO()
{
    // Generate and bind VBO.
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);

    // Determine VBO usage and size.
    GLenum usage = (mData.meshUsage == MeshUsage::Static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    GLsizeiptr size = mData.vertexCount * mData.vertexDefinition.CalculateSize();

    // For packed data, we'll assume that the vertex data is a struct containing ordered pointers to each packed section.
    if(mData.vertexDefinition.layout == VertexLayout::Packed)
    {
        // Create buffer of desired size, but don't fill it with anything.
        glBufferData(GL_ARRAY_BUFFER, size, NULL, usage);

        // Iterate each attribute and load packed data for that attribute into the VBO.
        // We assume attributes are specified in same order data is provided in.
        int offset = 0;
        int attributeIndex = 0;
        for(auto& attribute : mData.vertexDefinition.attributes)
        {
            // Determine size of this attribute's data.
            GLsizeiptr attributeSize = mData.vertexCount * attribute.GetSize();

            // Load attribute data to GPU.
            glBufferSubData(GL_ARRAY_BUFFER, offset, attributeSize, mData.vertexData[attributeIndex].data);

            // Next attribute's offset is calculated by adding this attribute's size.
            offset += attributeSize;
            ++attributeIndex;
        }
    }
    else // Interleaved vertex layout.
    {
        // Allocate VBO of needed size, and fill it with provided vertex data (if any).
        glBufferData(GL_ARRAY_BUFFER, size, mData.vertexData[0].data, usage);
    }
}

void VertexArray::CreateVAO()
{
    // Generate and bind VAO object.
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    // Stride can be calculated once and used over and over.
    // For packed data, stride is zero. For interleaved data, stride is size of vertex.
    GLsizei stride = mData.vertexDefinition.CalculateStride();

    // Iterate vertex attributes to define the vertex data layout in the VAO.
    int attributeIndex = 0;
    for(auto& attribute : mData.vertexDefinition.attributes)
    {
        int attributeId = static_cast<int>(attribute.semantic);

        // Must enable the attribute to use it in shader code.
        glEnableVertexAttribArray(attributeId);

        // Convert attribute values to GL types.
        GLint count = attribute.count;
        GLenum type = GL_FLOAT; //TODO: We can assume float for now...but when supporting other attribute types, we'll need some conversion logic.
        GLboolean normalize = attribute.normalize ? GL_TRUE : GL_FALSE;
        int offset = mData.vertexDefinition.CalculateAttributeOffset(attributeIndex, mData.vertexCount);

        // Define vertex attribute in VAO.
        glVertexAttribPointer(attributeId, count, type, normalize, stride, BUFFER_OFFSET(offset));
        ++attributeIndex;
    }
}

void VertexArray::RefreshIBOContents()
{
    // No need if index data is empty or count is zero.
    if(mData.indexData == nullptr || mData.indexCount <= 0)
    {
        return;
    }

    // Either create new buffer and fill with index data, or populate existing buffer with new data.
    if(mIBO == GL_NONE)
    {
        glGenBuffers(1, &mIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);

        GLenum glUsage = (mData.meshUsage == MeshUsage::Static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mData.indexCount * sizeof(GLushort), mData.indexData, glUsage);
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mData.indexCount * sizeof(GLushort), mData.indexData);
    }
}
