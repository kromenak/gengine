//
// ACT.cpp
//
// Clark Kromenaker
//
#include "ACT.h"
#include "BinaryReader.h"
#include <iostream>
#include <vector>
#include "Vector3.h"

ACT::ACT(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

void ACT::ParseFromData(char *data, int dataLength)
{
    //std::cout << "ACT " << mName << std::endl;
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier "HTCA" (ACT backwards, but what's the H for?)
    std::string identifier = reader.ReadString(4);
    if(identifier != "HTCA")
    {
        std::cout << "ACT file does not have ACTH identifier!" << std::endl;
        return;
    }
    
    // 4 bytes: unknown - has thus far always been 258.
    reader.ReadUInt();
    
    // 4 bytes: number of keyframes in the animation.
    uint keyframeCount = reader.ReadUInt();
    
    // 4 bytes: for each keyframe, the number of meshes that have data in that keyframe.
    // This appears to 100% correlate to the mesh count for the model file itself, which makes sense.
    uint meshCount = reader.ReadUInt();
    
    // File contents size after header info.
    reader.ReadUInt();
    
    // Name of .MOD file this animation data is for.
    std::string modelName = reader.ReadString(32);
    //std::cout << "  Animation for model " << modelName << std::endl;
    //std::cout << "  Model Mesh Count: " << meshCount << std::endl;
    //std::cout << "  Keyframe Count: " << keyframeCount << std::endl;
    
    // Read offsets for each keyframe section in the file.
    // (just for validation at this point)
    std::vector<uint> offsets;
    for(int i = 0; i < keyframeCount; i++)
    {
        offsets.push_back(reader.ReadUInt());
    }
    
    // Iterate and read in data for each keyframe.
    for(int i = 0; i < keyframeCount; i++)
    {
        //std::cout << "  Keyframe " << i << std::endl;
        ACTKeyframe keyframe;
        
        // Validation: this should always be true unless we have a parsing error.
        assert(reader.GetPosition() == offsets[i]);
        
        // Each keyframe will have data for each mesh in the model.
        // However, it IS possible for there to be no data (byteCount == 0).
        for(int j = 0; j < meshCount; j++)
        {
            //std::cout << "    Mesh " << j << std::endl;
            ACTMeshData meshData;
            
            // 2 bytes: Index of mesh in MOD file.
            // It should always be ordered (0, 1, 2, 3), so we check that here as well.
            ushort index = reader.ReadUShort();
            assert(index == j);
            
            // 4 bytes: Number of bytes (after this spot) to get to next mesh's data.
            // All bytes from here to end of this mesh's data persists of 0 or more blocks
            // of animation data for this mesh.
            uint byteCount = reader.ReadUInt();
            while(byteCount > 0)
            {
                // 1 byte: some type of flag/identifier for upcoming block.
                uint8_t identifier = reader.ReadUByte();
                byteCount -= 1;
                
                // Identifier 0 appears to be vertex position data.
                if(identifier == 0)
                {
                    //std::cout << "      Vertex Data Block" << std::endl;
                    
                    // 4 bytes: number of bytes in block after this position.
                    uint blockByteCount = reader.ReadUInt();
                    //std::cout << "        Byte Count: " << blockByteCount << std::endl;
                    byteCount -= 4;
                    
                    // 2 bytes: mesh group within mesh this block refers to.
                    /*ushort meshGroupIndex = */reader.ReadUShort();
                    //std::cout << "        Mesh Group Index: " << meshGroupIndex << std::endl;
                    
                    // 2 bytes: vertex count - this seems to always be equal to
                    // the number of vertices for a mesh group in MOD file.
                    ushort vertexCount = reader.ReadUShort();
                    //std::cout << "        Vertex Count: " << vertexCount << std::endl;
                    
                    // Next, we have float values for each vertex, possibly vertex positions?
                    for(int i = 0; i < vertexCount; i++)
                    {
                        Vector3 vec(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                        //std::cout << "        " << vec << std::endl;
                        meshData.positions.push_back(vec);
                    }
                    byteCount -= blockByteCount;
                }
                // Identifier 1 also appears to be vertex data, but in a compressed format.
                else if(identifier == 1)
                {
                    //std::cout << "      Compressed Vertex Data Block" << std::endl;
                    
                    // 4 bytes: number of bytes in block after this position.
                    uint blockByteCount = reader.ReadUInt();
                    //std::cout << "        Byte Count: " << blockByteCount << std::endl;
                    byteCount -= 4;
                    
                    // 2 bytes: mesh group within mesh this block refers to.
                    /*ushort meshGroupIndex = */reader.ReadUShort();
                    //std::cout << "        Mesh Group Index: " << meshGroupIndex << std::endl;
                    
                    // 2 bytes: vertex count - this seems to always be equal to
                    // the number of vertices for a mesh group in MOD file.
                    ushort vertexCount = reader.ReadUShort();
                    //std::cout << "        Vertex Count: " << vertexCount << std::endl;
                    
                    // Next ((VertexCount/4) + 1) bytes: compression info for vertex data.
                    // Every 2 bits indicates the status of that vertex's data.
                    ushort compressionInfoSize = (vertexCount / 4) + 1;
                    unsigned char* compressionInfo = new unsigned char[compressionInfoSize];
                    reader.Read(compressionInfo, compressionInfoSize);
                    
                    // Iterate through the compresion info and, for each vertex, determine
                    // how it is stored in the upcoming data (uncompressed, compressed, not present at all)
                    unsigned char* vertexDataFormat = new unsigned char[vertexCount];
                    for(int k = 0; k < vertexCount; k += 4)
                    {
                        unsigned char byte = compressionInfo[k / 4];
                        uint val1 = (byte & 0x3);
                        uint val2 = (byte & 0xC) >> 2;
                        uint val3 = (byte & 0x30) >> 4;
                        uint val4 = (byte & 0xC0) >> 6;
                        
                        vertexDataFormat[k] = val1;        // Masking 0000 0011
                        vertexDataFormat[k + 1] = val2;    // Masking 0000 1100
                        vertexDataFormat[k + 2] = val3;    // Masking 0011 0000
                        vertexDataFormat[k + 3] = val4;    // Masking 1100 0000
                    }
                    delete[] compressionInfo;
                    
                    // Now that we have deciphered the vertex data formats,
                    // we can read in each vertex's data.
                    ACTKeyframe& lastKeyframe = mKeyframes[mKeyframes.size() - 1];
                    ACTMeshData& lastMeshData = lastKeyframe.meshDatas[j];
                    for(int k = 0; k < vertexCount; k++)
                    {
                        // 0 means no vertex data, so don't do anything!
                        if(vertexDataFormat[k] == 0)
                        {
                            meshData.positions.push_back(lastMeshData.positions[k]);
                        }
                        else if(vertexDataFormat[k] == 1)
                        {
                            // 1 means next 3 bytes are X, Y, Z compressed.
                            Vector3 vec(DecompressFloatFromByte(reader.ReadByte()),
                                        DecompressFloatFromByte(reader.ReadByte()),
                                        DecompressFloatFromByte(reader.ReadByte()));
                            //std::cout << "        " << vec << std::endl;
                            meshData.positions.push_back(lastMeshData.positions[k] + vec);
                        }
                        else if(vertexDataFormat[k] == 2)
                        {
                            // 2 means next 3 ushort are X, Y, Z compressed.
                            Vector3 vec(DecompressFloatFromUShort(reader.ReadUShort()),
                                        DecompressFloatFromUShort(reader.ReadUShort()),
                                        DecompressFloatFromUShort(reader.ReadUShort()));
                            //std::cout << "        " << vec << std::endl;
                            meshData.positions.push_back(lastMeshData.positions[k] + vec);
                        }
                        else if(vertexDataFormat[k] == 3)
                        {
                            Vector3 vec(reader.ReadFloat(),
                                        reader.ReadFloat(),
                                        reader.ReadFloat());
                            //std::cout << "        " << vec << std::endl;
                            meshData.positions.push_back(lastMeshData.positions[k] + vec);
                        }
                    }
                    delete[] vertexDataFormat;
                    
                    byteCount -= blockByteCount;
                }
                // Identifier 2 is transform matrix data.
                else if(identifier == 2)
                {
                    //std::cout << "      Matrix Data Block" << std::endl;
                    
                    // 4 bytes: number of bytes in block after this position.
                    // Appears to derive the number of floats in block, so must be divisible by 4.
                    uint blockByteCount = reader.ReadUInt();
                    //std::cout << "        Byte Count: " << blockByteCount << std::endl;
                    byteCount -= 4;
                    
                    assert(blockByteCount == 48);
                    
                    // This is followed by a number of floats, but I don't know what they are yet!
                    // Thus far, it has always been 12 floats (48 bytes)
                    uint numFloats = blockByteCount / 4;
                    for(int k = 0; k < numFloats / 3; k++)
                    {
                        Vector3 vec(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                        //std::cout << "        " << vec << std::endl;
                    }
                    byteCount -= blockByteCount;
                }
                // Identifier 3 is min/max data.
                else if(identifier == 3)
                {
                    //std::cout << "      Min/Max Data Block" << std::endl;
                    
                    // 4 bytes: number of bytes in block after this position.
                    // Appears to derive the number of floats in block, so must be divisible by 4.
                    uint blockByteCount = reader.ReadUInt();
                    //std::cout << "        Byte Count: " << blockByteCount << std::endl;
                    byteCount -= 4;
                    
                    assert(blockByteCount == 24);
                    
                    // This is followed by a number of floats, but I don't know what they are yet!
                    // Thus far, it has always been 6 floats (24 bytes).
                    meshData.min = Vector3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                    meshData.max = Vector3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
                    
                    byteCount -= blockByteCount;
                }
                else
                {
                    std::cout << "Unexpected identifier " << (int)identifier << std::endl;
                }
            } // while byteCount > 0
            
            keyframe.meshDatas.push_back(meshData);
        } // iterate mesh groups
        
        mKeyframes.push_back(keyframe);
    } // iterate keyframes
}

float ACT::DecompressFloatFromByte(unsigned char byte)
{
    uint sign = (byte & 0x80) >> 7;
    float frac = (float)(byte & 0x1F) / 32.0f;
    float whole = (byte & 0x7F) >> 5;
    return sign * (whole + frac);
}

float ACT::DecompressFloatFromUShort(unsigned short ushort)
{
    uint sign = (ushort & 0x8000) >> 15;
    float frac = (float)(ushort & 0x1FF) / 256.0f;
    float whole = (ushort & 0x7FFF) >> 8;
    return sign * (whole + frac);
}
