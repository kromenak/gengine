//
// VertexArray.cpp
//
// Clark Kromenaker
//
#include "VertexArray.h"

#include <iostream>

// Some OpenGL calls take in array indexes/offsets as pointers.
// This macro just makes the syntax clearer for the reader.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

VertexArray::VertexArray(const MeshDefinition& data) :
    mData(data)
{
    // Generate and bind VBO.
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    // Determine VBO usage and size.
    GLenum usage = (mData.meshUsage == MeshUsage::Static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    GLsizeiptr size = mData.vertexCount * mData.vertexDefinition.CalculateSize();
    
    // For packed data, we'll assume that the vertex data is a struct containing ordered pointers to each packed section.
    if(mData.vertexDefinition.layout == VertexDefinition::Layout::Packed)
    {
        // Create buffer of desired size, but don't fill it with anything.
        glBufferData(GL_ARRAY_BUFFER, size, NULL, usage);
        
        // For packed data, we'll assume the vertex data is a structure containing pointers to each packed attribute.
        // For example: struct VertexData { float* positions; float* uvs; }
        void* dataPtr = mData.vertexData;
        
        // Iterate each attribute and load packed data for that attribute into the VBO.
        // We assume attributes are specified in same order data is provided in.
        int offset = 0;
        for(auto& attribute : mData.vertexDefinition.attributes)
        {
            // Determine size of this attribute's data.
            GLsizeiptr attributeSize = mData.vertexCount * attribute.GetSize();
            
            // "dataPtr" is pointing at some memory that we want to interpret as a void*
            // To do this, we can cast to a pointer-to-a-pointer. Dereferencing interprets the memory at that location as a void*.
            void** pointerToPointerToData = static_cast<void**>(dataPtr);
            void* actualDataPtr = *pointerToPointerToData;
            
            // Load attribute data to GPU.
            glBufferSubData(GL_ARRAY_BUFFER, offset, attributeSize, actualDataPtr);
            
            // Next attribute's offset is calculated by adding this attribute's size.
            offset += attributeSize;
            
            // Increment dataPtr to the next data pointer in the vertex data struct.
            dataPtr = static_cast<char*>(dataPtr) + sizeof(char*);
        }
    }
    else
    {
        // Allocate VBO of needed size, and fill it with provided vertex data (if any).
        glBufferData(GL_ARRAY_BUFFER, size, data.vertexData, usage);
    }
    
    // If index data was provided, populate IBO.
    RefreshIBOContents(mData.indexData, mData.indexCount);
    
    // Generate and bind VAO object.
    {
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
    
    // Clear vertex data and index data pointers.
    // Remember, we can only assume those pointers are valid during construction anyway.
    mData.vertexData = nullptr;
    mData.indexData = nullptr;
}

VertexArray::~VertexArray()
{
    glDeleteBuffers(1, &mVBO);
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mIBO);
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
    
    other.mVBO = GL_NONE;
    other.mVAO = GL_NONE;
    other.mIBO = GL_NONE;
    return *this;
}

void VertexArray::ChangeVertexData(void* data)
{
    // Assuming that the data is the correct size to fill the entire buffer.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mData.vertexCount * mData.vertexDefinition.CalculateSize(), data);
}

void VertexArray::ChangeVertexData(VertexAttribute::Semantic semantic, void *data)
{
    // We can really only update a single attribute's data if attribute data is tightly packed.
    // If data is interleaved, we'll just fall back on overwriting all data.
    if(mData.vertexDefinition.layout == VertexDefinition::Layout::Interleaved)
    {
        std::cout << "WARNING: You can only update an individual vertex attribute's data when using non-interleaved data!" << std::endl;
        ChangeVertexData(data);
        return;
    }
    
    // For tightly packed data, we can determine the "sub data" and update just a portion.
    int offset = 0;
    for(auto& attribute : mData.vertexDefinition.attributes)
    {
        // Determine size of this attribute's data.
        GLsizeiptr attributeSize = mData.vertexCount * attribute.GetSize();
        
        // Update sub-data, if semantic matches.
        if(attribute.semantic == semantic)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mVBO);
            glBufferSubData(GL_ARRAY_BUFFER, offset, attributeSize, data);
            return;
        }
        
        // Next attribute's offset is calculated by adding this attribute's size.
        offset += attributeSize;
    }
}

void VertexArray::ChangeIndexData(unsigned short* indexes, unsigned int count)
{
    // If changing existing buffer contents, but the count is different, we must create delete old buffer and make a new one.
    if(mIBO != GL_NONE && mData.indexCount != count)
    {
        glDeleteBuffers(1, &mIBO);
        mIBO = GL_NONE;
    }
    
    // Update index count.
    mData.indexCount = count;
    
    // Refresh IBO contents with new data.
    RefreshIBOContents(indexes, count);
}

void VertexArray::DrawTriangles() const
{
    DrawTriangles(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangles(unsigned int offset, unsigned int count) const
{
    Draw(GL_TRIANGLES, offset, count);
}

void VertexArray::DrawTriangleStrips() const
{
    DrawTriangleStrips(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangleStrips(unsigned int offset, unsigned int count) const
{
    Draw(GL_TRIANGLE_STRIP, offset, count);
}

void VertexArray::DrawTriangleFans() const
{
    DrawTriangleFans(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawTriangleFans(unsigned int offset, unsigned int count) const
{
    Draw(GL_TRIANGLE_FAN, offset, count);
}

void VertexArray::DrawLines() const
{
    DrawLines(0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::DrawLines(unsigned int offset, unsigned int count) const
{
    Draw(GL_LINES, offset, count);
}

void VertexArray::Draw(GLenum mode) const
{
    Draw(mode, 0, mData.indexCount > 0 ? mData.indexCount : mData.vertexCount);
}

void VertexArray::Draw(GLenum mode, unsigned int offset, unsigned int count) const
{
    // Bind vertex array object.
    glBindVertexArray(mVAO);
    
    // Draw method depends on whether we have indexes or not.
    if(mIBO != GL_NONE)
    {
        // Bind index buffer.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        
        // Draw "count" indices at offset.
        glDrawElements(mode, count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(offset * sizeof(GLushort)));
    }
    else
    {
        // Draw "count" triangles at offset.
        glDrawArrays(mode, offset, count);
    }
}
                    
void VertexArray::RefreshIBOContents(unsigned short* indexData, int indexCount)
{
    if(indexData != nullptr && indexCount > 0)
    {
        // Either create new buffer and fill with index data,
        // Or populate existing buffer with new data.
        if(mIBO == GL_NONE)
        {
            glGenBuffers(1, &mIBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
            
            GLenum glUsage = (mData.meshUsage == MeshUsage::Static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), indexData, glUsage);
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexCount * sizeof(GLushort), indexData);
        }
    }
}
