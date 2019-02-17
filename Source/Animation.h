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

#include "Vector3.h"

class Animation;
class VertexAnimation;

struct AnimNode
{
	int frameNumber = 0;
	
	virtual ~AnimNode() { }
	virtual void Play(Animation* anim) = 0;
	virtual void Sample(Animation* anim, int frame) { } // Sampling support is optional. Does nothing by default.
};

struct VertexAnimNode : public AnimNode
{
	// A vertex animation to play.
	VertexAnimation* vertexAnimation = nullptr;
	
	// This is a bit confusing, and I'm not totally sure why it is structured this way.
	// In ANM file, syntax for vertex anim is <frame_num>, <act_name>, <x1>, <y1>, <z1>, <angle1>, <x2>, <y2>, <z2>, <angle2>
	// The first x/y/z/angle appear to be the offset from the model's authored center to the origin.
	// The second x/y/z/angle appear to be the desired offset from the origin.
	// In other words, to properly position an object for an animation, we do (position - offsetFromOrigin)
	Vector3 offsetFromOrigin;
	float headingFromOrigin = 0.0f;
	
	Vector3 position;
	float heading = 0.0f;
	
	void Play(Animation* anim) override;
	void Sample(Animation* anim, int frame) override;
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
