//
// AnimationPlayer.cpp
//
// Clark Kromenaker
//
// Plays animations!
//
#pragma once
#include "Component.h"

class Animation;
class VertexAnimation;
class MeshRenderer;

class AnimationPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    AnimationPlayer(Actor* owner);
    
    void Play(Animation* animation);
    void Play(VertexAnimation* vertexAnimation);
    
    void SetMeshRenderer(MeshRenderer* meshRenderer) { mMeshRenderer = meshRenderer; }
    MeshRenderer* GetMeshRenderer() { return mMeshRenderer; }
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
    // Needs a MeshRenderer to update the mesh data every frame.
    MeshRenderer* mMeshRenderer = nullptr;
    
    // The animation currently being played.
    Animation* mAnimation = nullptr;
    
    // The frame we're currently on in the animation.
    int mCurrentAnimationFrame = 0;
    float mAnimationTimer = 0.0f;
    
    // If defined, a currently running vertex animation.
    VertexAnimation* mVertexAnimation = nullptr;
    float mVertexAnimationTimer = 0.0f;
};
