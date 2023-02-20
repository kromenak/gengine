#include "SidneyAddData.h"

#include "GameProgress.h"
#include "InventoryManager.h"
#include "Random.h"
#include "Scene.h"
#include "Sidney.h"
#include "SidneyFiles.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"

void SidneyAddData::Init(Sidney* sidney, SidneyFiles* sidneyFiles)
{
    mSidney = sidney;
    mSidneyFiles = sidneyFiles;

    // Add "Add Data" dialog box.
    {
        mAddDataBox = new Actor(TransformType::RectTransform);
        mAddDataBox->GetTransform()->SetParent(sidney->GetTransform());

        UIImage* outerBoxImage = mAddDataBox->AddComponent<UIImage>();
        outerBoxImage->SetColor(Color32(0, 0, 0, 160)); // Lighter Black Semi-Transparent
        outerBoxImage->GetRectTransform()->SetSizeDelta(248.0f, 44.0f);

        Actor* innerBoxActor = new Actor(TransformType::RectTransform);
        innerBoxActor->GetTransform()->SetParent(mAddDataBox->GetTransform());

        UIImage* innerBoxImage = innerBoxActor->AddComponent<UIImage>();
        innerBoxImage->SetColor(Color32(0, 0, 0, 180));  // Darker Black Semi-Transparent
        innerBoxImage->GetRectTransform()->SetSizeDelta(220.0f, 17.0f);

        mGreenFont = Services::GetAssets()->LoadFont("SID_TEXT_14_GRN.FON");
        mYellowFont = Services::GetAssets()->LoadFont("SID_TEXT_14.FON");

        mAddDataLabel = innerBoxActor->AddComponent<UILabel>();
        mAddDataLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mAddDataLabel->SetFont(mGreenFont);

        // Hide by default.
        mAddDataBox->SetActive(false);
    }

    // Add "Input Complete" dialog box.
    {
        mInputCompleteBox = new Actor(TransformType::RectTransform);
        mInputCompleteBox->GetTransform()->SetParent(sidney->GetTransform());

        UIImage* boxImage = mInputCompleteBox->AddComponent<UIImage>();
        boxImage->SetColor(Color32::Black); // Black
        boxImage->GetRectTransform()->SetSizeDelta(240.0f, 105.0f);

        {
            Actor* staticTextActor = new Actor(TransformType::RectTransform);
            staticTextActor->GetTransform()->SetParent(mInputCompleteBox->GetTransform());

            UILabel* staticTextLabel = staticTextActor->AddComponent<UILabel>();
            staticTextLabel->SetFont(Services::GetAssets()->LoadFont("SID_TEXT_14.FON"));
            staticTextLabel->SetText("** INPUT COMPLETE **");
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
            staticTextLabel->SetFont(Services::GetAssets()->LoadFont("SID_TEXT_14.FON"));
            staticTextLabel->SetText("FILE NAME:");
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
            mFileNameLabel->SetFont(Services::GetAssets()->LoadFont("SID_TEXT_14.FON"));
            mFileNameLabel->SetText(" ");
            mFileNameLabel->SetVerticalAlignment(VerticalAlignment::Top);

            mFileNameLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);
            mFileNameLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
            mFileNameLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -56.5f); // .5 to keep it pixel-perfect (since height of box is odd)
            mFileNameLabel->GetRectTransform()->SetSizeDelta(226.0f, 16.0f);

            mInputCompleteOKButton = SidneyUtil::CreateTextButton(mInputCompleteBox, "OK", "SID_PDN_10_L.FON",
                                                                  Vector2(0.0f, 1.0f), Vector2(15.0f, -77.5f), Vector2(40.0f, 13.0f));
            mInputCompleteOKButton->SetCanInteract(false);
        }

        // Hide by default.
        mInputCompleteBox->SetActive(false);
    }
}

