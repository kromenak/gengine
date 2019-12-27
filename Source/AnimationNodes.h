//
// AnimationNodes.h
//
// Clark Kromenaker
//
// Description
//
#pragma once
#include <string>

#include "FaceController.h"
#include "Vector3.h"

class Animation;
class AnimationState;
class Audio;
class VertexAnimation;

struct AnimNode
{
	int frameNumber = 0;
	
	virtual ~AnimNode() { }
	
	virtual void Play(AnimationState* animState) = 0;
	virtual void Stop() { } // Stop support is optional. Does nothing by default.
	virtual void Sample(Animation* anim, int frame) { } // Sampling support is optional. Does nothing by default.
};

struct VertexAnimNode : public AnimNode
{
	// A vertex animation to play.
	VertexAnimation* vertexAnimation = nullptr;
	
	// Absolute anims need to have position and heading for the target actor set exactly for the anim to play correctly.
	// Relative anims (aka not absolute anims) can play on the actor regardless of the actor's position.
	bool absolute = false;
	
	// This is a bit confusing, and I'm not totally sure why it is structured this way.
	// In ANM file, syntax for vertex anim is <frame_num>, <act_name>, <x1>, <y1>, <z1>, <angle1>, <x2>, <y2>, <z2>, <angle2>
	// The first x/y/z/angle appear to be the offset from the model's authored center to the origin.
	// The second x/y/z/angle appear to be the desired offset from the origin.
	// In other words, to properly position an object for an animation, we do (position - offsetFromOrigin)
	Vector3 offsetFromOrigin;
	float headingFromOrigin = 0.0f;
	
	Vector3 position;
	float heading = 0.0f;
	
	void Play(AnimationState* animState) override;
	void Stop() override;
	void Sample(Animation* anim, int frame) override;
};

struct SceneTextureAnimNode : public AnimNode
{
	std::string sceneName;
	std::string sceneModelName;
	std::string textureName;
	
	void Play(AnimationState* animState) override;
};

struct SceneModelVisibilityAnimNode : public AnimNode
{
	std::string sceneName;
	std::string sceneModelName;
	bool visible = false;
	
	void Play(AnimationState* animState) override;
};

struct ModelTextureAnimNode : public AnimNode
{
	std::string modelName;
	unsigned char meshIndex = 0;
	unsigned char submeshIndex = 0;
	std::string textureName;
	
	void Play(AnimationState* animState) override;
};

struct ModelVisibilityAnimNode : public AnimNode
{
	std::string modelName;
	bool visible = false;
	
	void Play(AnimationState* animState) override;
};

struct SoundAnimNode : public AnimNode
{
	Audio* audio = nullptr;
	
	// 0 = no sound, 100 = max volume
	int volume = 100;
	
	// If a model name is defined, play the sound at the model's position.
	// Otherwise, use the specific position defined.
	std::string modelName;
	Vector3 position;
	
	// Min and max distances for hearing the sound effect.
	float minDistance = -1.0f;
	float maxDistance = -1.0f;
	
	void Play(AnimationState* animState) override;
};

struct FootstepAnimNode : public AnimNode
{
	std::string actorNoun;
	
	void Play(AnimationState* animState) override;
};

struct FootscuffAnimNode : public AnimNode
{
	std::string actorNoun;
	
	void Play(AnimationState* animState) override;
};

struct PlaySoundtrackAnimNode : public AnimNode
{
	std::string soundtrackName;
	//TODO: TBS variant?
	
	void Play(AnimationState* animState) override;
};

struct StopSoundtrackAnimNode : public AnimNode
{
	// If empty, means "stop all soundtracks."
	std::string soundtrackName;
	
	void Play(AnimationState* animState) override;
};

struct CameraAnimNode : public AnimNode
{
	std::string cameraPositionName;
	
	void Play(AnimationState* animState) override;
};

struct FaceTexAnimNode : public AnimNode
{
	std::string actorNoun;
	std::string textureName;
	FaceElement faceElement = FaceElement::Mouth;
	
	void Play(AnimationState* animState) override;
};

struct UnFaceTexAnimNode : public AnimNode
{
	std::string actorNoun;
	FaceElement faceElement = FaceElement::Mouth;
	
	void Play(AnimationState* animState) override;
};

struct LipSyncAnimNode : public AnimNode
{
	std::string actorNoun;
	std::string mouthTextureName;
	
	void Play(AnimationState* animState) override;
};

struct GlanceAnimNode : public AnimNode
{
	// This node is used a grand total of ONCE in the entire game - EMLCONCENTRATE.ANM
	std::string actorNoun;
	Vector3 position;
	
	void Play(AnimationState* animState) override;
};

struct MoodAnimNode : public AnimNode
{
	std::string actorNoun;
	std::string moodName;
	
	void Play(AnimationState* animState) override;
};
