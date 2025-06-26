#include "TimeblockScreen.h"

#include "AssetManager.h"
#include "GK3UI.h"
#include "InputManager.h"
#include "LocationManager.h"
#include "SceneManager.h"
#include "Sequence.h"
#include "SoundtrackPlayer.h"
#include "Timeblock.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"

namespace
{
    // Each timeblock screen has an image-based animation that shows the name of the current timeblock (e.g. Day 1, 10AM).
    // Unfortunately, the positioning of that text is NOT consistent for each timeblock. Feels sloppy, but what can you do.
    // So, I'm going to hardcode the offsets for each timeblock for now.
    std::pair<Timeblock, Vector2> timeblockTextPositions[] = {
        { Timeblock(1, 10), Vector2(14.0f, 64.0f) },
        { Timeblock(1, 12), Vector2(14.0f, 63.0f) },
        { Timeblock(1, 14), Vector2(8.0f,  63.0f) },
        { Timeblock(1, 16), Vector2(14.0f, 63.0f) },
        { Timeblock(1, 18), Vector2(14.0f, 64.0f) },
        { Timeblock(2, 2),  Vector2(14.0f, 64.0f) },
        { Timeblock(2, 7),  Vector2(14.0f, 64.0f) },
        { Timeblock(2, 10), Vector2(14.0f, 65.0f) },
        { Timeblock(2, 12), Vector2(14.0f, 64.0f) },
        { Timeblock(2, 14), Vector2(14.0f, 63.0f) },
        { Timeblock(2, 17), Vector2(14.0f, 64.0f) },
        { Timeblock(3, 7),  Vector2(14.0f, 64.0f) },
        { Timeblock(3, 10), Vector2(14.0f, 63.0f) },
        { Timeblock(3, 12), Vector2(13.0f, 64.0f) },
        { Timeblock(3, 15), Vector2(14.0f, 64.0f) },
        { Timeblock(3, 18), Vector2(14.0f, 64.0f) },
        { Timeblock(3, 21), Vector2(14.0f, 64.0f) }
    };

    UIButton* CreateButton(Actor* parent, const std::string& buttonId)
    {
        UIButton* button = UI::CreateWidgetActor<UIButton>(buttonId, parent);
        button->SetUpTexture(gAssetManager.LoadTexture(buttonId + "_U.BMP"));
        button->SetDownTexture(gAssetManager.LoadTexture(buttonId + "_D.BMP"));
        button->SetHoverTexture(gAssetManager.LoadTexture(buttonId + "_H.BMP"));
        button->SetDisabledTexture(gAssetManager.LoadTexture(buttonId + "_X.BMP"));
        return button;
    }
}

TimeblockScreen::TimeblockScreen() : Actor("TimeblockScreen", TransformType::RectTransform)
{
    UI::AddCanvas(this, 20, Color32::Black);

    // Add background image.
    mBackgroundImage = UI::CreateWidgetActor<UIImage>("Background", this);

    // Add timeblock text image.
    mTextImage = UI::CreateWidgetActor<UIImage>("Text", mBackgroundImage);
    mTextImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mTextImage->GetRectTransform()->SetAnchoredPosition(14.0f, 64.0f);

    // Add "continue" button.
    mContinueButton = CreateButton(mBackgroundImage->GetOwner(), "TB_CONT");
    mContinueButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mContinueButton->GetRectTransform()->SetAnchoredPosition(90.0f, 16.0f);
    mContinueButton->SetPressCallback([this](UIButton* button){
        Hide();
        if(mCallback) { mCallback(); }
    });

    // Add "save" button.
    mSaveButton = CreateButton(mBackgroundImage->GetOwner(), "TB_SAVE");
    mSaveButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mSaveButton->GetRectTransform()->SetAnchoredPosition(180.0f, 16.0f);
    mSaveButton->SetPressCallback([](UIButton* button){
        gGK3UI.ShowSaveScreen();
    });
}

