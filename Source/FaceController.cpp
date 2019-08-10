//
// FaceController.cpp
//
// Clark Kromenaker
//
#include "FaceController.h"

#include "stb_image_resize.h"

#include "AnimationPlayer.h"
#include "CharacterManager.h"
#include "Texture.h"
#include "Random.h"
#include "Services.h"
#include "Scene.h"

TYPE_DEF_CHILD(Component, FaceController);

FaceController::FaceController(Actor* owner) : Component(owner)
{
	mDownSampledLeftEyeTexture = new Texture(25, 26, Color32::Black);
	mDownSampledRightEyeTexture = new Texture(25, 26, Color32::Black);
}

FaceController::~FaceController()
{
	delete mDownSampledLeftEyeTexture;
	delete mDownSampledRightEyeTexture;
}

void FaceController::SetCharacterConfig(const CharacterConfig& characterConfig)
{
	// Save character config.
	mCharacterConfig = &characterConfig;
	
	// Save reference to face texture.
	mFaceTexture = mCharacterConfig->faceConfig.faceTexture;
	
	// Grab references to default mouth/eyelids/forehead textures.
	mDefaultMouthTexture = Services::GetAssets()->LoadTexture(mCharacterConfig->identifier + "_MOUTH00");
	mDefaultEyelidsTexture = mCharacterConfig->faceConfig.eyelidsTexture;
	mDefaultForeheadTexture = mCharacterConfig->faceConfig.foreheadTexture;
	mDefaultLeftEyeTexture = mCharacterConfig->faceConfig.leftEyeTexture;
	mDefaultRightEyeTexture = mCharacterConfig->faceConfig.rightEyeTexture;
	
	// Currents are just the defaults...uhh, by default.
	mCurrentMouthTexture = mDefaultMouthTexture;
	mCurrentEyelidsTexture = mDefaultEyelidsTexture;
	mCurrentForeheadTexture = mDefaultForeheadTexture;
	mCurrentLeftEyeTexture = mDefaultLeftEyeTexture;
	mCurrentRightEyeTexture = mDefaultRightEyeTexture;
	
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
	mCurrentMouthTexture = texture;
	UpdateFaceTexture();
}

void FaceController::ClearMouth()
{
	mCurrentMouthTexture = mDefaultMouthTexture;
	UpdateFaceTexture();
}

void FaceController::SetEyelids(Texture* texture)
{
	mCurrentEyelidsTexture = texture;
	UpdateFaceTexture();
}

void FaceController::ClearEyelids()
{
	mCurrentEyelidsTexture = mDefaultEyelidsTexture;
	UpdateFaceTexture();
}

void FaceController::SetForehead(Texture* texture)
{
	mCurrentForeheadTexture = texture;
	UpdateFaceTexture();
}

void FaceController::ClearForehead()
{
	mCurrentForeheadTexture = mDefaultForeheadTexture;
	UpdateFaceTexture();
}

void FaceController::SetEyes(Texture* texture)
{
	mCurrentLeftEyeTexture = texture;
	mCurrentRightEyeTexture = texture;
	UpdateFaceTexture();
}

void FaceController::ClearEyes()
{
	mCurrentLeftEyeTexture = mDefaultLeftEyeTexture;
	mCurrentRightEyeTexture = mDefaultRightEyeTexture;
	UpdateFaceTexture();
}

void FaceController::SetEye(EyeType type, Texture* texture)
{
	if(type == EyeType::Left)
	{
		mCurrentLeftEyeTexture = texture;
	}
	else
	{
		mCurrentRightEyeTexture = texture;
	}
	UpdateFaceTexture();
}

void FaceController::ClearEye(EyeType type)
{
	if(type == EyeType::Left)
	{
		mCurrentLeftEyeTexture = mDefaultLeftEyeTexture;
	}
	else
	{
		mCurrentRightEyeTexture = mDefaultRightEyeTexture;
	}
	UpdateFaceTexture();
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

void FaceController::UpdateFaceTexture()
{
	// Can't do much if face texture is missing!
	if(mFaceTexture == nullptr) { return; }
	
	// Copy mouth texture.
	if(mCurrentMouthTexture != nullptr)
	{
		const Vector2& mouthOffset = mCharacterConfig->faceConfig.mouthOffset;
		Texture::BlendPixels(*mCurrentMouthTexture, *mFaceTexture, mouthOffset.GetX(), mouthOffset.GetY());
	}
		
	// Downsample & copy left eye.
	if(mCurrentLeftEyeTexture != nullptr)
	{
		stbir_resize_uint8(mCurrentLeftEyeTexture->GetPixelData(), mCurrentLeftEyeTexture->GetWidth(), mCurrentLeftEyeTexture->GetHeight(), 0,
						   mDownSampledLeftEyeTexture->GetPixelData(), mDownSampledLeftEyeTexture->GetWidth(), mDownSampledLeftEyeTexture->GetHeight(), 0, 4);
		mDownSampledLeftEyeTexture->UploadToGPU();
		const Vector2& leftEyeOffset = mCharacterConfig->faceConfig.leftEyeOffset;
		Texture::BlendPixels(*mDownSampledLeftEyeTexture, *mFaceTexture, leftEyeOffset.GetX(), leftEyeOffset.GetY());
	}
	
	// Downsample & copy right eye.
	if(mCurrentRightEyeTexture != nullptr)
	{
		stbir_resize_uint8(mCurrentRightEyeTexture->GetPixelData(), mCurrentRightEyeTexture->GetWidth(), mCurrentRightEyeTexture->GetHeight(), 0,
						   mDownSampledRightEyeTexture->GetPixelData(), mDownSampledRightEyeTexture->GetWidth(), mDownSampledRightEyeTexture->GetHeight(), 0, 4);
		mDownSampledRightEyeTexture->UploadToGPU();
		const Vector2& rightEyeOffset = mCharacterConfig->faceConfig.rightEyeOffset;
		Texture::BlendPixels(*mDownSampledRightEyeTexture, *mFaceTexture, rightEyeOffset.GetX(), rightEyeOffset.GetY());
	}
	
	// Copy eyelids texture.
	if(mCurrentEyelidsTexture != nullptr)
	{
		const Vector2& eyelidsOffset = mCharacterConfig->faceConfig.eyelidsOffset;
		Texture::BlendPixels(*mCurrentEyelidsTexture, *mFaceTexture, eyelidsOffset.GetX(), eyelidsOffset.GetY());
	}
	
	// Copy forehead texture.
	if(mCurrentForeheadTexture != nullptr)
	{
		const Vector2& foreheadOffset = mCharacterConfig->faceConfig.foreheadOffset;
		Texture::BlendPixels(*mCurrentForeheadTexture, *mFaceTexture, foreheadOffset.GetX(), foreheadOffset.GetY());
	}
		
	// Upload all changes to the GPU.
	mFaceTexture->UploadToGPU();
}
