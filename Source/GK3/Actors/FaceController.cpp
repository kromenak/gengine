#include "FaceController.h"

#include "stb_image_resize.h"

#include "Animation.h"
#include "Animator.h"
#include "AssetManager.h"
#include "CharacterManager.h"
#include "Texture.h"
#include "Random.h"
#include "ReportManager.h"
#include "SceneManager.h"
#include "StringUtil.h"

TYPE_DEF_CHILD(Component, FaceController);

FaceController::FaceController(Actor* owner) : Component(owner)
{
	mDownsampledLeftEyeTexture = new Texture(kDownSampledEyeWidth, kDownSampledEyeHeight, Color32::Black);
	mDownsampledRightEyeTexture = new Texture(kDownSampledEyeWidth, kDownSampledEyeHeight, Color32::Black);
}

FaceController::~FaceController()
{
	delete mDownsampledLeftEyeTexture;
	delete mDownsampledRightEyeTexture;
}

void FaceController::SetCharacterConfig(const CharacterConfig& characterConfig)
{
	// Save character config.
	mCharacterConfig = &characterConfig;

	// Save reference to face texture.
	mFaceTexture = mCharacterConfig->faceConfig->faceTexture;
	
	// Grab references to default mouth/eyelids/forehead textures.
    mDefaultMouthTexture = mCharacterConfig->faceConfig->mouthTexture;
	mDefaultEyelidsTexture = mCharacterConfig->faceConfig->eyelidsTexture;
	mDefaultForeheadTexture = mCharacterConfig->faceConfig->foreheadTexture;
	mDefaultLeftEyeTexture = mCharacterConfig->faceConfig->leftEyeTexture;
	mDefaultRightEyeTexture = mCharacterConfig->faceConfig->rightEyeTexture;
	
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
		if(rand < mCharacterConfig->faceConfig->blinkAnim1Probability)
		{
			blinkAnim = mCharacterConfig->faceConfig->blinkAnim1;
		}
		else
		{
			blinkAnim = mCharacterConfig->faceConfig->blinkAnim2;
		}
	}
	
	// Play it if you got it!
	gSceneManager.GetScene()->GetAnimator()->Start(blinkAnim);
}

void FaceController::Blink(const std::string& animName)
{
	Animation* blinkAnim = mCharacterConfig->faceConfig->blinkAnim1;
	if(!StringUtil::EqualsIgnoreCase(blinkAnim->GetNameNoExtension(), animName))
	{
		blinkAnim = mCharacterConfig->faceConfig->blinkAnim2;
		if(!StringUtil::EqualsIgnoreCase(blinkAnim->GetNameNoExtension(), animName))
		{
			//TODO: This seems to be an AssetManager-level warning?
			//TODO: So, maybe OG game finds and applies blink anim indescriminantly (which seems not good, tbh).
			gReportManager.Log("Error", "gk3 animation '" + animName + ".anm' not found.");
			//TODO: This also causes a warning OS dialog to appear (in debug, I guess)
			//TODO: If a non-blink anim is specified, you get a cryptic error about "attempt to apply multiple actions"
			return;
		}
	}
	gSceneManager.GetScene()->GetAnimator()->Start(blinkAnim);
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
	float maxX = mCharacterConfig->faceConfig->maxEyeJitterDistance.x;
	mEyeJitterX = Random::Range(-maxX, maxX);
	
	float maxY = mCharacterConfig->faceConfig->maxEyeJitterDistance.y;
	mEyeJitterY = Random::Range(-maxY, maxY);
	
	UpdateFaceTexture();
}

void FaceController::DoExpression(const std::string& expression)
{
	// Expressions are named as combination of identifier and expression string.
	// E.g. Gabriel Frown becomes GABFROWN.ANM.
	std::string animName = mCharacterConfig->faceConfig->identifier + expression;
	Animation* animation = gAssetManager.LoadAnimation(animName);
	if(animation != nullptr)
	{
		gSceneManager.GetScene()->GetAnimator()->Start(animation);
	}
	else
	{
		//TODO: This seems to be an AssetManager-level warning?
		//TODO: So, maybe OG game finds and applies expression anim indescriminantly (which seems not good, tbh).
		gReportManager.Log("Error", "gk3 animation '" + animName + ".anm' not found.");
	}
}

