#include "InventoryInspectScreen.h"

#include "ActionBar.h"
#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "InventoryManager.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIButton.h"
#include "UIImage.h"

InventoryInspectScreen::InventoryInspectScreen() : Actor(TransformType::RectTransform),
    mLayer("CloseUpLayer")
{
    // Inventory overrides SFX/VO, but continues ambient audio from scene.
    mLayer.OverrideAudioState(true, true, false);

    // Add canvas to render UI elements.
	AddComponent<UICanvas>(1);
	
	// Add black background image that blocks out the scene entirely.
    {
        UIImage* background = AddComponent<UIImage>();
        background->SetTexture(&Texture::Black);

        RectTransform* inventoryRectTransform = GetComponent<RectTransform>();
        inventoryRectTransform->SetSizeDelta(0.0f, 0.0f);
        inventoryRectTransform->SetAnchorMin(Vector2::Zero);
        inventoryRectTransform->SetAnchorMax(Vector2::One);
    }
	
	// Add exit button to bottom-left corner of screen.
    {
        Actor* exitButtonActor = new Actor(TransformType::RectTransform);
        exitButtonActor->GetTransform()->SetParent(GetTransform());
        UIButton* exitButton = exitButtonActor->AddComponent<UIButton>();

        exitButton->SetUpTexture(gAssetManager.LoadTexture("EXITN.BMP"));
        exitButton->SetDownTexture(gAssetManager.LoadTexture("EXITD.BMP"));
        exitButton->SetHoverTexture(gAssetManager.LoadTexture("EXITHOV.BMP"));
        exitButton->SetDisabledTexture(gAssetManager.LoadTexture("EXITDIS.BMP"));
        exitButton->SetPressCallback([this](UIButton* button){
            Hide();
        });

        RectTransform* exitButtonRectTransform = exitButtonActor->GetComponent<RectTransform>();
        exitButtonRectTransform->SetSizeDelta(58.0f, 26.0f); // texture width/height
        exitButtonRectTransform->SetAnchor(Vector2::Zero);
        exitButtonRectTransform->SetAnchoredPosition(10.0f, 10.0f);
        exitButtonRectTransform->SetPivot(0.0f, 0.0f);
    }
	
	// Create closeup image. It's just positioned at center of screen, which is default.
    {
        Actor* closeupActor = new Actor(TransformType::RectTransform);
        closeupActor->GetTransform()->SetParent(GetTransform());

        mCloseupImage = closeupActor->AddComponent<UIButton>();
    }

    // Create extra LSR buttons, which are only used by the LSR inventory item.
    {
        for(int i = 0; i < 3; ++i)
        {
            Actor* lsrActor = new Actor(TransformType::RectTransform);
            lsrActor->GetTransform()->SetParent(mCloseupImage->GetRectTransform());

            mLSRButtons[i] = lsrActor->AddComponent<UIButton>();
            mLSRButtons[i]->SetEnabled(false);
            mLSRButtons[i]->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        }
    }
	
	// Hide by default.
    SetActive(false);
}

void InventoryInspectScreen::Show(const std::string& itemName)
{
    gLayerManager.PushLayer(&mLayer);

    // Save item name.
    mInspectItemName = itemName;

    // If we're viewing LSR, we have to do some special stuff.
    if(IsLSR())
    {
        // Figure out which page we should view initially. This depends on what part you're on.
        if(gGameProgress.GetFlag("Ophiuchus"))
        {
            mLSRPage = 6;
        }
        else if(gGameProgress.GetFlag("Libra"))
        {
            mLSRPage = 5;
        }
        else if(gGameProgress.GetFlag("Leo"))
        {
            mLSRPage = 4;
        }
        else if(gGameProgress.GetFlag("Taurus"))
        {
            mLSRPage = 3;
        }
        else if(gGameProgress.GetFlag("Pisces"))
        {
            mLSRPage = 2;
        }
        else
        {
            mLSRPage = 1;
        }
        InitLSR();
    }
    else
    {
        // Set closeup image.
        mCloseupImage->SetUpTexture(gInventoryManager.GetInventoryItemCloseupTexture(itemName));

        // Set press callback.
        mCloseupImage->SetPressCallback([this](UIButton* button){
            OnClicked(mInspectItemName);
        });

        // Hide LSR buttons.
        for(int i = 0; i < 3; ++i)
        {
            mLSRButtons[i]->SetEnabled(false);
        }
    }
	
	// Actually show the stuff!
	SetActive(true);
}

