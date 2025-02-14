#include "FingerprintScreen.h"

#include "AssetManager.h"
#include "CursorManager.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"

FingerprintScreen::FingerprintScreen() : Actor("FingerprintScreen", TransformType::RectTransform),
    mLayer("FingerprintLayer")
{
    // Add canvas to render UI elements.
	AddComponent<UICanvas>(1);
	
	// Add black background image that blocks out the scene entirely.
    UIImage* background = AddComponent<UIImage>();
    background->SetTexture(&Texture::Black);
    background->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
    background->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);

    // Add base background image, which shows the fingerprint box up-close.
    UIImage* baseImage = UIUtil::NewUIActorWithWidget<UIImage>(this);
    baseImage->SetTexture(gAssetManager.LoadTexture("FP_BASE.BMP"), true);
	
	// Add exit button to bottom-left corner of screen.
    {
        UIButton* exitButton = UIUtil::NewUIActorWithWidget<UIButton>(baseImage->GetOwner());
        exitButton->SetUpTexture(gAssetManager.LoadTexture("EXITN.BMP"));
        exitButton->SetDownTexture(gAssetManager.LoadTexture("EXITD.BMP"));
        exitButton->SetHoverTexture(gAssetManager.LoadTexture("EXITHOV.BMP"));
        exitButton->SetDisabledTexture(gAssetManager.LoadTexture("EXITDIS.BMP"));
        exitButton->SetPressCallback([this](UIButton* button){
            Hide();
        });
        exitButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        exitButton->GetRectTransform()->SetAnchoredPosition(5.0f, 9.0f);
    }

    // Add brush image along with button to pick up brush.
    {
        mBrushImage = UIUtil::NewUIActorWithWidget<UIImage>(baseImage->GetOwner());
        mBrushImage->SetTexture(gAssetManager.LoadTexture("FP_BRUSH.BMP"), true);
        mBrushImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mBrushImage->GetRectTransform()->SetAnchoredPosition(128.0f, 90.0f);

        UIButton* brushButton = mBrushImage->GetOwner()->AddComponent<UIButton>();
        brushButton->SetPressCallback([this](UIButton* button){
            OnBrushButtonPressed();
        });
    }

    // Add button for dipping brush in the ink area.
    {
        UIButton* inkButton = UIUtil::NewUIActorWithWidget<UIButton>(baseImage->GetOwner());
        inkButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        inkButton->GetRectTransform()->SetAnchoredPosition(149.0f, 480.0f - 337.0f);
        inkButton->GetRectTransform()->SetSizeDelta(127.0f, 108.0f);
        inkButton->SetPressCallback([this](UIButton* button){
            OnInkButtonPressed();
        });
    }

    // Create an actor that takes up the whole right-panel area of the interface.
    // This will be the parent of the fingerprint object's image.
    Actor* rightPanelActor = new Actor(TransformType::RectTransform);
    rightPanelActor->GetTransform()->SetParent(baseImage->GetRectTransform());
    rightPanelActor->GetComponent<RectTransform>()->SetAnchor(AnchorPreset::TopRight);
    rightPanelActor->GetComponent<RectTransform>()->SetAnchoredPosition(-2.0f, -2.0f);
    rightPanelActor->GetComponent<RectTransform>()->SetSizeDelta(249.0f, 476.0f);

    // Add image to show the object being fingerprinted.
    mFingerprintObjectImage = UIUtil::NewUIActorWithWidget<UIImage>(rightPanelActor);
    mFingerprintObjectImage->SetTexture(&Texture::Black);
    mFingerprintObjectImage->GetRectTransform()->SetAnchor(AnchorPreset::Center);
    mFingerprintObjectImage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);

    // UNFORTUNATELY, this is another spot where the logic doesn't seem data-driven.
    // So, we'll enumerate all the possible fingerprint instances here and nowwww.....
    {
        FingerprintObject& object = mObjects["HAND_MIRROR"];
        object.textureName = "FP_LHOMIR.BMP";
    }
    {
        FingerprintObject& object = mObjects["BOOK_IN_DRAWER"];
        object.textureName = "FP_BOOKIMMORTALS.BMP";
    }
    {
        FingerprintObject& object = mObjects["BLOODLINE_MANUSCRIPT"];
        object.textureName = "FP_BLOMAN.BMP";
    }
    {
        FingerprintObject& object = mObjects["LSR_ENVELOPE_INV"];
        object.textureName = "FP_LSRENV.BMP";
    }
    {
        FingerprintObject& object = mObjects["HBHG_BOOK"];
        object.textureName = "FP_HOLYGBOOK.BMP";
    }
    {
        FingerprintObject& object = mObjects["DIRTY_GLASS_BUCHELLI"];
        object.textureName = "FP_SQRSHOT.BMP";
    }
    {
        FingerprintObject& object = mObjects["DIRTY_GLASS_WILKES"];
        object.textureName = "FP_OCTSHOT.BMP";
    }
    {
        FingerprintObject& object = mObjects["DIRTY_WINE_GLASS_BUCHELLI"];
        object.textureName = "FP_BUCHGLASS.BMP";
    }
    {
        FingerprintObject& object = mObjects["GLASS"];
        object.textureName = "FP_GLASS.BMP";
    }
    {
        FingerprintObject& object = mObjects["WATER_BOTTLE_ON_MOPED"];
        object.textureName = "FP_WATBTL.BMP";
    }
    {
        FingerprintObject& object = mObjects["CIG_PACK_IN_DRAWER"];
        object.textureName = "FP_CIGS.BMP";
    }
    {
        FingerprintObject& object = mObjects["SUITCASE"];
        object.textureName = "FP_SUITCA.BMP";
    }
    {
        FingerprintObject& object = mObjects["POP_BOTTLE"];
        object.textureName = "FP_SODA.BMP";
    }
    {
        FingerprintObject& object = mObjects["JESUS_PICTURE"];
        object.textureName = "FP_JESUS.BMP";
    }
    {
        FingerprintObject& object = mObjects["GUN_IN_CASE"];
        object.textureName = "FP_COLT45.BMP";
    }

	// Hide by default.
    SetActive(false);
}

