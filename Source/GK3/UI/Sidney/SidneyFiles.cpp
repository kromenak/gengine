#include "SidneyFiles.h"

#include "AssetManager.h"
#include "GameProgress.h"
#include "Sidney.h"
#include "SidneyUtil.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"

void SidneyFiles::Init(Sidney* parent)
{
    // Build directory structure.
    mData.emplace_back();
    mData.back().name = "ImageDir";
    mData.back().type = SidneyFileType::Image;
    mData.emplace_back();
    mData.back().name = "AudioDir";
    mData.back().type = SidneyFileType::Audio;
    mData.emplace_back();
    mData.back().name = "TextDir";
    mData.back().type = SidneyFileType::Text;
    mData.emplace_back();
    mData.back().name = "FingerDir";
    mData.back().type = SidneyFileType::Fingerprint;
    mData.emplace_back();
    mData.back().name = "LicenseDir";
    mData.back().type = SidneyFileType::License;
    mData.emplace_back();
    mData.back().name = "ShapeDir";
    mData.back().type = SidneyFileType::Shape;

    // Build list of known files. (TODO: Would be cool if this was data-driven?)
    // Fingerprints (1-19)
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileAbbePrint",        "ABBE_FINGERPRINT",      "e_sidney_add_fingerprint_abbe");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileBuchelliPrint",    "BUCHELLIS_FINGERPRINT", "e_sidney_add_fingerprint_buchelli");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileButhanePrint",     "BUTHANES_FINGERPRINT",  "e_sidney_add_fingerprint_buthane");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileEstellePrint",     "ESTELLES_FINGERPRINT",  "e_sidney_add_fingerprint_estelle");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileLadyHowardPrint",  "HOWARDS_FINGERPRINT",   "e_sidney_add_fingerprint_howard");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileMontreauxPrint",   "MONTREAUX_FINGERPRINT", "e_sidney_add_fingerprint_montreaux");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileWilkesPrint",      "WILKES_FINGERPRINT",    "e_sidney_add_fingerprint_wilkes");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileMoselyPrint",      "MOSELYS_FINGERPRINT");  //TODO: Hmm, no score event for this one?
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileLarryPrint",       "LARRYS_FINGERPRINT",    "e_sidney_add_fingerprint_larry");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileWilkesPrint2",     "WILKES_FINGERPRINT_LABELED_BUCHELLI");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileBuchelliPrint2",   "BUCHELLIS_FINGERPRINT_LABELED_WILKES");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileUnknownPrint1",    "UNKNOWN_PRINT_1",       "e_sidney_add_manuscript_prints_buthane");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileUnknownPrint2",    "UNKNOWN_PRINT_2",       "e_sidney_add_manuscript_prints_mosley");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileUnknownPrint3",    "UNKNOWN_PRINT_3",       "e_sidney_add_manuscript_prints_buchelli");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileUnknownPrint4",    "UNKNOWN_PRINT_4");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileUnknownPrint5",    "UNKNOWN_PRINT_5");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileUnknownPrint6",    "UNKNOWN_PRINT_6");
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileLSR1Print",        "UNKNOWN_PRINT_7");      // Unused?
    mAllFiles.emplace_back(SidneyFileType::Fingerprint, "fileEstellesLSRPrint", "UNKNOWN_PRINT_8");

    // Images (20-27)
    mAllFiles.emplace_back(SidneyFileType::Image, "fileMap",              "MAP",                      "e_sidney_add_map");
    mAllFiles.emplace_back(SidneyFileType::Image, "fileParchment1",       "PARCHMENT_1",              "e_sidney_add_parch1");
    mAllFiles.emplace_back(SidneyFileType::Image, "fileParchment2",       "PARCHMENT_2",              "e_sidney_add_parch2");
    mAllFiles.emplace_back(SidneyFileType::Image, "filePoussinPostcard",  "POUSSIN_POSTCARD",         "e_sidney_add_postcard_1");
    mAllFiles.emplace_back(SidneyFileType::Image, "fileTeniersPostcard1", "TENIERS_POSTCARD_NO_TEMP", "e_sidney_add_postcard_2");
    mAllFiles.emplace_back(SidneyFileType::Image, "fileTeniersPostcard2", "TENIERS_POSTCARD_TEMP",    "e_sidney_add_postcard_3");
    mAllFiles.emplace_back(SidneyFileType::Image, "fileHermeticSymbols",  "HERM_SYMBOLS",             "e_sidney_add_hermetical_symbols_from_serres");
    mAllFiles.emplace_back(SidneyFileType::Image, "fileSUMNote",          "I_AM_WORDS",               "e_sidney_add_sum_note");

    // Audio (28-29)
    mAllFiles.emplace_back(SidneyFileType::Audio, "fileAbbeTape",     "", "e_sidney_add_tape_abbe");
    mAllFiles.emplace_back(SidneyFileType::Audio, "fileBuchelliTape", "", "e_sidney_add_tape_buchelli");

    // Text (30-31)
    mAllFiles.emplace_back(SidneyFileType::Text, "fileArcadiaText", "");
    mAllFiles.emplace_back(SidneyFileType::Text, "fileTempleOfSolomonText", "");     //TODO: Is this a text type?

    mAllFiles.emplace_back(SidneyFileType::Image, "fileHermeticSymbols", "");         //TODO: Seems doubled up and unused?

    // Licenses (33-37)
    mAllFiles.emplace_back(SidneyFileType::License, "fileBuchelliLicense",   "BUCHELLIS_LICENSE", "e_sidney_add_license_buchelli");
    mAllFiles.emplace_back(SidneyFileType::License, "fileEmilioLicense",     "EMILIOS_LICENSE",   "e_sidney_add_license_emilio");
    mAllFiles.emplace_back(SidneyFileType::License, "fileLadyHowardLicense", "HOWARDS_LICENSE",   "e_sidney_add_license_howard");
    mAllFiles.emplace_back(SidneyFileType::License, "fileMoselyLicense",     "MOSELYS_LICENSE",   "e_sidney_add_license_mosely");
    mAllFiles.emplace_back(SidneyFileType::License, "fileWilkesLicense",     "WILKES_LICENSE",    "e_sidney_add_license_wilkes");

    // Set file indexes (as this is how files are referenced when adding).
    for(size_t i = 0; i < mAllFiles.size(); ++i)
    {
        mAllFiles[i].index = i;
    }

    // The Files UI is a bit unique. It's more of a "floating dialog" that can appear over other screens.
    // Though files UI doesn't have a background image, it's helpful to create a "dummy" rect to help with positioning things.
    {
        mRoot = new Actor("Files", TransformType::RectTransform);
        mRoot->GetTransform()->SetParent(parent->GetTransform());

        RectTransform* rt = mRoot->GetComponent<RectTransform>();
        Texture* backgroundTexture = gAssetManager.LoadTexture("S_BKGND.BMP");
        rt->SetSizeDelta(static_cast<float>(backgroundTexture->GetWidth()),
                         static_cast<float>(backgroundTexture->GetHeight()));
    }

    // Add dialog background.
    {
        // Create a root actor for the dialog.
        mDialogRoot = new Actor(TransformType::RectTransform);
        mDialogRoot->GetTransform()->SetParent(mRoot->GetTransform());

        // Create a black background.
        UIImage* backgroundImage = mDialogRoot->AddComponent<UIImage>();
        backgroundImage->SetColor(Color32::Black);

        // Receive input to avoid sending inputs to main screen below this screen.
        backgroundImage->SetReceivesInput(true);

        // Set to correct size and position.
        RectTransform* rt = backgroundImage->GetRectTransform();
        rt->SetSizeDelta(153.0f, 350.0f);
        rt->SetAnchor(0.0f, 1.0f); // Top-Left
        rt->SetPivot(0.0f, 1.0f); // Top-Left
        rt->SetAnchoredPosition(40.0f, -66.0f);
    }

    // Add close button.
    {
        Actor* closeActor = new Actor(TransformType::RectTransform);
        closeActor->GetTransform()->SetParent(mDialogRoot->GetTransform());

        UIButton* closeButton = closeActor->AddComponent<UIButton>();
        closeButton->GetRectTransform()->SetPivot(0.0f, 1.0f);
        closeButton->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        closeButton->GetRectTransform()->SetAnchoredPosition(2.0f, -2.0f);

        closeButton->SetUpTexture(gAssetManager.LoadTexture("CLOSEWIN_UP.BMP"));
        closeButton->SetDownTexture(gAssetManager.LoadTexture("CLOSEWIN_DOWN.BMP"));
        closeButton->SetHoverTexture(gAssetManager.LoadTexture("CLOSEWIN_HOVER.BMP"));
        
        closeButton->SetPressCallback([this](UIButton* button){
            Hide();
        });
    }

    // Add title/header.
    {
        Actor* titleActor = new Actor(TransformType::RectTransform);
        titleActor->GetTransform()->SetParent(mDialogRoot->GetTransform());

        UILabel* titleLabel = titleActor->AddComponent<UILabel>();
        titleLabel->GetRectTransform()->SetPivot(0.5f, 1.0f); // Top-Center
        titleLabel->GetRectTransform()->SetAnchorMin(0.0f, 1.0f);
        titleLabel->GetRectTransform()->SetAnchorMax(1.0f, 1.0f); // Fill space horizontally, anchor to top.
        titleLabel->GetRectTransform()->SetSizeDeltaY(20.0f);

        titleLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_18.FON"));
        titleLabel->SetText("File List");
        titleLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        titleLabel->SetVerticalAlignment(VerticalAlignment::Top);
    }

    // Hide by default.
    Hide();
}

