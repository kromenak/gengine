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
        object.uncoverPrintLicensePlate = "2O8A805PF1";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_LHOMIR_P1.BMP";
        object.fingerprints.back().position = Vector2(162.0f, 132.0f);
        object.fingerprints.back().invItemName = "HOWARDS_FINGERPRINT";
    }
    {
        FingerprintObject& object = mObjects["BOOK_IN_DRAWER"];
        object.textureName = "FP_BOOKIMMORTALS.BMP";
        object.uncoverPrintLicensePlate = "08BBY59411";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BOOKIMMORTALS_P1.BMP";
        object.fingerprints.back().position = Vector2(113.0f, 144.0f);
        object.fingerprints.back().invItemName = "MONTREAUX_FINGERPRINT";
    }
    {
        FingerprintObject& object = mObjects["BLOODLINE_MANUSCRIPT"];
        object.textureName = "FP_BLOMAN.BMP";
        object.collectPrintLicensePlate = "1037H59291";

        // The first fingerprint, collected during 302A.
        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BLOMAN_P6.BMP";
        object.fingerprints.back().position = Vector2(206.0f, 310.0f);
        object.fingerprints.back().invItemName = "LARRYS_FINGERPRINT";

        // The next three fingerprints, collected during 312P.
        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BLOMAN_P1.BMP";
        object.fingerprints.back().position = Vector2(202.0f, 256.0f);
        object.fingerprints.back().invItemName = "UNKNOWN_PRINT_1";
        // Uncover VO: 1EP02593L1

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BLOMAN_P2.BMP";
        object.fingerprints.back().position = Vector2(209.0f, 108.0f);
        object.fingerprints.back().invItemName = "UNKNOWN_PRINT_2";
        // Uncover VO: 1077H59291

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BLOMAN_P3.BMP";
        object.fingerprints.back().position = Vector2(200.0f, 357.0f);
        object.fingerprints.back().invItemName = "UNKNOWN_PRINT_3";
        // Uncover VO: 1077H59291

        // When you collect first print VO: 1077H59292
        // When you collect second print VO: 1077H59293
        // When you collect third print VO: 1077H49294
    }
    {
        FingerprintObject& object = mObjects["LSR_ENVELOPE_INV"];
        object.textureName = "FP_LSRENV.BMP";
        object.uncoverPrintLicensePlate = "10LEM59291";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_LSRENV_P1.BMP";
        object.fingerprints.back().position = Vector2(178.0f, 305.0f);
        object.fingerprints.back().invItemName = "ESTELLES_FINGERPRINT_LSR";
    }
    {
        FingerprintObject& object = mObjects["HBHG_BOOK"];
        object.textureName = "FP_HOLYGBOOK.BMP";
        object.noPrintLicensePlate = "10LCQ59291";

        // No prints on this one!
    }
    {
        FingerprintObject& object = mObjects["DIRTY_GLASS_BUCHELLI"];
        object.textureName = "FP_SQRSHOT.BMP";
        object.uncoverPrintLicensePlate = "0A89N052H2";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_SQRSHOT_P1.BMP";
        object.fingerprints.back().position = Vector2(150.0f, 202.0f);
        object.fingerprints.back().invItemName = "BUCHELLIS_FINGERPRINT";
        //NOTE: If you already have Buchelli's print, Gabe will say: 1EK0259NS1
    }
    {
        FingerprintObject& object = mObjects["DIRTY_GLASS_WILKES"];
        object.textureName = "FP_OCTSHOT.BMP";
        object.uncoverPrintLicensePlate = "0A89N052H2";
        object.collectPrintLicensePlate = "1EK4259NS1";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_OCTSHOT_P1.BMP";
        object.fingerprints.back().position = Vector2(81.0f, 214.0f);
        object.fingerprints.back().invItemName = "WILKES_FINGERPRINT";
    }
    {
        FingerprintObject& object = mObjects["DIRTY_WINE_GLASS_BUCHELLI"];
        object.textureName = "FP_BUCHGLASS.BMP";
        object.collectPrintLicensePlate = "1077H59291";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BUCHGLASS_P1.BMP";
        object.fingerprints.back().position = Vector2(82.0f, 149.0f);
        object.fingerprints.back().invItemName = "BUCHELLIS_FINGERPRINT";
    }
    {
        FingerprintObject& object = mObjects["GLASS"];
        object.textureName = "FP_GLASS.BMP";
        object.noPrintLicensePlate = "1EPXH59291";

        // No prints on this one!
    }
    {
        FingerprintObject& object = mObjects["WATER_BOTTLE_ON_MOPED"];
        object.textureName = "FP_WATBTL.BMP";
        //TODO: VOs for this one?

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_WATBTL_P1.BMP";
        object.fingerprints.back().position = Vector2(65.0f, 279.0f); //TODO: Verify Position
        object.fingerprints.back().invItemName = "ESTELLES_FINGERPRINT";
    }
    {
        FingerprintObject& object = mObjects["CIG_PACK_IN_DRAWER"];
        object.textureName = "FP_CIGS.BMP";
        object.uncoverPrintLicensePlate = "0D83M05ML1";
        object.collectPrintLicensePlate = "0A89N052H2";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_CIGS_P1.BMP";
        object.fingerprints.back().position = Vector2(112.0f, 200.0f);
        object.fingerprints.back().invItemName = "ABBE_FINGERPRINT";
    }
    {
        FingerprintObject& object = mObjects["SUITCASE"];
        object.textureName = "FP_SUITCA.BMP";
        object.uncoverPrintLicensePlate = "0A89N052H1";
        object.collectPrintLicensePlate = "0A89N052H2";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_SUITCA_P1.BMP";
        object.fingerprints.back().position = Vector2(111.0f, 370.0f);
        object.fingerprints.back().invItemName = "BUCHELLIS_FINGERPRINT";
    }
    {
        FingerprintObject& object = mObjects["POP_BOTTLE"];
        object.textureName = "FP_SODA.BMP";
        // No VO for this one.

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_SODA_P1.BMP";
        object.fingerprints.back().position = Vector2(150.0f, 153.0f);
        object.fingerprints.back().invItemName = "MOSELYS_FINGERPRINT";
    }
    {
        FingerprintObject& object = mObjects["JESUS_PICTURE"];
        object.textureName = "FP_JESUS.BMP";
        object.noPrintLicensePlate = "0K86F05PF1";

        // No prints on this one!
    }
    {
        FingerprintObject& object = mObjects["GUN_IN_CASE"];
        object.textureName = "FP_COLT45.BMP";
        object.uncoverPrintLicensePlate = "0J8ES05291";
        object.collectPrintLicensePlate = "0A89N052H2";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_COLT45_P1.BMP";
        object.fingerprints.back().position = Vector2(65.0f, 279.0f);
        object.fingerprints.back().invItemName = "BUTHANES_FINGERPRINT";
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