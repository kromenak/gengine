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

class AnimNode;

class Animation : public Asset
{
public:
    Animation(std::string name, char* data, int dataLength);
	~Animation();
    
    std::vector<AnimNode*>* GetFrame(int num);
    
    int GetFrameCount() { return mFrameCount; }
    int GetFramesPerSecond() { return mFramesPerSecond; }
    float GetDuration() { return (float)mFrameCount / (float)mFramesPerSecond; }
    
private:
    // Number of frames in this animation.
    int mFrameCount = 0;
    
    // Frames per second for this animation.
    // Default value "15" is taken from the defaults written to registry file.
    int mFramesPerSecond = 15;
    
    // Mapping of frame number to frame data.
    // "Frame data" consists of one or more animation nodes, which do things like
    // playing mesh animations, setting textures, playing sounds, etc.
    std::unordered_map<int, std::vector<AnimNode*>> mFrames;
    
    void ParseFromData(char* data, int dataLength);
};
