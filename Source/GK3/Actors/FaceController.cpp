#include "FaceController.h"

#include <stb_image_resize.h>

#include "Animation.h"
#include "Animator.h"
#include "AssetManager.h"
#include "CharacterManager.h"
#include "Texture.h"
#include "Random.h"
#include "ReportManager.h"
#include "SceneManager.h"
#include "StringUtil.h"

TYPEINFO_INIT(FaceController, Component, 6)
{
    TYPEINFO_VAR(FaceController, VariableType::Float, mBlinkTimer);
    TYPEINFO_VAR(FaceController, VariableType::Bool, mEyeJitterEnabled);
    TYPEINFO_VAR(FaceController, VariableType::Float, mEyeJitterTimer);
    TYPEINFO_VAR(FaceController, VariableType::Vector2, mEyeJitterOffset);
    TYPEINFO_VAR(FaceController, VariableType::String, mMood);
}

FaceController::FaceController(Actor* owner) : Component(owner)
{
    mDownsampledLeftEyeTexture = new Texture(kDownSampledEyeWidth, kDownSampledEyeHeight, Color32::Black, Texture::Format::RGB);
    mDownsampledRightEyeTexture = new Texture(kDownSampledEyeWidth, kDownSampledEyeHeight, Color32::Black, Texture::Format::RGB);
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
        case FaceElement::LeftEye:
            SetEye(EyeType::Left, texture);
            break;
        case FaceElement::RightEye:
            SetEye(EyeType::Right, texture);
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
        case FaceElement::LeftEye:
            ClearEye(EyeType::Left);
            break;
        case FaceElement::RightEye:
            ClearEye(EyeType::Right);
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

    // If an eyelid texture is explicitly set, this disables the blink behavior.
    // The eyelid texture must be cleared to re-enable blinking.
    mBlinkEnabled = false;
}

void FaceController::ClearEyelids()
{
    mCurrentEyelidsTexture = mDefaultEyelidsTexture;
    UpdateFaceTexture();

    // Upon clearing eyelid texture, re-enable blinking.
    mBlinkEnabled = true;
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
    // The eye jitter is defined in sub-pixels.
    // Since we downsample to 25% size, this means each subpixel is 1/4th of a pixel.
    float maxX = mCharacterConfig->faceConfig->maxEyeJitterDistance.x / 4.0f;
    float maxY = mCharacterConfig->faceConfig->maxEyeJitterDistance.y / 4.0f;
    mEyeJitterOffset = Vector2(Random::Range(-maxX, maxX),
                               Random::Range(-maxY, maxY));

    // Changing the eye jitter causes the face to change - update it.
    UpdateFaceTexture();
}

