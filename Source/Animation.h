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

class Animation;
class VertexAnimation;

struct AnimNode
{
	int frameNumber = 0;
	
	virtual ~AnimNode() { }
	virtual void Play(Animation* anim) = 0;
};

struct VertexAnimNode : public AnimNode
{
	VertexAnimation* vertexAnimation = nullptr;
	
	void Play(Animation* anim) override;
};

struct SceneTextureAnimNode : public AnimNode
{
	std::string sceneName;
	std::string sceneModelName;
	std::string textureName;
	
	void Play(Animation* anim) override;
};

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
