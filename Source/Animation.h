//
// Animation.cpp
//
// Clark Kromenaker
//
// An animation that can be played against an actor or model.
// In-memory representation of a ".ANM" asset.
//
// Contains animation metadata, plus pointer to the animation data itself.
//
#pragma once
#include "Asset.h"
#include <unordered_map>
#include <vector>

class ACT;

struct AnimationNode
{
    ACT* action = nullptr;
};

class Animation : public Asset
{
public:
    Animation(std::string name, char* data, int dataLength);
    
    std::vector<AnimationNode*>* GetFrame(int num);
    
    float GetDuration() { return (float)mFrameCount / (float)mFramesPerSecond; }
    
private:
    // Number of frames in this animation.
    int mFrameCount = 0;
    
    // Frames per second for this animation.
    int mFramesPerSecond = 30;
    
    // Mapping of frame number to frame data.
    // "Frame data" consists of one or more animation nodes, which do things like
    // playing mesh animations, setting textures, playing sounds, etc.
    std::unordered_map<int, std::vector<AnimationNode*>> mFrames;
    
    void ParseFromData(char* data, int dataLength);
};
