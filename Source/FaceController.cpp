//
// FaceController.cpp
//
// Clark Kromenaker
//
#include "FaceController.h"

#include "stb_image_resize.h"

#include "Animator.h"
#include "CharacterManager.h"
#include "Texture.h"
#include "Random.h"
#include "Services.h"
#include "Scene.h"
#include "StringUtil.h"

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

void FaceController::SetEye(EyeType eyeType, Texture* texture)
{
	if(eyeType == EyeType::Left)
	{
		mCurrentLeftEyeTexture = texture;
	}
	else
	{
		mCurrentRightEyeTexture = texture;
	}
	UpdateFaceTexture();
}

void FaceController::ClearEye(EyeType eyeType)
{
	if(eyeType == EyeType::Left)
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
	GEngine::inst->GetScene()->GetAnimator()->Start(blinkAnim, false, false, nullptr);
}

void FaceController::Blink(const std::string& animName)
{
	Animation* blinkAnim = mCharacterConfig->faceConfig.blinkAnim1;
	if(!StringUtil::EqualsIgnoreCase(blinkAnim->GetNameNoExtension(), animName))
	{
		blinkAnim = mCharacterConfig->faceConfig.blinkAnim2;
		if(!StringUtil::EqualsIgnoreCase(blinkAnim->GetNameNoExtension(), animName))
		{
			//TODO: This seems to be an AssetManager-level warning?
			//TODO: So, maybe OG game finds and applies blink anim indescriminantly (which seems not good, tbh).
			Services::GetReports()->Log("Error", "gk3 animation '" + animName + ".anm' not found.");
			//TODO: This also causes a warning OS dialog to appear (in debug, I guess)
			//TODO: If a non-blink anim is specified, you get a cryptic error about "attempt to apply multiple actions"
			return;
		}
	}
	GEngine::inst->GetScene()->GetAnimator()->Start(blinkAnim, false, false, nullptr);
}

void FaceController::SetEyeJitterEnabled(bool enabled)
{
	mEyeJitterEnabled = enabled;
	if(mEyeJitterEnabled)
	{
		RollEyeJitterTimer();
	}
}

void FaceController::EyeJitter()
{
	//TODO: This eye jitter behavior doesn't seem quite right...
	//TODO: Maybe I have to limit x/y between max, but ALSO only allow at most 2px movement each time?
	//TODO: Or something else...anyway, it can definitely be better.
	float maxX = mCharacterConfig->faceConfig.maxEyeJitterDistance.GetX();
	mEyeJitterX = Random::Range(-maxX, maxX);
	
	float maxY = mCharacterConfig->faceConfig.maxEyeJitterDistance.GetY();
	mEyeJitterY = Random::Range(-maxY, maxY);
	
	UpdateFaceTexture();
}

void FaceController::DoExpression(const std::string& expression)
{
	// Expressions are named as combination of identifier and expression string.
	// E.g. Gabriel Frown becomes GABFROWN.ANM.
	std::string animName = mCharacterConfig->identifier + expression;
	Animation* animation = Services::GetAssets()->LoadAnimation(animName);
	if(animation != nullptr)
	{
		GEngine::inst->GetScene()->GetAnimator()->Start(animation);
	}
	else
	{
		//TODO: This seems to be an AssetManager-level warning?
		//TODO: So, maybe OG game finds and applies expression anim indescriminantly (which seems not good, tbh).
		Services::GetReports()->Log("Error", "gk3 animation '" + animName + ".anm' not found.");
	}
}

void FaceController::SetMood(const std::string& mood)
{
	std::string moodOnName = mCharacterConfig->identifier + mood + "on";
	std::string moodOffName = mCharacterConfig->identifier + mood + "off";
	
	// Make sure mood animations exist.
	Animation* enterAnimation = Services::GetAssets()->LoadAnimation(moodOnName);
	Animation* exitAnimation = Services::GetAssets()->LoadAnimation(moodOffName);
	if(enterAnimation == nullptr || exitAnimation == nullptr)
	{
		//TODO: Log error?
		return;
	}
	
	// Save mood.
	mMood = mood;
	mEnterMoodAnimation = enterAnimation;
	mExitMoodAnimation = exitAnimation;
	
	// Play mood on animation.
	GEngine::inst->GetScene()->GetAnimator()->Start(mEnterMoodAnimation);
}

void FaceController::ClearMood()
{
	// Mood is already clear!
	if(mMood.empty()) { return; }
	
	// Play mood off animation.
	GEngine::inst->GetScene()->GetAnimator()->Start(mExitMoodAnimation);
	
	// Clear mood state.
	mMood.clear();
	mEnterMoodAnimation = nullptr;
	mExitMoodAnimation = nullptr;
}

