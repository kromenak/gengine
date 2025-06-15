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

    // If true, this anim will not do any parenting, even if it would otherwise be autodetected and applied.
    bool noParenting = false;

    // If set, this actor is set as the parent for vertex animations (except when the parent IS the object, or for walker DOR models).
    Actor* parent = nullptr;

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
    TYPEINFO_SUB(Animator, Component);
public:
    Animator(Actor* owner);

    // Animation Playback
    void Start(Animation* animation, std::function<void()> finishCallback = nullptr);
    void Start(const AnimParams& animParams, std::function<void()> finishCallback = nullptr);
    void Stop(Animation* animation, bool skipFinishCallback = false);
    void StopAll();

    // Animation Sampling
    void Sample(Animation* animation, int frame);
    void Sample(Animation* animation, int frame, const std::string& modelName);

protected:
    void OnUpdate(float deltaTime) override;
    void OnLateUpdate(float deltaTime) override;

private:
    // Animations that are currently active.
    std::vector<AnimationState> mActiveAnimations;

    void ExecuteFrame(int animIndex, int frameNumber);
};
