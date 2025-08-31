#include "FingerprintScreen.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "CursorManager.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "InputManager.h"
#include "InventoryManager.h"
#include "Scene.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIUtil.h"

FingerprintScreen::FingerprintScreen() : Actor("FingerprintScreen", TransformType::RectTransform),
    mLayer("FingerprintLayer")
{
    // Add canvas to render UI elements.
    UI::AddCanvas(this, 7, Color32::Black);

    // The background has a button so we can capture clicks on it.
    UIButton* backgroundButton = AddComponent<UIButton>();
    backgroundButton->SetPressCallback([this](UIButton* button){
        OnBackgroundButtonPressed();
    });

    // Add base background image, which shows the fingerprint box up-close.
    UIImage* baseImage = UI::CreateWidgetActor<UIImage>("Background", this);
    baseImage->SetTexture(gAssetManager.LoadTexture("FP_BASE.BMP"), true);

    // Add exit button to bottom-left corner of screen.
    {
        UIButton* exitButton = UI::CreateWidgetActor<UIButton>("ExitButton", baseImage);
        exitButton->SetUpTexture(gAssetManager.LoadTexture("EXITN.BMP"));
        exitButton->SetDownTexture(gAssetManager.LoadTexture("EXITD.BMP"));
        exitButton->SetHoverTexture(gAssetManager.LoadTexture("EXITHOV.BMP"));
        exitButton->SetDisabledTexture(gAssetManager.LoadTexture("EXITDIS.BMP"));
        exitButton->SetPressCallback([this](UIButton* button){
            Hide();
        });
        exitButton->SetTooltipText("fingerprintexit");

        exitButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        exitButton->GetRectTransform()->SetAnchoredPosition(5.0f, 9.0f);
        mExitButton = exitButton;
    }

    // Add brush image along with button to pick up brush.
    {
        mBrushImage = UI::CreateWidgetActor<UIImage>("Brush", baseImage);
        mBrushImage->SetTexture(gAssetManager.LoadTexture("FP_BRUSH.BMP"), true);
        mBrushImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mBrushImage->GetRectTransform()->SetAnchoredPosition(128.0f, 90.0f);

        UIButton* brushButton = mBrushImage->GetOwner()->AddComponent<UIButton>();
        brushButton->SetPressCallback([this](UIButton* button){
            OnBrushButtonPressed();
        });
    }

    // Add button for dipping brush in the dust area.
    {
        UIButton* dustButton = UI::CreateWidgetActor<UIButton>("DustButton", baseImage);
        dustButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        dustButton->GetRectTransform()->SetAnchoredPosition(149.0f, 480.0f - 337.0f);
        dustButton->GetRectTransform()->SetSizeDelta(127.0f, 108.0f);
        dustButton->SetPressCallback([this](UIButton* button){
            OnDustButtonPressed();
        });
    }

    // Add button for tape dispenser.
    {
        UIButton* tapeButton = UI::CreateWidgetActor<UIButton>("TapeButton", baseImage);
        tapeButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        tapeButton->GetRectTransform()->SetAnchoredPosition(278.0f, 103.0f);
        tapeButton->GetRectTransform()->SetSizeDelta(106.0f, 154.0f);
        tapeButton->SetPressCallback([this](UIButton* button){
            OnTapeButtonPressed();
        });
    }

    // Add button for fingerprint cloth.
    {
        UIButton* clothButton = UI::CreateWidgetActor<UIButton>("ClothButton", baseImage);
        clothButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        clothButton->GetRectTransform()->SetAnchoredPosition(0.0f, 94.0f);
        clothButton->GetRectTransform()->SetSizeDelta(140.0f, 160.0f);
        clothButton->SetPressCallback([this](UIButton* button){
            OnClothButtonPressed();
        });
    }

    // Create right-panel area that shows a closeup of the object being dusted.
    {
        // Create a button that takes up the whole right-panel area of the interface.
        // This has two purposes: to catch input when clicking/dragging in this area. And to help center the closeup image.
        mRightPanelButton = UI::CreateWidgetActor<UIButton>("RightPanelButton", baseImage);
        mRightPanelButton->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mRightPanelButton->GetRectTransform()->SetAnchoredPosition(-2.0f, -2.0f);
        mRightPanelButton->GetRectTransform()->SetSizeDelta(249.0f, 476.0f);
        mRightPanelButton->SetPressCallback([this](UIButton* button) {
            OnRightPanelPressed();
        });

        // Add image to show the object being fingerprinted.
        mFingerprintObjectImage = UI::CreateWidgetActor<UIImage>("FingerprintObjectImage", mRightPanelButton);
        mFingerprintObjectImage->SetTexture(&Texture::Black);
        mFingerprintObjectImage->GetRectTransform()->SetAnchor(AnchorPreset::Left);
        mFingerprintObjectImage->GetRectTransform()->SetAnchoredPosition(0.0f, -8.0f);

         // Create a set of images used to display fingerprints.
        for(int i = 0; i < kMaxFingerprintImages; ++i)
        {
            UIImage* image = UI::CreateWidgetActor<UIImage>("Fingerprint" + std::to_string(i), mRightPanelButton);
            image->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            image->SetEnabled(false);
            mPrintsToCollect[i].image = image;
            mPrintsToCollect[i].button = image->GetOwner()->AddComponent<UIButton>();
        }
    }

    // Load needed cursors.
    mBrushCursor = gAssetManager.LoadCursor("C_FPBRUSH.CUR");
    mDustedBrushCursor = gAssetManager.LoadCursor("C_FPBRUSH_WDUST.CUR");
    mTapeCursor = gAssetManager.LoadCursor("C_FPTAPE_NOFP.CUR");
    mUsedTapeCursor = gAssetManager.LoadCursor("C_FPTAPE_FP.CUR");

    // UNFORTUNATELY, this is another spot where the logic doesn't seem data-driven.
    // So, we'll enumerate all the possible fingerprint instances here and nowwww.....

    // Day 2, 7AM Items (as Grace).
    {
        FingerprintObject& object = mObjects["HBHG_BOOK"];
        object.textureName = "FP_HOLYGBOOK.BMP";
        object.noPrintLicensePlate = "10LCQ59291";
    }

    // Day 2, 10AM Items (while sneaking around as Gabe).
    {
        FingerprintObject& object = mObjects["HAND_MIRROR"];
        object.textureName = "FP_LHOMIR.BMP";
        object.anchor = AnchorPreset::Center;

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_LHOMIR_P1.BMP";
        object.fingerprints.back().position = Vector2(159.0f, 134.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "2O8A805PF1";
        object.fingerprints.back().invItemName = "HOWARDS_FINGERPRINT";
        object.fingerprints.back().flagName = "GotMirrorHowardPrint";
        object.fingerprints.back().scoreName = "e_210a_r31_fingerprint_kit_mirror";
    }
    {
        FingerprintObject& object = mObjects["GUN_IN_CASE"];
        object.textureName = "FP_COLT45.BMP";
        object.anchoredPosition = Vector2(0.0f, 12.0f);
        object.collectPrintLicensePlates.push_back("0A89N052H2");

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_COLT45_P1.BMP";
        object.fingerprints.back().position = Vector2(65.0f, 279.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "0J8ES05291";
        object.fingerprints.back().invItemName = "BUTHANES_FINGERPRINT";
        object.fingerprints.back().flagName = "GotGunButhanePrint";
        object.fingerprints.back().scoreName = "e_210a_r29_fingerprint_kit_on_gun";
    }
    {
        FingerprintObject& object = mObjects["CIG_PACK_IN_DRAWER"];
        object.textureName = "FP_CIGS.BMP";
        object.collectPrintLicensePlates.push_back("0A89N052H2");

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_CIGS_P1.BMP";
        object.fingerprints.back().position = Vector2(101.0f, 201.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "0D83M05ML1";
        object.fingerprints.back().invItemName = "ABBE_FINGERPRINT";
        object.fingerprints.back().flagName = "GotCigPackAbbePrint";
        object.fingerprints.back().scoreName = "e_210a_fingerprint_kit_cigarette_pack";
    }
    {
        FingerprintObject& object = mObjects["SUITCASE"];
        object.textureName = "FP_SUITCA.BMP";
        object.anchoredPosition = Vector2(0.0f, -8.0f);
        object.collectPrintLicensePlates.push_back("0A89N052H2");

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_SUITCA_P1.BMP";
        object.fingerprints.back().position = Vector2(111.0f, 370.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "0A89N052H1";
        object.fingerprints.back().invItemName = "BUCHELLIS_FINGERPRINT";
        object.fingerprints.back().flagName = "GotSuitcaseBuchelliPrint";
        object.fingerprints.back().scoreName = "e_210a_r21_fingerprint_kit_suitcase";
    }
    {
        FingerprintObject& object = mObjects["JESUS_PICTURE"];
        object.textureName = "FP_JESUS.BMP";
        object.noPrintLicensePlate = "0K86F05PF1";
    }

    // Day 2, 12PM Items (as Grace at Chateau de Serres).
    {
        FingerprintObject& object = mObjects["BOOK_IN_DRAWER"];
        object.textureName = "FP_BOOKIMMORTALS.BMP";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BOOKIMMORTALS_P1.BMP";
        object.fingerprints.back().position = Vector2(113.0f, 144.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "08BBY59411";
        object.fingerprints.back().invItemName = "MONTREAUX_FINGERPRINT";
        object.fingerprints.back().flagName = "GotImmortalsMontreauxPrint";
        object.fingerprints.back().scoreName = "e_212p_cs2_fingerprint_kit_immortals_book";
    }

    // Day 2, 2PM Items (as Gabe after 2 Men are murdered).
    {
        FingerprintObject& object = mObjects["DIRTY_GLASS_WILKES"];
        object.textureName = "FP_OCTSHOT.BMP";
        object.anchor = AnchorPreset::Center;
        object.anchoredPosition = Vector2(0.0f, 8.0f);

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_OCTSHOT_P1.BMP";
        object.fingerprints.back().position = Vector2(81.0f, 214.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "0A89N052H2";
        object.fingerprints.back().invItemName = "WILKES_FINGERPRINT";
        object.fingerprints.back().flagName = ""; // there doesn't seem to be any flag for this print
        object.fingerprints.back().scoreName = "e_202p_lby_fingerprint_kit_wilke_glass";

        // The logic for collecting this print is much more complex.
        // Depending on how thorough the player has been so far, there may be some confusion about whose fingerprint this is...
        object.fingerprints.back().onCollectCustomLogicFunc = [this](){
            OnCollectWilkesDirtyGlass();
        };
    }
    {
        FingerprintObject& object = mObjects["DIRTY_GLASS_BUCHELLI"];
        object.textureName = "FP_SQRSHOT.BMP";
        object.anchor = AnchorPreset::Center;

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_SQRSHOT_P1.BMP";
        object.fingerprints.back().position = Vector2(150.0f, 202.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "0A89N052H2";
        object.fingerprints.back().invItemName = "BUCHELLIS_FINGERPRINT";
        object.fingerprints.back().flagName = "";  // there doesn't seem to be any flag for this print
        object.fingerprints.back().scoreName = ""; // "e_202p_lby_fingerprint_kit_buchelli_glass";

        // The logic for collecting this print is much more complex.
        // Depending on how thorough the player has been so far, there may be some confusion about whose fingerprint this is...
        object.fingerprints.back().onCollectCustomLogicFunc = [this](){
            OnCollectBuchelliDirtyGlass();
        };
    }
    {
        FingerprintObject& object = mObjects["POP_BOTTLE"];
        object.textureName = "FP_SODA.BMP";
        object.anchor = AnchorPreset::Center;

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_SODA_P1.BMP";
        object.fingerprints.back().position = Vector2(147.0f, 150.0f);
        object.fingerprints.back().invItemName = ""; // No inventory item added for this one.
        object.fingerprints.back().flagName = "GotPMoselyPrint";
        object.fingerprints.back().scoreName = "e_202p_r25_fingerprint_kit_soda_bottle";
    }

    // Day 2, 5PM Items (as Grace during LSR).
    {
        FingerprintObject& object = mObjects["LSR_ENVELOPE_INV"];
        object.textureName = "FP_LSRENV.BMP";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_LSRENV_P1.BMP";
        object.fingerprints.back().position = Vector2(178.0f, 305.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "10LEM59291";
        object.fingerprints.back().invItemName = "ESTELLES_FINGERPRINT_LSR";
        object.fingerprints.back().flagName = "GotLEstellePrint";
        object.fingerprints.back().scoreName = "e_205p_inventory_fingerprint_kit_envelope";
    }
    {
        FingerprintObject& object = mObjects["DIRTY_WINE_GLASS_BUCHELLI"];
        object.textureName = "FP_BUCHGLASS.BMP";
        object.collectPrintLicensePlates.push_back("1077H59291");

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BUCHGLASS_P1.BMP";
        object.fingerprints.back().position = Vector2(82.0f, 149.0f);
        object.fingerprints.back().invItemName = "BUCHELLIS_FINGERPRINT";
        object.fingerprints.back().flagName = "GotWineBuchelliPrint";
        object.fingerprints.back().scoreName = ""; // ???
    }
    {
        FingerprintObject& object = mObjects["GLASS"];
        object.textureName = "FP_GLASS.BMP";
        object.noPrintLicensePlate = "1EPXH59291";
    }

    // Day 3 2AM (as Gabe, when we retrieves the manuscript).
    // Also, Day 3 12PM (as Grace, when she again retrieves the manuscript).
    {
        FingerprintObject& object = mObjects["BLOODLINE_MANUSCRIPT_202A"];
        object.textureName = "FP_BLOMAN.BMP";
        object.collectPrintLicensePlates.push_back("1037H59291");

        // The first fingerprint, collected during 302A.
        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BLOMAN_P6.BMP";
        object.fingerprints.back().position = Vector2(206.0f, 301.0f);
        object.fingerprints.back().invItemName = "LARRYS_FINGERPRINT";
        object.fingerprints.back().flagName = "GotMLarryPrint";
        object.fingerprints.back().scoreName = "e_302a_inventory_fingerprint_kit_manuscript";
    }

    {
        FingerprintObject& object = mObjects["BLOODLINE_MANUSCRIPT_312P"];
        object.textureName = "FP_BLOMAN.BMP";
        object.collectPrintLicensePlates.push_back("1077H59292");
        object.collectPrintLicensePlates.push_back("1077H59293");
        object.collectPrintLicensePlates.push_back("1077H59294");

        // The next three fingerprints, collected during 312P.
        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BLOMAN_P1.BMP";
        object.fingerprints.back().position = Vector2(200.0f, 243.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "1EP02593L1";
        object.fingerprints.back().invItemName = "UNKNOWN_PRINT_1";
        object.fingerprints.back().flagName = "GotMMoselyPrint";
        object.fingerprints.back().scoreName = "e_312p_bmb_fingerprint_kit_manuscript1";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BLOMAN_P2.BMP";
        object.fingerprints.back().position = Vector2(205.0f, 98.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "1077H59291";
        object.fingerprints.back().invItemName = "UNKNOWN_PRINT_2";
        object.fingerprints.back().flagName = "GotMButhanePrint";
        object.fingerprints.back().scoreName = "e_312p_bmb_fingerprint_kit_manuscript2";

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_BLOMAN_P3.BMP";
        object.fingerprints.back().position = Vector2(199.0f, 344.0f);
        object.fingerprints.back().uncoverPrintLicensePlate = "1077H59291";
        object.fingerprints.back().invItemName = "UNKNOWN_PRINT_3";
        object.fingerprints.back().flagName = "GotMBuchelliPrint";
        object.fingerprints.back().scoreName = "e_312p_bmb_fingerprint_kit_manuscript3";
    }

    // Day 3, 3PM (as Gabe) or 6PM (as Grace).
    {
        FingerprintObject& object = mObjects["WATER_BOTTLE_ON_MOPED"];
        object.textureName = "FP_WATBTL.BMP";
        object.anchor = AnchorPreset::Center;
        object.collectPrintLicensePlates.push_back("0A89N052H2");

        object.fingerprints.emplace_back();
        object.fingerprints.back().textureName = "FP_WATBTL_P1.BMP";
        object.fingerprints.back().position = Vector2(132.0f, 278.0f);
        object.fingerprints.back().invItemName = "ESTELLES_FINGERPRINT";
        object.fingerprints.back().flagName = "GotWaterBottleEstellePrint";
        object.fingerprints.back().flagNameGrace = "GotWaterBottleEstellePrintGrace";
        object.fingerprints.back().scoreName = "e_303p_wod_fingerprint_kit_water_bottle";
    }

    // Hide by default.
    SetActive(false);
}

void FingerprintScreen::Show(const std::string& nounName)
{
    gLayerManager.PushLayer(&mLayer);

    // Find the object associated with this noun.
    mActiveObject = nullptr;
    auto it = mObjects.find(nounName);
    if(it != mObjects.end())
    {
        mActiveObject = &it->second;
    }
    else
    {
        // If couldn't find the object, try appending the timeblock.
        // There's at least one case, with bloodline manuscript, where you fingerprint the same item twice at different times.
        it = mObjects.find(nounName + "_" + gGameProgress.GetTimeblock().ToString());
        if(it != mObjects.end())
        {
            mActiveObject = &it->second;
        }
    }

    // Show the object being fingerprinted in the right panel.
    if(mActiveObject != nullptr)
    {
        mFingerprintObjectImage->SetTexture(gAssetManager.LoadTexture(mActiveObject->textureName), true);
        mFingerprintObjectImage->GetRectTransform()->SetAnchor(mActiveObject->anchor);
        mFingerprintObjectImage->GetRectTransform()->SetAnchoredPosition(mActiveObject->anchoredPosition);
    }
    else
    {
        mFingerprintObjectImage->SetTexture(&Texture::Black);
    }

    // Reset collected print count for new object.
    mCollectedPrintCount = 0;

    // Position fingerprint images and hide them to start.
    int imageIndex = 0;
    if(mActiveObject != nullptr)
    {
        for(int fpIndex = 0; fpIndex < mActiveObject->fingerprints.size(); ++fpIndex)
        {
            // Can't show this fingerprint because we ran out of images!
            if(imageIndex >= kMaxFingerprintImages)
            {
                printf("ERROR: Ran out of fingerprint images when showing fingerprint object!\n");
                continue;
            }

            // Set fingerprint texture on the image.
            FingerprintObject::Fingerprint& fp = mActiveObject->fingerprints[fpIndex];
            Texture* printTexture = gAssetManager.LoadTexture(fp.textureName);
            mPrintsToCollect[imageIndex].image->SetTexture(printTexture, true);

            // Fingerprint images are surrounded by black pixels that should render as transparent.
            mPrintsToCollect[imageIndex].image->SetTransparentColor(Color32::Black);

            // Enable the image and position it correctly.
            mPrintsToCollect[imageIndex].image->SetEnabled(true);
            mPrintsToCollect[imageIndex].image->GetRectTransform()->SetAnchoredPosition(fp.position);

            // Set alpha to zero so it's initially invisible.
            mPrintsToCollect[imageIndex].image->SetColor(Color32(0, 0, 0, 0));
            mPrintsToCollect[imageIndex].alpha = 0.0f;

            // Set "collected" flag appropriately.
            // If you've already gotten this print before, its associated flag(s) will be set.
            mPrintsToCollect[imageIndex].collected = gGameProgress.GetFlag(fp.flagName) || gGameProgress.GetFlag(fp.flagNameGrace);
            if(mPrintsToCollect[imageIndex].collected)
            {
                ++mCollectedPrintCount;
            }

            // Hook up some action to take when collecting this print.
            mPrintsToCollect[imageIndex].button->SetPressCallback([this, imageIndex](UIButton* button){
                OnFingerprintPressed(imageIndex);
            });

            // On to the next image index.
            ++imageIndex;
        }
    }

    // Hide any remaining fingerprint images.
    for(; imageIndex < kMaxFingerprintImages; ++imageIndex)
    {
        mPrintsToCollect[imageIndex].image->SetEnabled(false);
    }

    // Reset some state vars.
    PutDownBrush(); // resets cursor state and brush image state
    mDistanceDusted = 0.0f;
    mTapePrintIndex = -1;

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
    // Update cursor based on state.
    switch(mCursorState)
    {
    default:
    case CursorState::Normal:
        gCursorManager.UseDefaultCursor();
        break;

    case CursorState::Brush:
        gCursorManager.UseCustomCursor(mBrushCursor, 10);
        break;

    case CursorState::DustedBrush:
        gCursorManager.UseCustomCursor(mDustedBrushCursor, 10);
        break;

    case CursorState::Tape:
        gCursorManager.UseCustomCursor(mTapeCursor, 10);
        break;

    case CursorState::TapeWithPrint:
        gCursorManager.UseCustomCursor(mUsedTapeCursor, 10);
        break;
    }

    // If using the dusted brush, AND hovering the right panel AND left button is pressed...
    // ...we are actively dusting the object for fingerprints!
    if(mActiveObject != nullptr && mCursorState == CursorState::DustedBrush && gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left))
    {
        // Distance to drag brush before you get the "no prints" dialogue.
        static const float kNoPrintDialogueDistance = 800.0f;
        if(mActiveObject->fingerprints.empty())
        {
            if(mRightPanelButton->IsHovered())
            {
                if(mDistanceDusted < kNoPrintDialogueDistance)
                {
                    mDistanceDusted += gInputManager.GetMouseDelta().GetLength();
                    if(mDistanceDusted > kNoPrintDialogueDistance)
                    {
                        gActionManager.ExecuteSheepAction(StringUtil::Format("wait StartDialogue(\"%s\", 1)", mActiveObject->noPrintLicensePlate.c_str()), [this](const Action* action){
                            Hide();
                        });
                    }
                }
            }
        }
        else
        {
            for(int i = 0; i < kMaxFingerprintImages; ++i)
            {
                if(mPrintsToCollect[i].image->IsEnabled() && mPrintsToCollect[i].image->GetRectTransform()->GetWorldRect().Contains(gInputManager.GetMousePosition()))
                {
                    float currentAlpha = mPrintsToCollect[i].alpha;
                    float prevAlpha = currentAlpha;

                    // Increase alpha based on mouse movement.
                    currentAlpha += gInputManager.GetMouseDelta().GetLength() * deltaTime * 0.2f;
                    currentAlpha = Math::Clamp(currentAlpha, 0.0f, 1.0f);
                    mPrintsToCollect[i].alpha = currentAlpha;
                    //printf("Alpha %f\n", currentAlpha);

                    mPrintsToCollect[i].image->SetColor(Color32(255, 255, 255, currentAlpha * 255));

                    if(prevAlpha < 1.0f && currentAlpha >= 1.0f)
                    {
                        if(!mActiveObject->fingerprints[i].uncoverPrintLicensePlate.empty())
                        {
                            gActionManager.ExecuteDialogueAction(mActiveObject->fingerprints[i].uncoverPrintLicensePlate);
                        }
                    }
                }
            }
        }
    }

    // Check for exit button shortcut.
    if(gGK3UI.CanExitScreen(mLayer) && gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
    {
        mExitButton->AnimatePress();
    }
}

void FingerprintScreen::PickUpBrush()
{
    mCursorState = CursorState::Brush;
    mBrushImage->SetEnabled(false);
}

void FingerprintScreen::PutDownBrush()
{
    mCursorState = CursorState::Normal;
    mBrushImage->SetEnabled(true);
}

void FingerprintScreen::OnBackgroundButtonPressed()
{
    // If you click anywhere on this screen's background with any brush, it gets put away.
    if(mCursorState == CursorState::Brush || mCursorState == CursorState::DustedBrush)
    {
        PutDownBrush();
    }
}

void FingerprintScreen::OnBrushButtonPressed()
{
    // Clicking the brush picks it up.
    // Clicking here when you're holding the brush puts it down.
    if(mCursorState == CursorState::Normal)
    {
        PickUpBrush();
    }
    else if(mCursorState == CursorState::Brush || mCursorState == CursorState::DustedBrush)
    {
        PutDownBrush();
    }
}

void FingerprintScreen::OnDustButtonPressed()
{
    if(mCursorState == CursorState::Brush)
    {
        mCursorState = CursorState::DustedBrush;
    }
}

void FingerprintScreen::OnTapeButtonPressed()
{
    // If you click on this with the normal cursor, you get a piece of tape.
    // With tape, you put the tape down.
    // With a brush (dusted or not), you put the brush down.
    if(mCursorState == CursorState::Normal)
    {
        mCursorState = CursorState::Tape;
    }
    else if(mCursorState == CursorState::Tape)
    {
        mCursorState = CursorState::Normal;
    }
    else if(mCursorState == CursorState::Brush || mCursorState == CursorState::DustedBrush)
    {
        PutDownBrush();
    }
}

void FingerprintScreen::OnClothButtonPressed()
{
    if(mCursorState == CursorState::TapeWithPrint)
    {
        if(mPrintsToCollect[mTapePrintIndex].collected)
        {
            // I *think* this case only applies to Grace, when she has to collect more than one print from the bloodline manuscript.
            // In this case, she says "I've done that already!".
            gActionManager.ExecuteDialogueAction("2FL8S27SG1");
            mCursorState = CursorState::Normal;
        }
        else
        {
            // We've collected this print.
            mPrintsToCollect[mTapePrintIndex].collected = true;
            ++mCollectedPrintCount;

            // In some complex cases, we need very bespoke logic for collecting a print.
            // If such a function is set, use it and early out of here.
            FingerprintObject::Fingerprint& fp = mActiveObject->fingerprints[mTapePrintIndex];
            if(fp.onCollectCustomLogicFunc != nullptr)
            {
                fp.onCollectCustomLogicFunc();
                return;
            }

            // From here, normal collect logic used in 99% of cases.

            // Grant inventory item, score, and logic flag associated with this print.
            GrantInvItemFlagAndScore(fp);

            // Play collect VO if any is available.
            int voIndex = mCollectedPrintCount - 1;
            if(voIndex < mActiveObject->collectPrintLicensePlates.size())
            {
                gActionManager.ExecuteDialogueAction(mActiveObject->collectPrintLicensePlates[voIndex], 1, [this](const Action* action){
                    OnCollectedFingerprint(mTapePrintIndex);
                });
            }
            else
            {
                OnCollectedFingerprint(mTapePrintIndex);
            }
        }
    }
    else if(mCursorState == CursorState::Brush || mCursorState == CursorState::DustedBrush)
    {
        PutDownBrush();
    }
}

void FingerprintScreen::OnRightPanelPressed()
{
    // Clicking here with undusted brush will - yep, you guessed it - put the brush away.
    // However, clicking here with a DUSTED brush doesn't do anything - see OnUpdate for the drag logic!
    if(mCursorState == CursorState::Brush)
    {
        PutDownBrush();
    }
}

void FingerprintScreen::OnFingerprintPressed(int printToCollectIndex)
{
    // These buttons are not responsive until they've been fully uncovered.
    if(mPrintsToCollect[printToCollectIndex].alpha < 1.0f) { return; }

    if(mCursorState == CursorState::Brush)
    {
        PutDownBrush();
    }
    else if(mCursorState == CursorState::Tape)
    {
        mCursorState = CursorState::TapeWithPrint;
        mTapePrintIndex = printToCollectIndex;
    }
}

void FingerprintScreen::OnCollectedFingerprint(int printToCollectIndex)
{
    // See if there are any other prints to collect.
    bool stillHasPrintToCollect = false;
    for(int i = 0; i < kMaxFingerprintImages; ++i)
    {
        if(mPrintsToCollect[i].image->IsEnabled() && !mPrintsToCollect[i].collected)
        {
            stillHasPrintToCollect = true;
            break;
        }
    }

    // If still prints to collect, go back to normal cursor state - the players still has work to do.
    // If all prints were collected, exit this UI.
    if(stillHasPrintToCollect)
    {
        mCursorState = CursorState::Normal;
    }
    else
    {
        Hide();
    }
}

void FingerprintScreen::GrantInvItemFlagAndScore(const FingerprintObject::Fingerprint& fp)
{
    // Put the associated fingerprint inventory item in Ego's inventory.
    if(!fp.invItemName.empty())
    {
        gInventoryManager.AddInventoryItem(fp.invItemName);
    }

    // Set associated flag.
    std::string flagName = fp.flagName;
    if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Grace") && !fp.flagNameGrace.empty())
    {
        flagName = fp.flagNameGrace;
    }
    if(!flagName.empty())
    {
        gGameProgress.SetFlag(flagName);
    }

    // Grant associated score.
    if(!fp.scoreName.empty())
    {
        gGameProgress.ChangeScore(fp.scoreName);
    }
}

namespace
{
    enum DirtyGlassState
    {
        DG_NoPrints = 0,
        DG_GotBuchelliPrint = 1,
        DG_GotWilkesPrint = 2,
        DG_GotBuchelliPrintLabeledWilkes = 3,
        DG_GotWilkesPrintLabeledBuchelli = 4,
        DG_GotBothPrints = 5
    };
}

void FingerprintScreen::OnCollectWilkesDirtyGlass()
{
    // Go back to normal cursor.
    mCursorState = CursorState::Normal;

    // If we got Buchelli's fingerprint during 210A, then we KNOW this is Wilkes' fingerprint. Easy and done.
    if(gGameProgress.GetFlag("GotSuitcaseBuchelliPrint"))
    {
        // Update state var for this little puzzle...
        if(gGameProgress.GetGameVariable("DirtyGlassPrint") == DG_NoPrints)
        {
            gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotWilkesPrint);
        }
        else
        {
            gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBothPrints);
        }

        // Gabe says "this must be Wilkes' print because it doesn't look like Buchelli's print".
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"1EK4259NS1\", 1)", [this](const Action* action){
            GrantInvItemFlagAndScore(mObjects["DIRTY_GLASS_WILKES"].fingerprints[0]);
            Hide();
        });
        return;
    }

    // OK, now we have our work cut out for us: we have two dirty glasses, and we don't know whose is whose.
    // We haven't picked any dirty glass prints yet...
    if(gGameProgress.GetGameVariable("DirtyGlassPrint") == DG_NoPrints)
    {
        // This could be either Wilkes' or Buchelli's print...
        // Gabe says "hmm...who's print is this?"
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"1EK0259291\", 1)", [this](const Action* action){

            // Weirdly, the noun for this choice in the NVC is "Crow"...
            gActionManager.ShowTopicBar("Crow", false, [this](const Action* action){

                // If chose Wilkes...you were right! And you actually get the points for guessing this correctly (or being observant earlier in the timeblock).
                if(StringUtil::EqualsIgnoreCase(action->verb, "T_WILKES"))
                {
                    gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotWilkesPrint);
                    GrantInvItemFlagAndScore(mObjects["DIRTY_GLASS_WILKES"].fingerprints[0]);
                }
                else
                {
                    // No points in this case, but you do get a bogus inventory item.
                    gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotWilkesPrintLabeledBuchelli);
                    gInventoryManager.AddInventoryItem("WILKES_FINGERPRINT_LABELED_BUCHELLI");
                }

                // Gabe says "right...ok", not entirely convinced.
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"1EK0259292\", 1)", [this](const Action* action){
                    Hide();
                });
            });
        });
    }
    else if(gGameProgress.GetGameVariable("DirtyGlassPrint") == DG_GotBuchelliPrint)
    {
        // In this case, we already got Buchelli's print successfully from the other glass.
        // So, we can deduce this print. And get the points for it.
        gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBothPrints);
        GrantInvItemFlagAndScore(mObjects["DIRTY_GLASS_WILKES"].fingerprints[0]);
        Hide();
    }
    else if(gGameProgress.GetGameVariable("DirtyGlassPrint") == DG_GotBuchelliPrintLabeledWilkes)
    {
        // In this case, we incorrectly labeled Buchelli's print, so we think THIS is Buchelli's print. Whoops.
        gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBothPrints);
        gInventoryManager.AddInventoryItem("WILKES_FINGERPRINT_LABELED_BUCHELLI");
        Hide();
    }
}

void FingerprintScreen::OnCollectBuchelliDirtyGlass()
{
    // Go back to normal cursor.
    mCursorState = CursorState::Normal;

    // If we got Buchelli's fingerprint during 201A, then we KNOW this is Buchelli's print. Easy and done.
    if(gGameProgress.GetFlag("GotSuitcaseBuchelliPrint"))
    {
        // Update state var for this little puzzle...
        if(gGameProgress.GetGameVariable("DirtyGlassPrint") == DG_NoPrints)
        {
            gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBuchelliPrint);
        }
        else
        {
            gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBothPrints);
        }

        // Gabriel says "I already have this print!".
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"1EK0259NS1\", 1)", [this](const Action* action){
            Hide();
        });
        return;
    }

    // OK, now we have our work cut out for us: we have two dirty glasses, and we don't know whose is whose.
    // We haven't picked any dirty glass prints yet...
    if(gGameProgress.GetGameVariable("DirtyGlassPrint") == DG_NoPrints)
    {
        // This could be either Wilkes' or Buchelli's print...
        // Gabe says "hmm...who's print is this?"
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"1EK0259291\", 1)", [this](const Action* action){

            // Weirdly, the noun for this choice in the NVC is "Dagger"...
            gActionManager.ShowTopicBar("Dagger", false, [this](const Action* action){

                // If chose Buchelli...you were right!
                // There are actually no points for this, but you will get points when you take Wilkes' print next.
                if(StringUtil::EqualsIgnoreCase(action->verb, "T_BUCHELLI"))
                {
                    gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBuchelliPrint);
                    GrantInvItemFlagAndScore(mObjects["DIRTY_GLASS_BUCHELLI"].fingerprints[0]);
                }
                else
                {
                    // You guessed wrong - you are a bad detective.
                    gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBuchelliPrintLabeledWilkes);
                    gInventoryManager.AddInventoryItem("BUCHELLIS_FINGERPRINT_LABELED_WILKES");
                }

                // Gabe says "right...ok", not entirely convinced.
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"1EK0259292\", 1)", [this](const Action* action){
                    Hide();
                });
            });
        });
    }
    else if(gGameProgress.GetGameVariable("DirtyGlassPrint") == DG_GotWilkesPrint)
    {
        // In this case, we already got Wilkes' print successfully from the other glass.
        // So, we can deduce this print. But no points are granted in this case.
        gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBothPrints);
        GrantInvItemFlagAndScore(mObjects["DIRTY_GLASS_BUCHELLI"].fingerprints[0]);
        Hide();
    }
    else if(gGameProgress.GetGameVariable("DirtyGlassPrint") == DG_GotWilkesPrintLabeledBuchelli)
    {
        // In this case, we incorrectly labeled Wilkes' print, so we think THIS is Wilkes' print. Whoops.
        gGameProgress.SetGameVariable("DirtyGlassPrint", DG_GotBothPrints);
        gInventoryManager.AddInventoryItem("BUCHELLIS_FINGERPRINT_LABELED_WILKES");
        Hide();
    }
}