void SidneyFiles::Show(std::function<void(SidneyFile*)> selectFileCallback)
{
    mRoot->SetActive(true);

    // We must refresh this each time we show it, since the files may have changed since last time.
    RefreshFileListUI();

    // Save the select callback.
    mSelectFileCallback = selectFileCallback;
}

void SidneyFiles::Hide()
{
    mRoot->SetActive(false);
}

void SidneyFiles::AddFile(size_t fileIndex)
{
    if(fileIndex >= mAllFiles.size()) { return; }

    // Find appropriate directory.
    for(auto& dir : mData)
    {
        // Add file if not already in this directory.
        if(dir.type == mAllFiles[fileIndex].type && !dir.HasFile(mAllFiles[fileIndex].name))
        {
            dir.files.emplace_back(&mAllFiles[fileIndex]);

            // Add to score if there's a score event.
            if(!mAllFiles[fileIndex].scoreName.empty())
            {
                gGameProgress.ChangeScore(mAllFiles[fileIndex].scoreName);
            }
        }
    }
}

bool SidneyFiles::HasFile(size_t fileIndex)
{
    if(fileIndex >= mAllFiles.size()) { return false; }
    return HasFile(mAllFiles[fileIndex].name);
}

bool SidneyFiles::HasFile(const std::string& fileName) const
{
    for(auto& dir : mData)
    {
        if(dir.HasFile(fileName))
        {
            return true;
        }
    }
    return false;
}