void FaceController::OnUpdate(float deltaTime)
{
	// Count down and blink after some time.
	mBlinkTimer -= deltaTime;
	if(mBlinkTimer <= 0.0f)
	{
		Blink();
		RollBlinkTimer();
	}
	
	// Update eye jitter.
	if(mEyeJitterEnabled)
	{
		mEyeJitterTimer -= deltaTime;
		if(mEyeJitterTimer <= 0.0f)
		{
			EyeJitter();
			RollEyeJitterTimer();
		}
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

void FaceController::RollEyeJitterTimer()
{
	// Calculate wait milliseconds for next jitter.
	int waitMs = 0;
	if(mCharacterConfig != nullptr)
	{
		const Vector2& jitterFrequency = mCharacterConfig->faceConfig.eyeJitterFrequency;
		waitMs = Random::Range((int)jitterFrequency.GetX(), (int)jitterFrequency.GetY());
	}
	
	// Convert to seconds and set timer.
	mEyeJitterTimer = (float)waitMs / 1000.0f;
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
		//stbir_resize_uint8(mCurrentLeftEyeTexture->GetPixelData(), mCurrentLeftEyeTexture->GetWidth(), mCurrentLeftEyeTexture->GetHeight(), 0,
		//				   mDownSampledLeftEyeTexture->GetPixelData(), mDownSampledLeftEyeTexture->GetWidth(), mDownSampledLeftEyeTexture->GetHeight(), 0, 4);
		 
		//TODO: Am I using the "bias" correctly?
		//TODO: Is CATMULLROM the best filter? Some filters trigger an assertion if the x/y offset become too big...
		//const Vector2& leftEyeBias = mCharacterConfig->faceConfig.leftEyeBias;
		stbir_resize_subpixel(mCurrentLeftEyeTexture->GetPixelData(), mCurrentLeftEyeTexture->GetWidth(), mCurrentLeftEyeTexture->GetHeight(), 0,
							  mDownSampledLeftEyeTexture->GetPixelData(), mDownSampledLeftEyeTexture->GetWidth(), mDownSampledLeftEyeTexture->GetHeight(), 0,
							  STBIR_TYPE_UINT8, 4, -1, 0,
							  STBIR_EDGE_WRAP, STBIR_EDGE_WRAP, STBIR_FILTER_CATMULLROM, STBIR_FILTER_CATMULLROM,
							  STBIR_COLORSPACE_LINEAR, NULL,
							  //0.25f, 0.25f, mEyeJitterX + leftEyeBias.GetX(), mEyeJitterY + leftEyeBias.GetY());
							  0.25f, 0.25f, 0.0f, 0.0f);
		
		mDownSampledLeftEyeTexture->UploadToGPU();
		
		const Vector2& leftEyeOffset = mCharacterConfig->faceConfig.leftEyeOffset;
		Texture::BlendPixels(*mDownSampledLeftEyeTexture, *mFaceTexture, leftEyeOffset.GetX(), leftEyeOffset.GetY());
	}
	
	// Downsample & copy right eye.
	if(mCurrentRightEyeTexture != nullptr)
	{
		//stbir_resize_uint8(mCurrentRightEyeTexture->GetPixelData(), mCurrentRightEyeTexture->GetWidth(), mCurrentRightEyeTexture->GetHeight(), 0,
		//				   mDownSampledRightEyeTexture->GetPixelData(), mDownSampledRightEyeTexture->GetWidth(), mDownSampledRightEyeTexture->GetHeight(), 0, 4);
		
		//TODO: Am I using the "bias" correctly?
		//TODO: Is CATMULLROM the best filter? Some filters trigger an assertion if the x/y offset become too big...
		//const Vector2& rightEyeBias = mCharacterConfig->faceConfig.rightEyeBias;
		stbir_resize_subpixel(mCurrentRightEyeTexture->GetPixelData(), mCurrentRightEyeTexture->GetWidth(), mCurrentRightEyeTexture->GetHeight(), 0,
							  mDownSampledRightEyeTexture->GetPixelData(), mDownSampledRightEyeTexture->GetWidth(), mDownSampledRightEyeTexture->GetHeight(), 0,
							  STBIR_TYPE_UINT8, 4, -1, 0,
							  STBIR_EDGE_WRAP, STBIR_EDGE_WRAP, STBIR_FILTER_CATMULLROM, STBIR_FILTER_CATMULLROM,
							  STBIR_COLORSPACE_LINEAR, NULL,
							  //0.25f, 0.25f, mEyeJitterX + rightEyeBias.GetX(), mEyeJitterY + rightEyeBias.GetY());
							  0.25f, 0.25f, 0.0f, 0.0f);
							  
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