void InventoryInspectScreen::Hide()
{
    if(!IsActive()) { return; }
	SetActive(false);
    gLayerManager.PopLayer(&mLayer);
}

bool InventoryInspectScreen::IsShowing() const
{
    return gLayerManager.IsTopLayer(&mLayer);
}

void InventoryInspectScreen::TurnLSRPageLeft()
{
    if(IsLSR())
    {
        --mLSRPage;
        if(mLSRPage < 1) { mLSRPage = 1; }
        InitLSR();
    }
}

void InventoryInspectScreen::TurnLSRPageRight()
{
    if(IsLSR())
    {
        ++mLSRPage;
        if(mLSRPage > 6) { mLSRPage = 6; }
        InitLSR();
    }
}

void InventoryInspectScreen::OnClicked(const std::string& noun)
{
    // In demo mode, the inventory inspect screen is "misused" to show some placards to the player explaining what the game is about.
    // In this context, clicking each item just goes to the next one with the TURN_RIGHT action.
    if(IsDemoIntro())
    {
        gActionManager.ExecuteAction(noun, "TURN_RIGHT");
        return;
    }

    // Show the action bar for this noun.
    gActionManager.ShowActionBar(noun, [this, noun](const Action* action){

        // Perform the action.
        gActionManager.ExecuteAction(action, [this, noun](const Action* action){

            // After the action completes, check if we still have the inventory item shown.
            // In some rare cases (ex: eating a candy), the item no longer exists, so we should close this screen.
            if(StringUtil::EqualsIgnoreCase(noun, mInspectItemName))
            {
                bool isInInventory = gLayerManager.IsLayerInStack("InventoryLayer");
                if(isInInventory && !gInventoryManager.HasInventoryItem(mInspectItemName))
                {
                    Hide();
                }
            }
        });
    });

    // Since we're showing the close-up, add INSPECT_UNDO to back out of the close-up.
    ActionBar* actionBar = gActionManager.GetActionBar();
    if(!actionBar->HasVerb("INSPECT_UNDO"))
    {
        actionBar->AddVerbToFront("INSPECT_UNDO", [](){
            gInventoryManager.InventoryUninspect();
        });
    }
}

bool InventoryInspectScreen::IsDemoIntro() const
{
    return GEngine::Instance()->IsDemoMode() && StringUtil::StartsWithIgnoreCase(mInspectItemName, "MS3I");
}

bool InventoryInspectScreen::IsLSR() const
{
    return StringUtil::EqualsIgnoreCase(mInspectItemName, "LSR");
}

