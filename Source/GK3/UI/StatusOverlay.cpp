#include "StatusOverlay.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "InputManager.h"
#include "Localizer.h"
#include "LocationManager.h"
#include "StringUtil.h"
#include "UICanvas.h"
#include "UILabel.h"
#include "UIUtil.h"

StatusOverlay::StatusOverlay() : Actor("StatusOverlay", TransformType::RectTransform)
{
    // Needs to be a canvas so it can render stuff.
    UI::AddCanvas(this, 10);

    // Create status text actor at top of screen.
    mStatusLabel = UI::CreateWidgetActor<UILabel>("Label", this);
    mStatusLabel->SetFont(gAssetManager.LoadFont("F_STATUS_FADE"));
    mStatusLabel->SetVerticalAlignment(VerticalAlignment::Top);

    // Status text is anchored to top of screen with pivot at top-center.
    // The size of this RT is the "hover area" for showing the status text.
    mStatusLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);

    // Make tall enough to fit the text.
    // Reduce horizontal space a bit to give some padding on left/right edges of screen.
    mStatusLabel->GetRectTransform()->SetSizeDelta(-8.0f, 20.0f);

    // Move down a little bit to give padding on top.
    mStatusLabel->GetRectTransform()->SetAnchoredPosition(0.0f, -6.0f);

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

    // Set status label using retrieved location, timeblock, and score data.
    // In one case (map), there is no location name, so exclude the location name if it's invalid!
    std::string statusText;
    if(StringUtil::StartsWithIgnoreCase(locationName, "loc_"))
    {
        statusText = StringUtil::Format("%s, %s", timeblockName.c_str(), scoreText.c_str());
    }
    else
    {
        statusText = StringUtil::Format("%s, %s, %s", locationName.c_str(), timeblockName.c_str(), scoreText.c_str());
    }
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
    color.a = Math::Lerp<uint8_t>(0, 255, t);
    mStatusLabel->SetColor(color);
}
