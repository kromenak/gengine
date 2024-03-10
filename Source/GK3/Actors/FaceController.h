//
// Clark Kromenaker
//
// Controls facial elements, such as eye movement, blinking, lip movement,
// and expressions, etc.
//
#pragma once
#include "Component.h"

#include "Vector2.h"

class Animation;
struct CharacterConfig;
class Texture;

enum class FaceElement
{
	Forehead,
	Eyelids,
	Mouth
};

enum class EyeType
{
	Left,
	Right
};

class FaceController : public Component
{
    TYPEINFO(FaceController, Component);
public:
	FaceController(Actor* owner);
	~FaceController();
	
	void SetCharacterConfig(const CharacterConfig& characterConfig);
	
	void Set(FaceElement element, Texture* texture);
	void Clear(FaceElement element);
	
	void SetMouth(Texture* texture);
	void ClearMouth();
	
	void SetEyelids(Texture* texture);
	void ClearEyelids();
	
	void SetEyes(Texture* texture);
	void ClearEyes();
	
	void SetEye(EyeType type, Texture* texture);
	void ClearEye(EyeType type);
	
	void SetForehead(Texture* texture);
	void ClearForehead();
	
	void Blink();
	void Blink(const std::string& animName);
	
	void SetEyeJitterEnabled(bool enabled);
	void EyeJitter();
	
	void DoExpression(const std::string& expression);
	
	void SetMood(const std::string& mood);
	void ClearMood();
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	// The character config assigned by owner.
	const CharacterConfig* mCharacterConfig = nullptr;
	
	// The "final" face texture, which is written to for the final result.
	// Currently, we just write directly into the face asset from disk...maybe not smart.
	Texture* mFaceTexture = nullptr;
	
	// Whatever is currently set for each texture, so we can reconstruct the face whenever we need to.
	Texture* mCurrentMouthTexture = nullptr;
	Texture* mCurrentEyelidsTexture = nullptr;
	Texture* mCurrentForeheadTexture = nullptr;
	
	Texture* mCurrentLeftEyeTexture = nullptr;
	Texture* mCurrentRightEyeTexture = nullptr;
	
	// Default mouth/eyelids/forehead textures, for clearing expressions.
	Texture* mDefaultMouthTexture = nullptr;
	Texture* mDefaultEyelidsTexture = nullptr;
	Texture* mDefaultForeheadTexture = nullptr;
	
	Texture* mDefaultLeftEyeTexture = nullptr;
	Texture* mDefaultRightEyeTexture = nullptr;

    // Eye textures are fairly large (100x104) and are downsampled to ~1/4th the size.
    // Exactly 1/4th would be 25x26, BUT that height is too big for some faces - 22 seems to work pretty well.
    const int kDownSampledEyeWidth = 25;
    const int kDownSampledEyeHeight = 22;
	Texture* mDownsampledLeftEyeTexture = nullptr;
	Texture* mDownsampledRightEyeTexture = nullptr;
	
	// A timer for how frequently the face should blink.
	// Set randomly based on interval specified in face config.
	float mBlinkTimer = 0.0f;
	
	// If eye jitter is enabled, apply some jitter within a range (based on character config)
	// before downsampling eyes to construct the face.
	bool mEyeJitterEnabled = true;
	float mEyeJitterTimer = 0.0f;
	float mEyeJitterX = 0.0f;
	float mEyeJitterY = 0.0f;
	
	// A mood for the actor, if any. Default is "no mood" (aka "normal").
	// Actors can enter and exit moods. This requires "on" and "off" anims to exist.
	std::string mMood;
	Animation* mEnterMoodAnimation = nullptr;
	Animation* mExitMoodAnimation = nullptr;
	
	void RollBlinkTimer();
	void RollEyeJitterTimer();

	void UpdateFaceTexture();
    void DownsampleEyeTexture(Texture* src, Texture* dst, const Vector2& offset);
};
