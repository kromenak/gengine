//
// Clark Kromenaker
//
// Plays animations (ANM assets)!
//
// This is more of a "manager" style class than a per-actor instance class.
// See VertexAnimator for actor instance vertex animation logic.
//
#pragma once
#include "Component.h"

#include <functional>
#include <vector>

class Animation;
class VertexAnimation;
class MeshRenderer;

struct AnimParams
{
    // The anim to play.
    Animation* animation = nullptr;

    // Frame to start on.
    int startFrame = 0;

    // If true, animation can move associated character (kind of like "root motion").
    bool allowMove = false;

    // If true, this anim was started from an autoscript (GAS).
    bool fromAutoScript = false;

    // If true, this is a "yak" animation - which mainly indicates that audio should play as VO.
    bool isYak = false;

    // If true, the animation loops!
    bool loop = false;

    // A callback to fire on animation finish.
    std::function<void()> finishCallback = nullptr;
};

struct AnimationState
{
    // Params that were used to initiate this animation.
    AnimParams params;

    // The current frame in the animation.
    int currentFrame = 0;

    // The frame that is currently executing in the animation.
    // The only time "current frame" != "executing frame" is if we "fast forwarded" the animation.
    // Ex: if anim starts on frame 20, we still must execute 0-19 to start any vertex anims.
    int executingFrame = 0;

    // A timer to track when we need to execute one or more additional frames.
    // This doesn't track total animation time, just time until the next frame!
    float timer = 0.0f;

    // If true, this AnimState is no longer being actively used and can be erased/recycled.
    bool done = false;

    void Stop();
};

class Animator : public Component
{
    TYPE_DECL_CHILD();
public:
    Animator(Actor* owner);
	
	// Animation Playback
    void Start(Animation* animation, std::function<void()> finishCallback = nullptr);
    void Start(const AnimParams& animParams);
	void Stop(Animation* animation);
    void StopAll();
    
	// Animation Sampling
	void Sample(Animation* animation, int frame);
    void Sample(Animation* animation, int frame, const std::string& modelName);
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	// I chose to use a linked list here b/c we may need to remove animations that are
	// in the middle of the list at arbitrary times...not sure if the list is big enough or we do it often enough to get benefits?
	std::vector<AnimationState> mActiveAnimations;
	
	void ExecuteFrame(int animIndex, int frameNumber);
};
