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
private:
    string mName;
    
public:
    Model(string name, char* data, int dataLength);
    
private:
    void ParseFromModFileData(char* data, int dataLength);
};
