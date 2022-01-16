#include "TimeblockScreen.h"

#include "Timeblock.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"

// Each timeblock screen has an image-based animation that shows the name of the current timeblock (e.g. Day 1, 10AM).
// Unfortunately, the positioning of that text is NOT consistent for each timeblock. Feels sloppy, but what can you do.
// So, I'm going to hardcode the offsets for each timeblock for now.
static std::pair<Timeblock, Vector2> timeblockTextPositions[] = {
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

TimeblockScreen::TimeblockScreen() : Actor(Actor::TransformType::RectTransform)
{
    UICanvas* canvas = AddComponent<UICanvas>(1);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Add a black background that covers the entire canvas.
    UIImage* backgroundImage = AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetColor(Color32::Black);

    // Add background image.
    Actor* backgroundImageActor = new Actor(Actor::TransformType::RectTransform);
    mBackgroundImage = backgroundImageActor->AddComponent<UIImage>();
    canvas->AddWidget(mBackgroundImage);

    // Add timeblock text image.
    Actor* textActor = new Actor(Actor::TransformType::RectTransform);
    textActor->GetTransform()->SetParent(backgroundImageActor->GetTransform());
    mTextImage = textActor->AddComponent<UIImage>();
    canvas->AddWidget(mTextImage);

    mTextImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    mTextImage->GetRectTransform()->SetPivot(0.0f, 0.0f);
    mTextImage->GetRectTransform()->SetAnchoredPosition(14.0f, 64.0f);
}

void TimeblockScreen::Show(const Timeblock& timeblock, std::function<void()> callback)
{
    mCallback = callback;

    SetActive(true);

    std::string timeblockString = timeblock.ToString();
    mBackgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture("TBT" + timeblockString + ".BMP"));

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
    mAnimSequence = Services::GetAssets()->LoadSequence("D" + timeblockString);
    mTextImage->SetEnabled(mAnimSequence != nullptr);

    // Populate first image in text animation sequence.
    if(mAnimSequence != nullptr)
    {
        mTextImage->SetTextureAndSize(mAnimSequence->GetTexture(0));
    }

    // Play "tick tock" sound effect.
    Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("CLOCKTIMEBLOCK.WAV"), nullptr);

    // Reset timers.
    mAnimTimer = 0.0f;
    mScreenTimer = 0.0f;
}

void TimeblockScreen::Hide()
{
    SetActive(false);
}

//int timeblockIndex = 0;
void TimeblockScreen::OnUpdate(float deltaTime)
{
    /*
    if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_RIGHTBRACKET))
    {
        timeblockIndex = Math::Clamp(timeblockIndex + 1, 0, 16);
        Show(timeblockTextPositions[timeblockIndex].first);
    }
    else if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_LEFTBRACKET))
    {
        timeblockIndex = Math::Clamp(timeblockIndex - 1, 0, 16);
        Show(timeblockTextPositions[timeblockIndex].first);
    }
    
    Vector2 anchoredPos = mTextImage->GetRectTransform()->GetAnchoredPosition();
    if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_UP))
    {
        anchoredPos.y++;
    }
    else if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_DOWN))
    {
        anchoredPos.y--;
    }
    else if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_RIGHT))
    {
        anchoredPos.x++;
    }
    else if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_LEFT))
    {
        anchoredPos.x--;
    }
    mTextImage->GetRectTransform()->SetAnchoredPosition(anchoredPos);
    //std::cout << mBackgroundImage->GetRectTransform()->GetSize() << ", " << mTextImage->GetRectTransform()->GetSize() << ", " << anchoredPos << std::endl;
    */

    // Animate the timeblock text sequence.
    if(mAnimSequence != nullptr)
    {
        mAnimTimer += mAnimSequence->GetFramesPerSecond() * deltaTime;

        int frameIndex = Math::Clamp(static_cast<int>(mAnimTimer), 0, mAnimSequence->GetTextureCount() - 1);
        mTextImage->SetTextureAndSize(mAnimSequence->GetTexture(frameIndex));
    }

    // Track how long we have been on this screen.
    // The timeblock screen automatically exits after enough time has passed.
    mScreenTimer += deltaTime;
    if(mScreenTimer > 5.0f)
    {
        Hide();
        if(mCallback) { mCallback(); }
    }
}