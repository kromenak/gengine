#include "Sidney.h"

#include "ActionManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "Scene.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"

namespace
{
    UIButton* CreateMainButton(Actor* parent, const std::string& buttonId, float xPos)
    {
        Actor* actor = new Actor(TransformType::RectTransform);
        actor->GetTransform()->SetParent(parent->GetTransform());
        UIButton* button = actor->AddComponent<UIButton>();

        button->GetRectTransform()->SetPivot(0.0f, 1.0f);
        button->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        button->GetRectTransform()->SetAnchoredPosition(xPos, -24.0f);

        button->SetUpTexture(Services::GetAssets()->LoadTexture("B_" + buttonId + "_U.BMP"));
        button->SetHoverTexture(Services::GetAssets()->LoadTexture("B_" + buttonId + "_H.BMP"));
        button->SetDownTexture(Services::GetAssets()->LoadTexture("B_" + buttonId + "_D.BMP"));
        button->SetDisabledTexture(Services::GetAssets()->LoadTexture("B_" + buttonId + "_X.BMP"));
        return button;
    }
}

Sidney::Sidney() : Actor(TransformType::RectTransform)
{
    // Sidney will be layered near the bottom.
    // A lot of stuff needs to appear above it (inventory, status overlay, etc).
    AddComponent<UICanvas>(-1);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Add black background that eats input.
    UIImage* background = AddComponent<UIImage>();
    background->SetTexture(&Texture::Black);
    background->SetReceivesInput(true);

    // Add desktop background image.
    Actor* desktopBackground = new Actor(TransformType::RectTransform);
    desktopBackground->GetTransform()->SetParent(GetTransform());
    UIImage* desktopBackgroundImage = desktopBackground->AddComponent<UIImage>();
    desktopBackgroundImage->SetTexture(Services::GetAssets()->LoadTexture("S_MAIN_SCN.BMP"), true);

    // Add exit button as child of desktop background.
    {
        Actor* exitButtonActor = new Actor(TransformType::RectTransform);
        exitButtonActor->GetTransform()->SetParent(desktopBackground->GetTransform());
        UIButton* exitButton = exitButtonActor->AddComponent<UIButton>();

        exitButton->GetRectTransform()->SetPivot(1.0f, 0.0f); // Bottom-Right
        exitButton->GetRectTransform()->SetAnchor(1.0f, 0.0f); // Bottom-Right
        exitButton->GetRectTransform()->SetAnchoredPosition(-10.0f, 10.0f); // 10x10 offset from Bottom-Right
        exitButton->GetRectTransform()->SetSizeDelta(80.0f, 18.0f);

        //TODO: Exit button uses a stretch-based image solution (kind of like 9-slice, more like 3-slice).
        exitButton->SetResizeBasedOnTexture(false);
        exitButton->SetUpTexture(&Texture::White);

        exitButton->SetPressCallback([this](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDBUTTON3.WAV"));
            Hide();
        });

        // Add exit button text.
        UILabel* exitLabel = exitButtonActor->AddComponent<UILabel>();
        exitLabel->SetFont(Services::GetAssets()->LoadFont("SID_TEXT_18.FON"));
        exitLabel->SetText("EXIT");
        exitLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        exitLabel->SetVerticalAlignment(VerticalAlignment::Center);
    }

    // Add button bar for subscreens.
    {
        const float kButtonStart = 5.0f;
        const float kButtonSpacing = 79.0f;

        float buttonPos = kButtonStart;
        UIButton* searchButton = CreateMainButton(desktopBackground, "SEARCH", buttonPos);
        searchButton->SetPressCallback([this](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));

            // Gabe refuses to use the search system.
            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
            {
                Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"02O6I2ZQR1\", 1)");
            }
            else
            {
                mSearch.Show();
            }
        });

        buttonPos += kButtonSpacing;
        UIButton* emailButton = CreateMainButton(desktopBackground, "EMAIL", buttonPos);
        emailButton->SetPressCallback([this](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));

            // Gabe also doesn't want to use email.
            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
            {
                Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"02O1E2ZQR1\", 1)");
            }
            else
            {
                mEmail.Show();
            }
        });

        buttonPos += kButtonSpacing;
        UIButton* filesButton = CreateMainButton(desktopBackground, "FILES", buttonPos);
        filesButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Files\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* analyzeButton = CreateMainButton(desktopBackground, "ANALYZE", buttonPos);
        analyzeButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));

            // Gabe also doesn't want to analyze stuff.
            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
            {
                Services::Get<ActionManager>()->ExecuteSheepAction("wait StartDialogue(\"02O7A2ZQR1\", 1)");
            }
            else
            {
                printf("Analyze\n");
            }
        });

        buttonPos += kButtonSpacing;
        UIButton* translateButton = CreateMainButton(desktopBackground, "TRANSL", buttonPos);
        translateButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Translate\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* dataButton = CreateMainButton(desktopBackground, "ADDATA", buttonPos);
        dataButton->SetPressCallback([this](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));

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
                mAddDataLabel->SetText("** AWAITING INPUT **");
                mAddDataBox->SetActive(true);

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
        });

        buttonPos += kButtonSpacing;
        UIButton* idButton = CreateMainButton(desktopBackground, "MAKEID", buttonPos);
        idButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Make ID\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* suspectsButton = CreateMainButton(desktopBackground, "SUSPT", buttonPos);
        suspectsButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Suspects\n");
        });
    }

    // Add "New Email" label.
    {
        Actor* newEmailActor = new Actor(TransformType::RectTransform);
        newEmailActor->GetTransform()->SetParent(desktopBackground->GetTransform());
        mNewEmailLabel = newEmailActor->AddComponent<UILabel>();

        mNewEmailLabel->SetFont(Services::GetAssets()->LoadFont("SID_PDN_10_GRN.FON"));
        mNewEmailLabel->SetText("NEW E-MAIL");
        mNewEmailLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mNewEmailLabel->SetVerticalAlignment(VerticalAlignment::Top);

        mNewEmailLabel->GetRectTransform()->SetPivot(1.0f, 1.0f);
        mNewEmailLabel->GetRectTransform()->SetAnchor(1.0f, 1.0f);
        mNewEmailLabel->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        mNewEmailLabel->GetRectTransform()->SetSizeDelta(100.0f, 20.0f);
    }

    // Add "Add Data" label and backgrounds.
    {
        Actor* outerBoxActor = new Actor(TransformType::RectTransform);
        outerBoxActor->GetTransform()->SetParent(desktopBackground->GetTransform());
        mAddDataBox = outerBoxActor;

        UIImage* outerBoxImage = outerBoxActor->AddComponent<UIImage>();
        outerBoxImage->SetColor(Color32(0, 0, 0, 160)); // Lighter Black Semi-Transparent
        outerBoxImage->GetRectTransform()->SetSizeDelta(248.0f, 44.0f);

        Actor* innerBoxActor = new Actor(TransformType::RectTransform);
        innerBoxActor->GetTransform()->SetParent(outerBoxActor->GetTransform());

        UIImage* innerBoxImage = innerBoxActor->AddComponent<UIImage>();
        innerBoxImage->SetColor(Color32(0, 0, 0, 180));  // Darker Black Semi-Transparent
        innerBoxImage->GetRectTransform()->SetSizeDelta(220.0f, 17.0f);

        mAddDataLabel = innerBoxActor->AddComponent<UILabel>();
        mAddDataLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mAddDataLabel->SetFont(Services::GetAssets()->LoadFont("SID_TEXT_14_GRN.FON"));

        // Hide by default.
        mAddDataBox->SetActive(false);
    }

    // Create subscreens.
    mSearch.Init(this);
    mEmail.Init(this);

    // Build directory structure.
    mData.emplace_back();
    mData.back().name = "Images";
    mData.emplace_back();
    mData.back().name = "Audio";
    mData.emplace_back();
    mData.back().name = "Text";
    mData.emplace_back();
    mData.back().name = "Fingerprints";
    mData.emplace_back();
    mData.back().name = "Licenses";
    mData.emplace_back(); // TODO: this one doesn't appear in the list until you find your first shape
    mData.back().name = "Shapes";

    // Build list of known files.
    //TODO: Would be cool if this was data-driven?
    mKnownFiles.emplace_back("Fingerprints", "fileAbbePrint"); // 1
    mKnownFiles.emplace_back("Fingerprints", "fileBuchelliPrint");
    mKnownFiles.emplace_back("Fingerprints", "fileButhanePrint");
    mKnownFiles.emplace_back("Fingerprints", "fileEstellePrint");
    mKnownFiles.emplace_back("Fingerprints", "fileLadyHowardPrint");
    mKnownFiles.emplace_back("Fingerprints", "fileMontreauxPrint");
    mKnownFiles.emplace_back("Fingerprints", "fileWilkesPrint");
    mKnownFiles.emplace_back("Fingerprints", "fileMoselyPrint");
    mKnownFiles.emplace_back("Fingerprints", "fileLarryPrint");
    mKnownFiles.emplace_back("Fingerprints", "fileWilkesPrint2"); // 10
    mKnownFiles.emplace_back("Fingerprints", "fileBuchelliPrint2"); 
    mKnownFiles.emplace_back("Fingerprints", "fileUnknownPrint1");
    mKnownFiles.emplace_back("Fingerprints", "fileUnknownPrint2");
    mKnownFiles.emplace_back("Fingerprints", "fileUnknownPrint3");
    mKnownFiles.emplace_back("Fingerprints", "fileUnknownPrint4");
    mKnownFiles.emplace_back("Fingerprints", "fileUnknownPrint5");
    mKnownFiles.emplace_back("Fingerprints", "fileUnknownPrint6");
    mKnownFiles.emplace_back("Fingerprints", "fileLSR1Print"); // Unused?
    mKnownFiles.emplace_back("Fingerprints", "fileEstellesLSRPrint");
    mKnownFiles.emplace_back("Images", "fileMap"); // 20
    mKnownFiles.emplace_back("Images", "fileParchment1");
    mKnownFiles.emplace_back("Images", "fileParchment2");
    mKnownFiles.emplace_back("Images", "filePoussinPostcard");
    mKnownFiles.emplace_back("Images", "fileTeniersPostcard1");
    mKnownFiles.emplace_back("Images", "fileTeniersPostcard2");
    mKnownFiles.emplace_back("Images", "fileHermeticSymbols");
    mKnownFiles.emplace_back("Images", "fileSUMNote");
    mKnownFiles.emplace_back("Audio", "fileAbbeTape");
    mKnownFiles.emplace_back("Audio", "fileBuchelliTape");
    mKnownFiles.emplace_back("Text", "fileArcadiaText"); // 30
    mKnownFiles.emplace_back("Text", "fileTempleOfSolomonText"); //TODO: Is this a text type?
    mKnownFiles.emplace_back("Images", "fileHermeticSymbols"); //TODO: Seems doubled up and unused?
    mKnownFiles.emplace_back("Licenses", "fileBuchelliLicense");
    mKnownFiles.emplace_back("Licenses", "fileEmilioLicense");
    mKnownFiles.emplace_back("Licenses", "fileLadyHowardLicense");
    mKnownFiles.emplace_back("Licenses", "fileMoselyLicense");
    mKnownFiles.emplace_back("Licenses", "fileWilkesLicense");
}

