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
    GAS(const std::string& name, AssetScope scope);
    ~GAS();

    void Load(char* data, int dataLength);
    
    GasNode* GetNode(int index) { return mNodes[index]; }
    int GetNodeCount() { return (int)mNodes.size(); }
    
private:
    std::vector<GasNode*> mNodes;
};