void FaceController::SetMood(const std::string& mood)
{
	std::string moodOnName = mCharacterConfig->faceConfig->identifier + mood + "on";
	std::string moodOffName = mCharacterConfig->faceConfig->identifier + mood + "off";
	
	// Make sure mood animations exist.
	Animation* enterAnimation = gAssetManager.LoadAnimation(moodOnName);
	Animation* exitAnimation = gAssetManager.LoadAnimation(moodOffName);
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
	gSceneManager.GetScene()->GetAnimator()->Start(mEnterMoodAnimation);
}

void FaceController::ClearMood()
{
	// Mood is already clear!
	if(mMood.empty()) { return; }
	
	// Play mood off animation.
	gSceneManager.GetScene()->GetAnimator()->Start(mExitMoodAnimation);
	
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
		const Vector2& blinkFrequency = mCharacterConfig->faceConfig->blinkFrequency;
		waitMs = Random::Range((int)blinkFrequency.x, (int)blinkFrequency.y);
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
		const Vector2& jitterFrequency = mCharacterConfig->faceConfig->eyeJitterFrequency;
		waitMs = Random::Range((int)jitterFrequency.x, (int)jitterFrequency.y);
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
		const Vector2& mouthOffset = mCharacterConfig->faceConfig->mouthOffset;
		Texture::BlendPixels(*mCurrentMouthTexture, *mFaceTexture, mouthOffset.x, mouthOffset.y);
	}
		
	// Downsample & copy left eye.
	if(mCurrentLeftEyeTexture != nullptr)
	{
        Vector2 offset;
        DownsampleEyeTexture(mCurrentLeftEyeTexture, mDownsampledLeftEyeTexture, offset);

		const Vector2& leftEyeOffset = mCharacterConfig->faceConfig->leftEyeOffset;
		Texture::BlendPixels(*mDownsampledLeftEyeTexture, *mFaceTexture, leftEyeOffset.x, leftEyeOffset.y);
	}
	
	// Downsample & copy right eye.
	if(mCurrentRightEyeTexture != nullptr)
	{
        Vector2 offset;
        DownsampleEyeTexture(mCurrentRightEyeTexture, mDownsampledRightEyeTexture, offset);

		const Vector2& rightEyeOffset = mCharacterConfig->faceConfig->rightEyeOffset;
		Texture::BlendPixels(*mDownsampledRightEyeTexture, *mFaceTexture, rightEyeOffset.x, rightEyeOffset.y);
	}
	
	// Copy eyelids texture.
	if(mCurrentEyelidsTexture != nullptr)
	{
		const Vector2& eyelidsOffset = mCharacterConfig->faceConfig->eyelidsOffset;
		Texture::BlendPixels(*mCurrentEyelidsTexture, *mFaceTexture, eyelidsOffset.x, eyelidsOffset.y);
	}
	
	// Copy forehead texture.
	if(mCurrentForeheadTexture != nullptr)
	{
		const Vector2& foreheadOffset = mCharacterConfig->faceConfig->foreheadOffset;
		Texture::BlendPixels(*mCurrentForeheadTexture, *mFaceTexture, foreheadOffset.x, foreheadOffset.y);
	}
		
	// Upload all changes to the GPU.
	mFaceTexture->UploadToGPU();
}

void FaceController::DownsampleEyeTexture(Texture* src, Texture* dst, const Vector2& offset)
{
    // Resize source image to fit into dst.
    stbir_resize_subpixel(src->GetPixelData(), src->GetWidth(), src->GetHeight(), 0,    // input image data
                          dst->GetPixelData(), dst->GetWidth(), dst->GetHeight(), 0,    // output image data
                          STBIR_TYPE_UINT8,                                             // data type
                          4,                                                            // number of channels; these textures are RGBA
                          -1,                                                           // alpha channel; ignore
                          0,                                                            // flags
                          STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,                           // horizontal & vertical edge mode
                          STBIR_FILTER_CATMULLROM, STBIR_FILTER_CATMULLROM,             // horizontal & vertical filter mode
                          STBIR_COLORSPACE_LINEAR,                                      // colorspace
                          nullptr,                                                      // alloc context (???)
                          0.25f, 0.25f,                                                 // x/y scale
                          offset.x, offset.y);                                          // x/y offset
}