void InventoryInspectScreen::InitLSR()
{
    // Set main image based on current page of LSR.
    mCloseupImage->SetUpTexture(gAssetManager.LoadTexture("LSR_PG" + std::to_string(mLSRPage) + "_BASE.BMP"));

    // Closeup image doesn't have a press callback in this case. You have to click on the individual sections.
    mCloseupImage->SetPressCallback(nullptr);

    // Show appropriate buttons for each section.
    if(mLSRPage == 1)
    {
        bool finishedAquarius = gGameProgress.GetFlag("Aquarius");
        bool finishedPisces = gGameProgress.GetFlag("Pisces");

        // Because Aquarius is the first one, it's only got two states: finished or in-progress.
        mLSRButtons[0]->SetEnabled(true);
        mLSRButtons[0]->GetRectTransform()->SetAnchoredPosition(150.0f, 188.0f);
        if(finishedAquarius)
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG1_AQU_FIN.BMP"));
        }
        else
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG1_AQU_LIT.BMP"));
        }
        mLSRButtons[0]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_AQUARIUS");
        });

        // Pisces can be unstarted, in-progress, or finished.
        mLSRButtons[1]->SetEnabled(true);
        mLSRButtons[1]->GetRectTransform()->SetAnchoredPosition(150.0f, 12.0f);
        mLSRButtons[1]->GetRectTransform()->SetSizeDelta(307.0f, 178.0f);
        if(!finishedAquarius) // Unstarted
        {
            mLSRButtons[1]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedAquarius && !finishedPisces) // In-Progress
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG1_PIS_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG1_PIS_FIN.BMP"));
        }
        mLSRButtons[1]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_PISCES");
        });
        
        // Third button is unused (there's only two sections on this page).
        mLSRButtons[2]->SetEnabled(false);
    }
    else if(mLSRPage == 2)
    {
        bool finishedPisces = gGameProgress.GetFlag("Pisces");
        bool finishedAries = gGameProgress.GetFlag("Aries");
        bool finishedTaurus = gGameProgress.GetFlag("Taurus");

        // Aries can be unstarted, in-progress, or finished.
        mLSRButtons[0]->SetEnabled(true);
        mLSRButtons[0]->GetRectTransform()->SetAnchoredPosition(165.0f, 217.0f);
        mLSRButtons[0]->GetRectTransform()->SetSizeDelta(294.0f, 172.0f);
        if(!finishedPisces) // Unstarted
        {
            mLSRButtons[0]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedPisces && !finishedAries) // In-Progress
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG2_ARI_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG2_ARI_FIN.BMP"));
        }
        mLSRButtons[0]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_ARIES");
        });

        // Taurus can be unstarted, in-progress, or finished.
        mLSRButtons[1]->SetEnabled(true);
        mLSRButtons[1]->GetRectTransform()->SetAnchoredPosition(156.0f, 23.0f);
        mLSRButtons[1]->GetRectTransform()->SetSizeDelta(308.0f, 183.0f);
        if(!finishedAries) // Unstarted
        {
            mLSRButtons[1]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedAries && !finishedTaurus) // In-Progress
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG2_TAU_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG2_TAU_FIN.BMP"));
        }
        mLSRButtons[1]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_TAURUS");
        });

        // Third button is unused (there's only two sections on this page).
        mLSRButtons[2]->SetEnabled(false);
    }
    else if(mLSRPage == 3)
    {
        bool finishedTaurus = gGameProgress.GetFlag("Taurus");
        bool finishedGemini = gGameProgress.GetFlag("Gemini");
        bool finishedCancer = gGameProgress.GetFlag("Cancer");
        bool finishedLeo = gGameProgress.GetFlag("Leo");

        // Gemini can be unstarted, in-progress, or finished.
        mLSRButtons[0]->SetEnabled(true);
        mLSRButtons[0]->GetRectTransform()->SetAnchoredPosition(170.0f, 162.0f);
        mLSRButtons[0]->GetRectTransform()->SetSizeDelta(299.0f, 232.0f);
        if(!finishedTaurus) // Unstarted
        {
            mLSRButtons[0]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedTaurus && !finishedGemini) // In-Progress
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG3_GEM_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG3_GEM_FIN.BMP"));
        }
        mLSRButtons[0]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_GEMINI");
        });

        // Cancer can be unstarted, in-progress, or finished.
        // Cancer is a bit weird - it shows as "in-progress" while Gemini is in-progress too.
        mLSRButtons[1]->SetEnabled(true);
        mLSRButtons[1]->GetRectTransform()->SetAnchoredPosition(170.0f, 163.0f);
        mLSRButtons[1]->GetRectTransform()->SetSizeDelta(299.0f, 97.0f);
        if(!finishedTaurus) // Unstarted
        {
            mLSRButtons[1]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if((finishedTaurus && !finishedGemini) || (finishedGemini && !finishedCancer)) // In-Progress
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG3_CAN_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG3_CAN_FIN.BMP"));
        }
        mLSRButtons[1]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_CANCER");
        });

        // Leo can be unstarted, in-progress, or finished.
        mLSRButtons[2]->SetEnabled(true);
        mLSRButtons[2]->GetRectTransform()->SetAnchoredPosition(154.0f, 14.0f);
        mLSRButtons[2]->GetRectTransform()->SetSizeDelta(311.0f, 149.0f);
        if(!finishedGemini) // Unstarted
        {
            mLSRButtons[2]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedGemini && !finishedCancer) // In-Progress
        {
            mLSRButtons[2]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG3_LEO_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[2]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG3_LEO_FIN.BMP"));
        }
        mLSRButtons[2]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_LEO");
        });
    }
    else if(mLSRPage == 4)
    {
        bool finishedLeo = gGameProgress.GetFlag("Leo");
        bool finishedVirgo = gGameProgress.GetFlag("Virgo");
        bool finishedLibra = gGameProgress.GetFlag("Libra");

        // Virgo can be unstarted, in-progress, or finished.
        mLSRButtons[0]->SetEnabled(true);
        mLSRButtons[0]->GetRectTransform()->SetAnchoredPosition(170.0f, 219.0f);
        mLSRButtons[0]->GetRectTransform()->SetSizeDelta(292.0f, 176.0f);
        if(!finishedLeo) // Unstarted
        {
            mLSRButtons[0]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedLeo && !finishedVirgo) // In-Progress
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG4_VIR_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG4_VIR_FIN.BMP"));
        }
        mLSRButtons[0]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_VIRGO");
        });

        // Libra can be unstarted, in-progress, or finished.
        mLSRButtons[1]->SetEnabled(true);
        mLSRButtons[1]->GetRectTransform()->SetAnchoredPosition(155.0f, 10.0f);
        mLSRButtons[1]->GetRectTransform()->SetSizeDelta(309.0f, 209.0f);
        if(!finishedVirgo) // Unstarted
        {
            mLSRButtons[1]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedVirgo && !finishedLibra) // In-Progress
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG4_LIB_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG4_LIB_FIN.BMP"));
        }
        mLSRButtons[1]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_LIBRA");
        });

        // Third button is unused (there's only two sections on this page).
        mLSRButtons[2]->SetEnabled(false);
    }
    else if(mLSRPage == 5)
    {
        bool finishedLibra = gGameProgress.GetFlag("Libra");
        bool finishedScorpio = gGameProgress.GetFlag("Scorpio");
        bool finishedOphiuchus = gGameProgress.GetFlag("Ophiuchus");

        // Scorpio can be unstarted, in-progress, or finished.
        mLSRButtons[0]->SetEnabled(true);
        mLSRButtons[0]->GetRectTransform()->SetAnchoredPosition(172.0f, 204.0f);
        mLSRButtons[0]->GetRectTransform()->SetSizeDelta(294.0f, 191.0f);
        if(!finishedLibra) // Unstarted
        {
            mLSRButtons[0]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedLibra && !finishedScorpio) // In-Progress
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG5_SCO_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG5_SCO_FIN.BMP"));
        }
        mLSRButtons[0]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_SCORPIO");
        });

        // Ophiuchus can be unstarted, in-progress, or finished.
        mLSRButtons[1]->SetEnabled(true);
        mLSRButtons[1]->GetRectTransform()->SetAnchoredPosition(155.0f, 12.0f);
        mLSRButtons[1]->GetRectTransform()->SetSizeDelta(312.0f, 190.0f);
        if(!finishedScorpio) // Unstarted
        {
            mLSRButtons[1]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedScorpio && !finishedOphiuchus) // In-Progress
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG5_OPH_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG5_OPH_FIN.BMP"));
        }
        mLSRButtons[1]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_OPHIUCHUS");
        });

        // Third button is unused (there's only two sections on this page).
        mLSRButtons[2]->SetEnabled(false);
    }
    else if(mLSRPage == 6)
    {
        bool finishedOphiuchus = gGameProgress.GetFlag("Ophiuchus");
        bool finishedSagittarius = gGameProgress.GetFlag("Sagittarius");
        bool finishedCapricorn = gGameProgress.GetFlag("Capricorn");

        // Sagittarius can be unstarted, in-progress, or finished.
        mLSRButtons[0]->SetEnabled(true);
        mLSRButtons[0]->GetRectTransform()->SetAnchoredPosition(149.0f, 232.0f);
        mLSRButtons[0]->GetRectTransform()->SetSizeDelta(323.0f, 166.0f);
        if(!finishedOphiuchus) // Unstarted
        {
            mLSRButtons[0]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedOphiuchus && !finishedSagittarius) // In-Progress
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG6_SAG_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[0]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG6_SAG_FIN.BMP"));
        }
        mLSRButtons[0]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_SAGITTARIUS");
        });

        // Capricorn can be unstarted, in-progress, or finished.
        mLSRButtons[1]->SetEnabled(true);
        mLSRButtons[1]->GetRectTransform()->SetAnchoredPosition(135.0f, 12.0f);
        mLSRButtons[1]->GetRectTransform()->SetSizeDelta(340.0f, 221.0f);
        if(!finishedSagittarius) // Unstarted
        {
            mLSRButtons[1]->SetUpTexture(nullptr, Color32::Clear);
        }
        else if(finishedSagittarius && !finishedCapricorn) // In-Progress
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG6_CAP_LIT.BMP"));
        }
        else // Finished
        {
            mLSRButtons[1]->SetUpTexture(gAssetManager.LoadTexture("LSR_PG6_CAP_FIN.BMP"));
        }
        mLSRButtons[1]->SetPressCallback([this](UIButton* button){
            OnClicked("LSR_CAPRICORN");
        });

        // Third button is unused (there's only two sections on this page).
        mLSRButtons[2]->SetEnabled(false);
    }
}