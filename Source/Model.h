//
// Model.h
//
// Clark Kromenaker
//
// 3D model asset type. The in-memory representation
// of .MOD assets.
//

#pragma once
#include <string>

using namespace std;

class Model
{
public:
    Model(string name, char* data, int dataLength);
    
    int GetVertexCount() const { return mVertexCount; }
    float* GetVertexPositions() const { return mVertexPositions; }
    
    int GetIndexCount() const { return mIndexCount; }
    unsigned short* GetIndexes() const { return mVertexIndexes; }
    
private:
    string mName;
    
    int mVertexCount = 0;
    float* mVertexPositions = nullptr;
    float* mVertexNormals = nullptr;
    float* mVertexUVs = nullptr;
    
    int mIndexCount = 0;
    unsigned short* mVertexIndexes = nullptr;
    
    void ParseFromModFileData(char* data, int dataLength);
};
