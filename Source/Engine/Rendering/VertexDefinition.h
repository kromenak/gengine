//
// Clark Kromenaker
//
// When sending mesh data to the GPU, we need to specify the format of the vertex data.
//
// For example, one mesh's vertices might have only position & uv data, while another
// mesh might have position, uv, and normal data. One mesh may provide vertex data
// in blocks, while another may provide interleaved data.
//
// A "vertex definition" lets us specify the format used by a particular mesh
// in a flexible format. Rather than our VertexArray code only supporting certain
// layouts, that data can be encapsulated in the vertex definition object.
//
#pragma once
#include <vector>

extern const char* gAttributeNames[];

struct VertexAttribute
{
     // Different combinations of type/count/etc allow for great flexibility in the format of an attribute.
    // That being said, there are several common attribute types that are used 99% of the time. Those are pre-defined here.
    static VertexAttribute Position;
    static VertexAttribute Normal;
    static VertexAttribute Color;
    static VertexAttribute UV1;
    static VertexAttribute UV2;

    // The semantic (aka meaning) the type/usage of the attribute.
    // There are some common semantics in this enum - but it's also possible to use custom integers.
    // No two attributes in a vertex definition should have the same semantic!
    enum class Semantic
    {
        Position,
        Normal,
        Color,
        UV1,
        UV2,
        SemanticCount
    };
    Semantic semantic = Semantic::Position;

    // The data type for the attribute.
    // See https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml for other possible types.
    enum class Type
    {
        Float
    };
    Type type = Type::Float;

    // The number of components of the type. For example, position data is 3 Floats.
    int count = 0;

    // Should the data be normalized before use in shaders?
    // Some data (like colors) usually should be normalized, while others (like position) should not be.
    bool normalize = false;

    int GetSize() const;
    bool operator==(const VertexAttribute& other) const { return semantic == other.semantic; }
};

enum class VertexLayout
{
    Interleaved,    // [Vertex1Pos][Vertex1UV][Vertex2Pos][Vertex2UV]
    Packed          // [Vertex1Pos][Vertex2Pos][Vertex1UV][Vertex2UV]
};

struct VertexDefinition
{
    // Layout of the vertex data.
    // I've read that interleaved data results in better GPU cache performance (since reading a single vertex requires no seeking).
    VertexLayout layout = VertexLayout::Interleaved;

    // A vertex definition consists of one or more attributes.
    // Order IS important!
    std::vector<VertexAttribute> attributes;

    int CalculateSize() const;

    // "Stride" is the byte offset between attributes of the same type.
    // For packed data, this is zero. For interleaved data, it is essentially equal to the size of a single vertex.
    int CalculateStride() const;

    // Byte offset to the first instance of an attribute is required by GPU.
    // For packed data, the vertex count is required to properly calculate this.
    int CalculateAttributeOffset(int index, int vertexCount = 0) const;
};