void FaceController::DoExpression(const std::string& expression)
{
    // Expressions are named as combination of identifier and expression string.
    // E.g. Gabriel Frown becomes GABFROWN.ANM.
    std::string animName = mCharacterConfig->faceConfig->identifier + expression;
    Animation* animation = gAssetManager.LoadAnimation(animName, AssetScope::Scene);
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
    Animation* enterAnimation = gAssetManager.LoadAnimation(moodOnName, AssetScope::Scene);
    Animation* exitAnimation = gAssetManager.LoadAnimation(moodOffName, AssetScope::Scene);
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
    if(mBlinkEnabled)
    {
        mBlinkTimer -= deltaTime;
        if(mBlinkTimer <= 0.0f)
        {
            Blink();
            RollBlinkTimer();
        }
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

    // Copy eye textures.
    UpdateEyeOnFaceTexture(mCurrentLeftEyeTexture, mDownsampledLeftEyeTexture, mCharacterConfig->faceConfig->leftEyeOffset, mCharacterConfig->faceConfig->leftEyeBias);
    UpdateEyeOnFaceTexture(mCurrentRightEyeTexture, mDownsampledRightEyeTexture, mCharacterConfig->faceConfig->rightEyeOffset, mCharacterConfig->faceConfig->rightEyeBias);

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

void FaceController::UpdateEyeOnFaceTexture(Texture* eyeTexture, Texture* downsampledTexture, const Vector2& offset, const Vector2& bias)
{
    if(eyeTexture == nullptr || downsampledTexture == nullptr || mFaceTexture == nullptr) { return; }

    // Before downsampling the 100x104 eye texture to 25% resolution, we need to calculate an "offset" for the downsample.
    // This offset allows us to adjust the eye position at the sub-pixel level. During downsample, the offset "nudges" the eye slightly left/right/down/up.

    // After some experimentation, I found that with no bias and no eye jitter, the value (0.5, 1) matches what you'd see in the original game.
    Vector2 downsampleOffset(0.5f, 1.0f);

    // Apply the bias from the character's config file. This can be thought of as a global nudge from the initial/default offset.
    // From testing, this value needs to be subtracted, rather than added, to get the correct result (likely a mismatch between their conventions and sbt's conventions).
    downsampleOffset -= bias;

    // Eye jitter is an additional offset, which gets recalculated every few seconds. Add that as well.
    // (It may be that the most correct would be to also subtract this, but it's already randomized, so it doesn't really matter...)
    downsampleOffset += mEyeJitterOffset;

    // Downsample the eye to the smaller texture.
    DownsampleEyeTexture(eyeTexture, downsampledTexture, downsampleOffset);

    // HACK: Due to the way we downsample textures, there's a limit on how much affect the "downsample offset" has.
    // HACK: In at least one case (Prince James), the offset is high enough that even with the maximum downsample offset, he still looks goofy.
    // HACK: To get around this, we can apply extra pixel offset to compensate if the offset is beyond some maximums.
    // TODO: There's probably a more generalized solution here (e.g. if downsampleOffset is beyond some limits, convert to pixel offsets).
    float extraOffsetX = 0.0f;
    if(bias.x <= -2.0f)
    {
        extraOffsetX = bias.x + 2.0f;
    }

    // In some cases (Buthane), the eye texture is slightly larger than the eyelids texture, so you get an "under eye face paint" effect.
    // To fix this, don't write any eye texture pixels that would extend beyond the eyelid texture height.
    uint32_t blendHeight = downsampledTexture->GetHeight();
    if(mCurrentEyelidsTexture != nullptr && mCurrentEyelidsTexture->GetHeight() < blendHeight)
    {
        blendHeight = mCurrentEyelidsTexture->GetHeight();
    }

    // Slap the downsampled texture onto the face texture at the correct pixel offset.
    Texture::BlendPixels(*downsampledTexture, 0, 0, downsampledTexture->GetWidth(), blendHeight,
                         *mFaceTexture, offset.x + extraOffsetX, offset.y);
}

void FaceController::DownsampleEyeTexture(Texture* src, Texture* dst, const Vector2& offset)
{
    // The downsample scale can be derived from the sizes of the source and dest textures.
    float scaleX = static_cast<float>(dst->GetWidth()) / static_cast<float>(src->GetWidth());
    float scaleY = static_cast<float>(dst->GetHeight()) / static_cast<float>(src->GetHeight());

    // The offset is a normalized value that affects what set of subpixels are used during downscaling.
    // For example, if we're downscaling to 25%, each pixel in the dest texture corresponds to a 4x4 block of pixels in the source texture.
    // The pixel in the dest texture can average all those pixels equally (a 0,0 offset), or we can nudge it left/right/up/down a bit with the offset.
    //
    // It's important to clamp this to -1 to 1. Using values outside this range will actually crash the game (seems like memory corruption in stb).
    // Somewhat unintuitively, -1.0f corresponds to further down/right, and 1.0f corresponds to further up/left.
    float offsetX = Math::Clamp(offset.x, -1.0f, 1.0f);
    float offsetY = Math::Clamp(offset.y, -1.0f, 1.0f);

    // Resize source image to fit into dst.
    stbir_resize_subpixel(src->GetPixelData(), src->GetWidth(), src->GetHeight(), 0,    // input image data
                          dst->GetPixelData(), dst->GetWidth(), dst->GetHeight(), 0,    // output image data
                          STBIR_TYPE_UINT8,                                             // data type
                          src->GetBytesPerPixel(),                                      // number of channels
                          -1,                                                           // alpha channel; ignore
                          0,                                                            // flags
                          STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,                           // horizontal & vertical edge mode
                          STBIR_FILTER_CATMULLROM, STBIR_FILTER_CATMULLROM,             // horizontal & vertical filter mode
                          STBIR_COLORSPACE_LINEAR,                                      // colorspace
                          nullptr,                                                      // alloc context (???)
                          scaleX, scaleY,                                               // x/y scale
                          offsetX, offsetY);                                            // x/y offset
}