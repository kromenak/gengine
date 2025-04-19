#include "GPSOverlay.h"

#include "AssetManager.h"
#include "IniParser.h"
#include "TextAsset.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIUtil.h"

GPSOverlay::GPSOverlay() : Actor("GPSOverlay", TransformType::RectTransform)
{
    UICanvas* canvas = AddComponent<UICanvas>(-1);
    canvas->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    UIImage* background = UIUtil::NewUIActorWithWidget<UIImage>(this);
    background->SetTexture(gAssetManager.LoadTexture("GPSLER_L.BMP"), true);
    background->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    background->GetRectTransform()->SetAnchoredPosition(2.0f, -2.0f);

    mLonLabel = UIUtil::NewUIActorWithWidget<UILabel>(background->GetOwner());
    mLonLabel->SetFont(gAssetManager.LoadFont("F_GPS_L.FON"));
    mLonLabel->SetText("02°18'46.00\"");
    mLonLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mLonLabel->GetRectTransform()->SetAnchoredPosition(78.0f, -144.0f);

    mLatLabel = UIUtil::NewUIActorWithWidget<UILabel>(background->GetOwner());
    mLatLabel->SetFont(gAssetManager.LoadFont("F_GPS_L.FON"));
    mLatLabel->SetText("02°18'46.00\"");
    mLatLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mLatLabel->GetRectTransform()->SetAnchoredPosition(78.0f, -170.0f);

    mVerticalLine = UIUtil::NewUIActorWithWidget<UIImage>(background->GetOwner());
    mVerticalLine->SetTexture(gAssetManager.LoadTexture("GPSVLINE_L.BMP"), true);
    mVerticalLine->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mVerticalLine->GetRectTransform()->SetAnchoredPosition(18.0f, -8.0f);

    mHorizontalLine = UIUtil::NewUIActorWithWidget<UIImage>(background->GetOwner());
    mHorizontalLine->SetTexture(gAssetManager.LoadTexture("GPSHLINE_L.BMP"), true);
    mHorizontalLine->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mHorizontalLine->GetRectTransform()->SetAnchoredPosition(9.0f, -18.0f);

    mTargetSquare = UIUtil::NewUIActorWithWidget<UIImage>(background->GetOwner());
    mTargetSquare->SetTexture(gAssetManager.LoadTexture("GPSTARGET_L.BMP"), true);
    mTargetSquare->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mTargetSquare->GetRectTransform()->SetAnchoredPosition(9.0f, -8.0f);

    UIImage* powerOffImage = UIUtil::NewUIActorWithWidget<UIImage>(background->GetOwner());
    powerOffImage->SetTexture(gAssetManager.LoadTexture("GPSPOWER_OFF_UP_L.BMP"), true);
    powerOffImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
    powerOffImage->GetRectTransform()->SetAnchoredPosition(-7.0f, 8.0f);

    mPowerButton = UIUtil::NewUIActorWithWidget<UIButton>(background->GetOwner());
    mPowerButton->SetUpTexture(gAssetManager.LoadTexture("GPSPOWER_ON_UP_L.BMP"));
    mPowerButton->SetDownTexture(gAssetManager.LoadTexture("GPSPOWER_ON_DOWN_L.BMP"));
    mPowerButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
    mPowerButton->GetRectTransform()->SetAnchoredPosition(-7.0f, 8.0f);
    mPowerButton->SetPressCallback([this](UIButton* button) {
        Hide();
    });

    //TextAsset* textAsset = gAssetManager.LoadText("BINOCS.TXT", AssetScope::Manual);
    //IniParser parser(textAsset->GetText(), textAsset->GetTextLength());
    //parser.SetMultipleKeyValuePairsPerLine(false);

    // Hide until shown.
    SetActive(false);
}

void GPSOverlay::Show()
{
    SetPoweredOnUIVisible(false);
    SetActive(true);
    mPowerDelayTimer = kPowerDelay;
    mPoweringOn = true;
}

void GPSOverlay::Hide()
{
    SetPoweredOnUIVisible(false);
    mPowerDelayTimer = kPowerDelay;
    mPoweringOn = false;
}

void GPSOverlay::OnUpdate(float deltaTime)
{
    if(mPowerDelayTimer > 0.0f)
    {
        mPowerDelayTimer -= deltaTime;
        if(mPowerDelayTimer <= 0.0f)
        {
            if(mPoweringOn)
            {
                SetPoweredOnUIVisible(true);
            }
            else
            {
                SetActive(false);
            }
        }
    }
}

void GPSOverlay::SetPoweredOnUIVisible(bool visible)
{
    mLonLabel->SetEnabled(visible);
    mLatLabel->SetEnabled(visible);

    mVerticalLine->SetEnabled(visible);
    mHorizontalLine->SetEnabled(visible);
    mTargetSquare->SetEnabled(visible);

    mPowerButton->SetEnabled(visible);
}
