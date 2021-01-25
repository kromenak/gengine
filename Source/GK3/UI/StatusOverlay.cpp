//
// StatusOverlay.cpp
//
// Clark Kromenaker
//
#include "StatusOverlay.h"

#include "GameProgress.h"
#include "Localizer.h"
#include "LocationManager.h"
#include "Services.h"
#include "StringUtil.h"
#include "UICanvas.h"
#include "UILabel.h"

StatusOverlay::StatusOverlay() : Actor(TransformType::RectTransform)
{
    // Needs to be a canvas so it can render stuff.
    UICanvas* canvas = AddComponent<UICanvas>();
    
    // Take up full screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);
    
    // Create status text actor at top of screen.
    Actor* statusTextActor = new Actor(TransformType::RectTransform);
    mStatusLabel = statusTextActor->AddComponent<UILabel>();
    mStatusLabel->SetFont(Services::GetAssets()->LoadFont("F_STATUS_FADE"));
    mStatusLabel->SetVerticalAlignment(VerticalAlignment::Top);
    canvas->AddWidget(mStatusLabel);
    
    // Status text is anchored to top of screen with pivot at top-center.
    // The size of this RT is the "hover area" for showing the status text.
    RectTransform* statusTextRT = mStatusLabel->GetRectTransform();
    statusTextRT->SetParent(rectTransform);
    
    statusTextRT->SetAnchorMin(Vector2(0.0f, 1.0f));
    statusTextRT->SetAnchorMax(Vector2(1.0f, 1.0f));
    
    // Some care is actually needed so that these numbers result in "pixel perfect" rendering.
    // So, be careful! Maybe we can take care of this as the canvas layer eventually.
    statusTextRT->SetPivot(0.5f, 1.0f);
    statusTextRT->SetSizeDeltaX(-4.0f);
    statusTextRT->SetSizeDeltaY(20.0f);
    
    // Refresh text with latest location, timeblock, score, etc.
    Refresh();
}

void StatusOverlay::Refresh()
{
    std::string locationName = Services::Get<LocationManager>()->GetLocationDisplayName();
    std::string timeblockName = Services::Get<GameProgress>()->GetTimeblockDisplayName();
    
    std::string scoreLoc = Services::Get<Localizer>()->GetText("ScoreText");
    std::string scoreText = StringUtil::Format(scoreLoc,
                                               Services::Get<GameProgress>()->GetScore(),
                                               Services::Get<GameProgress>()->GetMaxScore());
    
    std::string statusText = StringUtil::Format("%s, %s, %s",
                                                locationName.c_str(),
                                                timeblockName.c_str(),
                                                scoreText.c_str());
    mStatusLabel->SetText(statusText);
    
    // Refreshing the text should force the overlay to show.
    mShowTimer = kShowTime;
}

void StatusOverlay::OnUpdate(float deltaTime)
{
    if(mStatusLabel->GetRectTransform()->GetWorldRect().Contains(Services::GetInput()->GetMousePosition()))
    {
        mShowTimer = kShowTime;
    }
    
    if(mShowTimer > 0.0f)
    {
        mShowTimer -= deltaTime;
    }
    
    Color32 color = mStatusLabel->GetColor();
    float t = Math::Clamp(mShowTimer / kStartFadeTime, 0.0f, 1.0f);
    color.SetA(Math::Lerp((unsigned char)0, (unsigned char)255, t));
    mStatusLabel->SetColor(color);
}
