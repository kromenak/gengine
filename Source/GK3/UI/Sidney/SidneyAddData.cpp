#include "SidneyAddData.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "Random.h"
#include "Scene.h"
#include "Sidney.h"
#include "SidneyButton.h"
#include "SidneyFiles.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIUtil.h"

void SidneyAddData::Init(Sidney* sidney, SidneyFiles* sidneyFiles)
{
    mSidney = sidney;
    mSidneyFiles = sidneyFiles;

    // Add "Add Data" dialog box.
    {
        mAddDataBox = new Actor("Add Data", TransformType::RectTransform);
        mAddDataBox->GetTransform()->SetParent(sidney->GetTransform());

        UINineSlice* outerBoxImage = mAddDataBox->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(Color32(0, 0, 0, 160)));
        outerBoxImage->GetRectTransform()->SetSizeDelta(248.0f, 44.0f);

        Actor* innerBoxActor = new Actor(TransformType::RectTransform);
        innerBoxActor->GetTransform()->SetParent(mAddDataBox->GetTransform());

        UINineSlice* innerBoxImage = innerBoxActor->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(Color32(0, 0, 0, 180)));
        innerBoxImage->GetRectTransform()->SetSizeDelta(220.0f, 17.0f);

        mGreenFont = gAssetManager.LoadFont("SID_TEXT_14_GRN.FON");
        mYellowFont = gAssetManager.LoadFont("SID_TEXT_14.FON");

        mAddDataLabel = UIUtil::NewUIActorWithWidget<UILabel>(innerBoxImage->GetOwner());
        mAddDataLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mAddDataLabel->SetFont(mGreenFont);
        mAddDataLabel->SetMasked(true);
        mAddDataLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        mAddDataLabel->GetRectTransform()->SetSizeDelta(0.0f, -1.0f);

        // Hide by default.
        mAddDataBox->SetActive(false);
    }

    // Add "Input Complete" dialog box.
    {
        mInputCompleteBox = new Actor("Add Data Complete", TransformType::RectTransform);
        mInputCompleteBox->GetTransform()->SetParent(sidney->GetTransform());

        UINineSlice* boxImage = mInputCompleteBox->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(Color32::Black));
        boxImage->GetRectTransform()->SetSizeDelta(240.0f, 105.0f);

        {
            Actor* staticTextActor = new Actor(TransformType::RectTransform);
            staticTextActor->GetTransform()->SetParent(mInputCompleteBox->GetTransform());

            UILabel* staticTextLabel = staticTextActor->AddComponent<UILabel>();
            staticTextLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            staticTextLabel->SetText(SidneyUtil::GetAddDataLocalizer().GetText("InputComplete"));
            staticTextLabel->SetVerticalAlignment(VerticalAlignment::Top);

            staticTextLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);
            staticTextLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
            staticTextLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -12.5f); // .5 to keep it pixel-perfect (since height of box is odd)
            staticTextLabel->GetRectTransform()->SetSizeDelta(226.0f, 42.0f);
        }
        {
            Actor* staticTextActor = new Actor(TransformType::RectTransform);
            staticTextActor->GetTransform()->SetParent(mInputCompleteBox->GetTransform());

            UILabel* staticTextLabel = staticTextActor->AddComponent<UILabel>();
            staticTextLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            staticTextLabel->SetText(SidneyUtil::GetAddDataLocalizer().GetText("InputPrompt"));
            staticTextLabel->SetVerticalAlignment(VerticalAlignment::Top);

            staticTextLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);
            staticTextLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
            staticTextLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -34.5f); // .5 to keep it pixel-perfect (since height of box is odd)
            staticTextLabel->GetRectTransform()->SetSizeDelta(226.0f, 42.0f);
        }
        {
            Actor* fileNameActor = new Actor(TransformType::RectTransform);
            fileNameActor->GetTransform()->SetParent(mInputCompleteBox->GetTransform());

            mFileNameLabel = fileNameActor->AddComponent<UILabel>();
            mFileNameLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            mFileNameLabel->SetText(" ");
            mFileNameLabel->SetVerticalAlignment(VerticalAlignment::Top);

            mFileNameLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);
            mFileNameLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
            mFileNameLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -56.5f); // .5 to keep it pixel-perfect (since height of box is odd)
            mFileNameLabel->GetRectTransform()->SetSizeDelta(226.0f, 16.0f);

            
            mInputCompleteOKButton = new SidneyButton(mInputCompleteBox);
            mInputCompleteOKButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
            mInputCompleteOKButton->SetText(SidneyUtil::GetAddDataLocalizer().GetText("OKButton"));
            mInputCompleteOKButton->GetButton()->SetCanInteract(false);

            mInputCompleteOKButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            mInputCompleteOKButton->GetRectTransform()->SetAnchoredPosition(15.0f, -77.5f);
            mInputCompleteOKButton->SetWidth(40.0f);
            mInputCompleteOKButton->SetHeight(13.0f);
        }

        // Hide by default.
        mInputCompleteBox->SetActive(false);
    }
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
        if(!gInventoryManager.IsInventoryShowing() && !gActionManager.IsActionPlaying())
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
                    mInputCompleteBox->SetActive(true);

                    // Figure out name of this item, which will be added to the input box.
                    mTextToType = SidneyUtil::GetAddDataLocalizer().GetText("ScanItem" + std::to_string(sidneyFileId + 1));

                    // Reset type text.
                    mFileNameLabel->SetText("");
                    mTextToTypeIndex = 0;
                    mTextToTypeTimer = Random::Range(kMinMaxTypeInterval.x, kMinMaxTypeInterval.y);
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

    // Update the "input complete" box.
    if(mInputCompleteBox->IsActive())
    {
        // Wait until enough time has passed to type the next letter.
        if(mTextToTypeTimer > 0.0f)
        {
            mTextToTypeTimer -= deltaTime;
            if(mTextToTypeTimer <= 0.0f)
            {
                // We have more text to type?
                if(mTextToTypeIndex < mTextToType.size())
                {
                    // Add a letter.
                    mFileNameLabel->SetText(mTextToType.substr(0, mTextToTypeIndex + 1));
                    ++mTextToTypeIndex;

                    // Reroll timer for next letter.
                    if(mTextToTypeIndex < mTextToType.size())
                    {
                        mTextToTypeTimer = Random::Range(kMinMaxTypeInterval.x, kMinMaxTypeInterval.y);
                    }
                    else
                    {
                        // When all letters are typed, always use a constant one-second delay before closing the box.
                        mTextToTypeTimer = 1.0f;
                    }

                    // Play random "key press" SFX from set of sounds.
                    int index = Random::Range(1, 5);
                    Audio* audio = gAssetManager.LoadAudio("COMPKEYSIN" + std::to_string(index));
                    gAudioManager.PlaySFX(audio);
                }
                else
                {
                    // We typed everything and waited a moment.
                    // Programmatically press the OK button and hide the box.
                    mInputCompleteOKButton->SetPressCallback([this](){
                        mInputCompleteBox->SetActive(false);
                        mInputCompleteOKButton->GetButton()->SetCanInteract(false);
                    });
                    mInputCompleteOKButton->GetButton()->SetCanInteract(true);
                    mInputCompleteOKButton->Press();
                }
            }
        }
    }
}