void TimeblockScreen::Show(const Timeblock& timeblock, float timer, const std::function<void()>& callback)
{
    mScreenTimer = timer;
    mCallback = callback;

    // Show the screen.
    SetActive(true);

    // Load background image for this timeblock.
    std::string timeblockString = timeblock.ToString();
    mBackgroundImage->SetTexture(gAssetManager.LoadTexture("TBT" + timeblockString + ".BMP"), true);

    // Position the text image. This is unfortunately not consistent for every timeblock!
    mTextImage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
    for(auto& pair : timeblockTextPositions)
    {
        if(pair.first == timeblock)
        {
            mTextImage->GetRectTransform()->SetAnchoredPosition(pair.second);
        }
    }

    // Load sequence containing animation.
    mAnimSequence = gAssetManager.LoadSequence("D" + timeblockString, AssetScope::Scene);
    mTextImage->SetEnabled(mAnimSequence != nullptr);
    mAnimTimer = 0.0f;

    // Populate first image in text animation sequence.
    if(mAnimSequence != nullptr)
    {
        mTextImage->SetTexture(mAnimSequence->GetTexture(0), true);
    }

    // Play "tick tock" sound effect.
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("CLOCKTIMEBLOCK.WAV"));

    // Hide buttons if this screen is on a timer.
    mContinueButton->SetEnabled(mScreenTimer <= 0.0f);
    mSaveButton->SetEnabled(mScreenTimer <= 0.0f);

    // Fade out any soundtrack from the previous scene as well.
    Scene* scene = gSceneManager.GetScene();
    if(scene != nullptr)
    {
        SoundtrackPlayer* stp = scene->GetSoundtrackPlayer();
        if(stp != nullptr)
        {
            stp->StopAll();
        }
    }
}

void TimeblockScreen::Hide()
{
    if(!IsActive()) { return; }
    SetActive(false);

    // If leaving this screen to the MAP location, make sure the driving screen is showing!
    // Important when timeblock change occurs when entering map.
    if(StringUtil::EqualsIgnoreCase(gLocationManager.GetLocation(), "MAP"))
    {
        gGK3UI.ShowDrivingScreen();
    }
}

//int timeblockIndex = 0;
void TimeblockScreen::OnUpdate(float deltaTime)
{
    /*
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RIGHTBRACKET))
    {
        timeblockIndex = Math::Clamp(timeblockIndex + 1, 0, 16);
        Show(timeblockTextPositions[timeblockIndex].first);
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_LEFTBRACKET))
    {
        timeblockIndex = Math::Clamp(timeblockIndex - 1, 0, 16);
        Show(timeblockTextPositions[timeblockIndex].first);
    }

    Vector2 anchoredPos = mTextImage->GetRectTransform()->GetAnchoredPosition();
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_UP))
    {
        anchoredPos.y++;
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_DOWN))
    {
        anchoredPos.y--;
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RIGHT))
    {
        anchoredPos.x++;
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_LEFT))
    {
        anchoredPos.x--;
    }
    mTextImage->GetRectTransform()->SetAnchoredPosition(anchoredPos);
    //std::cout << mBackgroundImage->GetRectTransform()->GetSize() << ", " << mTextImage->GetRectTransform()->GetSize() << ", " << anchoredPos << std::endl;
    */

    // Shortcut key for pressing continue button.
    if(mContinueButton->IsEnabled() && (gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_C) || gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE)))
    {
        mContinueButton->AnimatePress();
    }

    // Pressing "s" is a shortcut for the save button.
    if(mSaveButton->IsEnabled() && gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_S))
    {
        mSaveButton->AnimatePress();
    }

    // Animate the timeblock text sequence.
    if(mAnimSequence != nullptr)
    {
        mAnimTimer += mAnimSequence->GetFramesPerSecond() * deltaTime;

        int frameIndex = Math::Clamp(static_cast<int>(mAnimTimer), 0, mAnimSequence->GetTextureCount() - 1);
        mTextImage->SetTexture(mAnimSequence->GetTexture(frameIndex), true);
    }

    // If a timer was specified, count down until we automatically exit.
    if(mScreenTimer > 0.0f)
    {
        mScreenTimer -= deltaTime;
        if(mScreenTimer <= 0.0f)
        {
            Hide();
            if(mCallback) { mCallback(); }
        }
    }
}