SidneyFiles::FileListButton SidneyFiles::GetFileListButton()
{
    // Need to create a new button maybe.
    if(mFileListLabelIndex >= mFileListButtons.size())
    {
        Actor* labelActor = new Actor(TransformType::RectTransform);
        labelActor->GetTransform()->SetParent(mDialogRoot->GetTransform());

        UILabel* label = labelActor->AddComponent<UILabel>();
        label->GetRectTransform()->SetPivot(0.0f, 1.0f); // Top-Left
        label->GetRectTransform()->SetAnchor(0.0f, 1.0f); // Top-Left
        label->GetRectTransform()->SetSizeDelta(128.0f, 16.0f);

        label->SetHorizonalAlignment(HorizontalAlignment::Left);
        label->SetVerticalAlignment(VerticalAlignment::Center);

        labelActor->AddComponent<UIButton>();

        mFileListButtons.emplace_back();
        mFileListButtons.back().button = labelActor->AddComponent<UIButton>();
        mFileListButtons.back().label = label;
    }

    // We'll return this index.
    int idx = mFileListLabelIndex;

    // Make sure it is enabled.
    mFileListButtons[idx].button->SetEnabled(true);
    mFileListButtons[idx].label->SetEnabled(true);

    // Increment index and return the button.
    ++mFileListLabelIndex;
    return mFileListButtons[idx];
}

void SidneyFiles::RefreshFileListUI()
{
    // Disable all existing labels.
    for(FileListButton& button : mFileListButtons)
    {
        button.button->SetEnabled(false);
        button.label->SetEnabled(false);
    }
    mFileListLabelIndex = 0;

    // Iterate and place each label.
    Vector2 topLeft(10.0f, -28.0f);
    for(auto& dir : mData)
    {
        // One unique bit of behavior: the shapes directory only appears if there's something in it.
        if(dir.type == SidneyFileType::Shape && dir.files.empty())
        {
            continue;
        }

        // Create the label for the directory itself.
        // This has a leading "-" and uses gray colored text.
        FileListButton dirButton = GetFileListButton();
        dirButton.label->GetRectTransform()->SetAnchoredPosition(topLeft);
        dirButton.label->SetFont(gAssetManager.LoadFont("SID_TEXT_14_UL.FON"));
        dirButton.label->SetText("-" + SidneyUtil::GetMainScreenLocalizer().GetText(dir.name));
        dirButton.button->SetPressCallback(nullptr);
        topLeft.y -= 16.0f;

        // Create labels for each file within the directory.
        // These are indented slightly and use gold colored text.
        for(auto& file : dir.files)
        {
            FileListButton fileButton = GetFileListButton();
            fileButton.label->GetRectTransform()->SetAnchoredPosition(topLeft);
            fileButton.label->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            fileButton.label->SetText("  " + file->GetDisplayName());
            fileButton.button->SetPressCallback([this, file](UIButton* button){
                if(mSelectFileCallback != nullptr)
                {
                    mSelectFileCallback(file);
                }
            });
            topLeft.y -= 16.0f;
        }
    }
}