#include "StatusOverlay.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "Localizer.h"
#include "LocationManager.h"
#include "StringUtil.h"
#include "UICanvas.h"
#include "UILabel.h"

StatusOverlay::StatusOverlay() : Actor("Status", TransformType::RectTransform)
{
    // Needs to be a canvas so it can render stuff.
    AddComponent<UICanvas>(2);
    
    // Take up full screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);
    
    // Create status text actor at top of screen.
    Actor* statusTextActor = new Actor(TransformType::RectTransform);
    statusTextActor->GetTransform()->SetParent(GetTransform());
    mStatusLabel = statusTextActor->AddComponent<UILabel>();
    mStatusLabel->SetFont(gAssetManager.LoadFont("F_STATUS_FADE"));
    mStatusLabel->SetVerticalAlignment(VerticalAlignment::Top);
    
    // Status text is anchored to top of screen with pivot at top-center.
    // The size of this RT is the "hover area" for showing the status text.
    RectTransform* statusTextRT = mStatusLabel->GetRectTransform();
    statusTextRT->SetParent(rectTransform);

    // Anchor to top-center of screen, stretch to fill horizontal space.
    statusTextRT->SetAnchorMin(Vector2(0.0f, 1.0f));
    statusTextRT->SetAnchorMax(Vector2(1.0f, 1.0f));
    statusTextRT->SetPivot(0.5f, 1.0f);

    // Make tall enough to fit the text.
    statusTextRT->SetSizeDeltaY(20.0f);

    // Reduce horizontal space a bit to give some padding on left/right edges of screen.
    statusTextRT->SetSizeDeltaX(-8.0f);

    // Move down a little bit to give padding on top.
    statusTextRT->SetAnchoredPosition(0.0f, -6.0f);
   
    // Refresh text with latest location, timeblock, score, etc.
    Refresh();
}

void StatusOverlay::Refresh()
{
    std::string locationName = gLocationManager.GetLocationDisplayName();
    std::string timeblockName = gGameProgress.GetTimeblockDisplayName();
    
    std::string scoreLoc = gLocalizer.GetText("ScoreText");
    std::string scoreText = StringUtil::Format(scoreLoc.c_str(),
                                               gGameProgress.GetScore(),
                                               gGameProgress.GetMaxScore());
    
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
    // Force show the status label if mouse is over it.
    if(mStatusLabel->GetRectTransform()->GetWorldRect().Contains(gInputManager.GetMousePosition()))
    {
        mShowTimer = kShowTime;
    }

    // Decrement timer for showing status overlay.
    // BUT don't decrement during action skip, so you can see the point change if you skipped an action.
    if(mShowTimer > 0.0f && !gActionManager.IsSkippingCurrentAction())
    {
        mShowTimer -= deltaTime;
    }

    // Update status color based on time remaining.
    Color32 color = mStatusLabel->GetColor();
    float t = Math::Clamp(mShowTimer / kStartFadeTime, 0.0f, 1.0f);
    color.SetA(Math::Lerp((unsigned char)0, (unsigned char)255, t));
    mStatusLabel->SetColor(color);
}
