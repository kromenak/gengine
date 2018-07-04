//
// ACT.h
//
// Clark Kromenaker
//
// In-memory representation of "ACT" file.
// An ACT file contains vertex animation data for a particular MOD file.
//
#pragma once
#include "Asset.h"
#include "Vector3.h"
#include <vector>

struct ACTMeshData
{
    std::vector<Vector3> positions;
    Vector3 min;
    Vector3 max;
};

struct ACTKeyframe
{
    std::vector<ACTMeshData> meshDatas;
};

class ACT : public Asset
{
public:
    ACT(std::string name, char* data, int dataLength);
    
private:
    std::vector<ACTKeyframe> mKeyframes;
    
    void ParseFromData(char* data, int dataLength);
    
    float DecompressFloatFromByte(unsigned char byte);
    float DecompressFloatFromUShort(unsigned short ushort);
};
