//
// AnimationNodes.h
//
// Clark Kromenaker
//
// Various nodes that are used in constructing animations.
//
// Animations proceed over time, executing each "frame" as enough time passes.
// Each frame can have one or more "animation nodes" associated with it.
// When a frame is executed, all associated animation nodes are executed.
// Each node can do something such as: play a vertex animation, trigger SFX, change face textures, lip sync, etc.
//
#pragma once
#include <string>

#include "FaceController.h"
#include "Vector3.h"

class Animation;
struct AnimationState;
class Audio;
class VertexAnimation;

// Base struct for all anim nodes.
struct AnimNode
{
	int frameNumber = 0;
	
	virtual ~AnimNode() { }
	
	virtual void Play(AnimationState* animState) = 0;
	virtual void Stop() { } // Stop support is optional. Does nothing by default.
	virtual void Sample(Animation* anim, int frame) { } // Sampling support is optional. Does nothing by default.
};

// A node that plays a vertex animation.
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

// Applies a texture to a *scene* model (aka a model within the BSP geometry).
struct SceneTextureAnimNode : public AnimNode
{
	std::string sceneName;
	std::string sceneModelName;
	std::string textureName;
	
	void Play(AnimationState* animState) override;
};

// Changes visibility of a *scene* model (aka a model within the BSP geometry).
struct SceneModelVisibilityAnimNode : public AnimNode
{
	std::string sceneName;
	std::string sceneModelName;
	bool visible = false;
	
	void Play(AnimationState* animState) override;
};

// Changes a models texture (a model loaded from a .MOD file).
struct ModelTextureAnimNode : public AnimNode
{
	std::string modelName;
	unsigned char meshIndex = 0;
	unsigned char submeshIndex = 0;
	std::string textureName;
	
	void Play(AnimationState* animState) override;
};

// Changes visibility of a model (a model loaded from a .MOD file).
struct ModelVisibilityAnimNode : public AnimNode
{
	std::string modelName;
	bool visible = false;
	
	void Play(AnimationState* animState) override;
};

// A node that plays a sound effect.
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

// A node that plays a footstep sound.
struct FootstepAnimNode : public AnimNode
{
	std::string actorNoun;
	
	void Play(AnimationState* animState) override;
};

// A node that plays a "footscuff' sound.
struct FootscuffAnimNode : public AnimNode
{
	std::string actorNoun;
	
	void Play(AnimationState* animState) override;
};

// A node that plays a soundtrack.
struct PlaySoundtrackAnimNode : public AnimNode
{
	std::string soundtrackName;
	//TODO: TBS variant?
	
	void Play(AnimationState* animState) override;
};

// A node that stops a soundtrack.
struct StopSoundtrackAnimNode : public AnimNode
{
	// If empty, means "stop all soundtracks."
	std::string soundtrackName;
	
	void Play(AnimationState* animState) override;
};

// A node that changes the game camera's position/angle.
struct CameraAnimNode : public AnimNode
{
	std::string cameraPositionName;
	
	void Play(AnimationState* animState) override;
};

// A node that changes a part of an actor's face.
struct FaceTexAnimNode : public AnimNode
{
	std::string actorNoun;
	std::string textureName;
	FaceElement faceElement = FaceElement::Mouth;
	
	void Play(AnimationState* animState) override;
};

// A node that reverts part of an actor's face to the default.
struct UnFaceTexAnimNode : public AnimNode
{
	std::string actorNoun;
	FaceElement faceElement = FaceElement::Mouth;
	
	void Play(AnimationState* animState) override;
};

// A node that changes an actor's mouth texture, for lip-sync during VO.
struct LipSyncAnimNode : public AnimNode
{
	std::string actorNoun;
	std::string mouthTextureName;
	
	void Play(AnimationState* animState) override;
};

// A node that causes an actor to look towards a position briefly.
struct GlanceAnimNode : public AnimNode
{
	// This node is used a grand total of ONCE in the entire game - EMLCONCENTRATE.ANM
	std::string actorNoun;
	Vector3 position;
	
	void Play(AnimationState* animState) override;
};

// A node that applies a certain "mood" (face combination) to an actor.
struct MoodAnimNode : public AnimNode
{
	std::string actorNoun;
	std::string moodName;
	
	void Play(AnimationState* animState) override;
};

// Specifies who the speaker is.
// This is used to determine caption colors (different characters get different colors).
// This is ALSO used (I think) to apply talk or listen animations to conversation participants.
struct SpeakerAnimNode : public AnimNode
{
	// The actor who will be speaking.
	std::string actorNoun;
	
	void Play(AnimationState* animState) override;
};

// Contains a caption to show on-screen.
// When do we stop showing it? Maybe on a timer, when next caption appears, when "DIALOGUECUE" occurs...?
struct CaptionAnimNode : public AnimNode
{
	// The caption to show.
	std::string caption;
	
	void Play(AnimationState* animState) override;
};

// Speaker and caption data rolled into one.
// Specifies who is speaking, what they're saying, and frame to end on.
// Seems to be used primarily for movie captions (vs. in-game captions).
struct SpeakerCaptionAnimNode : public AnimNode
{
	// The frame to stop showing the caption on.
	int endFrame = 0;
	
	// The actor who will be speaking.
	std::string actorNoun;
	
	// The caption to show.
	std::string caption;
	
	void Play(AnimationState* animState) override;
};

// An empty trigger node, seems to be used to signal the end of a piece of dialogue.
// There tends to be exactly one of these at the end of in-game VO animation (YAK) files.
// What does it do? Maybe clears captions, maybe signals to SheepScript that the VO is over. Not sure yet...
struct DialogueCueAnimNode : public AnimNode
{
	void Play(AnimationState* animState) override;
};

