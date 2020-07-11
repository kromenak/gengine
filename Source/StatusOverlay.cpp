//
// StatusOverlay.cpp
//
// Clark Kromenaker
//
#include "StatusOverlay.h"

#include "GameProgress.h"
#include "Localizer.h"
#include "LocationManager.h"
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
    
    RectTransform* statusTextRT = mStatusLabel->GetRectTransform();
    statusTextRT->SetParent(rectTransform);
    statusTextRT->SetPivot(0.0f, 1.0f);
    statusTextRT->SetAnchor(0.0f, 1.0f);
    statusTextRT->SetAnchoredPosition(5.0f, -5.0f);
    
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
}

void StatusOverlay::OnUpdate(float deltaTime)
{
	
}