void FingerprintScreen::Show(const std::string& nounName)
{
    gLayerManager.PushLayer(&mLayer);

    auto it = mObjects.find(nounName);
    if(it != mObjects.end())
    {
        mFingerprintObjectImage->SetTexture(gAssetManager.LoadTexture(it->second.textureName), true);
    }
    else
    {
        mFingerprintObjectImage->SetTexture(&Texture::Black);
    }

	// Actually show the stuff!
	SetActive(true);
}

void FingerprintScreen::Hide()
{
    if(!IsActive()) { return; }
	SetActive(false);
    gLayerManager.PopLayer(&mLayer);
}

bool FingerprintScreen::IsShowing() const
{
    return gLayerManager.IsTopLayer(&mLayer);
}

void FingerprintScreen::OnUpdate(float deltaTime)
{
    if(mCursorState == CursorState::CleanBrush)
    {
        gCursorManager.UseCustomCursor(gAssetManager.LoadCursor("C_FPBRUSH.BMP"));
    }
    else if(mCursorState == CursorState::InkedBrush)
    {
        gCursorManager.UseCustomCursor(gAssetManager.LoadCursor("C_FPBRUSH_WDUST.BMP"));
    }
    else
    {
        gCursorManager.UseDefaultCursor();
    }
}

void FingerprintScreen::PickUpBrush()
{
    mCursorState = CursorState::CleanBrush;
    mBrushImage->SetEnabled(false);
}

void FingerprintScreen::PutDownBrush()
{
    mCursorState = CursorState::Normal;
    mBrushImage->SetEnabled(true);
}

void FingerprintScreen::OnBrushButtonPressed()
{
    if(mCursorState == CursorState::Normal)
    {
        PickUpBrush();
    }
    else if(mCursorState == CursorState::CleanBrush || mCursorState == CursorState::InkedBrush)
    {
        PutDownBrush();
    }
}

void FingerprintScreen::OnInkButtonPressed()
{
    if(mCursorState == CursorState::CleanBrush)
    {
        mCursorState = CursorState::InkedBrush;
    }
}