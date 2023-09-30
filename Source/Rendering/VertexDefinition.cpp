#include "VertexDefinition.h"

const char* gAttributeNames[] = {
    "vPos",
    "vNormal",
    "vColor",
    "vUV1",
    "vUV2"
};

VertexAttribute VertexAttribute::Position {
    Semantic::Position,
    Type::Float,
    3,
    false
};

VertexAttribute VertexAttribute::Normal {
    Semantic::Normal,
    Type::Float,
    3,
    true
};

VertexAttribute VertexAttribute::Color {
    Semantic::Color,
    Type::Float,
    4,
    true
};

VertexAttribute VertexAttribute::UV1 {
    Semantic::UV1,
    Type::Float,
    2,
    true
};

VertexAttribute VertexAttribute::UV2 {
    Semantic::UV2,
    Type::Float,
    2,
    true
};

int VertexAttribute::GetSize() const
{
    //TODO: Since the only Type is "Float" right now, assume byte size of 4.
    //TODO: When more types are added, we'll need some sort of switch statement probably.
    int byteSize = 4;
    
    // Size of an attribute is just byte size (based on type) and the count of the type.
    return byteSize * count;
}

int VertexDefinition::CalculateSize() const
{
    // The size of a single vertex is just the summed size of all attributes.
    int size = 0;
    for(auto& attribute : attributes)
    {
        size += attribute.GetSize();
    }
    return size;
}

int VertexDefinition::CalculateStride() const
{
    // When data is tightly packed, the stride should be zero.
    if(layout == VertexLayout::Packed) { return 0; }
    
    // For interleaved data, the stride is really just the size of a single vertex.
    return CalculateSize();
}

int VertexDefinition::CalculateAttributeOffset(int index, int vertexCount) const
{
    // The offset to the first instance of an attribute can be calculated using the order and sizes of the attributes.
    int offset = 0;
    for(int i = 0; i < index; ++i)
    {
        // If an invalid index was specified, we'll just act like the last possible index was given (clamp).
        if(i >= attributes.size()) { break; }
        
        // When data is tightly packed, the offset can only be determined if we know the vertex count.
        if(layout == VertexLayout::Packed)
        {
            offset += vertexCount * attributes[i].GetSize();
        }
        else // For interleaved data, the vertex count isn't necessary.
        {
            offset += attributes[i].GetSize();
        }
    }
    return offset;
}