void Sidney::Show()
{
    // Show Sidney UI.
    SetActive(true);

    //TODO: We'll assume there is ALWAYS new email for the moment...
    bool newEmail = true;

    // Hide the "new email" label by default, regardless of new email state.
    // If we DO have new email, this will blink on in a moment.
    mNewEmailLabel->SetEnabled(false);

    // If no new email, set this timer to -1. The label will never appear.
    // Otherwise, set to blink interval to have it blink on and off.
    mNewEmailBlinkTimer = newEmail ? kNewEmailBlinkInterval : -1;

    // If there is new email, play the "new email" audio cue.
    if(newEmail)
    {
        Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("NEWEMAIL.WAV"));
    }
}

void Sidney::Hide()
{
    // Hide Sidney UI.
    SetActive(false);

    // Whenever you exit Sidney, no matter where you are in the game, you warp to R25.
    // This makes sense under the assumption that you only access Sidney in R25 anyway!
    Services::Get<LocationManager>()->ChangeLocation("R25", [](){

        // This special function warps Ego to the "sitting at desk" position and plays the stand up animation.
        Services::Get<ActionManager>()->ExecuteSheepAction("R25_ALL", "ExitSidney$");
    });
}

bool Sidney::HasFile(const std::string& fileName)
{
    for(auto& dir : mData)
    {
        for(auto& file : dir.files)
        {
            if(StringUtil::EqualsIgnoreCase(file.name, fileName))
            {
                return true;
            }
        }
    }
    return false;
}

