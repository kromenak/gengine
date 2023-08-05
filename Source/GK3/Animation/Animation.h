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

struct AnimNode;
class VertexAnimation;
struct VertexAnimNode;

class Animation : public Asset
{
public:
    Animation(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    ~Animation();

    void Load(uint8_t* data, uint32_t dataLength);

	// Gets all anim nodes associated with a particular frame number. Null may be returned!
	// Mainly used by Animator to get frame data as needed and play/sample.
	//TODO: Might be better to move code from Animator that uses this into Animation directly.
    std::vector<AnimNode*>* GetFrame(int frameNumber);
	
	// Just returns all vertex anim nodes! Used for stopping an animation.
	//TODO: Again, might make sense to move code from Animator into this class.
	const std::vector<VertexAnimNode*>& GetVertexAnimNodes() const { return mVertexAnimNodes; }
	
	// Finds a vertex animation, if any, that starts on the given frame for the given model.
	// Mainly used to support "anim" approach type - allows us to query what a model's position/facing will be when an animation starts.
    VertexAnimNode* GetVertexAnimationOnFrameForModel(int frameNumber, const std::string& modelName);
	
	// Length and duration.
    int GetFrameCount() const { return mFrameCount; }
    int GetFramesPerSecond() const { return mFramesPerSecond; }
	float GetFrameDuration() const { return 1.0f / mFramesPerSecond; }
    float GetDuration() const { return (float)mFrameCount / (float)mFramesPerSecond; }
    
private:
    // Number of frames in this animation.
	// Note that not every frame necessarily has data defined!
    int mFrameCount = 0;
    
    // Frames per second for this animation.
    // Default value "15" is taken from the defaults written to registry file.
    int mFramesPerSecond = 15;
    
    // Mapping of frame number to animation nodes. Each frame can have zero, one,
	// or many anim nodes representing animation events that should start on that frame.
    std::unordered_map<int, std::vector<AnimNode*>> mFrames;
	
	// All vertex anim nodes in the animation.
	// Kept separately because we sometimes need to iterate only over these.
	std::vector<VertexAnimNode*> mVertexAnimNodes;
    
    void ParseFromData(uint8_t* data, uint32_t dataLength);
};
