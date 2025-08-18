#include "SidneyAddData.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "Scene.h"
#include "Sidney.h"
#include "SidneyFakeInputPopup.h"
#include "SidneyFiles.h"
#include "SidneyUtil.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIUtil.h"

void SidneyAddData::Init(Sidney* sidney, SidneyFiles* sidneyFiles)
{
    mSidney = sidney;
    mSidneyFiles = sidneyFiles;

    // Add "Add Data" dialog box.
    {
        UINineSlice* outerBox = UI::CreateWidgetActor<UINineSlice>("Add Data", sidney, SidneyUtil::GetGrayBoxParams(Color32(0, 0, 0, 160)));
        outerBox->GetRectTransform()->SetSizeDelta(248.0f, 44.0f);
        mAddDataBox = outerBox->GetOwner();

        UINineSlice* innerBox = UI::CreateWidgetActor<UINineSlice>("LabelBox", mAddDataBox, SidneyUtil::GetGrayBoxParams(Color32(0, 0, 0, 180)));
        innerBox->GetRectTransform()->SetSizeDelta(220.0f, 17.0f);

        mGreenFont = gAssetManager.LoadFont("SID_TEXT_14_GRN.FON");
        mYellowFont = gAssetManager.LoadFont("SID_TEXT_14.FON");

        mAddDataLabel = UI::CreateWidgetActor<UILabel>("AddDataLabel", innerBox);
        mAddDataLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mAddDataLabel->SetFont(mGreenFont);
        mAddDataLabel->SetMasked(true);
        mAddDataLabel->GetRectTransform()->SetAnchor(AnchorPreset::Top);
        mAddDataLabel->GetRectTransform()->SetSizeDelta(200.0f, 16.0f);

        // Hide by default.
        mAddDataBox->SetActive(false);
    }

    // Add "Input Complete" dialog box.
    mInputCompletePopup = new SidneyFakeInputPopup(sidney, "Add Data Complete");
}

void SidneyAddData::Start()
{
    // If you try to use this button before 210A, Grace says she has nothing to scan.
    // After that, the player gets to decide.
    const Timeblock& timeblock = gGameProgress.GetTimeblock();
    if(timeblock == Timeblock(2, 7))
    {
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"0264G2ZPF1\", 2)");
    }
    else
    {
        // Show box indicating that we need input.
        mAddDataBox->SetActive(true);

        // Just use static green text.
        mAddDataLabel->SetText(SidneyUtil::GetAddDataLocalizer().GetText("WaitForInput"));
        mAddDataLabel->SetFont(mGreenFont);
        mAddDataColorTimer = -1.0f;

        // This puts the player in a non-interactive state for a moment (so they can read the text box) and then shows the inventory.
        gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2); ShowInventory();", [this](const Action* action){
            mAddDataBox->SetActive(false);
            mAddingData = true;

            // This is required for the Inventory NVC to know that scanning items is allowed.
            gGameProgress.SetFlag("UsingScanner");

            // Clear scanner variable. Inventory code will set this if we've successfully scanned any item.
            gGameProgress.SetGameVariable("SidScanner", 0);
        });
    }
}

void SidneyAddData::OnUpdate(float deltaTime)
{
    // Update when the player is trying to add data.
    if(mAddingData)
    {
        // Wait for player to close inventory.
        if(!gInventoryManager.IsInventoryShowing() && !gInventoryManager.IsInventoryInspectShowing() && !gActionManager.IsActionPlaying())
        {
            // Clear "using scanner" flag.
            gGameProgress.ClearFlag("UsingScanner");
            mAddingData = false;

            // Determine whether something was successfully scanned into Sidney.
            int sidneyFileId = gGameProgress.GetGameVariable("SidScanner") - 1;
            if(sidneyFileId < 0 || sidneyFileId > mSidneyFiles->GetMaxFileIndex())
            {
                // CASE 1: No valid object was selected to scan.
                // Show box indicating that input was aborted.
                mAddDataBox->SetActive(true);

                // Use green text, no color change.
                mAddDataLabel->SetText(SidneyUtil::GetAddDataLocalizer().GetText("AbortInput"));
                mAddDataLabel->SetFont(mGreenFont);
                mAddDataColorTimer = -1.0f;

                // This puts the player in a non-interactive state for a moment (so they can read the text box) and then puts them back on the main screen.
                gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2);", [this](const Action* action){
                    mAddDataBox->SetActive(false);
                });
            }
            else if(mSidneyFiles->HasFile(sidneyFileId))
            {
                // CASE 2: Valid object selected, but already scanned it.
                // Ego says "I already scanned it!"
                if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
                {
                    gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O4G2K6R1\", 1)");
                }
                else
                {
                    gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O4G716R1\", 1)");
                }
            }
            else
            {
                // CASE 3: Valid object selected, not scanned yet.
                // Show box (and SFX) indicating we are scanning an item.
                mAddDataBox->SetActive(true);
                gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDSCAN.WAV"));

                // Start with green text, but it will blink on an interval.
                mAddDataLabel->SetText(SidneyUtil::GetAddDataLocalizer().GetText("ScanningItem"));
                mAddDataLabel->SetFont(mGreenFont);
                mAddDataColorTimer = kAddDataColorToggleInterval;

                // This puts the player in a non-interactive state for a moment (so they can read the text box and hear SFX).
                gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2);", [this, sidneyFileId](const Action* action){

                    // Add the desired file to Sidney.
                    mSidneyFiles->AddFile(static_cast<size_t>(sidneyFileId));

                    // Hide "Add Data" box and show "Input Complete" box.
                    mAddDataBox->SetActive(false);
                    mInputCompletePopup->Show(SidneyUtil::GetAddDataLocalizer().GetText("InputComplete"),
                                              SidneyUtil::GetAddDataLocalizer().GetText("InputPrompt"),
                                              SidneyUtil::GetAddDataLocalizer().GetText("ScanItem" + std::to_string(sidneyFileId + 1)));
                });
            }
        }
        return;
    }

    // Handle toggling green/yellow texts.
    if(mAddDataBox->IsActive() && mAddDataColorTimer > 0.0f)
    {
        mAddDataColorTimer -= deltaTime;
        if(mAddDataColorTimer <= 0.0f)
        {
            if(mAddDataLabel->GetFont() == mGreenFont)
            {
                mAddDataLabel->SetFont(mYellowFont);
            }
            else
            {
                mAddDataLabel->SetFont(mGreenFont);
            }
            mAddDataColorTimer = kAddDataColorToggleInterval;
        }
    }
}