bool Sidney::HasFile(const std::string& type, const std::string& fileName)
{
    for(auto& dir : mData)
    {
        if(StringUtil::EqualsIgnoreCase(dir.name, type))
        {
            return dir.HasFile(fileName);
        }
    }
    return false;
}

void Sidney::AddFile(const std::string& type, const std::string& fileName)
{
    for(auto& dir : mData)
    {
        if(StringUtil::EqualsIgnoreCase(dir.name, type))
        {
            // Already have this one!
            if(dir.HasFile(fileName)) { return; }

            dir.files.emplace_back();
            dir.files.back().name = fileName;
        }
    }
}

void Sidney::OnUpdate(float deltaTime)
{
    // When adding data, Sidney should not update.
    // We mainly wait for the inventory layer to be closed.
    if(mAddingData)
    {
        // Wait for player to close inventory.
        if(!Services::Get<InventoryManager>()->IsInventoryShowing() && !Services::Get<ActionManager>()->IsActionPlaying())
        {
            // Clear "using scanner" flag.
            Services::Get<GameProgress>()->ClearFlag("UsingScanner");
            mAddingData = false;

            // Determine whether something was successfully scanned into Sidney.
            int sidneyScannerVal = Services::Get<GameProgress>()->GetGameVariable("SidScanner");
            if(sidneyScannerVal == 0)
            {
                // Show box indicating that input was aborted.
                mAddDataLabel->SetText("** INPUT ABORTED **");
                mAddDataBox->SetActive(true);

                // This puts the player in a non-interactive state for a moment (so they can read the text box) and then puts them back on the main screen.
                Services::Get<ActionManager>()->ExecuteSheepAction("wait SetTimerSeconds(2);", [this](const Action* action){
                    mAddDataBox->SetActive(false);
                });
            }
            else
            {
                // Show box (and SFX) indicating we are scanning an item.
                mAddDataLabel->SetText("** SCANNING ITEM **"); //TODO: Should blink Green/Gold.
                mAddDataBox->SetActive(true);
                Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDSCAN.WAV"));

                // Based on the scanner value set by inventory code, correlate that to a particular file.
                // Unfortunately, these numbers don't seem to correlate to anything in particular...
                switch(sidneyScannerVal)
                {
                case 21: // Parchment 1
                    break;
                case 22: // Parchment 2
                    break;
                }

                // This puts the player in a non-interactive state for a moment (so they can read the text box and hear SFX).
                Services::Get<ActionManager>()->ExecuteSheepAction("wait SetTimerSeconds(2.5);", [this](const Action* action){
                    mAddDataBox->SetActive(false);
                    
                    //TODO: Show the "Input Complete" box/unskippable cutscene.
                    //TODO: File names shown during this cutscene are accessibly BY sidneyScannerVal in ESIDNEY.TXT!
                });
            }
        }
        return;
    }

    // Track timer countdown for new email to blink in the corner.
    if(mNewEmailBlinkTimer > 0.0f)
    {
        mNewEmailBlinkTimer -= deltaTime;
        if(mNewEmailBlinkTimer <= 0.0f)
        {
            mNewEmailLabel->SetEnabled(!mNewEmailLabel->IsEnabled());
            mNewEmailBlinkTimer = kNewEmailBlinkInterval;
        }
    }

    // Update each screen in turn.
    // Each screen will early out if not active.
    mSearch.OnUpdate(deltaTime);
}