void SidneyAddData::Start()
{
    // If you try to use this button before 210A, Grace says she has nothing to scan.
    // After that, the player gets to decide.
    const Timeblock& timeblock = Services::Get<GameProgress>()->GetTimeblock();
    if(timeblock == Timeblock(2, 7))
    {
        Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"0264G2ZPF1\", 2)");
    }
    else
    {
        // Show box indicating that we need input.
        mAddDataBox->SetActive(true);

        // Just use static green text.
        mAddDataLabel->SetText("** AWAITING INPUT **");
        mAddDataLabel->SetFont(mGreenFont);
        mAddDataColorTimer = -1.0f;

        // This puts the player in a non-interactive state for a moment (so they can read the text box) and then shows the inventory.
        Services::Get<ActionManager>()->ExecuteSheepAction("wait SetTimerSeconds(2); ShowInventory();", [this](const Action* action){
            mAddDataBox->SetActive(false);
            mAddingData = true;

            // This is required for the Inventory NVC to know that scanning items is allowed.
            Services::Get<GameProgress>()->SetFlag("UsingScanner");

            // Clear scanner variable. Inventory code will set this if we've successfully scanned any item.
            Services::Get<GameProgress>()->SetGameVariable("SidScanner", 0);
        });
    }
}

void SidneyAddData::OnUpdate(float deltaTime)
{
    // Update when the player is trying to add data.
    if(mAddingData)
    {
        // Wait for player to close inventory.
        if(!Services::Get<InventoryManager>()->IsInventoryShowing() && !Services::Get<ActionManager>()->IsActionPlaying())
        {
            // Clear "using scanner" flag.
            Services::Get<GameProgress>()->ClearFlag("UsingScanner");
            mAddingData = false;

            // Determine whether something was successfully scanned into Sidney.
            int sidneyFileIndex = Services::Get<GameProgress>()->GetGameVariable("SidScanner") - 1;
            if(sidneyFileIndex < 0 || sidneyFileIndex > mSidneyFiles->GetMaxFileIndex())
            {
                // CASE 1: No valid object was selected to scan.
                // Show box indicating that input was aborted.
                mAddDataBox->SetActive(true);

                // Use green text, no color change.
                mAddDataLabel->SetText("** INPUT ABORTED **");
                mAddDataLabel->SetFont(mGreenFont);
                mAddDataColorTimer = -1.0f;
                
                // This puts the player in a non-interactive state for a moment (so they can read the text box) and then puts them back on the main screen.
                Services::Get<ActionManager>()->ExecuteSheepAction("wait SetTimerSeconds(2);", [this](const Action* action){
                    mAddDataBox->SetActive(false);
                });
            }
            else if(mSidneyFiles->HasFile(sidneyFileIndex))
            {
                // CASE 2: Valid object selected, but already scanned it.
                // Ego says "I already scanned it!"
                if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
                {
                    Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"02O4G2K6R1\", 1)");
                }
                else
                {
                    Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"02O4G716R1\", 1)");
                }
            }
            else 
            {
                // CASE 3: Valid object selected, not scanned yet.
                // Show box (and SFX) indicating we are scanning an item.
                mAddDataBox->SetActive(true);
                Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDSCAN.WAV"));

                // Start with green text, but it will blink on an interval.
                mAddDataLabel->SetText("** SCANNING ITEM **"); //TODO: Should blink Green/Gold.
                mAddDataLabel->SetFont(mGreenFont);
                mAddDataColorTimer = kAddDataColorToggleInterval;
                
                // This puts the player in a non-interactive state for a moment (so they can read the text box and hear SFX).
                Services::Get<ActionManager>()->ExecuteSheepAction("wait SetTimerSeconds(2);", [this, sidneyFileIndex](const Action* action){

                    // Add the desired file to Sidney.
                    mSidneyFiles->AddFile(sidneyFileIndex);

                    // Hide "Add Data" box and show "Input Complete" box.
                    mAddDataBox->SetActive(false);
                    mInputCompleteBox->SetActive(true);

                    // Figure out name of this item, which will be added to the input box.
                    mTextToType = SidneyUtil::GetAddDataLocalizer().GetText("ScanItem" + std::to_string(sidneyFileIndex + 1));

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
                        //TODO: "Artificially" press the "complete" button.
                    }

                    // Play random "key press" SFX from set of sounds.
                    int index = Random::Range(1, 5);
                    Audio* audio = Services::GetAssets()->LoadAudio("COMPKEYSIN" + std::to_string(index));
                    Services::GetAudio()->PlaySFX(audio);
                }
                else
                {
                    // We typed everything AND showed the OK button being pressed. Close this box!
                    Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDBUTTON5"));
                    mInputCompleteBox->SetActive(false);
                }
            }
        }
    }
}