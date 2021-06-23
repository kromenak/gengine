//
// GAS.cpp
//
// Clark Kromenaker
//
// A "G-Engine Auto Script". These are basic text-based logic trees
// that usually handle playing of animations.
//
// They are read into an in-memory format (this class) and used as input to
// a "player" or "executor" for the script.
//
#pragma once
#include "Asset.h"

#include <string>
#include <vector>

struct GasNode;

class GAS : public Asset
{
public:
    GAS(std::string name, char* data, int dataLength);
    ~GAS() override;
    
    GasNode* GetNode(int index) { return mNodes[index]; }
    int GetNodeCount() { return (int)mNodes.size(); }
    
private:
    std::vector<GasNode*> mNodes;
    
    void ParseFromData(char* data, int dataLength);
};
