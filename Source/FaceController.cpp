//
// FaceController.cpp
//
// Clark Kromenaker
//
#include "FaceController.h"

#include "AnimationPlayer.h"
#include "CharacterManager.h"
#include "Texture.h"
#include "Random.h"
#include "Services.h"
#include "Scene.h"

TYPE_DEF_CHILD(Component, FaceController);

FaceController::FaceController(Actor* owner) : Component(owner)
{
	
}

void FaceController::SetCharacterConfig(const CharacterConfig& characterConfig)
{
	// Save character config.
	mCharacterConfig = &characterConfig;
	
	// Grab references to default mouth/eyelids/forehead textures.
	mDefaultMouthTexture = Services::GetAssets()->LoadTexture(mCharacterConfig->identifier + "_MOUTH00");
	mDefaultEyelidsTexture = mCharacterConfig->faceConfig.eyelidsTexture;
	mDefaultForeheadTexture = mCharacterConfig->faceConfig.foreheadTexture;

	// Roll a random blink time in the near future.
	RollBlinkTimer();
}

void FaceController::Set(FaceElement element, Texture* texture)
{
	switch(element)
	{
		case FaceElement::Mouth:
			SetMouth(texture);
			break;
		case FaceElement::Eyelids:
			SetEyelids(texture);
			break;
		case FaceElement::Forehead:
			SetForehead(texture);
			break;
	}
}

void FaceController::Clear(FaceElement element)
{
	switch(element)
	{
		case FaceElement::Mouth:
			ClearMouth();
			break;
		case FaceElement::Eyelids:
			ClearEyelids();
			break;
		case FaceElement::Forehead:
			ClearForehead();
			break;
	}
}

void FaceController::SetMouth(Texture* texture)
{
	const Vector2& mouthOffset = mCharacterConfig->faceConfig.mouthOffset;
	mCharacterConfig->faceConfig.faceTexture->Blit(texture, mouthOffset.GetX(), mouthOffset.GetY());
}

void FaceController::ClearMouth()
{
	const Vector2& mouthOffset = mCharacterConfig->faceConfig.mouthOffset;
	mCharacterConfig->faceConfig.faceTexture->Blit(mDefaultMouthTexture, mouthOffset.GetX(), mouthOffset.GetY());
}

void FaceController::SetEyelids(Texture* texture)
{
	const Vector2& eyelidsOffset = mCharacterConfig->faceConfig.eyelidsOffset;
	mCharacterConfig->faceConfig.faceTexture->Blit(texture, eyelidsOffset.GetX(), eyelidsOffset.GetY());
}

void FaceController::ClearEyelids()
{
	const Vector2& eyelidsOffset = mCharacterConfig->faceConfig.eyelidsOffset;
	mCharacterConfig->faceConfig.faceTexture->Blit(mDefaultEyelidsTexture, eyelidsOffset.GetX(), eyelidsOffset.GetY());
}

void FaceController::SetForehead(Texture* texture)
{
	const Vector2& foreheadOffset = mCharacterConfig->faceConfig.foreheadOffset;
	mCharacterConfig->faceConfig.faceTexture->Blit(texture, foreheadOffset.GetX(), foreheadOffset.GetY());
}

void FaceController::ClearForehead()
{
	const Vector2& foreheadOffset = mCharacterConfig->faceConfig.foreheadOffset;
	mCharacterConfig->faceConfig.faceTexture->Blit(mDefaultForeheadTexture, foreheadOffset.GetX(), foreheadOffset.GetY());
}

void FaceController::Blink()
{
	Animation* blinkAnim = nullptr;
	
	// We're really gonna need a config for this.
	if(mCharacterConfig != nullptr)
	{
		// Get a random number from 0-100.
		// Each blink anim has a probability between 0 and 100.
		// So, we use this to decide which one to do.
		int rand = Random::Range(0, 101);
		if(rand < mCharacterConfig->faceConfig.blinkAnim1Probability)
		{
			blinkAnim = mCharacterConfig->faceConfig.blinkAnim1;
		}
		else
		{
			blinkAnim = mCharacterConfig->faceConfig.blinkAnim2;
		}
	}
	
	// Play it if you got it!
	GEngine::inst->GetScene()->GetAnimationPlayer()->Play(blinkAnim);
}

void FaceController::UpdateInternal(float deltaTime)
{
	mBlinkTimer -= deltaTime;
	if(mBlinkTimer <= 0.0f)
	{
		Blink();
		RollBlinkTimer();
	}
}

void FaceController::RollBlinkTimer()
{
	// Calculate wait milliseconds for next blink.
	int waitMs = 8000;
	if(mCharacterConfig != nullptr)
	{
		const Vector2& blinkFrequency = mCharacterConfig->faceConfig.blinkFrequency;
		waitMs = Random::Range((int)blinkFrequency.GetX(), (int)blinkFrequency.GetY());
	}
	
	// Convert to seconds and set timer.
	mBlinkTimer = (float)waitMs / 1000.0f;
}
